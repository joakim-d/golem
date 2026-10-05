#include "golem/song_text.h"

#include <gtest/gtest.h>

#include <string>

using namespace golem;

namespace {

constexpr const char* kExample = R"(# Example song
ticks_per_row 04

pulse 1 00 80 F3
pulse F 7B 41 A5
wave_instrument 1 00 20
noise 2 80 F1
wave 0 0123456789ABCDEFFEDCBA9876543210

order 00 01 00 01
order 01 01 01 01

pattern 00
00 C-4 1 ...   # first note
10 --- . F03
3F B-7 F C0F

pattern 01
05 C#2 . ...
)";

// Expects parse_song_text(text) to throw a SongError whose message starts with "line <n>:".
void expect_error_at_line(
    const std::string& text,
    int line)
{
    try {
        parse_song_text(text);
        ADD_FAILURE() << "no error for:\n" << text;
    } catch (const SongError& e) {
        const std::string prefix = "line " + std::to_string(line) + ":";
        EXPECT_EQ(std::string(e.what()).rfind(prefix, 0), 0u) << e.what();
    }
}

} // namespace

TEST(
    SongText,
    NoteNames)
{
    EXPECT_EQ(note_name(0), "---");
    EXPECT_EQ(note_name(1), "C-2");
    EXPECT_EQ(note_name(2), "C#2");
    EXPECT_EQ(note_name(34), "A-4");
    EXPECT_EQ(note_name(72), "B-7");
}

TEST(
    SongText,
    ParsesNoteNames)
{
    for (int note = 0; note <= 72; ++note) {
        EXPECT_EQ(parse_note_name(note_name(static_cast<std::uint8_t>(note))), note);
    }
    EXPECT_EQ(parse_note_name("C-1"), std::nullopt);
    EXPECT_EQ(parse_note_name("C-8"), std::nullopt);
    EXPECT_EQ(parse_note_name("E#4"), std::nullopt);
    EXPECT_EQ(parse_note_name("H-4"), std::nullopt);
    EXPECT_EQ(parse_note_name("c-4"), std::nullopt);
    EXPECT_EQ(parse_note_name("C-44"), std::nullopt);
}

TEST(
    SongText,
    ParsesExample)
{
    const Song song = parse_song_text(kExample);
    EXPECT_EQ(song.ticks_per_row, 4);

    EXPECT_EQ(song.pulse_instruments[0].bytes, (std::array<std::uint8_t, 3> {0x00, 0x80, 0xF3}));
    EXPECT_EQ(song.pulse_instruments[14].bytes, (std::array<std::uint8_t, 3> {0x7B, 0x41, 0xA5}));
    EXPECT_EQ(song.pulse_instruments[1].bytes, (std::array<std::uint8_t, 3> {}));
    EXPECT_EQ(song.wave_instruments[0].bytes, (std::array<std::uint8_t, 2> {0x00, 0x20}));
    EXPECT_EQ(song.noise_instruments[1].bytes, (std::array<std::uint8_t, 2> {0x80, 0xF1}));
    EXPECT_EQ(song.waves[0][0], 0x01);
    EXPECT_EQ(song.waves[0][15], 0x10);
    EXPECT_EQ(song.waves[1], Wave {});

    ASSERT_EQ(song.orders.size(), 2u);
    EXPECT_EQ(song.orders[0], (Order {0, 1, 0, 1}));
    EXPECT_EQ(song.orders[1], (Order {1, 1, 1, 1}));

    ASSERT_EQ(song.patterns.size(), 2u);
    EXPECT_EQ(song.patterns[0][0], (Cell {25, 1, 0, 0}));
    EXPECT_EQ(song.patterns[0][0x10], (Cell {0, 0, 0xF, 0x03}));
    EXPECT_EQ(song.patterns[0][0x3F], (Cell {72, 15, 0xC, 0x0F}));
    EXPECT_TRUE(song.patterns[0][1].empty());
    EXPECT_EQ(song.patterns[1][5], (Cell {2, 0, 0, 0}));
}

TEST(
    SongText,
    DefaultsTicksPerRow)
{
    const Song song = parse_song_text("order 00 00 00 00\npattern 00\n");
    EXPECT_EQ(song.ticks_per_row, 6);
}

