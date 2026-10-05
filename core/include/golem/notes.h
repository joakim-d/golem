#pragma once

#include <cstdint>

namespace golem {

constexpr std::uint8_t kNoteNone = 0;
constexpr std::uint8_t kFirstNote = 1; // C-2
constexpr std::uint8_t kLastNote = 72; // B-7

// Pitch of a note in Hz (equal temperament, A-4 = 440 Hz). Throws std::out_of_range
// outside kFirstNote..kLastNote.
double note_frequency(std::uint8_t note);

// 11-bit period for NRx3/NRx4: lround(2048 - 131072 / note_frequency(note)).
// Channel 3 uses the same table and therefore sounds one octave lower.
std::uint16_t note_period(std::uint8_t note);

// NR43 clock shift (bits 7-4) and divider (bits 2-0) whose noise frequency is the
// nearest to note_frequency(note) on a log scale; ties go to the smaller shift.
// The LFSR width bit (bit 3) is always clear.
std::uint8_t noise_nr43(std::uint8_t note);

} // namespace golem
