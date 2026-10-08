#include "golem/wav.h"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

using namespace golem;

namespace {

std::string bytes(std::initializer_list<int> values)
{
    std::string text;
    for (int value : values) {
        text += static_cast<char>(value);
    }
    return text;
}

std::string wav_header(
    std::uint32_t data_size,
    unsigned sample_rate)
{
    const auto le32 = [](std::uint32_t v) {
        return bytes({int(v & 0xFF), int(v >> 8 & 0xFF), int(v >> 16 & 0xFF), int(v >> 24)});
    };
    return "RIFF"
         + le32(36 + data_size)
         + "WAVE"
         + "fmt "
         + le32(16)
         + bytes({1, 0, 2, 0}) // PCM, 2 channels
         + le32(sample_rate)
         + le32(sample_rate * 4) // Byte rate
         + bytes({4, 0, 16, 0}) // Block align, bits per sample
         + "data"
         + le32(data_size);
}

std::string written(
    const std::vector<StereoSample>& samples,
    unsigned sample_rate)
{
    std::ostringstream out;
    write_wav(out, samples, sample_rate);
    return out.str();
}

} // namespace

TEST(
    Wav,
    WritesAStereo16BitPcmFile)
{
    const std::vector<StereoSample> samples {{1, -1}, {0x1234, -2}};
    EXPECT_EQ(
        written(samples, 44100),
        wav_header(8, 44100) + bytes({0x01, 0x00, 0xFF, 0xFF, 0x34, 0x12, 0xFE, 0xFF}));
}

TEST(
    Wav,
    WritesAnEmptyFile)
{
    EXPECT_EQ(written({}, 48000), wav_header(0, 48000));
}

TEST(
    Wav,
    ComparesSamples)
{
    EXPECT_EQ((StereoSample {1, 2}), (StereoSample {1, 2}));
    EXPECT_FALSE((StereoSample {1, 2}) == (StereoSample {1, 3}));
}
