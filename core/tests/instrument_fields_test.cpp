#include "golem/instrument_fields.h"

#include <gtest/gtest.h>

using namespace golem;
using namespace golem::edit;

// --- Field codecs ---

TEST(
    Edit,
    PulseFieldsDecode)
{
    // Length on, sweep pace 5 decreasing by 3 steps; duty 50%, length 42; volume F, down, pace 3.
    const PulseFields fields = pulse_fields(PulseInstrument {{0xDB, 0xAA, 0xF3}});
    EXPECT_TRUE(fields.length_enable);
    EXPECT_EQ(fields.sweep_pace, 5);
    EXPECT_TRUE(fields.sweep_decrease);
    EXPECT_EQ(fields.sweep_steps, 3);
    EXPECT_EQ(fields.duty, 2);
    EXPECT_EQ(fields.length, 42);
    EXPECT_EQ(fields.volume, 15);
    EXPECT_FALSE(fields.envelope_increase);
    EXPECT_EQ(fields.envelope_pace, 3);
    EXPECT_TRUE(pulse_fields(PulseInstrument {{0x00, 0x00, 0x08}}).envelope_increase);
}

TEST(
    Edit,
    PulseFieldsRoundTripEveryByte)
{
    for (int value = 0; value < 256; ++value) {
        const auto byte = static_cast<std::uint8_t>(value);
        for (const PulseInstrument instrument :
             {PulseInstrument {{byte, 0x00, 0x00}},
              PulseInstrument {{0x00, byte, 0x00}},
              PulseInstrument {{0x00, 0x00, byte}},
              PulseInstrument {{byte, byte, byte}}}) {
            EXPECT_EQ(pulse_instrument(pulse_fields(instrument)).bytes, instrument.bytes)
                << "byte value "
                << value;
        }
    }
}

TEST(
    Edit,
    PulseFieldsClampWhenEncoding)
{
    PulseFields fields;
    fields.length = 100;
    fields.sweep_pace = 9;
    fields.sweep_steps = -1;
    fields.duty = 7;
    fields.volume = 20;
    fields.envelope_pace = 8;
    PulseFields expected;
    expected.length = 63;
    expected.sweep_pace = 7;
    expected.sweep_steps = 0;
    expected.duty = 3;
    expected.volume = 15;
    expected.envelope_pace = 7;
    EXPECT_EQ(pulse_fields(pulse_instrument(fields)), expected);
}

TEST(
    Edit,
    WaveFieldsDecode)
{
    // Length timer 200, length on, volume 50%, wave 9.
    const WaveFields fields = wave_fields(WaveInstrument {{200, 0xC9}});
    EXPECT_EQ(fields.length, 200);
    EXPECT_TRUE(fields.length_enable);
    EXPECT_EQ(fields.volume, 2);
    EXPECT_EQ(fields.wave, 9);
}

TEST(
    Edit,
    WaveFieldsRoundTripEveryByte)
{
    for (int value = 0; value < 256; ++value) {
        const auto byte = static_cast<std::uint8_t>(value);
        const WaveInstrument length {{byte, 0x00}};
        EXPECT_EQ(wave_instrument(wave_fields(length)).bytes, length.bytes);
        // Bit 4 of byte 1 is reserved, and always encoded as 0.
        const WaveInstrument settings {{0x00, byte}};
        const std::array<std::uint8_t, 2> expected {0x00, static_cast<std::uint8_t>(byte & 0xEF)};
        EXPECT_EQ(wave_instrument(wave_fields(settings)).bytes, expected) << "byte " << value;
    }
}

TEST(
    Edit,
    WaveFieldsClampWhenEncoding)
{
    WaveFields fields;
    fields.length = 300;
    fields.volume = 4;
    fields.wave = 16;
    WaveFields expected;
    expected.length = 255;
    expected.volume = 3;
    expected.wave = 15;
    EXPECT_EQ(wave_fields(wave_instrument(fields)), expected);
}

