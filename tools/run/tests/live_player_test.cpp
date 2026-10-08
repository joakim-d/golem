#include "golem/live_player.h"

#include "golem/rom_runner.h"
#include "golem/song_text.h"
#include "golem/trace.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace golem;
namespace fs = std::filesystem;

namespace {

std::string read_file(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

Song test_song(const std::string& name)
{
    return parse_song_text(read_file(fs::path(GOLEM_TEST_SONGS_DIR) / (name + ".gsong")));
}

// A song whose encoding does not fit in the player's song slot: 90 patterns of 192 bytes.
Song too_big_song()
{
    Song song = test_song("silence");
    song.patterns.resize(90);
    return song;
}

std::vector<StereoSample> render(
    LivePlayer& player,
    std::size_t count)
{
    std::vector<StereoSample> samples(count, StereoSample {1, 1});
    player.render(samples.data(), samples.size());
    return samples;
}

int peak(const std::vector<StereoSample>& samples)
{
    int peak = 0;
    for (const auto& sample : samples) {
        peak = std::max({peak, std::abs(int {sample.left}), std::abs(int {sample.right})});
    }
    return peak;
}

} // namespace

#ifdef GOLEM_HAS_PLAYER_ROM

TEST(
    LivePlayer,
    PlaybackIsAvailable)
{
    EXPECT_EQ(playback_unavailable_reason(), "");
}

TEST(
    LivePlayer,
    GoldenSongsInThePlayerRomReproduceTheirTraces)
{
    std::size_t songs = 0;
    for (const auto& entry : fs::directory_iterator(GOLEM_TEST_SONGS_DIR)) {
        if (entry.path().extension() != ".gsong") {
            continue;
        }
        ++songs;
        SCOPED_TRACE(entry.path().filename().string());
        auto trace_path = entry.path();
        trace_path.replace_extension(".trace");
        std::ifstream trace_file(trace_path);
        const Trace golden = read_trace(trace_file);
        const auto rom = player_rom(parse_song_text(read_file(entry.path())));
        for (const auto emulator : available_emulators()) {
            SCOPED_TRACE(emulator_name(emulator));
            const auto mismatches =
                diff_traces(golden, run_rom(rom, golden.frames, emulator).trace);
            EXPECT_TRUE(mismatches.empty()) << mismatches.front().to_string();
        }
    }
    EXPECT_GT(songs, 0u);
}

TEST(
    LivePlayer,
    ATooBigSongIsRefused)
{
    EXPECT_THROW(player_rom(too_big_song()), RunError);
    LivePlayer player;
    EXPECT_THROW(player.play(too_big_song()), RunError);
    EXPECT_FALSE(player.is_playing());
}

TEST(
    LivePlayer,
    SilentBeforePlay)
{
    LivePlayer player;
    EXPECT_FALSE(player.is_playing());
    EXPECT_EQ(peak(render(player, 4410)), 0);
}

TEST(
    LivePlayer,
    PlaysASong)
{
    LivePlayer player;
    player.play(test_song("scale"));
    EXPECT_TRUE(player.is_playing());
    EXPECT_GT(peak(render(player, 22050)), 1000); // The first half second: C-4 then D-4.
}

TEST(
    LivePlayer,
    SilentAfterStop)
{
    LivePlayer player;
    player.play(test_song("scale"));
    render(player, 4410);
    player.stop();
    EXPECT_FALSE(player.is_playing());
    EXPECT_EQ(peak(render(player, 4410)), 0);
}

TEST(
    LivePlayer,
    PlayStartsOverFromTheBeginning)
{
    LivePlayer player;
    player.play(test_song("scale"));
    const auto first = render(player, 8820);
    render(player, 8820);
    player.play(test_song("scale"));
    EXPECT_EQ(render(player, 8820), first);
}

TEST(
    LivePlayer,
    RenderInSmallPiecesGivesTheSameAudio)
{
    LivePlayer whole;
    whole.play(test_song("scale"));
    const auto expected = render(whole, 4000);
    LivePlayer pieces;
    pieces.play(test_song("scale"));
    std::vector<StereoSample> actual;
    for (int i = 0; i < 40; ++i) {
        const auto piece = render(pieces, 100);
        actual.insert(actual.end(), piece.begin(), piece.end());
    }
    EXPECT_EQ(actual, expected);
}

// --- Position ---

namespace {

// Two orders of empty rows, one frame per row: the frame number gives the position.
Song row_per_frame_song()
{
    return parse_song_text("ticks_per_row 01\norder 00 00 00 00\norder 00 00 00 00\npattern 00\n");
}

// Samples per emulator frame: 44100 Hz / (4194304 Hz / 70224 cycles).
constexpr double kSamplesPerFrame = 44100.0 * 70224 / 4194304;

// Rows from the start of the song: order 0 has rows 0-63, order 1 rows 64-127.
long absolute_row(const Player::Position& position)
{
    return static_cast<long>(position.order * kRowsPerPattern + position.row);
}

} // namespace

TEST(
    LivePlayer,
    NoPositionWhenNotPlaying)
{
    LivePlayer player;
    EXPECT_FALSE(player.position());
    player.play(row_per_frame_song());
    render(player, 4410);
    player.stop();
    EXPECT_FALSE(player.position());
}

TEST(
    LivePlayer,
    PositionFollowsTheRenderedSamples)
{
    LivePlayer player;
    player.play(row_per_frame_song());
    EXPECT_EQ(player.position(), Player::Position {}); // Nothing rendered yet.
    render(player, static_cast<std::size_t>(100 * kSamplesPerFrame));
    // Frame n plays row n - 1; frame 1 starts within the first frame of emulation.
    const auto position = player.position();
    ASSERT_TRUE(position);
    EXPECT_NEAR(absolute_row(*position), 99, 1);
    EXPECT_EQ(position->tick, 0u);
}

TEST(
    LivePlayer,
    PositionAllowsForTheLatency)
{
    LivePlayer player;
    player.play(row_per_frame_song());
    for (int i = 0; i < 100; ++i) { // In pieces, as an audio callback would.
        render(player, static_cast<std::size_t>(kSamplesPerFrame));
    }
    const auto now = player.position();
    const auto earlier = player.position(static_cast<std::size_t>(30 * kSamplesPerFrame));
    ASSERT_TRUE(now && earlier);
    EXPECT_NEAR(absolute_row(*now) - absolute_row(*earlier), 30, 1);
    EXPECT_EQ(player.position(1000000), Player::Position {}); // Before the first row.
}

TEST(
    LivePlayer,
    PositionStartsOverWithPlay)
{
    LivePlayer player;
    player.play(row_per_frame_song());
    render(player, static_cast<std::size_t>(50 * kSamplesPerFrame));
    player.play(row_per_frame_song());
    render(player, static_cast<std::size_t>(10 * kSamplesPerFrame));
    const auto position = player.position();
    ASSERT_TRUE(position);
    EXPECT_NEAR(absolute_row(*position), 9, 1);
}

#else

TEST(
    LivePlayer,
    PlaybackIsUnavailable)
{
    EXPECT_NE(playback_unavailable_reason(), "");
    EXPECT_THROW(player_rom(test_song("scale")), RunError);
    LivePlayer player;
    EXPECT_THROW(player.play(test_song("scale")), RunError);
    EXPECT_EQ(peak(render(player, 100)), 0);
    EXPECT_FALSE(player.position());
}

#endif
