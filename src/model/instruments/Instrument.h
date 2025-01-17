#pragma once

#include <model/instruments/PulseInstrument.h>
#include <model/instruments/WaveInstrument.h>

#include <string>

namespace model {

class Instrument {
public:
    enum class Type {
        None,
        Pulse,
        Wave
    };

    Instrument();
    virtual ~Instrument() = default;

    std::string name() const;
    void setName(const std::string& name);

    Type type() const;
    void setType(Type type);

    PulseInstrument& pulse();
    const PulseInstrument& pulse() const;

    WaveInstrument& wave();
    const WaveInstrument& wave() const;

private:
    std::string m_name;
    Type m_type;
    PulseInstrument m_pulse;
    WaveInstrument m_wave;
};

}
