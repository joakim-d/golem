#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

namespace golem {

struct StereoSample {
    std::int16_t left = 0;
    std::int16_t right = 0;
};

bool operator==(
    const StereoSample& a,
    const StereoSample& b);

// Writes `samples` as a 16-bit stereo PCM WAV file at `sample_rate` Hz.
void write_wav(
    std::ostream& out,
    const std::vector<StereoSample>& samples,
    unsigned sample_rate);

} // namespace golem
