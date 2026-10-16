#include "golem/export.h"

#include "golem/live_player.h"
#include "golem/player.h"
#include "golem/rom_runner.h"
#include "golem/song_binary.h"
#include "golem/song_text.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>

using namespace golem;
namespace fs = std::filesystem;

namespace {

// A temporary file path, removed at the end of the test.
class TempFile {
public:
    explicit TempFile(const std::string& name)
        : path_(fs::temp_directory_path() / ("golem-export-test-" + name))
    {
    }

    ~TempFile()
    {
        std::error_code ignored;
        fs::remove(path_, ignored);
    }

    std::string str() const
    {
        return path_.string();
    }

private:
    fs::path path_;
};

std::vector<std::uint8_t> read_bytes(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

// A note on the first row, then 63 empty rows of 2 ticks: 128 frames a pass.
Song short_song()
{
    return parse_song_text(
        "ticks_per_row 02\npulse 1 00 80 F0\norder 01 00 00 00\npattern 00\npattern 01\n"
        "00 C-4 1 ...\n");
}

} // namespace

TEST(
    Export,
    BinarySongIsTheEncodedSong)
{
    TempFile file("song.bin");
    export_binary(short_song(), file.str());
    EXPECT_EQ(read_bytes(file.str()), encode_song(short_song(), kPlayerSongAddress));
}

TEST(
    Export,
    AnUnwritablePathThrows)
{
    EXPECT_THROW(export_binary(short_song(), "/nonexistent/golem/song.bin"), RunError);
}

#ifdef GOLEM_HAS_PLAYER_ROM

TEST(
    Export,
    RenderSongPlaysPassesThenTheTail)
{
    ASSERT_EQ(song_length(short_song()), 128u);
    // Frame 0 is the driver's init, within the first frame of emulation.
    const auto expected = render_audio(player_rom(short_song()), 1 + 2 * 128 + 30);
    EXPECT_EQ(render_song(short_song(), 2, 30), expected);
}

TEST(
    Export,
    RomIsThePlayerRom)
{
    TempFile file("song.gb");
    export_rom(short_song(), file.str());
    EXPECT_EQ(read_bytes(file.str()), player_rom(short_song()));
}

TEST(
    Export,
    WavHoldsThePassesAndTheTail)
{
    TempFile file("song.wav");
    export_wav(short_song(), file.str(), 1);
    const auto bytes = read_bytes(file.str());
    ASSERT_GT(bytes.size(), 44u);
    EXPECT_EQ(std::string(bytes.begin(), bytes.begin() + 4), "RIFF");
    EXPECT_EQ(std::string(bytes.begin() + 8, bytes.begin() + 12), "WAVE");
    const auto samples = render_song(short_song(), 1);
    EXPECT_EQ(bytes.size(), 44 + samples.size() * 4); // 16-bit stereo after a 44-byte header.
}

TEST(
    Export,
    AnUnwritableWavThrows)
{
    EXPECT_THROW(export_wav(short_song(), "/nonexistent/golem/song.wav", 1), RunError);
}

#else

TEST(
    Export,
    RomAndWavNeedPlayback)
{
    TempFile file("song.gb");
    EXPECT_THROW(export_rom(short_song(), file.str()), RunError);
    EXPECT_THROW(export_wav(short_song(), file.str(), 1), RunError);
}

#endif
