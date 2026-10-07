// golem-run <rom.gb> (--frames N | --expect golden.trace) [--cycles] [--max-cycles N]
//           [--emulator peanut|sameboy|both]
//
// Runs a driver test ROM headless and prints its APU trace for N frames, or compares it with
// a golden trace (over the golden trace's frame count). --cycles adds a report of the cycles
// spent in GolemInit and GolemPlay, as a `#` line, so the output stays a valid trace.
// --max-cycles fails if a GolemPlay call takes more than N cycles. --emulator picks the
// emulator (Peanut-GB by default); `both` runs Peanut-GB and SameBoy, checks each, and fails
// if their traces or their cycles per frame differ.
// Exit code: 0 on success or a match, 1 if the traces differ, a call is over the limit or
// the emulators disagree, 2 on usage, ROM or emulator errors.

#include "golem/rom_runner.h"
#include "golem/trace.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>

namespace {

constexpr const char* kUsage = "usage: golem-run <rom.gb> (--frames N | --expect golden.trace) "
                               "[--cycles] [--max-cycles N] [--emulator peanut|sameboy|both]\n";

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

struct PlayCycles {
    std::uint32_t max = 0;
    std::uint32_t max_frame = 0;
    std::uint64_t total = 0;
    std::uint32_t measured = 0;
};

// GolemPlay calls are frames 1 and later; frame 0 is GolemInit.
PlayCycles play_cycles(const golem::RunResult& result)
{
    PlayCycles cycles;
    for (std::size_t frame = 1; frame < result.frame_cycles.size(); ++frame) {
        if (const auto value = result.frame_cycles[frame]) {
            if (*value > cycles.max) {
                cycles.max = *value;
                cycles.max_frame = static_cast<std::uint32_t>(frame);
            }
            cycles.total += *value;
            ++cycles.measured;
        }
    }
    return cycles;
}

// `label` is empty for a single emulator, or the emulator's name when comparing two.
void print_cycles(
    const golem::RunResult& result,
    const std::string& label)
{
    const auto play = play_cycles(result);
    std::cout << "# cycles" << (label.empty() ? "" : " (" + label + ")") << ": GolemInit ";
    if (!result.frame_cycles.empty() && result.frame_cycles[0]) {
        std::cout << *result.frame_cycles[0];
    } else {
        std::cout << "not measured";
    }
    std::cout << ", GolemPlay ";
    if (play.measured == 0) {
        std::cout << "not measured\n";
        return;
    }
    char average[32];
    std::snprintf(average, sizeof average, "%.1f", double(play.total) / play.measured);
    std::cout
        << "max "
        << play.max
        << " (frame "
        << play.max_frame
        << "), average "
        << average
        << " over "
        << play.measured
        << " frames\n";
}

} // namespace

int main(
    int argc,
    char** argv)
{
    std::string rom_path, expect_path;
    long frames = -1;
    long max_cycles = -1;
    bool report_cycles = false;
    std::vector<golem::Emulator> emulators {golem::Emulator::PeanutGb};
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--frames" && i + 1 < argc) {
                frames = std::stol(argv[++i]);
            } else if (arg == "--expect" && i + 1 < argc) {
                expect_path = argv[++i];
            } else if (arg == "--cycles") {
                report_cycles = true;
            } else if (arg == "--max-cycles" && i + 1 < argc) {
                max_cycles = std::stol(argv[++i]);
            } else if (arg == "--emulator" && i + 1 < argc) {
                const std::string name = argv[++i];
                if (name == "peanut") {
                    emulators = {golem::Emulator::PeanutGb};
                } else if (name == "sameboy") {
                    emulators = {golem::Emulator::SameBoy};
                } else if (name == "both") {
                    emulators = {golem::Emulator::PeanutGb, golem::Emulator::SameBoy};
                } else {
                    std::cerr << kUsage;
                    return 2;
                }
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
        const bool compare = emulators.size() > 1;
        std::optional<golem::Trace> golden;
        if (!expect_path.empty()) {
            golden = read_golden(expect_path);
        }
        const auto frame_count = golden ? golden->frames : static_cast<std::uint32_t>(frames);

        int status = 0;
        std::vector<golem::RunResult> results;
        for (const auto emulator : emulators) {
            // With two emulators, every message says which one it is about.
            const std::string label = compare ? golem::emulator_name(emulator) : "";
            const std::string prefix = label.empty() ? "" : label + ": ";
            results.push_back(golem::run_rom(rom, frame_count, emulator));
            const auto& result = results.back();
            if (golden) {
                const auto mismatches = golem::diff_traces(*golden, result.trace);
                if (!mismatches.empty()) {
                    std::cout
                        << prefix
                        << mismatches.front().to_string()
                        << '\n'
                        << prefix
                        << mismatches.size()
                        << (mismatches.size() == 1 ? " mismatch\n" : " mismatches\n");
                    status = 1;
                }
            }
            if (max_cycles >= 0) {
                const auto play = play_cycles(result);
                if (play.measured == 0) {
                    std::cout
                        << prefix
                        << "GolemPlay cycles not measured: the ROM writes no end marker\n";
                    status = 1;
                } else if (play.max > max_cycles) {
                    std::cout
                        << prefix
                        << "frame "
                        << play.max_frame
                        << ": GolemPlay took "
                        << play.max
                        << " cycles, limit "
                        << max_cycles
                        << '\n';
                    status = 1;
                }
            }
        }

        if (!golden) {
            golem::write_trace(std::cout, results.front().trace);
        }
        if (report_cycles) { // After the trace: `#` lines keep it a valid trace.
            for (std::size_t i = 0; i < results.size(); ++i) {
                print_cycles(results[i], compare ? golem::emulator_name(emulators[i]) : "");
            }
        }
        if (compare) {
            const auto& first = results[0];
            const auto& second = results[1];
            const auto names =
                golem::emulator_name(emulators[0]) + " vs " + golem::emulator_name(emulators[1]);
            const auto mismatches = golem::diff_traces(first.trace, second.trace);
            if (!mismatches.empty()) {
                std::cout << names << ": " << mismatches.front().to_string() << '\n';
                status = 1;
            }
            for (std::size_t frame = 0; frame < first.frame_cycles.size(); ++frame) {
                if (first.frame_cycles[frame] != second.frame_cycles[frame]) {
                    const auto cycles = [](const std::optional<std::uint32_t>& value) {
                        return value ? std::to_string(*value) : std::string("not measured");
                    };
                    std::cout
                        << names
                        << ": frame "
                        << frame
                        << " cycles differ: "
                        << cycles(first.frame_cycles[frame])
                        << " vs "
                        << cycles(second.frame_cycles[frame])
                        << '\n';
                    status = 1;
                    break;
                }
            }
        }
        return status;
    } catch (const std::exception& e) {
        std::cerr << rom_path << ": " << e.what() << '\n';
        return 2;
    }
}
