#pragma once

#include "golem/trace.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace golem {

// Thrown when a ROM cannot be loaded or does not produce the expected frames.
class RunError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Test ROMs write any value to this unused APU address to start a frame: once before
// GolemInit (frame 0) and once before each GolemPlay call (frames 1, 2, ...).
constexpr std::uint16_t kFrameMarker = 0xFF15;

// Emulator frames allowed on top of the requested frames before giving up.
constexpr std::uint32_t kExtraEmulatorFrames = 60;

// Runs `rom` headless and returns the APU writes ($FF10-$FF3F) of its first `frames`
// frames, as delimited by kFrameMarker writes. Writes before the first marker are ignored
// and markers are not part of the trace. Throws RunError if the ROM is invalid, the
// emulator reports an error, or the frames are not complete within
// frames + kExtraEmulatorFrames emulator frames.
Trace run_rom(
    const std::vector<std::uint8_t>& rom,
    std::uint32_t frames);

} // namespace golem
