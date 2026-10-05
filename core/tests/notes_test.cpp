#include "golem/notes.h"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

using namespace golem;

namespace {

constexpr std::uint8_t kC2 = 1;
constexpr std::uint8_t kA4 = 34;
constexpr std::uint8_t kB7 = 72;

double noise_frequency(std::uint8_t nr43)
{
    const int shift = nr43 >> 4;
    const int divider = nr43 & 0x07;
    return 262144.0 / (divider == 0 ? 0.5 : divider) / std::pow(2.0, shift);
}

} // namespace

TEST(
    Notes,
    Frequencies)
{
    EXPECT_NEAR(note_frequency(kA4), 440.0, 1e-9);
    EXPECT_NEAR(note_frequency(kC2), 65.406, 1e-3);
    EXPECT_NEAR(note_frequency(kB7), 3951.066, 1e-3);
}

TEST(
    Notes,
    KnownPeriods)
{
    EXPECT_EQ(note_period(kC2), 44);
    EXPECT_EQ(note_period(kA4), 1750);
    EXPECT_EQ(note_period(kB7), 2015);
}

TEST(
    Notes,
    PeriodsIncreaseWithPitch)
{
    for (std::uint8_t note = kFirstNote; note < kLastNote; ++note) {
        EXPECT_LT(note_period(note), note_period(note + 1)) << int(note);
    }
}

TEST(
    Notes,
    OutOfRangeNotesThrow)
{
    EXPECT_THROW(note_period(kNoteNone), std::out_of_range);
    EXPECT_THROW(note_period(kLastNote + 1), std::out_of_range);
    EXPECT_THROW(noise_nr43(kNoteNone), std::out_of_range);
    EXPECT_THROW(noise_nr43(kLastNote + 1), std::out_of_range);
}

TEST(
    Notes,
    NoiseForA4)
{
    // 262144 / 5 / 2^7 = 409.6 Hz is the closest noise frequency to 440 Hz.
    EXPECT_EQ(noise_nr43(kA4), 0x75);
}

TEST(
    Notes,
    NoiseIsValidAndRisesWithPitch)
{
    for (std::uint8_t note = kFirstNote; note <= kLastNote; ++note) {
        const auto nr43 = noise_nr43(note);
        EXPECT_EQ(nr43 & 0x08, 0) << int(note);
        EXPECT_LE(nr43 >> 4, 13) << int(note);
        if (note > kFirstNote) {
            EXPECT_LE(noise_frequency(noise_nr43(note - 1)), noise_frequency(nr43)) << int(note);
        }
    }
}

TEST(
    Notes,
    NoiseIsNearestOnLogScale)
{
    for (std::uint8_t note = kFirstNote; note <= kLastNote; ++note) {
        const double target = std::log2(note_frequency(note));
        const double chosen = std::abs(std::log2(noise_frequency(noise_nr43(note))) - target);
        for (int shift = 0; shift <= 13; ++shift) {
            for (int divider = 0; divider <= 7; ++divider) {
                const auto nr43 = static_cast<std::uint8_t>(shift << 4 | divider);
                EXPECT_LE(chosen, std::abs(std::log2(noise_frequency(nr43)) - target) + 1e-12)
                    << int(note);
            }
        }
    }
}

TEST(
    Notes,
    NoiseTiesGoToSmallerShift)
{
    // 512 Hz is reachable as (shift 7, divider 4), (8, 2), (9, 1) and (10, 0).
    constexpr std::uint8_t kC5 = 37; // 523 Hz, nearest noise is 512 Hz.
    EXPECT_EQ(noise_nr43(kC5), 0x74);
    for (std::uint8_t note = kFirstNote; note <= kLastNote; ++note) {
        if (noise_frequency(noise_nr43(note)) == 512.0) {
            EXPECT_EQ(noise_nr43(note), 0x74) << int(note);
        }
    }
}
