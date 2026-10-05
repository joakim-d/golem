#include "golem/song_binary.h"

#include <gtest/gtest.h>

using namespace golem;

namespace {

constexpr std::size_t kHeaderSize = 19;
constexpr std::size_t kPatternSize = 192;
constexpr std::size_t kFixedBlocksSize = 45 + 30 + 30 + 256; // Instruments and waves.

std::uint16_t read16(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset)
{
    return static_cast<std::uint16_t>(bytes[offset] | bytes[offset + 1] << 8);
}

Song one_pattern_song()
{
    Song song;
    song.ticks_per_row = 5;
    song.patterns.resize(1);
    song.patterns[0][0] = Cell {25, 3, 0xC, 0xF1};
    song.patterns[0][63] = Cell {72, 15, 0xF, 0xFF};
    song.orders.push_back({0, 0, 0, 0});
    song.pulse_instruments[0].bytes = {0x11, 0x12, 0x13};
    song.pulse_instruments[14].bytes = {0xF1, 0xF2, 0xF3};
    song.wave_instruments[0].bytes = {0x21, 0x22};
    song.noise_instruments[0].bytes = {0x31, 0x32};
    song.waves[0][0] = 0x41;
    song.waves[15][15] = 0x42;
    return song;
}

Song rich_song()
{
    Song song = one_pattern_song();
    song.patterns.resize(3);
    song.patterns[1][10] = Cell {1, 1, 9, 0x80};
    song.patterns[2][20] = Cell {0, 0, 0xB, 0x01};
    song.orders = {{0, 1, 2, 0}, {2, 2, 1, 1}};
    return song;
}

} // namespace

TEST(
    SongBinary,
    HeaderLayout)
{
    const auto bytes = encode_song(one_pattern_song(), 0x4000);
    ASSERT_EQ(bytes.size(), kHeaderSize + 1 + 4 * 2 + kFixedBlocksSize + kPatternSize);

    EXPECT_EQ(bytes[0], 5); // Ticks per row.
    EXPECT_EQ(read16(bytes, 1), 0x4013); // Order count right after the header.
    EXPECT_EQ(bytes[0x13], 1);
    EXPECT_EQ(read16(bytes, 3), 0x4014); // Order table, channel 1.
    EXPECT_EQ(read16(bytes, 5), 0x4016);
    EXPECT_EQ(read16(bytes, 7), 0x4018);
    EXPECT_EQ(read16(bytes, 9), 0x401A);
    EXPECT_EQ(read16(bytes, 11), 0x401C); // Pulse instruments.
    EXPECT_EQ(read16(bytes, 13), 0x401C + 45); // Wave instruments.
    EXPECT_EQ(read16(bytes, 15), 0x401C + 75); // Noise instruments.
    EXPECT_EQ(read16(bytes, 17), 0x401C + 105); // Waves.

    const std::uint16_t pattern = 0x401C + 105 + 256;
    for (std::size_t table = 0x14; table < 0x1C; table += 2) {
        EXPECT_EQ(read16(bytes, table), pattern);
    }
}

TEST(
    SongBinary,
    BlockContents)
{
    const auto bytes = encode_song(one_pattern_song(), 0x4000);
    const std::size_t pulse = 0x1C, wave_instruments = pulse + 45, noise = pulse + 75,
                      waves = pulse + 105, pattern = waves + 256;

    EXPECT_EQ(bytes[pulse], 0x11);
    EXPECT_EQ(bytes[pulse + 2], 0x13);
    EXPECT_EQ(bytes[pulse + 42], 0xF1);
    EXPECT_EQ(bytes[pulse + 44], 0xF3);
    EXPECT_EQ(bytes[wave_instruments], 0x21);
    EXPECT_EQ(bytes[wave_instruments + 1], 0x22);
    EXPECT_EQ(bytes[noise], 0x31);
    EXPECT_EQ(bytes[noise + 1], 0x32);
    EXPECT_EQ(bytes[waves], 0x41);
    EXPECT_EQ(bytes[waves + 255], 0x42);

    // Row 0: note, instrument << 4 | effect, param.
    EXPECT_EQ(bytes[pattern + 0], 25);
    EXPECT_EQ(bytes[pattern + 1], 0x3C);
    EXPECT_EQ(bytes[pattern + 2], 0xF1);
    EXPECT_EQ(bytes[pattern + 63 * 3 + 0], 72);
    EXPECT_EQ(bytes[pattern + 63 * 3 + 1], 0xFF);
    EXPECT_EQ(bytes[pattern + 63 * 3 + 2], 0xFF);
}

TEST(
    SongBinary,
    SharedPatternsAreStoredOnce)
{
    const auto bytes = encode_song(rich_song(), 0x4000);
    EXPECT_EQ(bytes.size(), kHeaderSize + 1 + 4 * 2 * 2 + kFixedBlocksSize + 3 * kPatternSize);
}

TEST(
    SongBinary,
    RoundTrips)
{
    for (std::uint16_t base : {0x0000, 0x4000, 0xC000}) {
        EXPECT_EQ(decode_song(encode_song(rich_song(), base), base), rich_song()) << base;
    }
}

TEST(
    SongBinary,
    DecodeNumbersPatternsByFirstUse)
{
    Song song = rich_song();
    song.orders = {{2, 0, 2, 2}};
    Song expected = song;
    expected.patterns = {song.patterns[2], song.patterns[0]};
    expected.orders = {{0, 1, 0, 0}};
    EXPECT_EQ(decode_song(encode_song(song, 0x4000), 0x4000), expected);
}

TEST(
    SongBinary,
    EncodeRejectsInvalidOrTooLargeSongs)
{
    Song song = one_pattern_song();
    EXPECT_THROW(encode_song(song, 0xFF00), SongError);
    song.orders.clear();
    EXPECT_THROW(encode_song(song, 0x4000), SongError);
}

TEST(
    SongBinary,
    DecodeRejectsTruncatedData)
{
    auto bytes = encode_song(one_pattern_song(), 0x4000);
    EXPECT_THROW(decode_song({bytes.begin(), bytes.begin() + 18}, 0x4000), SongError);
    bytes.pop_back();
    EXPECT_THROW(decode_song(bytes, 0x4000), SongError);
}

TEST(
    SongBinary,
    DecodeRejectsPointersOutsideTheData)
{
    const auto bytes = encode_song(one_pattern_song(), 0x4000);
    EXPECT_THROW(decode_song(bytes, 0x0000), SongError);
    EXPECT_THROW(decode_song(bytes, 0x5000), SongError);
}

TEST(
    SongBinary,
    DecodeRejectsZeroOrders)
{
    auto bytes = encode_song(one_pattern_song(), 0x4000);
    bytes[0x13] = 0;
    EXPECT_THROW(decode_song(bytes, 0x4000), SongError);
}

TEST(
    SongBinary,
    DecodeRejectsBadNotes)
{
    auto bytes = encode_song(one_pattern_song(), 0x4000);
    const std::size_t pattern = read16(bytes, read16(bytes, 3) - 0x4000) - 0x4000;
    bytes[pattern] = 73;
    EXPECT_THROW(decode_song(bytes, 0x4000), SongError);
}
