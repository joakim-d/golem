#include "golem/instrument_fields.h"

#include <algorithm>

namespace golem::edit {

namespace {

    std::uint8_t bits(
        int value,
        int max,
        int shift)
    {
        return static_cast<std::uint8_t>(std::clamp(value, 0, max) << shift);
    }

    std::uint8_t flag(
        bool value,
        int bit)
    {
        return static_cast<std::uint8_t>(value ? 1 << bit : 0);
    }

    constexpr char kHexDigits[] = "0123456789ABCDEF";

    // 32 samples, given as hex digits, into a wave.
    Wave wave_from_samples(std::string_view samples)
    {
        return *parse_wave_hex(samples);
    }

} // namespace

bool PulseFields::operator==(const PulseFields& other) const
{
    return length_enable == other.length_enable
        && length == other.length
        && sweep_pace == other.sweep_pace
        && sweep_decrease == other.sweep_decrease
        && sweep_steps == other.sweep_steps
        && duty == other.duty
        && volume == other.volume
        && envelope_increase == other.envelope_increase
        && envelope_pace == other.envelope_pace;
}

bool WaveFields::operator==(const WaveFields& other) const
{
    return length_enable == other.length_enable
        && length == other.length
        && volume == other.volume
        && wave == other.wave;
}

bool NoiseFields::operator==(const NoiseFields& other) const
{
    return short_lfsr == other.short_lfsr
        && length_enable == other.length_enable
        && length == other.length
        && volume == other.volume
        && envelope_increase == other.envelope_increase
        && envelope_pace == other.envelope_pace;
}

PulseFields pulse_fields(const PulseInstrument& instrument)
{
    const auto& b = instrument.bytes;
    PulseFields fields;
    fields.length_enable = b[0] & 0x80;
    fields.sweep_pace = b[0] >> 4 & 0x07;
    fields.sweep_decrease = b[0] & 0x08;
    fields.sweep_steps = b[0] & 0x07;
    fields.duty = b[1] >> 6;
    fields.length = b[1] & 0x3F;
    fields.volume = b[2] >> 4;
    fields.envelope_increase = b[2] & 0x08;
    fields.envelope_pace = b[2] & 0x07;
    return fields;
}

PulseInstrument pulse_instrument(const PulseFields& fields)
{
    PulseInstrument instrument;
    instrument.bytes[0] = static_cast<std::uint8_t>(
        flag(fields.length_enable, 7)
        | bits(fields.sweep_pace, 7, 4)
        | flag(fields.sweep_decrease, 3)
        | bits(fields.sweep_steps, 7, 0));
    instrument.bytes[1] =
        static_cast<std::uint8_t>(bits(fields.duty, 3, 6) | bits(fields.length, 63, 0));
    instrument.bytes[2] = static_cast<std::uint8_t>(
        bits(fields.volume, 15, 4)
        | flag(fields.envelope_increase, 3)
        | bits(fields.envelope_pace, 7, 0));
    return instrument;
}

WaveFields wave_fields(const WaveInstrument& instrument)
{
    const auto& b = instrument.bytes;
    WaveFields fields;
    fields.length = b[0];
    fields.length_enable = b[1] & 0x80;
    fields.volume = b[1] >> 5 & 0x03;
    fields.wave = b[1] & 0x0F;
    return fields;
}

WaveInstrument wave_instrument(const WaveFields& fields)
{
    WaveInstrument instrument;
    instrument.bytes[0] = bits(fields.length, 255, 0);
    instrument.bytes[1] = static_cast<std::uint8_t>(
        flag(fields.length_enable, 7) | bits(fields.volume, 3, 5) | bits(fields.wave, 15, 0));
    return instrument;
}

NoiseFields noise_fields(const NoiseInstrument& instrument)
{
    const auto& b = instrument.bytes;
    NoiseFields fields;
    fields.short_lfsr = b[0] & 0x80;
    fields.length_enable = b[0] & 0x40;
    fields.length = b[0] & 0x3F;
    fields.volume = b[1] >> 4;
    fields.envelope_increase = b[1] & 0x08;
    fields.envelope_pace = b[1] & 0x07;
    return fields;
}

NoiseInstrument noise_instrument(const NoiseFields& fields)
{
    NoiseInstrument instrument;
    instrument.bytes[0] = static_cast<std::uint8_t>(
        flag(fields.short_lfsr, 7) | flag(fields.length_enable, 6) | bits(fields.length, 63, 0));
    instrument.bytes[1] = static_cast<std::uint8_t>(
        bits(fields.volume, 15, 4)
        | flag(fields.envelope_increase, 3)
        | bits(fields.envelope_pace, 7, 0));
    return instrument;
}

std::uint8_t wave_sample(
    const Wave& wave,
    std::size_t index)
{
    const std::uint8_t byte = wave[index / 2];
    return index % 2 == 0 ? byte >> 4 : byte & 0x0F;
}

std::string wave_hex(const Wave& wave)
{
    std::string text;
    for (std::size_t index = 0; index < kWaveSamples; ++index) {
        text += kHexDigits[wave_sample(wave, index)];
    }
    return text;
}

std::optional<Wave> parse_wave_hex(std::string_view text)
{
    Wave wave {};
    std::size_t index = 0;
    for (const char c : text) {
        if (c == ' ') {
            continue;
        }
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else {
            return std::nullopt;
        }
        if (index == kWaveSamples) {
            return std::nullopt;
        }
        wave[index / 2] |= static_cast<std::uint8_t>(index % 2 == 0 ? digit << 4 : digit);
        ++index;
    }
    if (index != kWaveSamples) {
        return std::nullopt;
    }
    return wave;
}

Wave wave_preset(WavePreset preset)
{
    switch (preset) {
    case WavePreset::Square:
        return wave_from_samples("FFFFFFFFFFFFFFFF0000000000000000");
    case WavePreset::Saw:
        return wave_from_samples("00112233445566778899AABBCCDDEEFF");
    case WavePreset::Triangle:
        return wave_from_samples("0123456789ABCDEFFEDCBA9876543210");
    case WavePreset::Sine:
        // A fixed table rather than rounded sin(): sample i + 16 is 15 - sample i.
        return wave_from_samples("89ACDEEFFFEEDCA97653211000112356");
    }
    return {};
}

} // namespace golem::edit
