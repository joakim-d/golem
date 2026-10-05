#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace golem {

// One write to an APU register, as seen by the hardware.
struct ApuWrite {
    std::uint16_t address;
    std::uint8_t value;
};

bool operator==(
    const ApuWrite& a,
    const ApuWrite& b);
bool operator!=(
    const ApuWrite& a,
    const ApuWrite& b);

namespace reg {
    constexpr std::uint16_t NR10 = 0xFF10;
    constexpr std::uint16_t NR11 = 0xFF11;
    constexpr std::uint16_t NR12 = 0xFF12;
    constexpr std::uint16_t NR13 = 0xFF13;
    constexpr std::uint16_t NR14 = 0xFF14;
    constexpr std::uint16_t NR21 = 0xFF16;
    constexpr std::uint16_t NR22 = 0xFF17;
    constexpr std::uint16_t NR23 = 0xFF18;
    constexpr std::uint16_t NR24 = 0xFF19;
    constexpr std::uint16_t NR30 = 0xFF1A;
    constexpr std::uint16_t NR31 = 0xFF1B;
    constexpr std::uint16_t NR32 = 0xFF1C;
    constexpr std::uint16_t NR33 = 0xFF1D;
    constexpr std::uint16_t NR34 = 0xFF1E;
    constexpr std::uint16_t NR41 = 0xFF20;
    constexpr std::uint16_t NR42 = 0xFF21;
    constexpr std::uint16_t NR43 = 0xFF22;
    constexpr std::uint16_t NR44 = 0xFF23;
    constexpr std::uint16_t NR50 = 0xFF24;
    constexpr std::uint16_t NR51 = 0xFF25;
    constexpr std::uint16_t NR52 = 0xFF26;
    constexpr std::uint16_t WAVE_RAM = 0xFF30; // 16 bytes, FF30..FF3F
} // namespace reg

// "NR22" for sound registers, "WAVE0".."WAVEF" for wave RAM, "FF15" (hex) otherwise.
std::string register_name(std::uint16_t address);

// Inverse of register_name(); also accepts any 4-digit hex address. Case-sensitive.
std::optional<std::uint16_t> register_address(std::string_view name);

} // namespace golem
