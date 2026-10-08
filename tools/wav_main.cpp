// golem-wav <rom.gb> <out.wav> [--frames N] [--rate HZ]
//
// Plays a driver test ROM in SameBoy and saves what it outputs as a 16-bit stereo WAV file:
// N emulator frames from power-on (default 600, about 10 seconds), at HZ samples per second
// (default 44100). For listening only: the tests compare APU writes, not sound. Needs
// SameBoy (built with GCC or Clang). Exit code: 0 on success, 2 on usage or ROM errors.

#include "golem/audio.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

constexpr const char* kUsage = "usage: golem-wav <rom.gb> <out.wav> [--frames N] [--rate HZ]\n";
constexpr long kDefaultFrames = 600;

} // namespace

int main(
    int argc,
    char** argv)
{
    std::string rom_path, wav_path;
    long frames = kDefaultFrames;
    long rate = golem::kDefaultSampleRate;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--frames" && i + 1 < argc) {
                frames = std::stol(argv[++i]);
            } else if (arg == "--rate" && i + 1 < argc) {
                rate = std::stol(argv[++i]);
            } else if (arg.rfind("--", 0) == 0) {
                std::cerr << kUsage;
                return 2;
            } else if (rom_path.empty()) {
                rom_path = arg;
            } else if (wav_path.empty()) {
                wav_path = arg;
            } else {
                std::cerr << kUsage;
                return 2;
            }
        }
    } catch (const std::exception&) {
        std::cerr << kUsage;
        return 2;
    }
    if (rom_path.empty() || wav_path.empty() || frames < 0 || rate <= 0) {
        std::cerr << kUsage;
        return 2;
    }

    try {
        std::ifstream in(rom_path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("cannot open " + rom_path);
        }
        const std::vector<std::uint8_t> rom(
            (std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        const auto samples = golem::render_audio(
            rom, static_cast<std::uint32_t>(frames), static_cast<unsigned>(rate));
        std::ofstream out(wav_path, std::ios::binary);
        golem::write_wav(out, samples, static_cast<unsigned>(rate));
        if (!out) {
            throw std::runtime_error("cannot write " + wav_path);
        }
    } catch (const std::exception& e) {
        std::cerr << rom_path << ": " << e.what() << '\n';
        return 2;
    }
    return 0;
}
