#include <audio/apu_emulator/AudioRegisters.h>

AudioRegisters::AudioRegisters()
{
    for (auto reg = Register::NR10; reg <= Register::NR52;
         reg = static_cast<Register>(static_cast<unsigned>(reg) + 1)) {
        m_register_values[reg] = 0;
    }
}

unsigned AudioRegisters::getChannel1SweepPace() const
{
    const auto nr10 = m_register_values.at(Register::NR10);
    return (nr10 & 0b01110000) >> 4;
}

bool AudioRegisters::getChannel1SweepDirection() const
{
    const auto nr10 = m_register_values.at(Register::NR10);
    return !(nr10 & 0b00001000);
}

unsigned AudioRegisters::getChannel1SweepSteps() const
{
    const auto nr10 = m_register_values.at(Register::NR10);
    return nr10 & 0b00000111;
}

unsigned AudioRegisters::getChannel1DutyCycles() const
{
    const auto nr11 = m_register_values.at(Register::NR11);
    return (nr11 & 0b11000000) >> 6;
}

uint8_t AudioRegisters::getChannel1DutyCycleBitmask() const
{

    switch (getChannel1DutyCycles()) {
    case 0:
        return 0b00000001;
    case 1:
        return 0b10000001;
    case 2:
        return 0b10000111;
    case 3:
        return 0b01111110;
    }
    return 0;
}

unsigned AudioRegisters::getChannel1InitialLengthTimer() const
{
    const auto nr11 = m_register_values.at(Register::NR11);
    return (nr11 & 0b00111111);
}

unsigned AudioRegisters::getChannel1InitialVolume() const
{
    const auto nr12 = m_register_values.at(Register::NR12);
    return (nr12 >> 4);
}

bool AudioRegisters::getChannel1EnvelopeDirection() const
{
    const auto nr12 = m_register_values.at(Register::NR12);
    return (nr12 & 0x08);
}

unsigned AudioRegisters::getChannel1EnvelopeSweepPace() const
{
    const auto nr12 = m_register_values.at(Register::NR12);
    return nr12 & 0b00000111;
}

bool AudioRegisters::getChannel1LengthEnable() const
{
    const auto nr14 = m_register_values.at(Register::NR14);
    return (nr14 & 0x40);
}

void AudioRegisters::setChannel1Period(unsigned period)
{
    auto& nr13 = m_register_values[Register::NR13];
    auto& nr14 = m_register_values[Register::NR14];
    nr13 = period & 0xFF;
    nr14 = (nr14 & 0b11111000) | (period >> 8) & 0x7;
}

unsigned AudioRegisters::getChannel1Period() const
{
    const auto nr13 = m_register_values.at(Register::NR13);
    const auto nr14 = m_register_values.at(Register::NR14);
    return ((nr14 & 0b00000111) << 8) + nr13;
}

unsigned AudioRegisters::getChannel2DutyCycles() const
{
    const auto nr21 = m_register_values.at(Register::NR21);
    return (nr21 & 0b11000000) >> 6;
}

uint8_t AudioRegisters::getChannel2DutyCycleBitmask() const
{

    switch (getChannel2DutyCycles()) {
    case 0:
        return 0b00000001;
    case 1:
        return 0b10000001;
    case 2:
        return 0b10000111;
    case 3:
        return 0b01111110;
    }
    return 0;
}

unsigned AudioRegisters::getChannel2InitialLengthTimer() const
{
    const auto nr21 = m_register_values.at(Register::NR21);
    return (nr21 & 0b00111111);
}

unsigned AudioRegisters::getChannel2InitialVolume() const
{
    const auto nr22 = m_register_values.at(Register::NR22);
    return (nr22 >> 4);
}

bool AudioRegisters::getChannel2EnvelopeDirection() const
{
    const auto nr22 = m_register_values.at(Register::NR22);
    return (nr22 & 0x08);
}

unsigned AudioRegisters::getChannel2EnvelopeSweepPace() const
{
    const auto nr22 = m_register_values.at(Register::NR22);
    return nr22 & 0b00000111;
}

bool AudioRegisters::getChannel2LengthEnable() const
{
    const auto nr24 = m_register_values.at(Register::NR24);
    return (nr24 & 0x40);
}

void AudioRegisters::setChannel2Period(unsigned period)
{
    auto& nr23 = m_register_values[Register::NR23];
    auto& nr24 = m_register_values[Register::NR24];
    nr23 = period & 0xFF;
    nr24 = (nr24 & 0b11111000) | (period >> 8) & 0x7;
}

unsigned AudioRegisters::getChannel2Period() const
{
    const auto nr23 = m_register_values.at(Register::NR23);
    const auto nr24 = m_register_values.at(Register::NR24);
    return ((nr24 & 0b00000111) << 8) + nr23;
}

unsigned AudioRegisters::getChannel3Period() const
{
    const auto nr33 = m_register_values.at(Register::NR33);
    const auto nr34 = m_register_values.at(Register::NR34);
    return ((nr34 & 0b00000111) << 8) + nr33;
}

unsigned AudioRegisters::getChannel3OutputLevel() const
{
    const auto nr32 = m_register_values.at(Register::NR32);
    return (nr32 & 0b01100000) >> 5;
}

bool AudioRegisters::getChannel3On() const
{
    return m_register_values.at(Register::NR30) & 0b10000000;
}

bool AudioRegisters::getChannel3LengthEnable() const
{
    const auto nr34 = m_register_values.at(Register::NR34);
    return (nr34 & 0x40);
}

unsigned AudioRegisters::getChannel3LengthTimer() const
{
    return m_register_values.at(Register::NR31);
}

void AudioRegisters::setRegisterValue(
    Register reg,
    uint8_t value,
    uint8_t mask)
{
    auto register_value = m_register_values[reg] & (~mask);
    m_register_values[reg] = register_value | (value & mask);
}

void AudioRegisters::setWaveRam(
    const std::array<uint8_t, 16>& ram)
{
    m_wave_ram = ram;
}

std::array<uint8_t, 32> AudioRegisters::getFlatWaveRam() const
{
    std::array<uint8_t, 32> result;
    for (uint8_t i = 0; i < m_wave_ram.size(); ++i) {
        result[i * 2] = (m_wave_ram[i] & 0xF0) >> 4;
        result[i * 2 + 1] = (m_wave_ram[i] & 0xF);
    }
    return result;
}
