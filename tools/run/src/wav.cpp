#include "golem/wav.h"

#include <ostream>
#include <string>

namespace golem {

namespace {

    constexpr std::uint16_t kPcm = 1;
    constexpr std::uint16_t kChannels = 2;
    constexpr std::uint16_t kBitsPerSample = 16;
    constexpr std::uint32_t kBytesPerFrame = kChannels * kBitsPerSample / 8;

    void put16(
        std::ostream& out,
        std::uint16_t value)
    {
        out.put(static_cast<char>(value & 0xFF));
        out.put(static_cast<char>(value >> 8));
    }

    void put32(
        std::ostream& out,
        std::uint32_t value)
    {
        put16(out, static_cast<std::uint16_t>(value & 0xFFFF));
        put16(out, static_cast<std::uint16_t>(value >> 16));
    }

} // namespace

bool operator==(
    const StereoSample& a,
    const StereoSample& b)
{
    return a.left == b.left && a.right == b.right;
}

void write_wav(
    std::ostream& out,
    const std::vector<StereoSample>& samples,
    unsigned sample_rate)
{
    const auto data_size = static_cast<std::uint32_t>(samples.size() * kBytesPerFrame);
    out << "RIFF";
    put32(out, 36 + data_size);
    out << "WAVE" << "fmt ";
    put32(out, 16); // Size of the fmt chunk.
    put16(out, kPcm);
    put16(out, kChannels);
    put32(out, sample_rate);
    put32(out, sample_rate * kBytesPerFrame); // Byte rate.
    put16(out, static_cast<std::uint16_t>(kBytesPerFrame)); // Block align.
    put16(out, kBitsPerSample);
    out << "data";
    put32(out, data_size);
    for (const auto& sample : samples) {
        put16(out, static_cast<std::uint16_t>(sample.left));
        put16(out, static_cast<std::uint16_t>(sample.right));
    }
}

} // namespace golem
