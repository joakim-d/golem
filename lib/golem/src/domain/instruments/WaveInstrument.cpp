#include <golem/domain/instruments/WaveInstrument.h>

#include <cstddef>

namespace domain {

bool WaveInstrument::lengthEnable() const
{
    return m_length_enable;
}

void WaveInstrument::setLengthEnable(bool value)
{
    m_length_enable = value;
}

unsigned WaveInstrument::volume() const
{
    return m_volume;
}

void WaveInstrument::setVolume(unsigned value)
{
    m_volume = value;
}

unsigned WaveInstrument::lengthTimer() const
{
    return m_length_timer;
}

void WaveInstrument::setLengthTimer(unsigned value)
{
    m_length_timer = value;
}

void WaveInstrument::setSamples(const std::array<uint8_t, 32>& samples)
{
    m_samples = samples;
}

void WaveInstrument::setSample(unsigned index, uint8_t value)
{
    m_samples[index] = value;
}

const std::array<uint8_t, 32>& WaveInstrument::getSamples() const
{
    return m_samples;
}

std::array<uint8_t, 16> WaveInstrument::getRawSamples() const
{
    std::array<uint8_t, 16> raw_samples;
    for (size_t i = 0; i < raw_samples.size(); ++i) {
        raw_samples[i] = m_samples[i * 2] << 4 | m_samples[i * 2 + 1];
    }
    return raw_samples;
}

uint8_t WaveInstrument::getSample(unsigned index) const
{
    return m_samples[index];
}

bool WaveInstrument::operator!=(const WaveInstrument& other) const
{
    return m_length_enable != other.m_length_enable
        || m_length_timer != other.m_length_timer
        || m_volume != other.m_volume
        || m_samples != other.m_samples;
}
}
