#include "golem/rom_runner.h"

#include "gb_core.h"
#ifdef GOLEM_HAS_SAMEBOY
#include "sameboy_core.h"
#endif

#include <memory>
#include <string>

namespace golem {

namespace {

    constexpr std::size_t kMinRomSize = 0x150; // Up to the end of the cartridge header.

    struct Recorder {
        RunResult* result;
        std::int64_t frame = -1; // -1 until the first marker.
        std::uint16_t frame_start = 0; // Clock at the current frame's marker.
        bool done = false;
    };

    void record(
        void* user,
        std::uint16_t address,
        std::uint8_t value,
        std::uint16_t clock)
    {
        auto& recorder = *static_cast<Recorder*>(user);
        if (recorder.done) {
            return;
        }
        if (address == kFrameMarker) {
            recorder.frame_start = clock;
            recorder.done = ++recorder.frame == recorder.result->trace.frames;
            return;
        }
        if (address == kCallEndMarker) {
            // The first end marker of a frame closes the measured call.
            if (recorder.frame >= 0) {
                auto& cycles = recorder.result->frame_cycles[recorder.frame];
                if (!cycles) {
                    const auto elapsed = static_cast<std::uint16_t>(clock - recorder.frame_start);
                    cycles = elapsed - kMarkerCycles;
                }
            }
            return;
        }
        if (recorder.frame >= 0) {
            recorder.result->trace.entries.push_back(
                {static_cast<std::uint32_t>(recorder.frame), {address, value}});
        }
    }

    // An emulator running one ROM, reporting APU writes to `record`.
    class Machine {
    public:
        virtual ~Machine() = default;
        // Throws RunError on an emulation error.
        virtual void run_frame() = 0;
    };

    class PeanutGbMachine : public Machine {
    public:
        PeanutGbMachine(
            const std::vector<std::uint8_t>& rom,
            Recorder& recorder)
        {
            const char* error = nullptr;
            gb_ = golem_gb_create(rom.data(), rom.size(), record, &recorder, &error);
            if (gb_ == nullptr) {
                throw RunError(error);
            }
        }

        ~PeanutGbMachine() override
        {
            golem_gb_destroy(gb_);
        }

        void run_frame() override
        {
            const char* error = nullptr;
            if (golem_gb_run_frame(gb_, &error) != 0) {
                throw RunError(std::string("emulator stopped: ") + error);
            }
        }

    private:
        golem_gb* gb_ = nullptr;
    };

#ifdef GOLEM_HAS_SAMEBOY
    class SameBoyMachine : public Machine {
    public:
        SameBoyMachine(
            const std::vector<std::uint8_t>& rom,
            Recorder& recorder)
        {
            const char* error = nullptr;
            sb_ = golem_sb_create(rom.data(), rom.size(), record, &recorder, &error);
            if (sb_ == nullptr) {
                throw RunError(error);
            }
        }

        ~SameBoyMachine() override
        {
            golem_sb_destroy(sb_);
        }

        void run_frame() override
        {
            const char* error = nullptr;
            golem_sb_run_frame(sb_, &error);
        }

    private:
        golem_sb* sb_ = nullptr;
    };
#endif

    std::unique_ptr<Machine> make_machine(
        Emulator emulator,
        const std::vector<std::uint8_t>& rom,
        Recorder& recorder)
    {
        switch (emulator) {
        case Emulator::PeanutGb:
            return std::make_unique<PeanutGbMachine>(rom, recorder);
        case Emulator::SameBoy:
#ifdef GOLEM_HAS_SAMEBOY
            return std::make_unique<SameBoyMachine>(rom, recorder);
#else
            break;
#endif
        }
        throw RunError(emulator_name(emulator) + " is not built in");
    }

} // namespace

std::vector<Emulator> available_emulators()
{
#ifdef GOLEM_HAS_SAMEBOY
    return {Emulator::PeanutGb, Emulator::SameBoy};
#else
    return {Emulator::PeanutGb};
#endif
}

std::string emulator_name(Emulator emulator)
{
    return emulator == Emulator::PeanutGb ? "Peanut-GB" : "SameBoy";
}

RunResult run_rom(
    const std::vector<std::uint8_t>& rom,
    std::uint32_t frames,
    Emulator emulator)
{
    RunResult result;
    result.trace.frames = frames;
    if (frames == 0) {
        return result;
    }
    result.frame_cycles.resize(frames);
    if (rom.size() < kMinRomSize) {
        throw RunError("ROM is smaller than its header");
    }

    Recorder recorder {&result};
    const auto machine = make_machine(emulator, rom, recorder);
    const std::uint64_t limit = std::uint64_t {frames} + kExtraEmulatorFrames;
    for (std::uint64_t emulated = 0; emulated < limit && !recorder.done; ++emulated) {
        machine->run_frame();
    }
    if (!recorder.done) {
        const auto completed = recorder.frame < 0 ? 0 : recorder.frame;
        throw RunError(
            "only "
            + std::to_string(completed)
            + " of "
            + std::to_string(frames)
            + " frames completed after "
            + std::to_string(limit)
            + " emulator frames");
    }
    return result;
}

} // namespace golem
