#pragma once

#include "golem/song.h"

#include <cstdint>
#include <vector>

namespace golem {

// Address the test songs are built for (start of ROMX bank 1).
constexpr std::uint16_t kDefaultSongBase = 0x4000;

// Binary song format of docs/song-format.md. Pointers are absolute GB addresses, so a song
// is encoded for the address `base` it will be loaded at.
//
// encode_song() lays the blocks out as: header (19 bytes), order count, order tables for
// channels 1..4, pulse, wave and noise instruments, waves, then patterns in index order.
// Throws SongError if the song is invalid or does not fit below $10000.
std::vector<std::uint8_t> encode_song(
    const Song& song,
    std::uint16_t base);

// Throws SongError on truncated data, pointers outside `bytes`, or an invalid song.
// Patterns are numbered by first use (orders in sequence, channels 1..4); patterns that
// no order references are not part of the binary.
Song decode_song(
    const std::vector<std::uint8_t>& bytes,
    std::uint16_t base);

} // namespace golem
