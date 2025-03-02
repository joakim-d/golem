#include <golem/use_cases/CreateNewProject.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

CreateNewProject::CreateNewProject(domain::IProjectRepository& repository)
    : m_project_repository(repository)
{
}

void CreateNewProject::execute()
{
    auto& project = m_project_repository.getProject();
    project = domain::Project {};
}

}
