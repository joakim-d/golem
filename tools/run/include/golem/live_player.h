#pragma once

#include "golem/audio.h"
#include "golem/player.h"
#include "golem/song.h"
#include "golem/wav.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Plays a song with the real driver in SameBoy, for the editor.
// See docs/editor-steps/01-live-playback.md.
namespace golem {

// Address and size of the song slot of the player ROM: ROMX bank 1, $4000-$7FFF.
constexpr std::uint16_t kPlayerSongAddress = 0x4000;
constexpr std::size_t kPlayerSongSlotSize = 0x4000;

// Empty when songs can be played; otherwise why not (SameBoy or RGBDS missing in the build).
std::string playback_unavailable_reason();

// The player ROM template (the driver test ROM shell and the driver) with `song` encoded in
// its song slot: the driver plays it from power-on. Throws RunError if playback is not built
// in or the song does not fit in the slot, SongError if the song is invalid.
std::vector<std::uint8_t> player_rom(const Song& song);

// A song playing in SameBoy, producing audio on demand. render() may run on an audio thread
// while play() and stop() run on another.
class LivePlayer {
public:
    explicit LivePlayer(unsigned sample_rate = kDefaultSampleRate);
    ~LivePlayer();
    LivePlayer(const LivePlayer&) = delete;
    LivePlayer& operator=(const LivePlayer&) = delete;

    // Starts `song` from the beginning, replacing what was playing. Throws like player_rom().
    void play(const Song& song);
    void stop();
    bool is_playing() const;

    // While playing: where the song is at the sample `latency` samples before the last one
    // rendered, i.e. what is heard while `latency` samples wait in the audio output. The
    // reference player follows the driver frame by frame, by the player ROM's frame markers.
    // Order 0, row 0, tick 0 before the first row; std::nullopt when not playing.
    std::optional<Player::Position> position(std::size_t latency = 0) const;

    // Fills `samples` with the next `count` samples: the song while playing, else silence.
    void render(
        StereoSample* samples,
        std::size_t count);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace golem
