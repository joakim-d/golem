#include <golem/domain/instruments/Instrument.h>

namespace domain {

Instrument::Instrument()
{
}

std::string Instrument::name() const
{
    return m_name;
}

void Instrument::setName(const std::string& name)
{
    m_name = name;
}

PulseInstrument& Instrument::pulse()
{
    return m_pulse;
}

const PulseInstrument& Instrument::pulse() const
{
    return m_pulse;
}

WaveInstrument& Instrument::wave()
{
    return m_wave;
}

const WaveInstrument& Instrument::wave() const
{
    return m_wave;
}

}
