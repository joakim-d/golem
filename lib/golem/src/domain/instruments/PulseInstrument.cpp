#include <golem/domain/instruments/PulseInstrument.h>

namespace domain {

PulseInstrument::DutyCycle PulseInstrument::dutyCycle() const
{
    return m_duty_cycle;
}

void PulseInstrument::setDutyCycle(PulseInstrument::DutyCycle duty_cycle)
{
    m_duty_cycle = duty_cycle;
}

unsigned PulseInstrument::envelopeInitialVolume() const
{
    return m_envelope_initial_volume;
}

void PulseInstrument::setEnvelopeInitialVolume(unsigned value)
{
    m_envelope_initial_volume = value;
}

bool PulseInstrument::envelopeIncrease() const
{
    return m_envelope_increase;
}

void PulseInstrument::setEnvelopeIncrease(bool increase)
{
    m_envelope_increase = increase;
}

unsigned PulseInstrument::envelopePace() const
{
    return m_envelope_pace;
}

void PulseInstrument::setEnvelopePace(unsigned pace)
{
    m_envelope_pace = pace;
}

void PulseInstrument::setLengthEnable(bool enable)
{
    m_length_enable = enable;
}

void PulseInstrument::setSweepIncrease(bool increase)
{
    m_sweep_increase = increase;
}

void PulseInstrument::setSweepSteps(unsigned steps)
{
    m_sweep_steps = steps;
}

void PulseInstrument::setSweepPace(unsigned pace)
{
    m_sweep_pace = pace;
}

unsigned PulseInstrument::sweepPace() const
{
    return m_sweep_pace;
}

unsigned PulseInstrument::sweepSteps() const
{
    return m_sweep_steps;
}

bool PulseInstrument::sweepIncrease() const
{
    return m_sweep_increase;
}

unsigned PulseInstrument::initialLengthTimer() const
{
    return m_initial_length_timer;
}

void PulseInstrument::setInitialLengthTimer(unsigned value)
{
    m_initial_length_timer = value;
}

bool PulseInstrument::lengthEnable() const
{
    return m_length_enable;
}

}
