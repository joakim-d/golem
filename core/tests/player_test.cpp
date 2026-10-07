#include "golem/player.h"

#include "golem/notes.h"
#include "golem/song_text.h"

#include <gtest/gtest.h>

#include <string>

using namespace golem;

namespace {

using Writes = std::vector<ApuWrite>;

constexpr std::uint8_t kA4 = 34; // Period 1750 = $6D6.
constexpr std::uint8_t kC4 = 25;

// Instruments shared by most tests.
constexpr const char* kInstruments = R"(
pulse 1 FB 81 F3        # length enable, sweep 7B, duty 2, length 1, envelope F3
pulse 2 00 C0 A2        # no length, duty 3
wave_instrument 1 3F A1 # length timer 3F, length enable, volume 01 (100%), wave 1
wave_instrument 2 00 42 # volume 10 (50%), wave 2
noise 1 C5 B1           # 7-bit LFSR, length enable, length timer 05, envelope B1
noise 2 00 71
wave 1 00112233445566778899AABBCCDDEEFF
wave 2 FFEEDDCCBBAA99887766554433221100
)";

Song make_song(const std::string& text)
{
    return parse_song_text(kInstruments + text);
}

std::vector<Writes> frames_of(
    const std::string& text,
    std::size_t frames)
{
    return render(make_song(text), frames);
}

// NR30 off, the 16 bytes of `wave`, as written before a channel 3 trigger.
Writes wave_load(const Wave& wave)
{
    Writes writes {{reg::NR30, 0x00}};
    for (std::size_t i = 0; i < kWaveBytes; ++i) {
        writes.push_back({static_cast<std::uint16_t>(reg::WAVE_RAM + i), wave[i]});
    }
    return writes;
}

Writes concat(std::initializer_list<Writes> parts)
{
    Writes all;
    for (const auto& part : parts) {
        all.insert(all.end(), part.begin(), part.end());
    }
    return all;
}

const Wave kWave1 = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
const Wave kWave2 = {
    0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

// Song with one order: pattern 01 on `channel` (1..4), the empty pattern 00 elsewhere.
std::string on_channel(
    int channel,
    const std::string& rows,
    const std::string& ticks_per_row = "01")
{
    std::string order = "order";
    for (int c = 1; c <= 4; ++c) {
        order += c == channel ? " 01" : " 00";
    }
    return "ticks_per_row " + ticks_per_row + "\n" + order + "\npattern 00\npattern 01\n" + rows;
}

// Frame at which `row` (0-based, in a one-order song) starts with `ticks` per row.
std::size_t row_frame(
    std::size_t row,
    std::size_t ticks = 1)
{
    return 1 + row * ticks;
}

} // namespace

// --- Frames and timing ---

TEST(
    Player,
    FrameZeroIsInit)
{
    const auto frames = frames_of(on_channel(1, ""), 1);
    EXPECT_EQ(frames[0], (Writes {{reg::NR52, 0x80}, {reg::NR50, 0x77}, {reg::NR51, 0xFF}}));
}

TEST(
    Player,
    EmptySongWritesNothingAfterInit)
{
    const auto frames = frames_of(on_channel(1, ""), 200);
    for (std::size_t frame = 1; frame < frames.size(); ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    RenderMatchesStepping)
{
    const Song song = make_song(on_channel(1, "00 A-4 1 ...\n02 C-4 2 ...\n"));
    const auto frames = render(song, 10);
    ASSERT_EQ(frames.size(), 10u);
    Player player(song);
    for (const auto& frame : frames) {
        EXPECT_EQ(player.step(), frame);
    }
}

TEST(
    Player,
    RejectsInvalidSong)
{
    Song song = make_song(on_channel(1, ""));
    song.orders.clear();
    EXPECT_THROW(Player {song}, SongError);
}

TEST(
    Player,
    RowsLastTicksPerRowFrames)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 1 ...\n01 A-4 1 ...\n", "03"), 8);
    EXPECT_FALSE(frames[row_frame(0, 3)].empty());
    EXPECT_TRUE(frames[2].empty());
    EXPECT_TRUE(frames[3].empty());
    EXPECT_FALSE(frames[row_frame(1, 3)].empty());
    EXPECT_TRUE(frames[5].empty());
}

