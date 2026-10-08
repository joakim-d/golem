#include "golem/audio.h"
#include "golem/rom_runner.h"

#include "rom_builder.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>

using namespace golem;
using golem::test::RomBuilder;

namespace {

constexpr std::uint8_t kLdA = 0x3E; // ld a, n
constexpr std::uint8_t kLdh = 0xE0; // ldh [$FF00 + n], a

// Turns the APU on, then loops forever.
std::vector<std::uint8_t> silent_rom()
{
    RomBuilder rom;
    rom.code({kLdA, 0x80, kLdh, 0x26}) // NR52 = $80
        .code({0x18, 0xFE}); // jr to itself
    return rom.build();
}

// Plays A-4 (period 1750: 131072 / 298 = 439.8 Hz) on channel 2 at full volume, forever.
std::vector<std::uint8_t> tone_rom()
{
    RomBuilder rom;
    rom.code({kLdA, 0x80, kLdh, 0x26}) // NR52 = $80
        .code({kLdA, 0x77, kLdh, 0x24}) // NR50 = $77
        .code({kLdA, 0xFF, kLdh, 0x25}) // NR51 = $FF
        .code({kLdA, 0x80, kLdh, 0x16}) // NR21 = $80: 50% duty
        .code({kLdA, 0xF0, kLdh, 0x17}) // NR22 = $F0: volume F, no envelope
        .code({kLdA, 0xD6, kLdh, 0x18}) // NR23 = $D6
        .code({kLdA, 0x86, kLdh, 0x19}) // NR24 = $86: trigger, period high bits 6
        .code({0x18, 0xFE}); // jr to itself
    return rom.build();
}

int peak(const std::vector<StereoSample>& samples)
{
    int peak = 0;
    for (const auto& sample : samples) {
        peak = std::max({peak, std::abs(int {sample.left}), std::abs(int {sample.right})});
    }
    return peak;
}

} // namespace

TEST(
    Audio,
    MixAddsAndSaturates)
{
    std::vector<StereoSample> into {{100, -100}, {30000, -30000}, {-32768, 32767}};
    const std::vector<StereoSample> from {{20, 30}, {10000, -10000}, {-1, 1}};
    mix_into(into.data(), from.data(), into.size());
    EXPECT_EQ(into, (std::vector<StereoSample> {{120, -70}, {32767, -32768}, {-32768, 32767}}));
}

#ifdef GOLEM_HAS_SAMEBOY

TEST(
    Audio,
    LastsAsLongAsTheEmulatedFrames)
{
    const auto samples = render_audio(silent_rom(), 60, 44100);
    const double expected = 60 * 70224.0 / 4194304.0 * 44100;
    EXPECT_NEAR(double(samples.size()), expected, 2.0);
}

TEST(
    Audio,
    ASilentRomIsSilent)
{
    EXPECT_LT(peak(render_audio(silent_rom(), 60)), 16);
}

TEST(
    Audio,
    ATonePlaysAtItsPitch)
{
    const auto samples = render_audio(tone_rom(), 120, 44100);
    EXPECT_GT(peak(samples), 1000);
    // Rising zero crossings over the last second (after the high-pass filter has settled).
    const std::size_t second = 44100;
    ASSERT_GT(samples.size(), second);
    int crossings = 0;
    for (std::size_t i = samples.size() - second; i < samples.size(); ++i) {
        if (samples[i - 1].left < 0 && samples[i].left >= 0) {
            ++crossings;
        }
    }
    EXPECT_NEAR(crossings, 439.8, 439.8 * 0.02);
}

TEST(
    Audio,
    RejectsRomsSmallerThanTheHeader)
{
    EXPECT_THROW(render_audio(std::vector<std::uint8_t>(0x100), 1), RunError);
}

#else

TEST(
    Audio,
    NeedsSameBoy)
{
    EXPECT_THROW(render_audio(silent_rom(), 1), RunError);
}

#endif
