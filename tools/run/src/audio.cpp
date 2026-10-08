#include "golem/audio.h"

#include "golem/rom_runner.h"

#ifdef GOLEM_HAS_SAMEBOY
#include "sameboy_core.h"
#endif

#include <memory>

namespace golem {

namespace {

    constexpr std::size_t kMinRomSize = 0x150; // Up to the end of the cartridge header.

#ifdef GOLEM_HAS_SAMEBOY
    void collect(
        void* user,
        std::int16_t left,
        std::int16_t right)
    {
        static_cast<std::vector<StereoSample>*>(user)->push_back({left, right});
    }

    struct Destroy {
        void operator()(golem_sb* sb) const
        {
            golem_sb_destroy(sb);
        }
    };
#endif

} // namespace

std::vector<StereoSample> render_audio(
    const std::vector<std::uint8_t>& rom,
    std::uint32_t frames,
    unsigned sample_rate)
{
#ifdef GOLEM_HAS_SAMEBOY
    if (rom.size() < kMinRomSize) {
        throw RunError("ROM is smaller than its header");
    }
    std::vector<StereoSample> samples;
    const char* error = nullptr;
    std::unique_ptr<golem_sb, Destroy> sb(
        golem_sb_create(rom.data(), rom.size(), nullptr, nullptr, &error));
    if (!sb) {
        throw RunError(error);
    }
    golem_sb_set_audio(sb.get(), sample_rate, collect, &samples);
    for (std::uint32_t frame = 0; frame < frames; ++frame) {
        golem_sb_run_frame(sb.get(), &error);
    }
    return samples;
#else
    (void)rom;
    (void)frames;
    (void)sample_rate;
    (void)kMinRomSize;
    throw RunError("rendering audio needs SameBoy, which is not built in");
#endif
}

} // namespace golem
