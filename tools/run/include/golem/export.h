#pragma once

#include "golem/audio.h"
#include "golem/song.h"
#include "golem/wav.h"

#include <cstdint>
#include <string>
#include <vector>

// Songs out of the editor: a ROM that plays them, a WAV, a binary song.
// See docs/editor-steps/05-export.md.
namespace golem {

// Frames of audio after the last pass of an exported WAV: the release of the last notes.
constexpr std::uint32_t kExportTailFrames = 120; // About 2 seconds.

// The audio of `passes` passes of `song` (song_length() frames each), then `tail_frames`
// frames, as the driver plays it in SameBoy. Throws like player_rom() and render_audio().
std::vector<StereoSample> render_song(
    const Song& song,
    unsigned passes,
    std::uint32_t tail_frames = kExportTailFrames,
    unsigned sample_rate = kDefaultSampleRate);

// Write `song` to `path` as the player ROM with the song (it plays the song from power-on),
// as a 16-bit stereo WAV of `passes` passes and the tail, or as the binary song encoded at
// kPlayerSongAddress. Throw RunError if the file cannot be written, and like player_rom()
// or render_song().
void export_rom(
    const Song& song,
    const std::string& path);
void export_wav(
    const Song& song,
    const std::string& path,
    unsigned passes);
void export_binary(
    const Song& song,
    const std::string& path);

} // namespace golem
