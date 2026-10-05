#pragma once

#include "golem/apu.h"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace golem {

// Trace text format (.trace):
//
//   golem-trace 1 frames=600     # header: format version and number of frames
//   0000 NR52 80                 # <frame> <register> <value>, one write per line
//   0001 NR22 F3
//
// Frames are decimal (at least 4 digits when written) and never decrease; registers use
// register_name(); values are 2 hex digits. Frames without writes have no lines. Blank
// lines and lines starting with `#` are ignored when reading.

class TraceError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct TraceEntry {
    std::uint32_t frame;
    ApuWrite write;
};

struct Trace {
    std::uint32_t frames = 0;
    std::vector<TraceEntry> entries; // Sorted by frame, in write order within a frame.
};

bool operator==(
    const TraceEntry& a,
    const TraceEntry& b);
bool operator==(
    const Trace& a,
    const Trace& b);

// Trace of `frames[i]` written during frame i.
Trace make_trace(const std::vector<std::vector<ApuWrite>>& frames);

void write_trace(
    std::ostream& out,
    const Trace& trace);

// Throws TraceError("line N: ...") on malformed input.
Trace read_trace(std::istream& in);

struct Mismatch {
    enum class Kind {
        Write, // A write differs, is missing (no actual) or is extra (no expected).
        FrameCount,
    };

    Kind kind = Kind::Write;
    std::uint32_t frame = 0;
    std::size_t index = 0; // 1-based position of the write within the frame.
    std::optional<ApuWrite> expected;
    std::optional<ApuWrite> actual;
    std::uint32_t expected_frames = 0;
    std::uint32_t actual_frames = 0;

    // "frame 212: write #2: expected NR22=F3, got NR22=F1"
    // "frame 212: write #5: expected NR24=87, got nothing"
    // "frame count: expected 600, got 300"
    std::string to_string() const;
};

// Compares the writes of each frame position by position, over the frames both traces
// have, then the frame counts. Returns every mismatch in frame order; empty if equal.
std::vector<Mismatch> diff_traces(
    const Trace& expected,
    const Trace& actual);

} // namespace golem
