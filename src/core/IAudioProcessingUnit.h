#pragma once

#include <core/Channel.h>
#include <core/Register.h>
#include <model/Note.h>

#include <array>
#include <cstdint>

class IAudioProcessingUnit {
public:
    virtual ~IAudioProcessingUnit() = default;

    virtual void updateRegister(
        Register reg,
        uint8_t value,
        uint8_t mask = 0xFF)
        = 0;

    virtual void updateWaveRam(
        const std::array<uint8_t, 16>& wave_ram)
        = 0;
};
