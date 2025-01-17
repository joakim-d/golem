#pragma once

#include <core/Register.h>
#include <cstdint>
#include <map>

class AudioRegisters {
public:
    AudioRegisters();

    unsigned getChannel1SweepPace() const;

    bool getChannel1SweepDirection() const;

    unsigned getChannel1SweepSteps() const;

    unsigned getChannel1DutyCycles() const;
    uint8_t getChannel1DutyCycleBitmask() const;

    unsigned getChannel1InitialLengthTimer() const;

    unsigned getChannel1InitialVolume() const;

    bool getChannel1EnvelopeDirection() const;

    unsigned getChannel1EnvelopeSweepPace() const;

    bool getChannel1LengthEnable() const;

    void setChannel1Period(unsigned period);
    unsigned getChannel1Period() const;

    unsigned getChannel2DutyCycles() const;
    uint8_t getChannel2DutyCycleBitmask() const;

    unsigned getChannel2InitialLengthTimer() const;

    unsigned getChannel2InitialVolume() const;

    bool getChannel2EnvelopeDirection() const;

    unsigned getChannel2EnvelopeSweepPace() const;

    bool getChannel2LengthEnable() const;

    void setChannel2Period(unsigned period);
    unsigned getChannel2Period() const;

    bool getChannel3On() const;

    bool getChannel3LengthEnable() const;

    unsigned getChannel3LengthTimer() const;

    unsigned getChannel3OutputLevel() const;

    unsigned getChannel3Period() const;

    bool channel3LengthEnable() const;

    void setRegisterValue(Register reg, uint8_t value, uint8_t mask);
    uint8_t getRegisterValue(Register reg) const;

    void setWaveRam(const std::array<uint8_t, 16>& wave_ram);
    std::array<uint8_t, 32> getFlatWaveRam() const;

private:
    std::map<Register, uint8_t> m_register_values;
    std::array<uint8_t, 16> m_wave_ram;
};
