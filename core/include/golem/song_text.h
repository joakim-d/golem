#pragma once

#include "golem/song.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace golem {

// Text song format (.gsong). One directive per line, numbers in hexadecimal, `#` at the start
// of a word starts a comment. Anything not listed is zero, except ticks_per_row which defaults to
// 06.
//
//   ticks_per_row 06
//   pulse 1 00 80 F3                          # instrument 1..F, raw bytes
//   wave_instrument 1 00 20
//   noise 1 00 F1
//   wave 0 0123456789ABCDEFFEDCBA9876543210   # wave 0..F, 16 bytes
//   order 00 01 02 03                         # one line per order: pattern for ch1..ch4
//   pattern 00                                # following rows belong to this pattern
//   00 C-4 1 ...                              # row note(---|C-4|C#4) instrument(.|1..F)
//   10 --- . F03                              #     effect(...|code + param, e.g. F03)

// Throws SongError("line N: ...") on malformed text; the result passes validate_song().
Song parse_song_text(std::string_view text);

// Canonical text: lists only non-zero instruments and waves and non-empty rows.
// parse_song_text(format_song_text(song)) == song.
std::string format_song_text(const Song& song);

// "C-2".."B-7" for notes 1..72, "---" for 0.
std::string note_name(std::uint8_t note);

// Inverse of note_name(); std::nullopt for anything else.
std::optional<std::uint8_t> parse_note_name(std::string_view name);

} // namespace golem
