#pragma once

#include <golem/domain/Note.h>
#include <golem/domain/Register.h>

#include <golem/utils/observable.h>

#include <array>
#include <cstdint>

class IAudioProcessingUnit
{
public:
    utils::observable<> tick;

    virtual ~IAudioProcessingUnit() = default;

    virtual void
    updateRegister(domain::Register reg, uint8_t value, uint8_t mask = 0xFF)
        = 0;

    virtual void updateWaveRam(const std::array<uint8_t, 16>& wave_ram) = 0;
};
