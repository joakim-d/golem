#include "golem/song.h"

#include <gtest/gtest.h>

using namespace golem;

namespace {

Song one_pattern_song()
{
    Song song;
    song.patterns.resize(1);
    song.orders.push_back({0, 0, 0, 0});
    return song;
}

} // namespace

TEST(
    Song,
    CellEmpty)
{
    EXPECT_TRUE(Cell {}.empty());
    EXPECT_FALSE((Cell {1, 0, 0, 0}.empty()));
    EXPECT_FALSE((Cell {0, 0, 0, 1}.empty()));
}

TEST(
    Song,
    PulseInstrumentFields)
{
    const PulseInstrument instrument {{0xFB, 0x81, 0xF3}};
    EXPECT_TRUE(instrument.length_enable());
    EXPECT_EQ(instrument.nr10(), 0x7B);
    EXPECT_EQ(instrument.nrx1(), 0x81);
    EXPECT_EQ(instrument.nrx2(), 0xF3);
    EXPECT_FALSE((PulseInstrument {{0x7F, 0, 0}}.length_enable()));
}

TEST(
    Song,
    WaveInstrumentFields)
{
    const WaveInstrument instrument {{0x40, 0xFF}};
    EXPECT_EQ(instrument.nr31(), 0x40);
    EXPECT_TRUE(instrument.length_enable());
    EXPECT_EQ(instrument.nr32(), 0x60);
    EXPECT_EQ(instrument.wave_index(), 0x0F);
    EXPECT_FALSE((WaveInstrument {{0, 0x7F}}.length_enable()));
}

TEST(
    Song,
    NoiseInstrumentFields)
{
    const NoiseInstrument instrument {{0xFF, 0xA5}};
    EXPECT_TRUE(instrument.short_lfsr());
    EXPECT_TRUE(instrument.length_enable());
    EXPECT_EQ(instrument.nr41(), 0x3F);
    EXPECT_EQ(instrument.nr42(), 0xA5);
    EXPECT_FALSE((NoiseInstrument {{0x40, 0}}.short_lfsr()));
    EXPECT_FALSE((NoiseInstrument {{0x80, 0}}.length_enable()));
}

TEST(
    Song,
    Equality)
{
    Song a = one_pattern_song();
    Song b = one_pattern_song();
    EXPECT_EQ(a, b);
    b.patterns[0][63].param = 1;
    EXPECT_NE(a, b);
    b = a;
    b.waves[15][15] = 1;
    EXPECT_NE(a, b);
    b = a;
    b.noise_instruments[14].bytes[1] = 1;
    EXPECT_NE(a, b);
    b = a;
    b.ticks_per_row = 3;
    EXPECT_NE(a, b);
    b = a;
    b.orders.push_back({0, 0, 0, 0});
    EXPECT_NE(a, b);
}

TEST(
    Song,
    ValidSongPasses)
{
    EXPECT_NO_THROW(validate_song(one_pattern_song()));
}

TEST(
    Song,
    RejectsOrderCount)
{
    Song song = one_pattern_song();
    song.orders.clear();
    EXPECT_THROW(validate_song(song), SongError);
    song.orders.assign(kMaxOrders + 1, {0, 0, 0, 0});
    EXPECT_THROW(validate_song(song), SongError);
    song.orders.resize(kMaxOrders);
    EXPECT_NO_THROW(validate_song(song));
}

TEST(
    Song,
    RejectsMissingPattern)
{
    Song song = one_pattern_song();
    song.orders[0][3] = 1;
    EXPECT_THROW(validate_song(song), SongError);
}

TEST(
    Song,
    RejectsBadCells)
{
    Song song = one_pattern_song();
    song.patterns[0][5].note = 73;
    EXPECT_THROW(validate_song(song), SongError);
    song.patterns[0][5] = Cell {72, 16, 0, 0};
    EXPECT_THROW(validate_song(song), SongError);
    song.patterns[0][5] = Cell {72, 15, 0x10, 0};
    EXPECT_THROW(validate_song(song), SongError);
    song.patterns[0][5] = Cell {72, 15, 0xF, 0xFF};
    EXPECT_NO_THROW(validate_song(song));
}
