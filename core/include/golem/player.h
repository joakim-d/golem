#pragma once

#include "golem/apu.h"
#include "golem/song.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

namespace golem {

// Thrown when the song uses an effect the reference player does not implement yet.
class UnsupportedEffect : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Reference player: the definition of what the driver writes to the APU, frame by frame.
// See docs/driver-contract.md. It is deterministic and reads no input besides the song.
class Player {
public:
    // Throws SongError if the song is invalid.
    explicit Player(Song song);

    // Writes of the next frame, in order. The first call returns the init writes (frame 0),
    // call n + 1 the writes of the n-th play call (frame n).
    std::vector<ApuWrite> step();

private:
    struct Channel {
        std::uint8_t instrument = 1;
        std::uint16_t period = 0;
        std::uint8_t noise = 0; // NR43 without the width bit, for the last note (channel 4).
        std::optional<unsigned> cut_tick; // Tick of the current row with a pending E.
        std::optional<unsigned> delay_tick; // Tick of the current row with a pending 7,
        std::uint8_t delay_note = 0; // and the note it triggers then.
        std::uint8_t volume = 0; // 0-15: NRx2 volume of the last trigger, C or slide step.
        std::optional<std::uint8_t> slide; // Parameter of the current row's A.
    };

    // Effect 9 and C values folded into a trigger on the same row.
    struct Overrides {
        std::optional<std::uint8_t> timbre;
        std::optional<std::uint8_t> volume;
    };

    void init();
    void play_row();
    void play_cell(
        std::size_t channel,
        const Cell& cell);
    void trigger(
        std::size_t channel,
        std::uint8_t note,
        const Overrides& overrides);
    void apply_effect(
        std::size_t channel,
        const Cell& cell);
    // C without a note: NRx2 = param then retrigger (channels 1, 2, 4), NR32 (channel 3).
    void set_volume(
        std::size_t channel,
        std::uint8_t param);
    // E: silences the channel like C00.
    void cut(std::size_t channel);
    // One non-row tick of A: volume up by x (or down by y), clamped to 0-15; on a change,
    // NRx2 = volume << 4 and a retrigger.
    void slide_volume(std::size_t channel);
    void load_wave(std::uint8_t index);
    std::uint8_t length_enable_bit(std::size_t channel) const;
    void write(
        std::uint16_t address,
        std::uint8_t value);

    Song song_;
    std::vector<ApuWrite> writes_;
    std::uint32_t frame_ = 0;
    unsigned ticks_per_row_;
    unsigned row_length_ = 0;
    unsigned tick_ = 0;
    std::size_t order_ = 0;
    std::size_t row_ = 0;
    std::optional<std::uint8_t> jump_order_;
    std::optional<std::uint8_t> break_row_;
    std::array<Channel, kChannels> channels_;
    std::optional<std::uint8_t> loaded_wave_;
};

// Writes of frames 0..frames-1 (frame 0 is init).
std::vector<std::vector<ApuWrite>> render(
    const Song& song,
    std::size_t frames);

} // namespace golem