TEST(
    Player,
    TicksPerRowZeroMeans256)
{
    const auto frames = frames_of(on_channel(2, "01 A-4 1 ...\n", "00"), 258);
    for (std::size_t frame = 1; frame < 257; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
    EXPECT_FALSE(frames[257].empty());
}

// --- Triggers ---

TEST(
    Player,
    PulseChannel1Trigger)
{
    const auto frames = frames_of(on_channel(1, "00 A-4 1 ...\n"), 2);
    EXPECT_EQ(
        frames[1],
        (Writes {
            {reg::NR10, 0x7B},
            {reg::NR11, 0x81},
            {reg::NR12, 0xF3},
            {reg::NR13, 0xD6},
            {reg::NR14, 0xC6}}));
}

TEST(
    Player,
    PulseChannel2Trigger)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 ...\n"), 2);
    EXPECT_EQ(
        frames[1],
        (Writes {{reg::NR21, 0xC0}, {reg::NR22, 0xA2}, {reg::NR23, 0xD6}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    WaveChannelTriggerLoadsWaveOnlyWhenItChanges)
{
    const auto frames = frames_of(on_channel(3, "00 A-4 1 ...\n01 C-4 1 ...\n02 A-4 2 ...\n"), 4);
    const Writes trigger_a4_ins1 {
        {reg::NR30, 0x80},
        {reg::NR31, 0x3F},
        {reg::NR32, 0x20},
        {reg::NR33, 0xD6},
        {reg::NR34, 0xC6}};
    EXPECT_EQ(frames[1], concat({wave_load(kWave1), trigger_a4_ins1}));

    const auto c4 = note_period(kC4);
    EXPECT_EQ(
        frames[2],
        (Writes {
            {reg::NR30, 0x80},
            {reg::NR31, 0x3F},
            {reg::NR32, 0x20},
            {reg::NR33, static_cast<std::uint8_t>(c4 & 0xFF)},
            {reg::NR34, static_cast<std::uint8_t>(0xC0 | c4 >> 8)}}));

    EXPECT_EQ(
        frames[3],
        concat(
            {wave_load(kWave2),
             {{reg::NR30, 0x80},
              {reg::NR31, 0x00},
              {reg::NR32, 0x40},
              {reg::NR33, 0xD6},
              {reg::NR34, 0x86}}}));
}

TEST(
    Player,
    NoiseChannelTrigger)
{
    const auto frames = frames_of(on_channel(4, "00 A-4 1 ...\n01 A-4 2 ...\n"), 3);
    const auto nr43 = noise_nr43(kA4);
    EXPECT_EQ(
        frames[1],
        (Writes {
            {reg::NR41, 0x05},
            {reg::NR42, 0xB1},
            {reg::NR43, static_cast<std::uint8_t>(nr43 | 0x08)},
            {reg::NR44, 0xC0}}));
    EXPECT_EQ(
        frames[2],
        (Writes {{reg::NR41, 0x00}, {reg::NR42, 0x71}, {reg::NR43, nr43}, {reg::NR44, 0x80}}));
}

TEST(
    Player,
    ChannelsAreProcessedInOrder)
{
    const auto song = make_song("order 01 01 01 01\npattern 01\n00 A-4 1 ...\n");
    const auto frames = render(song, 2);
    std::vector<std::uint16_t> addresses;
    for (const auto& write : frames[1]) {
        addresses.push_back(write.address);
    }
    std::vector<std::uint16_t> expected {
        reg::NR10,
        reg::NR11,
        reg::NR12,
        reg::NR13,
        reg::NR14,
        reg::NR21,
        reg::NR22,
        reg::NR23,
        reg::NR24,
        reg::NR30};
    for (std::uint16_t i = 0; i < 16; ++i) {
        expected.push_back(static_cast<std::uint16_t>(reg::WAVE_RAM + i));
    }
    expected.insert(
        expected.end(),
        {reg::NR30,
         reg::NR31,
         reg::NR32,
         reg::NR33,
         reg::NR34,
         reg::NR41,
         reg::NR42,
         reg::NR43,
         reg::NR44});
    EXPECT_EQ(addresses, expected);
}

TEST(
    Player,
    InstrumentColumnSelectsWithoutWriting)
{
    const auto frames = frames_of(on_channel(2, "00 --- 2 ...\n01 A-4 . ...\n"), 3);
    EXPECT_TRUE(frames[1].empty());
    EXPECT_EQ(frames[2][0], (ApuWrite {reg::NR21, 0xC0}));
}

TEST(
    Player,
    InitialInstrumentIsOne)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 . ...\n"), 2);
    EXPECT_EQ(frames[1][0], (ApuWrite {reg::NR21, 0x81}));
}

// --- Effects ---

TEST(
    Player,
    MasterVolumeAndPanningAreWrittenAfterTheTrigger)
{
    const auto song = make_song(
        "ticks_per_row 01\norder 01 02 00 00\npattern 00\n"
        "pattern 01\n00 A-4 2 512\npattern 02\n00 --- . 8F0\n");
    const auto frames = render(song, 2);
    EXPECT_EQ(
        frames[1],
        (Writes {
            {reg::NR10, 0x00},
            {reg::NR11, 0xC0},
            {reg::NR12, 0xA2},
            {reg::NR13, 0xD6},
            {reg::NR14, 0x86},
            {reg::NR50, 0x12},
            {reg::NR51, 0xF0}}));
}

TEST(
    Player,
    CallRoutineIsANoOp)
{
    const auto frames = frames_of(on_channel(1, "00 --- . 6FF\n"), 2);
    EXPECT_TRUE(frames[1].empty());
}

TEST(
    Player,
    TimbreOnPulseChannel)
{
    const auto frames =
        frames_of(on_channel(2, "00 A-4 1 ...\n01 --- . 93F\n02 A-4 . 940\n03 A-4 . ...\n"), 5);
    EXPECT_EQ(frames[2], (Writes {{reg::NR21, 0x3F}}));
    EXPECT_EQ(
        frames[3],
        (Writes {{reg::NR21, 0x40}, {reg::NR22, 0xF3}, {reg::NR23, 0xD6}, {reg::NR24, 0xC6}}));
    // The next trigger takes the instrument's value again.
    EXPECT_EQ(frames[4][0], (ApuWrite {reg::NR21, 0x81}));
}

TEST(
    Player,
    TimbreOnWaveChannelReloadsAndRetriggers)
{
    const auto frames =
        frames_of(on_channel(3, "00 A-4 1 ...\n01 --- . 902\n02 --- . 902\n03 A-4 1 901\n"), 5);
    EXPECT_EQ(frames[2], concat({wave_load(kWave2), {{reg::NR30, 0x80}, {reg::NR34, 0xC6}}}));
    EXPECT_TRUE(frames[3].empty()); // Wave 2 is already loaded.
    EXPECT_EQ(
        frames[4],
        concat(
            {wave_load(kWave1),
             {{reg::NR30, 0x80},
              {reg::NR31, 0x3F},
              {reg::NR32, 0x20},
              {reg::NR33, 0xD6},
              {reg::NR34, 0xC6}}}));
}

TEST(
    Player,
    TimbreOnWaveChannelFoldsIntoTrigger)
{
    const auto frames = frames_of(on_channel(3, "00 A-4 1 902\n"), 2);
    EXPECT_EQ(
        frames[1],
        concat(
            {wave_load(kWave2),
             {{reg::NR30, 0x80},
              {reg::NR31, 0x3F},
              {reg::NR32, 0x20},
              {reg::NR33, 0xD6},
              {reg::NR34, 0xC6}}}));
}

TEST(
    Player,
    TimbreOnNoiseChannel)
{
    const auto frames =
        frames_of(on_channel(4, "00 A-4 2 ...\n01 --- . 901\n02 --- . 900\n03 A-4 2 901\n"), 5);
    const auto nr43 = noise_nr43(kA4);
    EXPECT_EQ(frames[2], (Writes {{reg::NR43, static_cast<std::uint8_t>(nr43 | 0x08)}}));
    EXPECT_EQ(frames[3], (Writes {{reg::NR43, nr43}}));
    EXPECT_EQ(
        frames[4],
        (Writes {
            {reg::NR41, 0x00},
            {reg::NR42, 0x71},
            {reg::NR43, static_cast<std::uint8_t>(nr43 | 0x08)},
            {reg::NR44, 0x80}}));
}

TEST(
    Player,
    SetVolumeRetriggersPulseAndNoise)
{
    auto frames = frames_of(on_channel(1, "00 A-4 1 ...\n01 --- . C80\n"), 3);
    EXPECT_EQ(frames[2], (Writes {{reg::NR12, 0x80}, {reg::NR14, 0xC6}}));

    frames = frames_of(on_channel(4, "00 A-4 1 ...\n01 --- . C80\n"), 3);
    EXPECT_EQ(frames[2], (Writes {{reg::NR42, 0x80}, {reg::NR44, 0xC0}}));
}

TEST(
    Player,
    SetVolumeFoldsIntoTrigger)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 C57\n"), 2);
    EXPECT_EQ(
        frames[1],
        (Writes {{reg::NR21, 0xC0}, {reg::NR22, 0x57}, {reg::NR23, 0xD6}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    SetVolumeOnWaveChannel)
{
    auto frames = frames_of(on_channel(3, "00 A-4 1 ...\n01 --- . C30\n"), 3);
    EXPECT_EQ(frames[2], (Writes {{reg::NR32, 0x60}}));

    frames = frames_of(on_channel(3, "00 A-4 1 C20\n"), 2);
    EXPECT_EQ(frames[1][19], (ApuWrite {reg::NR32, 0x40}));
}

TEST(
    Player,
    SetTempoAppliesFromTheNextRow)
{
    // Row 0 lasts 4 frames (1..4) even though it sets 2; row 1 starts at 5, row 2 at 7.
    const auto frames =
        frames_of(on_channel(2, "00 A-4 1 F02\n01 A-4 1 ...\n02 A-4 1 ...\n", "04"), 8);
    EXPECT_FALSE(frames[5].empty());
    EXPECT_TRUE(frames[6].empty());
    EXPECT_FALSE(frames[7].empty());
}

TEST(
    Player,
    SetTempoZeroMeans256)
{
    const auto frames = frames_of(on_channel(2, "00 --- . F00\n01 --- . ...\n02 A-4 1 ...\n"), 259);
    EXPECT_TRUE(frames[257].empty());
    EXPECT_FALSE(frames[258].empty());
}

// --- Note cut (E) ---

TEST(
    Player,
    NoteCutAtTickZeroFollowsTheTrigger)
{
    const auto frames = frames_of(on_channel(1, "00 A-4 1 E00\n", "04"), 3);
    EXPECT_EQ(
        frames[1],
        (Writes {
            {reg::NR10, 0x7B},
            {reg::NR11, 0x81},
            {reg::NR12, 0xF3},
            {reg::NR13, 0xD6},
            {reg::NR14, 0xC6},
            {reg::NR12, 0x00},
            {reg::NR14, 0xC6}}));
    EXPECT_TRUE(frames[2].empty());
}

TEST(
    Player,
    NoteCutAtALaterTick)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 E02\n", "04"), 5);
    EXPECT_EQ(frames[1].size(), 4u); // The trigger only.
    EXPECT_TRUE(frames[2].empty());
    EXPECT_EQ(frames[3], (Writes {{reg::NR22, 0x00}, {reg::NR24, 0x86}}));
    EXPECT_TRUE(frames[4].empty());
}

TEST(
    Player,
    NoteCutWithoutANoteCutsThePlayingNote)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 ...\n01 --- . E01\n", "04"), 7);
    EXPECT_TRUE(frames[5].empty()); // Row 1's row tick.
    EXPECT_EQ(frames[6], (Writes {{reg::NR22, 0x00}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    NoteCutPastTheRowDoesNothing)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 E04\n", "04"), 12);
    for (std::size_t frame = 2; frame < frames.size(); ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    NoteCutOnTheWaveChannel)
{
    const auto frames = frames_of(on_channel(3, "00 A-4 1 E01\n", "04"), 3);
    EXPECT_EQ(frames[2], (Writes {{reg::NR32, 0x00}}));
}

TEST(
    Player,
    NoteCutOnTheNoiseChannel)
{
    const auto frames = frames_of(on_channel(4, "00 A-4 1 E01\n", "04"), 3);
    EXPECT_EQ(frames[2], (Writes {{reg::NR42, 0x00}, {reg::NR44, 0xC0}}));
}

TEST(
    Player,
    NoteCutsAreWrittenInChannelOrder)
{
    const auto song =
        make_song("ticks_per_row 04\norder 01 01 00 00\npattern 00\npattern 01\n00 A-4 2 E01\n");
    const auto frames = render(song, 3);
    EXPECT_EQ(
        frames[2],
        (Writes {{reg::NR12, 0x00}, {reg::NR14, 0x86}, {reg::NR22, 0x00}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    NoteCutUsesTheLengthTheRowStartedWith)
{
    // F02 on channel 1 shortens the next row only: the cut at tick 3 still happens.
    const auto song = make_song(
        "ticks_per_row 04\norder 01 02 00 00\npattern 00\n"
        "pattern 01\n00 A-4 2 F02\npattern 02\n00 A-4 2 E03\n");
    const auto frames = render(song, 5);
    EXPECT_EQ(frames[4], (Writes {{reg::NR22, 0x00}, {reg::NR24, 0x86}}));
}

// --- Note delay (7) ---

TEST(
    Player,
    NoteDelayZeroTriggersOnTheRowTick)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 700\n", "04"), 2);
    EXPECT_EQ(
        frames[1],
        (Writes {{reg::NR21, 0xC0}, {reg::NR22, 0xA2}, {reg::NR23, 0xD6}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    NoteDelayTriggersAtTickWithTheRowsInstrument)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 702\n", "04"), 5);
    EXPECT_TRUE(frames[1].empty());
    EXPECT_TRUE(frames[2].empty());
    EXPECT_EQ(
        frames[3],
        (Writes {{reg::NR21, 0xC0}, {reg::NR22, 0xA2}, {reg::NR23, 0xD6}, {reg::NR24, 0x86}}));
    EXPECT_TRUE(frames[4].empty());
}

TEST(
    Player,
    NoteDelayPastTheRowIsDropped)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 704\n", "04"), 12);
    for (std::size_t frame = 1; frame < frames.size(); ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    NoteDelayWithoutANoteDoesNothing)
{
    const auto frames = frames_of(on_channel(2, "00 --- 2 702\n", "04"), 8);
    for (std::size_t frame = 1; frame < frames.size(); ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    DelayedTriggersAreWrittenInChannelOrder)
{
    const auto song =
        make_song("ticks_per_row 04\norder 01 01 00 00\npattern 00\npattern 01\n00 A-4 2 701\n");
    const auto frames = render(song, 3);
    EXPECT_TRUE(frames[1].empty());
    EXPECT_EQ(
        frames[2],
        (Writes {
            {reg::NR10, 0x00},
            {reg::NR11, 0xC0},
            {reg::NR12, 0xA2},
            {reg::NR13, 0xD6},
            {reg::NR14, 0x86},
            {reg::NR21, 0xC0},
            {reg::NR22, 0xA2},
            {reg::NR23, 0xD6},
            {reg::NR24, 0x86}}));
}

TEST(
    Player,
    DelayedWaveTriggerLoadsTheWave)
{
    const auto frames = frames_of(on_channel(3, "00 A-4 1 701\n", "04"), 3);
    EXPECT_TRUE(frames[1].empty());
    EXPECT_EQ(
        frames[2],
        concat(
            {wave_load(kWave1),
             {{reg::NR30, 0x80},
              {reg::NR31, 0x3F},
              {reg::NR32, 0x20},
              {reg::NR33, 0xD6},
              {reg::NR34, 0xC6}}}));
}

TEST(
    Player,
    DelayedNoteSetsThePeriodForLaterRetriggers)
{
    // The C on row 1 retriggers with the delayed note's period (A-4: high bits 6).
    const auto frames = frames_of(on_channel(2, "00 A-4 2 702\n01 --- . C80\n", "04"), 6);
    EXPECT_EQ(frames[5], (Writes {{reg::NR22, 0x80}, {reg::NR24, 0x86}}));
}

// --- Volume slide (A) ---

TEST(
    Player,
    VolumeSlideUpOnEachNonRowTick)
{
    // Instrument 2 starts at volume A: C, E, then F (clamped).
    const auto frames = frames_of(on_channel(2, "00 A-4 2 A20\n", "04"), 6);
    EXPECT_EQ(frames[1].size(), 4u); // The trigger only.
    EXPECT_EQ(frames[2], (Writes {{reg::NR22, 0xC0}, {reg::NR24, 0x86}}));
    EXPECT_EQ(frames[3], (Writes {{reg::NR22, 0xE0}, {reg::NR24, 0x86}}));
    EXPECT_EQ(frames[4], (Writes {{reg::NR22, 0xF0}, {reg::NR24, 0x86}}));
    EXPECT_TRUE(frames[5].empty()); // Next row: the slide is over.
}

TEST(
    Player,
    VolumeSlideDownStopsAtZero)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 A04\n01 --- . A04\n", "04"), 9);
    EXPECT_EQ(frames[2], (Writes {{reg::NR22, 0x60}, {reg::NR24, 0x86}}));
    EXPECT_EQ(frames[3], (Writes {{reg::NR22, 0x20}, {reg::NR24, 0x86}}));
    EXPECT_EQ(frames[4], (Writes {{reg::NR22, 0x00}, {reg::NR24, 0x86}}));
    for (std::size_t frame = 5; frame < 9; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame; // Already 0: nothing to write.
    }
}

TEST(
    Player,
    VolumeSlideWritesNothingWhenClamped)
{
    const auto frames = frames_of(on_channel(1, "00 A-4 1 A10\n", "04"), 5); // Volume F.
    for (std::size_t frame = 2; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    VolumeSlideZeroDoesNothing)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 A00\n", "04"), 5);
    for (std::size_t frame = 2; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    VolumeSlideStartsFromTheVolumeSetByC)
{
    const auto frames =
        frames_of(on_channel(2, "00 A-4 2 ...\n01 --- . C83\n02 --- . A10\n", "04"), 13);
    EXPECT_EQ(frames[10], (Writes {{reg::NR22, 0x90}, {reg::NR24, 0x86}}));
    EXPECT_EQ(frames[11], (Writes {{reg::NR22, 0xA0}, {reg::NR24, 0x86}}));
    EXPECT_EQ(frames[12], (Writes {{reg::NR22, 0xB0}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    VolumeSlideStartsFromAFoldedC)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 C30\n01 --- . A10\n", "04"), 7);
    EXPECT_EQ(frames[6], (Writes {{reg::NR22, 0x40}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    VolumeSlideAfterACutStartsFromZero)
{
    const auto frames = frames_of(on_channel(2, "00 A-4 2 E01\n01 --- . A20\n", "04"), 7);
    EXPECT_EQ(frames[6], (Writes {{reg::NR22, 0x20}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    VolumeSlideOnTheNoiseChannel)
{
    // Noise instrument 2 starts at volume 7.
    const auto frames = frames_of(on_channel(4, "00 A-4 2 A10\n", "04"), 3);
    EXPECT_EQ(frames[2], (Writes {{reg::NR42, 0x80}, {reg::NR44, 0x80}}));
}

TEST(
    Player,
    VolumeSlideIsIgnoredOnTheWaveChannel)
{
    const auto frames = frames_of(on_channel(3, "00 A-4 1 A20\n", "04"), 5);
    for (std::size_t frame = 2; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    VolumeSlidesAreWrittenInChannelOrder)
{
    const auto song =
        make_song("ticks_per_row 04\norder 01 01 00 00\npattern 00\npattern 01\n00 A-4 2 A10\n");
    const auto frames = render(song, 3);
    EXPECT_EQ(
        frames[2],
        (Writes {{reg::NR12, 0xB0}, {reg::NR14, 0x86}, {reg::NR22, 0xB0}, {reg::NR24, 0x86}}));
}

// --- Arpeggio (0) ---

namespace {

// NRx3/NRx4 writes (no trigger bit) that set `note` on a pulse or wave channel.
Writes pitch(
    std::uint16_t nrx3,
    std::uint16_t nrx4,
    const char* note,
    std::uint8_t length = 0)
{
    const auto period = note_period(*parse_note_name(note));
    return {
        {nrx3, static_cast<std::uint8_t>(period & 0xFF)},
        {nrx4, static_cast<std::uint8_t>(length | period >> 8)}};
}

} // namespace

TEST(
    Player,
    ArpeggioCyclesBaseXYAndRestoresTheBase)
{
    // C-4 + 4 = E-4, + 7 = G-4; tick % 3 = 0 is the base.
    const auto frames = frames_of(on_channel(2, "00 C-4 2 047\n", "06"), 8);
    EXPECT_EQ(frames[1].size(), 4u); // The trigger.
    EXPECT_EQ(frames[2], pitch(reg::NR23, reg::NR24, "E-4"));
    EXPECT_EQ(frames[3], pitch(reg::NR23, reg::NR24, "G-4"));
    EXPECT_EQ(frames[4], pitch(reg::NR23, reg::NR24, "C-4"));
    EXPECT_EQ(frames[5], pitch(reg::NR23, reg::NR24, "E-4"));
    EXPECT_EQ(frames[6], pitch(reg::NR23, reg::NR24, "G-4"));
    EXPECT_EQ(frames[7], pitch(reg::NR23, reg::NR24, "C-4")); // Next row: base restored.
}

TEST(
    Player,
    ArpeggioWritesOnlyWhenThePitchChanges)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 040\n", "06"), 7);
    EXPECT_EQ(frames[2], pitch(reg::NR23, reg::NR24, "E-4"));
    EXPECT_EQ(frames[3], pitch(reg::NR23, reg::NR24, "C-4")); // y = 0: the base.
    EXPECT_TRUE(frames[4].empty()); // Base again: no change.
    EXPECT_EQ(frames[5], pitch(reg::NR23, reg::NR24, "E-4"));
}

TEST(
    Player,
    ArpeggioWithoutANoteUsesTheLastNote)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 ...\n01 --- . 047\n", "04"), 9);
    EXPECT_TRUE(frames[5].empty()); // Row tick: already the base.
    EXPECT_EQ(frames[6], pitch(reg::NR23, reg::NR24, "E-4"));
    EXPECT_EQ(frames[7], pitch(reg::NR23, reg::NR24, "G-4"));
    EXPECT_EQ(frames[8], pitch(reg::NR23, reg::NR24, "C-4"));
}

TEST(
    Player,
    ArpeggioBeforeAnyNoteDoesNothing)
{
    const auto frames = frames_of(on_channel(2, "00 --- . 047\n", "04"), 5);
    for (std::size_t frame = 1; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    ArpeggioClampsToB7)
{
    const auto frames = frames_of(on_channel(2, "00 A-7 2 0F0\n", "04"), 4);
    EXPECT_EQ(frames[2], pitch(reg::NR23, reg::NR24, "B-7"));
    EXPECT_EQ(frames[3], pitch(reg::NR23, reg::NR24, "A-7"));
}

TEST(
    Player,
    ArpeggioKeepsTheLengthBit)
{
    const auto frames = frames_of(on_channel(1, "00 C-4 1 047\n", "04"), 3); // Length on.
    EXPECT_EQ(frames[2], pitch(reg::NR13, reg::NR14, "E-4", 0x40));
}

TEST(
    Player,
    RestoreComesBeforeTheNextRowsWrites)
{
    // Row 1's C retriggers from the base: the base pitch is written back first.
    const auto frames = frames_of(on_channel(2, "00 C-4 2 047\n01 --- . C80\n", "03"), 5);
    EXPECT_EQ(
        frames[4],
        concat({pitch(reg::NR23, reg::NR24, "C-4"), {{reg::NR22, 0x80}, {reg::NR24, 0x86}}}));
}

TEST(
    Player,
    NoRestoreWhenTheNextRowTriggers)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 047\n01 E-4 . ...\n", "03"), 5);
    EXPECT_EQ(frames[4].size(), 4u); // Only the trigger.
    EXPECT_EQ(frames[4][0], (ApuWrite {reg::NR21, 0xC0}));
}

TEST(
    Player,
    RestoreBeforeADelayedNote)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 047\n01 E-4 . 701\n", "03"), 6);
    EXPECT_EQ(frames[4], pitch(reg::NR23, reg::NR24, "C-4"));
    EXPECT_EQ(frames[5].size(), 4u); // The delayed trigger.
}

TEST(
    Player,
    ArpeggioOnTheWaveChannel)
{
    const auto frames = frames_of(on_channel(3, "00 C-4 1 047\n", "04"), 3);
    EXPECT_EQ(frames[2], pitch(reg::NR33, reg::NR34, "E-4", 0x40)); // Instrument 1: length on.
}

TEST(
    Player,
    ArpeggioIsIgnoredOnTheNoiseChannel)
{
    const auto frames = frames_of(on_channel(4, "00 C-4 1 047\n", "04"), 5);
    for (std::size_t frame = 2; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

// --- Portamento (1, 2) ---

namespace {

// NRx3/NRx4 writes (no trigger bit) that set `period`.
Writes period_writes(
    std::uint16_t nrx3,
    std::uint16_t nrx4,
    std::uint16_t period,
    std::uint8_t length = 0)
{
    return {
        {nrx3, static_cast<std::uint8_t>(period & 0xFF)},
        {nrx4, static_cast<std::uint8_t>(length | period >> 8)}};
}

const std::uint16_t kC4Period = 1547; // note_period(C-4)

} // namespace

TEST(
    Player,
    PortamentoUpAddsToThePeriodOnEachNonRowTick)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 102\n", "04"), 6);
    EXPECT_EQ(frames[2], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
    EXPECT_EQ(frames[3], period_writes(reg::NR23, reg::NR24, kC4Period + 4));
    EXPECT_EQ(frames[4], period_writes(reg::NR23, reg::NR24, kC4Period + 6));
    EXPECT_TRUE(frames[5].empty()); // The slid period stays: no restore.
}

TEST(
    Player,
    PortamentoDownSubtractsFromThePeriod)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 2FF\n", "03"), 4);
    EXPECT_EQ(frames[2], period_writes(reg::NR23, reg::NR24, kC4Period - 255));
    EXPECT_EQ(frames[3], period_writes(reg::NR23, reg::NR24, kC4Period - 510));
}

TEST(
    Player,
    PortamentoClampsToTheNoteTable)
{
    // A-7 = 2011 goes up to B-7 = 2015 and stops; C-2 = 44 cannot go down.
    auto frames = frames_of(on_channel(2, "00 A-7 2 10A\n", "04"), 5);
    EXPECT_EQ(frames[2], period_writes(reg::NR23, reg::NR24, 2015));
    EXPECT_TRUE(frames[3].empty());
    frames = frames_of(on_channel(2, "00 C-2 2 201\n", "04"), 5);
    EXPECT_TRUE(frames[2].empty());
}

TEST(
    Player,
    PortamentoZeroDoesNothing)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 100\n", "04"), 5);
    for (std::size_t frame = 2; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    PortamentoContinuesOnTheSlidPeriod)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 101\n01 --- . 101\n", "03"), 7);
    EXPECT_EQ(frames[5], period_writes(reg::NR23, reg::NR24, kC4Period + 3));
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kC4Period + 4));
}

TEST(
    Player,
    RetriggersUseTheSlidPeriod)
{
    // C-4 + 255, then + 255 again would pass B-7: clamped to 2015 ($7DF), high bits 7.
    const auto frames = frames_of(on_channel(2, "00 C-4 2 1FF\n01 --- . C80\n", "04"), 6);
    EXPECT_EQ(frames[5], (Writes {{reg::NR22, 0x80}, {reg::NR24, 0x87}}));
}

TEST(
    Player,
    ANewNoteResetsTheSlide)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 1FF\n01 C-4 . ...\n", "03"), 5);
    EXPECT_EQ(
        frames[4],
        (Writes {{reg::NR21, 0xC0}, {reg::NR22, 0xA2}, {reg::NR23, 0x0B}, {reg::NR24, 0x86}}));
}

TEST(
    Player,
    ArpeggioOnASlidChannelUsesTheSlidPeriodForItsBaseStep)
{
    // Row 0 slides C-4 up by 1 on its 3 non-row ticks; row 1's arpeggio: + x, + y from the note,
    // then the slid period on tick 3; row 2's tick has nothing to restore.
    const auto frames = frames_of(on_channel(2, "00 C-4 2 101\n01 --- . 047\n", "04"), 10);
    EXPECT_EQ(frames[6], pitch(reg::NR23, reg::NR24, "E-4"));
    EXPECT_EQ(frames[7], pitch(reg::NR23, reg::NR24, "G-4"));
    EXPECT_EQ(frames[8], period_writes(reg::NR23, reg::NR24, kC4Period + 3));
    EXPECT_TRUE(frames[9].empty()); // Already the slid period.
}

TEST(
    Player,
    PortamentoOnTheWaveChannel)
{
    const auto frames = frames_of(on_channel(3, "00 C-4 1 210\n", "04"), 3);
    EXPECT_EQ(frames[2], period_writes(reg::NR33, reg::NR34, kC4Period - 16, 0x40));
}

TEST(
    Player,
    PortamentoIsIgnoredOnTheNoiseChannel)
{
    const auto frames = frames_of(on_channel(4, "00 C-4 1 110\n", "04"), 5);
    for (std::size_t frame = 2; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

// --- Tone portamento (3) ---

namespace {

const std::uint16_t kE4Period = 1650;
const std::uint16_t kG4Period = 1714;

} // namespace

TEST(
    Player,
    TonePortamentoSlidesToTheNoteWithoutTriggering)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 ...\n01 E-4 . 340\n", "04"), 10);
    EXPECT_TRUE(frames[5].empty()); // No trigger.
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kC4Period + 0x40));
    EXPECT_EQ(frames[7], period_writes(reg::NR23, reg::NR24, kE4Period)); // Stops on E-4.
    EXPECT_TRUE(frames[8].empty());
    EXPECT_TRUE(frames[9].empty()); // Next row: nothing to restore.
}

TEST(
    Player,
    TonePortamentoSlidesDown)
{
    const auto frames = frames_of(on_channel(2, "00 G-4 2 ...\n01 C-4 . 380\n", "04"), 8);
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kG4Period - 0x80));
    EXPECT_EQ(frames[7], period_writes(reg::NR23, reg::NR24, kC4Period));
}

TEST(
    Player,
    TonePortamentoOnTheFirstNoteTriggers)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 340\n", "04"), 4);
    EXPECT_EQ(frames[1].size(), 4u); // The trigger.
    EXPECT_TRUE(frames[2].empty()); // Already at the note: nothing to slide.
}

TEST(
    Player,
    TonePortamentoContinuesTowardTheTarget)
{
    // Row 1 (frames 4-6) slides + 0x10 twice; row 2 (frames 7-9) continues + 0x20 per tick.
    const auto frames =
        frames_of(on_channel(2, "00 C-4 2 ...\n01 G-4 . 310\n02 --- . 320\n", "03"), 10);
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kC4Period + 0x20));
    EXPECT_TRUE(frames[7].empty()); // Row tick: nothing to restore.
    EXPECT_EQ(frames[8], period_writes(reg::NR23, reg::NR24, kC4Period + 0x40));
    EXPECT_EQ(frames[9], period_writes(reg::NR23, reg::NR24, kC4Period + 0x60));
}

TEST(
    Player,
    TonePortamentoZeroOrWithoutTargetDoesNothing)
{
    auto frames = frames_of(on_channel(2, "00 C-4 2 ...\n01 E-4 . 300\n", "04"), 9);
    for (std::size_t frame = 5; frame < 9; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
    frames = frames_of(on_channel(2, "00 C-4 2 ...\n01 --- . 340\n", "04"), 9);
    for (std::size_t frame = 5; frame < 9; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    ANewNoteClearsTheTarget)
{
    const auto frames = frames_of(
        on_channel(2, "00 C-4 2 ...\n01 E-4 . 301\n02 G-4 . ...\n03 --- . 310\n", "03"), 13);
    for (std::size_t frame = 11; frame < 13; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    TheTargetBecomesTheArpeggioBase)
{
    // Row 1 makes E-4 the channel's note but only slides 1 per tick (C-4 + 3 after row 1).
    // Row 2's arpeggio: + 12 from E-4 (E-5), + 0 (E-4 itself), then the slid period.
    const auto frames =
        frames_of(on_channel(2, "00 C-4 2 ...\n01 E-4 . 301\n02 --- . 0C0\n", "04"), 13);
    EXPECT_EQ(frames[10], pitch(reg::NR23, reg::NR24, "E-5"));
    EXPECT_EQ(frames[11], pitch(reg::NR23, reg::NR24, "E-4"));
    EXPECT_EQ(frames[12], period_writes(reg::NR23, reg::NR24, kC4Period + 3));
}

TEST(
    Player,
    PitchIsRestoredBeforeATonePortamentoRow)
{
    // Row 1 ends on an arpeggio step; row 2's note does not trigger (3): restore first.
    const auto frames =
        frames_of(on_channel(2, "00 C-4 2 ...\n01 --- . 047\n02 E-4 . 301\n", "03"), 8);
    EXPECT_EQ(frames[7], pitch(reg::NR23, reg::NR24, "C-4"));
}

TEST(
    Player,
    TonePortamentoOnTheWaveChannel)
{
    const auto frames = frames_of(on_channel(3, "00 C-4 1 ...\n01 E-4 . 340\n", "04"), 7);
    EXPECT_EQ(frames[6], period_writes(reg::NR33, reg::NR34, kC4Period + 0x40, 0x40));
}

TEST(
    Player,
    TonePortamentoIsIgnoredOnTheNoiseChannel)
{
    // On noise the note simply triggers, and nothing slides.
    const auto frames = frames_of(on_channel(4, "00 C-4 1 ...\n01 E-4 . 340\n", "04"), 9);
    EXPECT_EQ(frames[5].size(), 4u);
    for (std::size_t frame = 6; frame < 9; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

// --- Vibrato (4) ---

TEST(
    Player,
    VibratoAlternatesAroundThePeriod)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 412\n", "06"), 8);
    EXPECT_EQ(frames[2], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
    EXPECT_EQ(frames[3], period_writes(reg::NR23, reg::NR24, kC4Period - 2));
    EXPECT_EQ(frames[4], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
    EXPECT_EQ(frames[5], period_writes(reg::NR23, reg::NR24, kC4Period - 2));
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
    EXPECT_EQ(frames[7], period_writes(reg::NR23, reg::NR24, kC4Period)); // Restored.
}

TEST(
    Player,
    VibratoSwitchesEveryXTicks)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 422\n", "06"), 7);
    EXPECT_EQ(frames[2], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
    EXPECT_TRUE(frames[3].empty());
    EXPECT_EQ(frames[4], period_writes(reg::NR23, reg::NR24, kC4Period - 2));
    EXPECT_TRUE(frames[5].empty());
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
}

TEST(
    Player,
    VibratoSpeedZeroCountsAsOne)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 402\n", "04"), 4);
    EXPECT_EQ(frames[2], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
    EXPECT_EQ(frames[3], period_writes(reg::NR23, reg::NR24, kC4Period - 2));
}

TEST(
    Player,
    VibratoDepthZeroDoesNothing)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 410\n", "06"), 8);
    for (std::size_t frame = 2; frame < 8; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    VibratoClampsToTheNoteTable)
{
    // B-7 = 2015: + 15 clamps to 2015 (no change), - 15 gives 2000.
    const auto frames = frames_of(on_channel(2, "00 B-7 2 41F\n", "04"), 5);
    EXPECT_TRUE(frames[2].empty());
    EXPECT_EQ(frames[3], period_writes(reg::NR23, reg::NR24, 2000));
    EXPECT_EQ(frames[4], period_writes(reg::NR23, reg::NR24, 2015));
}

TEST(
    Player,
    VibratoBeforeAnyNoteDoesNothing)
{
    const auto frames = frames_of(on_channel(2, "00 --- . 412\n", "04"), 5);
    for (std::size_t frame = 1; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

TEST(
    Player,
    VibratoWithoutANoteUsesTheChannelsPeriod)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 ...\n01 --- . 412\n", "04"), 8);
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kC4Period + 2));
    EXPECT_EQ(frames[7], period_writes(reg::NR23, reg::NR24, kC4Period - 2));
}

TEST(
    Player,
    VibratoOnASlidPeriod)
{
    // Row 0 slides C-4 up by 4 on 3 ticks (+ 12); row 1 vibrates around that.
    const auto frames = frames_of(on_channel(2, "00 C-4 2 104\n01 --- . 412\n", "04"), 8);
    EXPECT_EQ(frames[6], period_writes(reg::NR23, reg::NR24, kC4Period + 12 + 2));
    EXPECT_EQ(frames[7], period_writes(reg::NR23, reg::NR24, kC4Period + 12 - 2));
}

TEST(
    Player,
    VibratoIsRestoredBeforeTheNextRowsWrites)
{
    const auto frames = frames_of(on_channel(2, "00 C-4 2 412\n01 --- . C80\n", "03"), 5);
    EXPECT_EQ(
        frames[4],
        concat(
            {period_writes(reg::NR23, reg::NR24, kC4Period),
             {{reg::NR22, 0x80}, {reg::NR24, 0x86}}}));
}

TEST(
    Player,
    VibratoOnTheWaveChannel)
{
    const auto frames = frames_of(on_channel(3, "00 C-4 1 412\n", "04"), 3);
    EXPECT_EQ(frames[2], period_writes(reg::NR33, reg::NR34, kC4Period + 2, 0x40));
}

TEST(
    Player,
    VibratoIsIgnoredOnTheNoiseChannel)
{
    const auto frames = frames_of(on_channel(4, "00 C-4 1 412\n", "04"), 5);
    for (std::size_t frame = 2; frame < 5; ++frame) {
        EXPECT_TRUE(frames[frame].empty()) << frame;
    }
}

// --- Flow control ---

namespace {

// Two orders: order 0 plays pattern 01 on channel 2, order 1 plays pattern 02. Every row of
// both patterns triggers a distinct note so the position can be read back. Flow effects go in
// pattern 03 (channel 1, order 0), 04 (channel 3, both orders) and 05 (channel 1, order 1).
std::string two_orders(
    const std::string& flow_rows_ch1,
    const std::string& flow_rows_ch3 = "",
    const std::string& flow_rows_order1 = "")
{
    std::string text = "ticks_per_row 01\norder 03 01 04 00\norder 05 02 04 00\npattern 00\n"
                       "pattern 01\n";
    for (int row = 0; row < 64; ++row) {
        char line[32];
        std::snprintf(line, sizeof line, "%02X %s 1 ...\n", row, note_name(1 + row % 36).c_str());
        text += line;
    }
    text += "pattern 02\n";
    for (int row = 0; row < 64; ++row) {
        char line[32];
        std::snprintf(line, sizeof line, "%02X %s 1 ...\n", row, note_name(37 + row % 36).c_str());
        text += line;
    }
    return text
         + "pattern 03\n"
         + flow_rows_ch1
         + "pattern 04\n"
         + flow_rows_ch3
         + "pattern 05\n"
         + flow_rows_order1;
}

// Note played on channel 2 in `frame`, decoded back from NR23/NR24.
std::uint8_t channel2_note(const Writes& frame)
{
    std::uint16_t period = 0;
    for (const auto& write : frame) {
        if (write.address == reg::NR23) {
            period = static_cast<std::uint16_t>(period | write.value);
        } else if (write.address == reg::NR24) {
            period = static_cast<std::uint16_t>(period | (write.value & 0x07) << 8);
        }
    }
    for (std::uint8_t note = kFirstNote; note <= kLastNote; ++note) {
        if (note_period(note) == period) {
            return note;
        }
    }
    return kNoteNone;
}

// Order and row of the row played in `frame` of a two_orders() song.
std::pair<
    int,
    int>
position(const Writes& frame)
{
    const int note = channel2_note(frame);
    return note <= 36 ? std::pair {0, note - 1} : std::pair {1, note - 37};
}

} // namespace

TEST(
    Player,
    OrdersPlayInSequenceAndLoop)
{
    const auto frames = render(make_song(two_orders("")), 130);
    EXPECT_EQ(position(frames[1]), (std::pair {0, 0}));
    EXPECT_EQ(position(frames[64]), (std::pair {0, 63 % 36}));
    EXPECT_EQ(position(frames[65]), (std::pair {1, 0}));
    EXPECT_EQ(position(frames[129]), (std::pair {0, 0}));
}

TEST(
    Player,
    PositionJump)
{
    const auto frames = render(make_song(two_orders("00 --- . B01\n")), 3);
    EXPECT_EQ(position(frames[2]), (std::pair {1, 0}));
}

TEST(
    Player,
    PatternBreak)
{
    const auto frames = render(make_song(two_orders("00 --- . D05\n")), 3);
    EXPECT_EQ(position(frames[2]), (std::pair {1, 5}));
}

TEST(
    Player,
    PatternBreakOnLastOrderWraps)
{
    const auto frames = render(make_song(two_orders("00 --- . B01\n", "", "00 --- . D07\n")), 4);
    EXPECT_EQ(position(frames[3]), (std::pair {0, 7}));
}

TEST(
    Player,
    JumpAndBreakCombine)
{
    const auto frames = render(make_song(two_orders("00 --- . B00\n", "00 --- . D03\n")), 3);
    EXPECT_EQ(position(frames[2]), (std::pair {0, 3}));
}

TEST(
    Player,
    HighestChannelWins)
{
    const auto frames = render(make_song(two_orders("00 --- . B01\n", "00 --- . B00\n")), 3);
    EXPECT_EQ(position(frames[2]), (std::pair {0, 0}));
}

TEST(
    Player,
    OutOfRangeJumpAndBreakGoToZero)
{
    auto frames = render(make_song(two_orders("00 --- . B09\n")), 3);
    EXPECT_EQ(position(frames[2]), (std::pair {0, 0}));
    frames = render(make_song(two_orders("00 --- . D40\n")), 3);
    EXPECT_EQ(position(frames[2]), (std::pair {1, 0}));
}
