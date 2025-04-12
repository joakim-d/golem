#include <golem/use_cases/SetInstrumentPulse.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

SetInstrumentPulse::SetInstrumentPulse(
    domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void SetInstrumentPulse::operator()(
    size_t song_index,
    const domain::PulseInstrument& pulse)
{
    auto& project = m_project_repository.getProject();

    auto instrument = project.instrument(song_index);
    if (instrument == nullptr)
    {
        return;
    }

    instrument->pulse() = pulse;
}

}
