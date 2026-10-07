#include "golem/rom_runner.h"

#include "gb_core.h"

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

    struct Destroy {
        void operator()(golem_gb* gb) const
        {
            golem_gb_destroy(gb);
        }
    };

} // namespace

RunResult run_rom(
    const std::vector<std::uint8_t>& rom,
    std::uint32_t frames)
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
    const char* error = nullptr;
    std::unique_ptr<golem_gb, Destroy> gb(
        golem_gb_create(rom.data(), rom.size(), record, &recorder, &error));
    if (!gb) {
        throw RunError(error);
    }

    const std::uint64_t limit = std::uint64_t {frames} + kExtraEmulatorFrames;
    for (std::uint64_t emulated = 0; emulated < limit && !recorder.done; ++emulated) {
        if (golem_gb_run_frame(gb.get(), &error) != 0) {
            throw RunError(std::string("emulator stopped: ") + error);
        }
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
