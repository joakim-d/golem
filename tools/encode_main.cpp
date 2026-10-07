// golem-encode <song.gsong> <out.bin> [--base ADDRESS]
//
// Converts a text song to the binary song format, encoded for the address it will be loaded
// at (default $4000).

#include "golem/song_binary.h"
#include "golem/song_text.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

constexpr const char* kUsage = "usage: golem-encode <song.gsong> <out.bin> [--base ADDRESS]\n";

} // namespace

int main(
    int argc,
    char** argv)
{
    std::string in_path, out_path;
    unsigned long base = golem::kDefaultSongBase;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--base" && i + 1 < argc) {
                base = std::stoul(argv[++i], nullptr, 0);
            } else if (arg.rfind("--", 0) == 0) {
                std::cerr << kUsage;
                return 2;
            } else if (in_path.empty()) {
                in_path = arg;
            } else if (out_path.empty()) {
                out_path = arg;
            } else {
                std::cerr << kUsage;
                return 2;
            }
        }
    } catch (const std::exception&) {
        std::cerr << kUsage;
        return 2;
    }
    if (in_path.empty() || out_path.empty() || base > 0xFFFF) {
        std::cerr << kUsage;
        return 2;
    }

    try {
        std::ifstream in(in_path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("cannot open " + in_path);
        }
        std::ostringstream text;
        text << in.rdbuf();
        const auto bytes = golem::encode_song(
            golem::parse_song_text(text.str()), static_cast<std::uint16_t>(base));
        std::ofstream out(out_path, std::ios::binary);
        out.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
        if (!out) {
            throw std::runtime_error("cannot write " + out_path);
        }
    } catch (const std::exception& e) {
        std::cerr << in_path << ": " << e.what() << '\n';
        return 1;
    }
    return 0;
}
