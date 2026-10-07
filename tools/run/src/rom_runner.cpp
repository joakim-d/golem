#include "golem/rom_runner.h"

#include "gb_core.h"

#include <memory>
#include <string>

namespace golem {

namespace {

    constexpr std::size_t kMinRomSize = 0x150; // Up to the end of the cartridge header.

    struct Recorder {
        Trace* trace;
        std::int64_t frame = -1; // -1 until the first marker.
        bool done = false;
    };

    void record(
        void* user,
        std::uint16_t address,
        std::uint8_t value)
    {
        auto& recorder = *static_cast<Recorder*>(user);
        if (recorder.done) {
            return;
        }
        if (address == kFrameMarker) {
            recorder.done = ++recorder.frame == recorder.trace->frames;
            return;
        }
        if (recorder.frame >= 0) {
            recorder.trace->entries.push_back(
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

Trace run_rom(
    const std::vector<std::uint8_t>& rom,
    std::uint32_t frames)
{
    Trace trace;
    trace.frames = frames;
    if (frames == 0) {
        return trace;
    }
    if (rom.size() < kMinRomSize) {
        throw RunError("ROM is smaller than its header");
    }

    Recorder recorder {&trace};
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
    return trace;
}

} // namespace golem