TEST(
    SongText,
    PatternsNotDeclaredAreEmpty)
{
    const Song song = parse_song_text("order 02 02 02 02\npattern 02\n00 C-4 1 ...\n");
    ASSERT_EQ(song.patterns.size(), 3u);
    EXPECT_EQ(song.patterns[0], Pattern {});
    EXPECT_EQ(song.patterns[2][0].note, 25);
}

TEST(
    SongText,
    ReportsErrorLines)
{
    expect_error_at_line("order 00 00 00 00\nbogus 1\n", 2);
    expect_error_at_line("00 C-4 1 ...\n", 1); // row outside pattern
    expect_error_at_line("pattern 00\n40 C-4 1 ...\n", 2); // row out of range
    expect_error_at_line("pattern 00\n00 C-4 1 ...\n00 C-4 1 ...\n", 3); // duplicate row
    expect_error_at_line("pattern 00\npattern 00\n", 2); // duplicate pattern
    expect_error_at_line("pattern 00\n00 C-4 1\n", 2); // missing effect
    expect_error_at_line("pattern 00\n00 H-4 1 ...\n", 2); // bad note
    expect_error_at_line("pattern 00\n00 C-4 0 ...\n", 2); // instrument 0
    expect_error_at_line("pattern 00\n00 C-4 1 G00\n", 2); // bad effect
    expect_error_at_line("pattern 00\n00 C-4 1 C0\n", 2); // short effect
    expect_error_at_line("ticks_per_row 100\n", 1); // > FF
    expect_error_at_line("ticks_per_row\n", 1); // missing value
    expect_error_at_line("pulse 0 00 00 00\n", 1); // instrument 0
    expect_error_at_line("pulse 1 00 00\n", 1); // missing byte
    expect_error_at_line("noise 1 00 0G\n", 1); // bad hex
    expect_error_at_line("wave 0 0123\n", 1); // short wave
    expect_error_at_line("wave 10 0123456789ABCDEFFEDCBA9876543210\n", 1); // wave index
    expect_error_at_line("order 00 00 00\n", 1); // 3 channels
}

TEST(
    SongText,
    ValidatesResult)
{
    EXPECT_THROW(parse_song_text("pattern 00\n"), SongError); // no orders
    EXPECT_THROW(parse_song_text("order 00 00 00 05\npattern 00\n"), SongError); // no pattern 05
}

TEST(
    SongText,
    FormatsCanonically)
{
    const std::string canonical = R"(ticks_per_row 04

pulse 1 00 80 F3
pulse F 7B 41 A5
wave_instrument 1 00 20
noise 2 80 F1
wave 0 0123456789ABCDEFFEDCBA9876543210

order 00 01 00 01
order 01 01 01 01

pattern 00
00 C-4 1 ...
10 --- . F03
3F B-7 F C0F

pattern 01
05 C#2 . ...
)";
    EXPECT_EQ(format_song_text(parse_song_text(kExample)), canonical);
    EXPECT_EQ(format_song_text(parse_song_text(canonical)), canonical);
}

TEST(
    SongText,
    FormatsEmptyPatterns)
{
    const Song song = parse_song_text("order 01 01 01 01\npattern 01\n");
    EXPECT_EQ(
        format_song_text(song),
        "ticks_per_row 06\n\norder 01 01 01 01\n\npattern 00\n\n"
        "pattern 01\n");
}

TEST(
    SongText,
    RoundTripsThroughText)
{
    Song song;
    song.ticks_per_row = 0;
    song.orders = {{0, 1, 2, 3}, {3, 2, 1, 0}};
    song.patterns.resize(4);
    for (std::size_t p = 0; p < song.patterns.size(); ++p) {
        for (std::size_t row = 0; row < kRowsPerPattern; row += p + 1) {
            song.patterns[p][row] = Cell {
                static_cast<std::uint8_t>((row + p) % 73),
                static_cast<std::uint8_t>(row % 16),
                static_cast<std::uint8_t>(p * 3 % 16),
                static_cast<std::uint8_t>(row * 7)};
        }
    }
    for (std::size_t i = 0; i < kInstruments; ++i) {
        const auto b = static_cast<std::uint8_t>(i * 17);
        song.pulse_instruments[i].bytes = {b, static_cast<std::uint8_t>(b + 1), 0xFF};
        song.wave_instruments[i].bytes = {b, 0x01};
        song.noise_instruments[i].bytes = {0x00, b};
    }
    song.waves[7].fill(0xA5);
    EXPECT_EQ(parse_song_text(format_song_text(song)), song);
}
