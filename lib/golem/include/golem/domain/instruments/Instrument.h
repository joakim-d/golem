#pragma once

#include <golem/domain/instruments/Adsr.h>
#include <golem/domain/instruments/PulseInstrument.h>
#include <golem/domain/instruments/WaveInstrument.h>

#include <string>

namespace domain
{

class Instrument
{
public:
    Instrument();
    virtual ~Instrument() = default;

    std::string name() const;
    void setName(const std::string& name);

    PulseInstrument& pulse();
    const PulseInstrument& pulse() const;

    WaveInstrument& wave();
    const WaveInstrument& wave() const;

    Adsr& adsr();
    const Adsr& adsr() const;

private:
    std::string m_name;
    PulseInstrument m_pulse;
    WaveInstrument m_wave;
    Adsr m_adsr;
};

}
