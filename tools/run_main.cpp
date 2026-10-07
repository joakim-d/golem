// golem-run <rom.gb> (--frames N | --expect golden.trace)
//
// Runs a driver test ROM headless and prints its APU trace for N frames, or compares it with
// a golden trace (over the golden trace's frame count). Exit code: 0 on success or a match,
// 1 if the traces differ, 2 on usage, ROM or emulator errors.

#include "golem/rom_runner.h"
#include "golem/trace.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

constexpr const char* kUsage = "usage: golem-run <rom.gb> (--frames N | --expect golden.trace)\n";

std::vector<std::uint8_t> read_rom(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot open " + path);
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

golem::Trace read_golden(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("cannot open " + path);
    }
    try {
        return golem::read_trace(file);
    } catch (const golem::TraceError& e) {
        throw std::runtime_error(path + ": " + e.what());
    }
}

} // namespace

int main(
    int argc,
    char** argv)
{
    std::string rom_path, expect_path;
    long frames = -1;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--frames" && i + 1 < argc) {
                frames = std::stol(argv[++i]);
            } else if (arg == "--expect" && i + 1 < argc) {
                expect_path = argv[++i];
            } else if (rom_path.empty() && arg.rfind("--", 0) != 0) {
                rom_path = arg;
            } else {
                std::cerr << kUsage;
                return 2;
            }
        }
    } catch (const std::exception&) {
        std::cerr << kUsage;
        return 2;
    }
    if (rom_path.empty() || (frames < 0) == expect_path.empty()) {
        std::cerr << kUsage;
        return 2;
    }

    try {
        const auto rom = read_rom(rom_path);
        if (expect_path.empty()) {
            golem::write_trace(std::cout, golem::run_rom(rom, static_cast<std::uint32_t>(frames)));
            return 0;
        }
        const auto golden = read_golden(expect_path);
        const auto mismatches = golem::diff_traces(golden, golem::run_rom(rom, golden.frames));
        if (mismatches.empty()) {
            return 0;
        }
        std::cout
            << mismatches.front().to_string()
            << '\n'
            << mismatches.size()
            << (mismatches.size() == 1 ? " mismatch\n" : " mismatches\n");
        return 1;
    } catch (const std::exception& e) {
        std::cerr << rom_path << ": " << e.what() << '\n';
        return 2;
    }
}
