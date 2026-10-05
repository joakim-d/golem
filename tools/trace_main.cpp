// golem-trace <song.gsong|song.bin> --frames N [--base ADDRESS]
//
// Prints the reference player's APU trace for the first N frames of a song. `.gsong` files
// are read as text, anything else as a binary song encoded for ADDRESS (default $4000).

#include "golem/player.h"
#include "golem/song_binary.h"
#include "golem/song_text.h"
#include "golem/trace.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

namespace {

constexpr const char* kUsage =
    "usage: golem-trace <song.gsong|song.bin> --frames N [--base ADDRESS]\n";

bool ends_with(
    const std::string& text,
    const std::string& suffix)
{
    return text.size() >= suffix.size()
        && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

golem::Song load_song(
    const std::string& path,
    std::uint16_t base)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot open " + path);
    }
    if (ends_with(path, ".gsong")) {
        std::ostringstream text;
        text << file.rdbuf();
        return golem::parse_song_text(text.str());
    }
    const std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return golem::decode_song(bytes, base);
}

} // namespace

int main(
    int argc,
    char** argv)
{
    std::string path;
    long frames = -1;
    unsigned long base = golem::kDefaultSongBase;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--frames" && i + 1 < argc) {
                frames = std::stol(argv[++i]);
            } else if (arg == "--base" && i + 1 < argc) {
                base = std::stoul(argv[++i], nullptr, 0);
            } else if (path.empty() && arg.rfind("--", 0) != 0) {
                path = arg;
            } else {
                std::cerr << kUsage;
                return 2;
            }
        }
    } catch (const std::exception&) {
        std::cerr << kUsage;
        return 2;
    }
    if (path.empty() || frames < 0 || base > 0xFFFF) {
        std::cerr << kUsage;
        return 2;
    }

    try {
        const auto song = load_song(path, static_cast<std::uint16_t>(base));
        golem::write_trace(
            std::cout, golem::make_trace(golem::render(song, static_cast<std::size_t>(frames))));
    } catch (const std::exception& e) {
        std::cerr << path << ": " << e.what() << '\n';
        return 1;
    }
    return 0;
}
