// golem-tracediff <expected.trace> <actual.trace>
//
// Compares two APU traces frame by frame. Prints the first mismatch and the number of
// mismatches. Exit code: 0 if equal, 1 if they differ, 2 on usage or read errors.

#include "golem/trace.h"

#include <fstream>
#include <iostream>
#include <string>

namespace {

golem::Trace load_trace(const std::string& path)
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
    if (argc != 3) {
        std::cerr << "usage: golem-tracediff <expected.trace> <actual.trace>\n";
        return 2;
    }
    try {
        const auto mismatches = golem::diff_traces(load_trace(argv[1]), load_trace(argv[2]));
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
        std::cerr << e.what() << '\n';
        return 2;
    }
}
