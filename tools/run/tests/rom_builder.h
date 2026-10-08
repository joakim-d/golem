#pragma once

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <vector>

namespace golem::test {

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

} // namespace golem::test
