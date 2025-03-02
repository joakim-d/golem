#include <golem/use_cases/AddInstrument.h>

#include <golem/domain/IProjectRepository.h>

#include <golem/domain/Project.h>

namespace use_cases
{

AddInstrument::AddInstrument(
    domain::IProjectRepository& project_repository,
    domain::events::ProjectDomainEventProcessor& events_processor)
    : m_project_repository(project_repository)
    , m_event_processor(events_processor)
{
}

void AddInstrument::execute()
{
    auto& project = m_project_repository.getProject();
    project.addInstrument(domain::Instrument {});

    m_event_processor.publishEvent(domain::events::InstrumentsChanged {});
}

}
