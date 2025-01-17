#include <model/instruments/Instrument.h>

namespace model {

Instrument::Instrument()
    : m_type(Instrument::Type::None)
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

Instrument::Type Instrument::type() const
{
    return m_type;
}

void Instrument::setType(Instrument::Type type)
{
    if (type == m_type)
        return;

    m_type = type;
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
