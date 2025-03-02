#include <golem/use_cases/GetInstruments.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

GetInstruments::GetInstruments(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

const std::vector<domain::Instrument>& GetInstruments::operator()()
{
    return m_project_repository.getProject().getInstruments();
}

}