TEST(
    Edit,
    NoiseFieldsDecode)
{
    // 7-bit LFSR, length on, length 33; volume A, up, pace 1.
    const NoiseFields fields = noise_fields(NoiseInstrument {{0xE1, 0xA9}});
    EXPECT_TRUE(fields.short_lfsr);
    EXPECT_TRUE(fields.length_enable);
    EXPECT_EQ(fields.length, 33);
    EXPECT_EQ(fields.volume, 10);
    EXPECT_TRUE(fields.envelope_increase);
    EXPECT_EQ(fields.envelope_pace, 1);
}

TEST(
    Edit,
    NoiseFieldsRoundTripEveryByte)
{
    for (int value = 0; value < 256; ++value) {
        const auto byte = static_cast<std::uint8_t>(value);
        for (const NoiseInstrument instrument :
             {NoiseInstrument {{byte, 0x00}},
              NoiseInstrument {{0x00, byte}},
              NoiseInstrument {{byte, byte}}}) {
            EXPECT_EQ(noise_instrument(noise_fields(instrument)).bytes, instrument.bytes)
                << "byte value "
                << value;
        }
    }
}

TEST(
    Edit,
    NoiseFieldsClampWhenEncoding)
{
    NoiseFields fields;
    fields.length = 64;
    fields.volume = -3;
    fields.envelope_pace = 100;
    NoiseFields expected;
    expected.length = 63;
    expected.volume = 0;
    expected.envelope_pace = 7;
    EXPECT_EQ(noise_fields(noise_instrument(fields)), expected);
}

// --- Waves ---

TEST(
    Edit,
    WaveSamples)
{
    const Wave wave {
        0x1F,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xA5};
    EXPECT_EQ(wave_sample(wave, 0), 0x1); // High nibble first.
    EXPECT_EQ(wave_sample(wave, 1), 0xF);
    EXPECT_EQ(wave_sample(wave, 2), 0x0);
    EXPECT_EQ(wave_sample(wave, 30), 0xA);
    EXPECT_EQ(wave_sample(wave, 31), 0x5);
}

TEST(
    Edit,
    WaveHexRoundTrip)
{
    const Wave wave = wave_preset(WavePreset::Triangle);
    EXPECT_EQ(wave_hex(wave), "0123456789ABCDEFFEDCBA9876543210");
    EXPECT_EQ(parse_wave_hex("0123456789ABCDEFFEDCBA9876543210"), wave);
    EXPECT_EQ(parse_wave_hex("01234567 89abcdef fedcba98 76543210"), wave); // Spaces, lowercase.
}

TEST(
    Edit,
    WaveHexRejectsInvalidText)
{
    EXPECT_FALSE(parse_wave_hex(""));
    EXPECT_FALSE(parse_wave_hex("0123456789ABCDEFFEDCBA987654321")); // 31 digits
    EXPECT_FALSE(parse_wave_hex("0123456789ABCDEFFEDCBA98765432100")); // 33 digits
    EXPECT_FALSE(parse_wave_hex("0123456789ABCDEFFEDCBA987654321G"));
    EXPECT_FALSE(parse_wave_hex("0123456789ABCDEF,FEDCBA9876543210"));
}

TEST(
    Edit,
    WavePresets)
{
    EXPECT_EQ(wave_hex(wave_preset(WavePreset::Square)), "FFFFFFFFFFFFFFFF0000000000000000");
    EXPECT_EQ(wave_hex(wave_preset(WavePreset::Saw)), "00112233445566778899AABBCCDDEEFF");
    EXPECT_EQ(wave_hex(wave_preset(WavePreset::Triangle)), "0123456789ABCDEFFEDCBA9876543210");
    EXPECT_EQ(wave_hex(wave_preset(WavePreset::Sine)), "89ACDEEFFFEEDCA97653211000112356");
}
