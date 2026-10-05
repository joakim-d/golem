#include "golem/apu.h"

#include <array>
#include <cstdio>

namespace golem {

namespace {

    struct NamedRegister {
        std::uint16_t address;
        const char* name;
    };

    constexpr std::array<NamedRegister, 21> kRegisters = {{
        {reg::NR10, "NR10"}, {reg::NR11, "NR11"}, {reg::NR12, "NR12"}, {reg::NR13, "NR13"},
        {reg::NR14, "NR14"}, {reg::NR21, "NR21"}, {reg::NR22, "NR22"}, {reg::NR23, "NR23"},
        {reg::NR24, "NR24"}, {reg::NR30, "NR30"}, {reg::NR31, "NR31"}, {reg::NR32, "NR32"},
        {reg::NR33, "NR33"}, {reg::NR34, "NR34"}, {reg::NR41, "NR41"}, {reg::NR42, "NR42"},
        {reg::NR43, "NR43"}, {reg::NR44, "NR44"}, {reg::NR50, "NR50"}, {reg::NR51, "NR51"},
        {reg::NR52, "NR52"},
    }};

    constexpr std::string_view kWavePrefix = "WAVE";
    constexpr const char* kHexDigits = "0123456789ABCDEF";

    std::optional<int> hex_digit(char c)
    {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        return std::nullopt;
    }

} // namespace

bool operator==(
    const ApuWrite& a,
    const ApuWrite& b)
{
    return a.address == b.address && a.value == b.value;
}

bool operator!=(
    const ApuWrite& a,
    const ApuWrite& b)
{
    return !(a == b);
}

std::string register_name(std::uint16_t address)
{
    for (const auto& r : kRegisters) {
        if (r.address == address) {
            return r.name;
        }
    }
    if (address >= reg::WAVE_RAM && address < reg::WAVE_RAM + 16) {
        return std::string(kWavePrefix) + kHexDigits[address - reg::WAVE_RAM];
    }
    char hex[5];
    std::snprintf(hex, sizeof hex, "%04X", address);
    return hex;
}

std::optional<std::uint16_t> register_address(std::string_view name)
{
    for (const auto& r : kRegisters) {
        if (name == r.name) {
            return r.address;
        }
    }
    if (name.size() == kWavePrefix.size() + 1
        && name.substr(0, kWavePrefix.size()) == kWavePrefix) {
        if (const auto digit = hex_digit(name.back())) {
            return static_cast<std::uint16_t>(reg::WAVE_RAM + *digit);
        }
        return std::nullopt;
    }
    if (name.size() != 4) {
        return std::nullopt;
    }
    std::uint16_t address = 0;
    for (char c : name) {
        const auto digit = hex_digit(c);
        if (!digit) {
            return std::nullopt;
        }
        address = static_cast<std::uint16_t>(address << 4 | *digit);
    }
    return address;
}

} // namespace golem
