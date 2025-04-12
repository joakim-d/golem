#include <golem/use_cases/SetInstrumentName.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

SetInstrumentName::SetInstrumentName(
    domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void SetInstrumentName::operator()(size_t song_index, std::string name)
{
    auto& project = m_project_repository.getProject();

    auto instrument = project.instrument(song_index);
    if (instrument == nullptr)
    {
        return;
    }

    instrument->setName(name);
}

}
