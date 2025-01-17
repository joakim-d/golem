#pragma once

namespace model {

class PulseInstrument {
public:
    enum class DutyCycle {
        Percent_12_5,
        Percent_25,
        Percent_50,
        Percent_75
    };

    unsigned sweepPace() const;
    void setSweepPace(unsigned value);

    bool sweepIncrease() const;
    void setSweepIncrease(bool value);

    unsigned sweepSteps() const;
    void setSweepSteps(unsigned steps);

    /// Wave form
    DutyCycle dutyCycle() const;
    void setDutyCycle(DutyCycle duty_cycle);

    /// Envelope initial value 0-15
    unsigned envelopeInitialVolume() const;
    void setEnvelopeInitialVolume(unsigned value);

    /// Increase or decrease
    bool envelopeIncrease() const;
    void setEnvelopeIncrease(bool increase);

    /// Envelope increase pace 0-7
    unsigned envelopePace() const;
    void setEnvelopePace(unsigned pace);

    unsigned initialLengthTimer() const;
    void setInitialLengthTimer(unsigned value);

    bool lengthEnable() const;
    void setLengthEnable(bool enable);

private:
    unsigned m_sweep_pace { 0 };
    unsigned m_sweep_steps { 0 };

    DutyCycle m_duty_cycle { DutyCycle::Percent_12_5 };
    unsigned m_initial_length_timer { 0 };

    unsigned m_envelope_initial_volume { 8 };
    unsigned m_envelope_pace { 0 };

    bool m_envelope_increase { true };
    bool m_sweep_increase { true };
    bool m_length_enable { true };
};

}
