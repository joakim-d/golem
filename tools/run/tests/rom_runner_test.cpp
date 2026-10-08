#include "golem/rom_runner.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <ostream>

#include <initializer_list>

using namespace golem;

namespace golem {

// Readable emulator names in GoogleTest output.
void PrintTo(
    Emulator emulator,
    std::ostream* out)
{
    *out << emulator_name(emulator);
}

} // namespace golem

namespace {

// Builds a 32 KiB ROM-only cartridge whose code starts at $0150.
class RomBuilder {
public:
    RomBuilder()
        : rom_(
              0x8000,
              0x00)
    {
        rom_[0x0040] = 0xD9; // VBlank vector: reti
        const std::uint8_t entry[] = {0x00, 0xC3, 0x50, 0x01}; // nop; jp $0150
        std::copy(std::begin(entry), std::end(entry), rom_.begin() + 0x100);
    }

    RomBuilder& code(std::initializer_list<std::uint8_t> bytes)
    {
        for (auto byte : bytes) {
            rom_[pc_++] = byte;
        }
        return *this;
    }

    // Writes `bytes` at `address`, independently of the code cursor.
    RomBuilder& at(
        std::uint16_t address,
        std::initializer_list<std::uint8_t> bytes)
    {
        for (auto byte : bytes) {
            rom_[address++] = byte;
        }
        return *this;
    }

    std::uint16_t pc() const
    {
        return pc_;
    }

    std::vector<std::uint8_t> build() const
    {
        auto rom = rom_;
        std::uint8_t checksum = 0;
        for (std::size_t i = 0x134; i <= 0x14C; ++i) {
            checksum = static_cast<std::uint8_t>(checksum - rom[i] - 1);
        }
        rom[0x14D] = checksum;
        return rom;
    }

private:
    std::vector<std::uint8_t> rom_;
    std::uint16_t pc_ = 0x150;
};

constexpr std::uint8_t kLdA = 0x3E; // ld a, n
constexpr std::uint8_t kLdh = 0xE0; // ldh [$FF00 + n], a
constexpr std::uint8_t kMarker = kFrameMarker & 0xFF;
constexpr std::uint8_t kEndMarker = kCallEndMarker & 0xFF;

// Writes NR52 before the first marker, NR50 in frame 0, then waits for VBlank and writes
// a marker and NR51 = frame number in every following frame.
std::vector<std::uint8_t> counting_rom()
{
    RomBuilder rom;
    rom.code({0xF3}) // di
        .code({kLdA, 0x80, kLdh, 0x26}) // NR52 = $80, before the first marker
        .code({kLdh, kMarker}) // frame 0
        .code({kLdA, 0x77, kLdh, 0x24}) // NR50 = $77
        .code({0x06, 0x00}) // ld b, 0
        .code({kLdA, 0x01, kLdh, 0xFF}) // IE = VBlank
        .code({0xFB}); // ei
    const auto loop = rom.pc();
    rom.code({0x76}) // halt
        .code({kLdh, kMarker}) // next frame
        .code({0x04, 0x78, kLdh, 0x25}); // inc b; ld a, b; NR51 = b
    const auto offset = static_cast<std::int8_t>(loop - (rom.pc() + 2));
    rom.code({0x18, static_cast<std::uint8_t>(offset)}); // jr loop
    return rom.build();
}

} // namespace

TEST(
    RomRunner,
    PeanutGbIsAlwaysAvailableFirst)
{
    const auto emulators = available_emulators();
    ASSERT_FALSE(emulators.empty());
    EXPECT_EQ(emulators.front(), Emulator::PeanutGb);
#ifdef GOLEM_HAS_SAMEBOY
    EXPECT_EQ(emulators, (std::vector<Emulator> {Emulator::PeanutGb, Emulator::SameBoy}));
#else
    EXPECT_EQ(emulators, (std::vector<Emulator> {Emulator::PeanutGb}));
#endif
}

TEST(
    RomRunner,
    EmulatorNames)
{
    EXPECT_EQ(emulator_name(Emulator::PeanutGb), "Peanut-GB");
    EXPECT_EQ(emulator_name(Emulator::SameBoy), "SameBoy");
}

