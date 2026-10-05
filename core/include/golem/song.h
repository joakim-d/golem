#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace golem {

constexpr std::size_t kChannels = 4;
constexpr std::size_t kRowsPerPattern = 64;
constexpr std::size_t kInstruments = 15; // Numbered 1..15; stored at index number - 1.
constexpr std::size_t kWaves = 16;
constexpr std::size_t kWaveBytes = 16;
constexpr std::size_t kMaxOrders = 255;

// Thrown for malformed songs, whatever their source (text, binary or in memory).
class SongError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct Cell {
    std::uint8_t note = 0; // 0 = none, 1..72 = C-2..B-7
    std::uint8_t instrument = 0; // 0 = keep current, 1..15
    std::uint8_t effect = 0; // 0..F
    std::uint8_t param = 0; // xx

    bool empty() const
    {
        return note == 0 && instrument == 0 && effect == 0 && param == 0;
    }
};

using Pattern = std::array<Cell, kRowsPerPattern>;

// Instruments keep the raw bytes of docs/song-format.md; accessors decode them.
struct PulseInstrument {
    std::array<std::uint8_t, 3> bytes {};

    bool length_enable() const
    {
        return bytes[0] & 0x80;
    }
    std::uint8_t nr10() const
    {
        return bytes[0] & 0x7F;
    }
    std::uint8_t nrx1() const
    {
        return bytes[1];
    }
    std::uint8_t nrx2() const
    {
        return bytes[2];
    }
};

struct WaveInstrument {
    std::array<std::uint8_t, 2> bytes {};

    std::uint8_t nr31() const
    {
        return bytes[0];
    }
    bool length_enable() const
    {
        return bytes[1] & 0x80;
    }
    std::uint8_t nr32() const
    {
        return bytes[1] & 0x60;
    }
    std::uint8_t wave_index() const
    {
        return bytes[1] & 0x0F;
    }
};

struct NoiseInstrument {
    std::array<std::uint8_t, 2> bytes {};

    bool short_lfsr() const
    {
        return bytes[0] & 0x80;
    } // 7-bit LFSR (NR43 bit 3)
    bool length_enable() const
    {
        return bytes[0] & 0x40;
    }
    std::uint8_t nr41() const
    {
        return bytes[0] & 0x3F;
    }
    std::uint8_t nr42() const
    {
        return bytes[1];
    }
};

using Wave = std::array<std::uint8_t, kWaveBytes>;
using Order = std::array<std::uint8_t, kChannels>; // Pattern index per channel.

struct Song {
    std::uint8_t ticks_per_row = 6; // 0 = 256
    std::vector<Order> orders;
    std::vector<Pattern> patterns;
    std::array<PulseInstrument, kInstruments> pulse_instruments {};
    std::array<WaveInstrument, kInstruments> wave_instruments {};
    std::array<NoiseInstrument, kInstruments> noise_instruments {};
    std::array<Wave, kWaves> waves {};
};

bool operator==(
    const Cell& a,
    const Cell& b);
bool operator!=(
    const Cell& a,
    const Cell& b);
bool operator==(
    const Song& a,
    const Song& b);
bool operator!=(
    const Song& a,
    const Song& b);

// Throws SongError unless: 1..255 orders, every order references an existing pattern,
// and every cell has note <= 72, instrument <= 15 and effect <= 0xF.
void validate_song(const Song& song);

} // namespace golem
