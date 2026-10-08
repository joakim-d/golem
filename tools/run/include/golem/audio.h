#pragma once

#include "golem/wav.h"

#include <cstdint>
#include <vector>

namespace golem {

constexpr unsigned kDefaultSampleRate = 44100;

// Plays `rom` in SameBoy (a DMG, with its hardware-like high-pass filter) for `frames`
// emulator frames from power-on, and returns what it outputs: about
// frames * 70224 / 4194304 seconds of audio at `sample_rate` Hz. For ears only: the tests
// compare APU writes, never sound. Throws RunError if SameBoy is not built in or the ROM is
// smaller than its header.
std::vector<StereoSample> render_audio(
    const std::vector<std::uint8_t>& rom,
    std::uint32_t frames,
    unsigned sample_rate = kDefaultSampleRate);

} // namespace golem
