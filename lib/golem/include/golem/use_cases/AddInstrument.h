#pragma once

#include <golem/domain/events/ProjectDomainEventProcessor.h>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class AddInstrument
{
public:
    AddInstrument(
        domain::IProjectRepository& project_repository,
        domain::events::ProjectDomainEventProcessor& event_processor);

    void execute();

private:
    domain::IProjectRepository& m_project_repository;
    domain::events::ProjectDomainEventProcessor& m_event_processor;
};

}
