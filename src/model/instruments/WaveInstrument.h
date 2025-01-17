#pragma once

#include <array>
#include <cstdint>

namespace model {

class WaveInstrument {
public:
    unsigned lengthTimer() const;
    void setLengthTimer(unsigned value);

    unsigned volume() const;
    void setVolume(unsigned value);

    bool lengthEnable() const;
    void setLengthEnable(bool enable);

    void setSamples(const std::array<uint8_t, 32>& samples);
    void setSample(unsigned index, uint8_t value);

    const std::array<uint8_t, 32>& getSamples() const;
    std::array<uint8_t, 16> getRawSamples() const;
    uint8_t getSample(unsigned index) const;

    bool operator!=(const WaveInstrument& other) const;

private:
    unsigned m_length_timer;
    unsigned m_volume;
    bool m_length_enable;
    std::array<uint8_t, 32> m_samples;
};

}
