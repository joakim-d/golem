#include "golem/trace.h"

#include <gtest/gtest.h>

#include <sstream>

using namespace golem;

namespace {

Trace sample_trace()
{
    return make_trace({
        {{reg::NR52, 0x80}, {reg::NR50, 0x77}},
        {},
        {{reg::NR22, 0xF3}, {0xFF3F, 0x0A}},
    });
}

std::string written(const Trace& trace)
{
    std::ostringstream out;
    write_trace(out, trace);
    return out.str();
}

Trace read(const std::string& text)
{
    std::istringstream in(text);
    return read_trace(in);
}

void expect_error_at_line(
    const std::string& text,
    int line)
{
    try {
        read(text);
        ADD_FAILURE() << "no error for:\n" << text;
    } catch (const TraceError& e) {
        const std::string prefix = "line " + std::to_string(line) + ":";
        EXPECT_EQ(std::string(e.what()).rfind(prefix, 0), 0u) << e.what();
    }
}

} // namespace

TEST(
    Trace,
    MakeTrace)
{
    const Trace trace = sample_trace();
    EXPECT_EQ(trace.frames, 3u);
    ASSERT_EQ(trace.entries.size(), 4u);
    EXPECT_EQ(trace.entries[0], (TraceEntry {0, {reg::NR52, 0x80}}));
    EXPECT_EQ(trace.entries[1], (TraceEntry {0, {reg::NR50, 0x77}}));
    EXPECT_EQ(trace.entries[2], (TraceEntry {2, {reg::NR22, 0xF3}}));
    EXPECT_EQ(trace.entries[3], (TraceEntry {2, {0xFF3F, 0x0A}}));
}

TEST(
    Trace,
    Writes)
{
    EXPECT_EQ(
        written(sample_trace()),
        "golem-trace 1 frames=3\n"
        "0000 NR52 80\n"
        "0000 NR50 77\n"
        "0002 NR22 F3\n"
        "0002 WAVEF 0A\n");
    EXPECT_EQ(written(Trace {}), "golem-trace 1 frames=0\n");
    EXPECT_EQ(
        written(Trace {12346, {{12345, {reg::NR10, 0x00}}}}),
        "golem-trace 1 frames=12346\n12345 NR10 00\n");
}

TEST(
    Trace,
    ReadsWhatItWrites)
{
    EXPECT_EQ(read(written(sample_trace())), sample_trace());
    EXPECT_EQ(read(written(Trace {})), Trace {});
}

TEST(
    Trace,
    ReadSkipsCommentsAndBlankLines)
{
    EXPECT_EQ(
        read(
            "golem-trace 1 frames=3\n# init\n\n0000 NR52 80\n0000 NR50 77\n"
            "0002 NR22 f3\n2 FF3F 0A\n"),
        sample_trace());
}

TEST(
    Trace,
    ReadReportsErrorLines)
{
    expect_error_at_line("", 1); // Missing header.
    expect_error_at_line("golem-trace 2 frames=1\n", 1);
    expect_error_at_line("golem-trace 1 frames=x\n", 1);
    expect_error_at_line("golem-trace 1 frames=2\n0000 NR15 00\n", 2);
    expect_error_at_line("golem-trace 1 frames=2\n0000 NR52 800\n", 2);
    expect_error_at_line("golem-trace 1 frames=2\n0000 NR52\n", 2);
    expect_error_at_line("golem-trace 1 frames=2\nabcd NR52 80\n", 2);
    expect_error_at_line("golem-trace 1 frames=2\n0002 NR52 80\n", 2); // Past the end.
    expect_error_at_line("golem-trace 1 frames=2\n0001 NR52 80\n0000 NR52 80\n", 3);
}

TEST(
    Trace,
    EqualTracesHaveNoMismatch)
{
    EXPECT_TRUE(diff_traces(sample_trace(), sample_trace()).empty());
}

TEST(
    Trace,
    ReportsDifferentWrite)
{
    Trace actual = sample_trace();
    actual.entries[2].write.value = 0xF1;
    const auto mismatches = diff_traces(sample_trace(), actual);
    ASSERT_EQ(mismatches.size(), 1u);
    EXPECT_EQ(mismatches[0].kind, Mismatch::Kind::Write);
    EXPECT_EQ(mismatches[0].frame, 2u);
    EXPECT_EQ(mismatches[0].index, 1u);
    EXPECT_EQ(mismatches[0].to_string(), "frame 2: write #1: expected NR22=F3, got NR22=F1");
}

TEST(
    Trace,
    ReportsMissingAndExtraWrites)
{
    Trace actual = sample_trace();
    actual.entries.pop_back();
    auto mismatches = diff_traces(sample_trace(), actual);
    ASSERT_EQ(mismatches.size(), 1u);
    EXPECT_EQ(mismatches[0].to_string(), "frame 2: write #2: expected WAVEF=0A, got nothing");

    mismatches = diff_traces(actual, sample_trace());
    ASSERT_EQ(mismatches.size(), 1u);
    EXPECT_EQ(mismatches[0].to_string(), "frame 2: write #2: expected nothing, got WAVEF=0A");
}

TEST(
    Trace,
    ReportsEveryMismatchInFrameOrder)
{
    const Trace actual = make_trace({
        {{reg::NR52, 0x80}, {reg::NR50, 0x00}},
        {{reg::NR51, 0xFF}},
        {{reg::NR22, 0xF3}, {0xFF3F, 0x0A}},
    });
    const auto mismatches = diff_traces(sample_trace(), actual);
    ASSERT_EQ(mismatches.size(), 2u);
    EXPECT_EQ(mismatches[0].to_string(), "frame 0: write #2: expected NR50=77, got NR50=00");
    EXPECT_EQ(mismatches[1].to_string(), "frame 1: write #1: expected nothing, got NR51=FF");
}

TEST(
    Trace,
    ReportsFrameCount)
{
    Trace actual = sample_trace();
    actual.frames = 5;
    auto mismatches = diff_traces(sample_trace(), actual);
    ASSERT_EQ(mismatches.size(), 1u);
    EXPECT_EQ(mismatches[0].kind, Mismatch::Kind::FrameCount);
    EXPECT_EQ(mismatches[0].to_string(), "frame count: expected 3, got 5");

    // Writes past the shorter trace are not compared, only the count is reported.
    actual = make_trace({{{reg::NR52, 0x80}, {reg::NR50, 0x77}}});
    mismatches = diff_traces(sample_trace(), actual);
    ASSERT_EQ(mismatches.size(), 1u);
    EXPECT_EQ(mismatches[0].to_string(), "frame count: expected 3, got 1");
}