TEST(
    RomRunner,
    PeanutGbRejectsABadHeaderChecksum)
{
    auto rom = counting_rom();
    rom[0x14D] ^= 0xFF;
    EXPECT_THROW(run_rom(rom, 1, Emulator::PeanutGb), RunError);
}

#ifndef GOLEM_HAS_SAMEBOY
TEST(
    RomRunner,
    SameBoyIsRejectedWhenNotBuiltIn)
{
    EXPECT_THROW(run_rom(counting_rom(), 1, Emulator::SameBoy), RunError);
}
#endif

// The same behaviour in every emulator built in.
class EmulatorTest : public ::testing::TestWithParam<Emulator> {
protected:
    void SetUp() override
    {
        const auto emulators = available_emulators();
        if (std::find(emulators.begin(), emulators.end(), GetParam()) == emulators.end()) {
            GTEST_SKIP() << emulator_name(GetParam()) << " is not built in";
        }
    }

    RunResult run(
        const std::vector<std::uint8_t>& rom,
        std::uint32_t frames) const
    {
        return run_rom(rom, frames, GetParam());
    }
};

INSTANTIATE_TEST_SUITE_P(
    Emulators,
    EmulatorTest,
    ::testing::Values(
        Emulator::PeanutGb,
        Emulator::SameBoy),
    [](const ::testing::TestParamInfo<Emulator>& info) {
        return info.param == Emulator::PeanutGb ? std::string("PeanutGb") : std::string("SameBoy");
    });

TEST_P(
    EmulatorTest,
    SplitsWritesIntoFramesAtMarkers)
{
    EXPECT_EQ(
        run(counting_rom(), 4).trace,
        make_trace({
            {{reg::NR50, 0x77}},
            {{reg::NR51, 0x01}},
            {{reg::NR51, 0x02}},
            {{reg::NR51, 0x03}},
        }));
}

TEST_P(
    EmulatorTest,
    ZeroFramesIsAnEmptyTrace)
{
    const auto result = run(counting_rom(), 0);
    EXPECT_EQ(result.trace, Trace {});
    EXPECT_TRUE(result.frame_cycles.empty());
}

TEST_P(
    EmulatorTest,
    FailsWhenFramesDoNotComplete)
{
    RomBuilder rom;
    rom.code({kLdh, kMarker, 0x18, 0xFE}); // One marker, then jr to itself forever.
    EXPECT_THROW(run(rom.build(), 2), RunError);
}

TEST_P(
    EmulatorTest,
    RejectsRomsSmallerThanTheHeader)
{
    EXPECT_THROW(run(std::vector<std::uint8_t>(0x100), 1), RunError);
}

TEST_P(
    EmulatorTest,
    FramesWithoutEndMarkerHaveNoCycles)
{
    const auto cycles = run(counting_rom(), 4).frame_cycles;
    ASSERT_EQ(cycles.size(), 4u);
    for (const auto& frame : cycles) {
        EXPECT_EQ(frame, std::nullopt);
    }
}

TEST_P(
    EmulatorTest,
    MeasuresCyclesFromFrameMarkerToEndMarker)
{
    RomBuilder rom;
    rom.at(0x0200, {0x00, 0xC9}) // Subroutine: nop (4), ret (16).
        .code({kLdh, kMarker}) // Frame 0
        .code({0x00, 0x00, 0x00}) // 3 x nop: 12 cycles
        .code({kLdh, kEndMarker})
        .code({kLdh, kMarker}) // Frame 1
        .code({0xCD, 0x00, 0x02}) // call $0200: 24 + 4 + 16 = 44 cycles
        .code({kLdh, kEndMarker})
        .code({kLdh, kMarker}) // Frame 2: no end marker
        .code({0x00})
        .code({kLdh, kMarker}) // Frame 3 completes frame 2
        .code({0x18, 0xFE}); // jr to itself
    const auto result = run(rom.build(), 3);
    EXPECT_EQ(result.trace, make_trace({{}, {}, {}})); // Markers are not APU writes.
    ASSERT_EQ(result.frame_cycles.size(), 3u);
    EXPECT_EQ(result.frame_cycles[0], 12u);
    EXPECT_EQ(result.frame_cycles[1], 44u);
    EXPECT_EQ(result.frame_cycles[2], std::nullopt);
}
