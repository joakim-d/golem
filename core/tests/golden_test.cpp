// Renders every tests/songs/*.gsong with the reference player and compares it with the
// .trace next to it. With GOLEM_UPDATE_GOLDEN=1 the traces are (re)written instead; new
// traces cover kDefaultFrames, existing ones keep their frame count.

#include "golem/player.h"
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

constexpr std::uint32_t kDefaultFrames = 600;
constexpr std::size_t kMaxReported = 10;

std::string read_file(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

Trace render_trace(
    const Song& song,
    std::uint32_t frames)
{
    return make_trace(render(song, frames));
}

} // namespace

TEST(
    Golden,
    SongsMatchTheirTraces)
{
    const bool update = std::getenv("GOLEM_UPDATE_GOLDEN") != nullptr;
    std::size_t songs = 0;
    for (const auto& entry : fs::directory_iterator(GOLEM_TEST_SONGS_DIR)) {
        if (entry.path().extension() != ".gsong") {
            continue;
        }
        ++songs;
        SCOPED_TRACE(entry.path().filename().string());
        const Song song = parse_song_text(read_file(entry.path()));
        auto trace_path = entry.path();
        trace_path.replace_extension(".trace");

        std::optional<Trace> golden;
        if (fs::exists(trace_path)) {
            std::ifstream file(trace_path);
            golden = read_trace(file);
        }

        if (update) {
            const auto frames = golden ? golden->frames : kDefaultFrames;
            std::ofstream file(trace_path, std::ios::binary);
            write_trace(file, render_trace(song, frames));
            continue;
        }

        if (!golden) {
            ADD_FAILURE()
                << "missing "
                << trace_path.filename().string()
                << "; generate it with GOLEM_UPDATE_GOLDEN=1";
            continue;
        }
        const auto mismatches = diff_traces(*golden, render_trace(song, golden->frames));
        for (std::size_t i = 0; i < std::min(mismatches.size(), kMaxReported); ++i) {
            ADD_FAILURE() << mismatches[i].to_string();
        }
        EXPECT_TRUE(mismatches.empty()) << mismatches.size() << " mismatch(es)";
    }
    EXPECT_GT(songs, 0u) << "no .gsong in " << GOLEM_TEST_SONGS_DIR;
}
