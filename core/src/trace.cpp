#include "golem/trace.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <istream>
#include <ostream>
#include <sstream>

namespace golem {

namespace {

    constexpr const char* kHeaderPrefix = "golem-trace 1 frames=";

    std::string hex2(std::uint8_t value)
    {
        char text[3];
        std::snprintf(text, sizeof text, "%02X", value);
        return text;
    }

    std::string describe(const std::optional<ApuWrite>& write)
    {
        if (!write) {
            return "nothing";
        }
        return register_name(write->address) + "=" + hex2(write->value);
    }

    std::optional<std::uint32_t> parse_decimal(const std::string& text)
    {
        if (text.empty() || text.size() > 9 || !std::all_of(text.begin(), text.end(), [](char c) {
                return c >= '0' && c <= '9';
            })) {
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(std::stoul(text));
    }

    std::optional<std::uint8_t> parse_value(const std::string& text)
    {
        if (text.size() != 2 || !std::all_of(text.begin(), text.end(), [](char c) {
                return std::isxdigit(static_cast<unsigned char>(c)) != 0;
            })) {
            return std::nullopt;
        }
        return static_cast<std::uint8_t>(std::stoul(text, nullptr, 16));
    }

    // Writes of each frame, for frames 0..frames-1.
    std::vector<std::vector<ApuWrite>> by_frame(const Trace& trace)
    {
        std::vector<std::vector<ApuWrite>> frames(trace.frames);
        for (const auto& entry : trace.entries) {
            if (entry.frame < trace.frames) {
                frames[entry.frame].push_back(entry.write);
            }
        }
        return frames;
    }

} // namespace

bool operator==(
    const TraceEntry& a,
    const TraceEntry& b)
{
    return a.frame == b.frame && a.write == b.write;
}

bool operator==(
    const Trace& a,
    const Trace& b)
{
    return a.frames == b.frames && a.entries == b.entries;
}

Trace make_trace(const std::vector<std::vector<ApuWrite>>& frames)
{
    Trace trace;
    trace.frames = static_cast<std::uint32_t>(frames.size());
    for (std::uint32_t frame = 0; frame < trace.frames; ++frame) {
        for (const auto& write : frames[frame]) {
            trace.entries.push_back({frame, write});
        }
    }
    return trace;
}

void write_trace(
    std::ostream& out,
    const Trace& trace)
{
    out << kHeaderPrefix << trace.frames << '\n';
    for (const auto& entry : trace.entries) {
        char frame[16];
        std::snprintf(frame, sizeof frame, "%04u", static_cast<unsigned>(entry.frame));
        out
            << frame
            << ' '
            << register_name(entry.write.address)
            << ' '
            << hex2(entry.write.value)
            << '\n';
    }
}

Trace read_trace(std::istream& in)
{
    Trace trace;
    std::string line;
    int number = 0;
    auto fail = [&](const std::string& message) {
        throw TraceError("line " + std::to_string(number) + ": " + message);
    };

    ++number;
    if (!std::getline(in, line) || line.rfind(kHeaderPrefix, 0) != 0) {
        fail(std::string("expected header '") + kHeaderPrefix + "<frames>'");
    }
    const auto frames = parse_decimal(line.substr(std::string(kHeaderPrefix).size()));
    if (!frames) {
        fail("invalid frame count");
    }
    trace.frames = *frames;

    while (std::getline(in, line)) {
        ++number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream fields(line);
        std::string frame_text, name, value_text, extra;
        if (!(fields >> frame_text >> name >> value_text) || (fields >> extra)) {
            fail("expected '<frame> <register> <value>'");
        }
        const auto frame = parse_decimal(frame_text);
        if (!frame) {
            fail("invalid frame '" + frame_text + "'");
        }
        if (*frame >= trace.frames) {
            fail("frame " + frame_text + " is past the frame count");
        }
        if (!trace.entries.empty() && *frame < trace.entries.back().frame) {
            fail("frame " + frame_text + " goes backwards");
        }
        const auto address = register_address(name);
        if (!address) {
            fail("unknown register '" + name + "'");
        }
        const auto value = parse_value(value_text);
        if (!value) {
            fail("invalid value '" + value_text + "'");
        }
        trace.entries.push_back({*frame, {*address, *value}});
    }
    return trace;
}

std::string Mismatch::to_string() const
{
    if (kind == Kind::FrameCount) {
        return "frame count: expected "
             + std::to_string(expected_frames)
             + ", got "
             + std::to_string(actual_frames);
    }
    return "frame "
         + std::to_string(frame)
         + ": write #"
         + std::to_string(index)
         + ": expected "
         + describe(expected)
         + ", got "
         + describe(actual);
}

std::vector<Mismatch> diff_traces(
    const Trace& expected,
    const Trace& actual)
{
    std::vector<Mismatch> mismatches;
    const auto expected_frames = by_frame(expected);
    const auto actual_frames = by_frame(actual);
    const auto frames = std::min(expected.frames, actual.frames);
    for (std::uint32_t frame = 0; frame < frames; ++frame) {
        const auto& want = expected_frames[frame];
        const auto& got = actual_frames[frame];
        for (std::size_t i = 0; i < std::max(want.size(), got.size()); ++i) {
            Mismatch mismatch;
            mismatch.kind = Mismatch::Kind::Write;
            mismatch.frame = frame;
            mismatch.index = i + 1;
            if (i < want.size()) {
                mismatch.expected = want[i];
            }
            if (i < got.size()) {
                mismatch.actual = got[i];
            }
            if (mismatch.expected != mismatch.actual) {
                mismatches.push_back(mismatch);
            }
        }
    }
    if (expected.frames != actual.frames) {
        Mismatch mismatch;
        mismatch.kind = Mismatch::Kind::FrameCount;
        mismatch.expected_frames = expected.frames;
        mismatch.actual_frames = actual.frames;
        mismatches.push_back(mismatch);
    }
    return mismatches;
}

} // namespace golem
