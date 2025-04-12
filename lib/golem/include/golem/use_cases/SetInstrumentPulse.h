#pragma once

#include <cstddef>

namespace domain
{
class PulseInstrument;
class IProjectRepository;
}

namespace use_cases
{

class SetInstrumentPulse
{
public:
    SetInstrumentPulse(domain::IProjectRepository& project_repository);

    void
    operator()(size_t instrument_index, const domain::PulseInstrument& pulse);

private:
    domain::IProjectRepository& m_project_repository;
};

}
