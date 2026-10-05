#include "golem/apu.h"

#include <gtest/gtest.h>

using namespace golem;

TEST(
    Apu,
    NamesSoundRegisters)
{
    EXPECT_EQ(register_name(reg::NR10), "NR10");
    EXPECT_EQ(register_name(reg::NR14), "NR14");
    EXPECT_EQ(register_name(reg::NR21), "NR21");
    EXPECT_EQ(register_name(reg::NR24), "NR24");
    EXPECT_EQ(register_name(reg::NR30), "NR30");
    EXPECT_EQ(register_name(reg::NR34), "NR34");
    EXPECT_EQ(register_name(reg::NR41), "NR41");
    EXPECT_EQ(register_name(reg::NR44), "NR44");
    EXPECT_EQ(register_name(reg::NR50), "NR50");
    EXPECT_EQ(register_name(reg::NR51), "NR51");
    EXPECT_EQ(register_name(reg::NR52), "NR52");
}

TEST(
    Apu,
    NamesWaveRam)
{
    EXPECT_EQ(register_name(0xFF30), "WAVE0");
    EXPECT_EQ(register_name(0xFF39), "WAVE9");
    EXPECT_EQ(register_name(0xFF3F), "WAVEF");
}

TEST(
    Apu,
    NamesUnknownAddressesInHex)
{
    EXPECT_EQ(register_name(0xFF15), "FF15");
    EXPECT_EQ(register_name(0xFF1F), "FF1F");
    EXPECT_EQ(register_name(0xFF27), "FF27");
    EXPECT_EQ(register_name(0x0000), "0000");
    EXPECT_EQ(register_name(0xFF40), "FF40");
}

TEST(
    Apu,
    AddressRoundTripsThroughName)
{
    for (std::uint32_t address = 0xFF00; address <= 0xFF4F; ++address) {
        const auto name = register_name(static_cast<std::uint16_t>(address));
        EXPECT_EQ(register_address(name), address) << name;
    }
}

TEST(
    Apu,
    ParsesHexAddresses)
{
    EXPECT_EQ(register_address("FF17"), reg::NR22);
    EXPECT_EQ(register_address("1234"), 0x1234);
}

TEST(
    Apu,
    RejectsUnknownNames)
{
    EXPECT_EQ(register_address(""), std::nullopt);
    EXPECT_EQ(register_address("NR15"), std::nullopt);
    EXPECT_EQ(register_address("WAVEG"), std::nullopt);
    EXPECT_EQ(register_address("FF1"), std::nullopt);
    EXPECT_EQ(register_address("FFXX"), std::nullopt);
    EXPECT_EQ(register_address("nr22"), std::nullopt);
}

TEST(
    Apu,
    ComparesWrites)
{
    EXPECT_EQ((ApuWrite {reg::NR22, 0xF3}), (ApuWrite {reg::NR22, 0xF3}));
    EXPECT_NE((ApuWrite {reg::NR22, 0xF3}), (ApuWrite {reg::NR22, 0xF1}));
    EXPECT_NE((ApuWrite {reg::NR22, 0xF3}), (ApuWrite {reg::NR12, 0xF3}));
}
