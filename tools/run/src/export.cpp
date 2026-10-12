#include "golem/export.h"

#include "golem/live_player.h"
#include "golem/player.h"
#include "golem/rom_runner.h"
#include "golem/song_binary.h"

#include <fstream>

namespace golem {

namespace {

    std::ofstream open_for_writing(const std::string& path)
    {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw RunError("cannot write " + path);
        }
        return file;
    }

    void write_bytes(
        const std::string& path,
        const std::vector<std::uint8_t>& bytes)
    {
        std::ofstream file = open_for_writing(path);
        file.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        if (!file) {
            throw RunError("cannot write " + path);
        }
    }

} // namespace

std::vector<StereoSample> render_song(
    const Song& song,
    unsigned passes,
    std::uint32_t tail_frames,
    unsigned sample_rate)
{
    const auto rom = player_rom(song);
    // Frame 0 (the driver's init) runs within the first frame of emulation.
    const std::uint64_t frames = 1 + std::uint64_t {passes} * song_length(song) + tail_frames;
    return render_audio(rom, static_cast<std::uint32_t>(frames), sample_rate);
}

void export_rom(
    const Song& song,
    const std::string& path)
{
    write_bytes(path, player_rom(song));
}

void export_wav(
    const Song& song,
    const std::string& path,
    unsigned passes)
{
    const auto samples = render_song(song, passes); // Before the file: rendering may throw.
    std::ofstream file = open_for_writing(path);
    write_wav(file, samples, kDefaultSampleRate);
    if (!file) {
        throw RunError("cannot write " + path);
    }
}

void export_binary(
    const Song& song,
    const std::string& path)
{
    write_bytes(path, encode_song(song, kPlayerSongAddress));
}

} // namespace golem
