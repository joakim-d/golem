#pragma once

#include "golem/song.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// Instruments as named fields, and wave helpers, for the instrument and wave editors.
// See docs/song-format.md and docs/editor-steps/03-instrument-editors.md.
namespace golem::edit {

constexpr std::size_t kWaveSamples = kWaveBytes * 2;

struct PulseFields {
    bool length_enable = false;
    int length = 0; // 0..63
    int sweep_pace = 0; // 0..7, 0 = no sweep
    bool sweep_decrease = false;
    int sweep_steps = 0; // 0..7
    int duty = 0; // 0..3: 12.5%, 25%, 50%, 75%
    int volume = 0; // 0..15
    bool envelope_increase = false;
    int envelope_pace = 0; // 0..7, 0 = held

    bool operator==(const PulseFields& other) const;
};

struct WaveFields {
    bool length_enable = false;
    int length = 0; // 0..255
    int volume = 0; // NR32 code 0..3: mute, 100%, 50%, 25%
    int wave = 0; // 0..15

    bool operator==(const WaveFields& other) const;
};

struct NoiseFields {
    bool short_lfsr = false; // 7-bit LFSR
    bool length_enable = false;
    int length = 0; // 0..63
    int volume = 0; // 0..15
    bool envelope_increase = false;
    int envelope_pace = 0; // 0..7, 0 = held

    bool operator==(const NoiseFields& other) const;
};

// Decoding reads every field; encoding clamps each field to its range.
PulseFields pulse_fields(const PulseInstrument& instrument);
PulseInstrument pulse_instrument(const PulseFields& fields);
WaveFields wave_fields(const WaveInstrument& instrument);
WaveInstrument wave_instrument(const WaveFields& fields);
NoiseFields noise_fields(const NoiseInstrument& instrument);
NoiseInstrument noise_instrument(const NoiseFields& fields);

// Sample `index` (0..31) of a wave, 0..15; the first sample is the high nibble of byte 0.
std::uint8_t wave_sample(
    const Wave& wave,
    std::size_t index);

// A wave as 32 uppercase hex digits, one per sample, and back. Parsing ignores spaces and
// accepts lowercase; anything else, or another number of digits, gives std::nullopt.
std::string wave_hex(const Wave& wave);
std::optional<Wave> parse_wave_hex(std::string_view text);

enum class WavePreset {
    Square,
    Saw,
    Triangle,
    Sine,
};

Wave wave_preset(WavePreset preset);

} // namespace golem::edit
