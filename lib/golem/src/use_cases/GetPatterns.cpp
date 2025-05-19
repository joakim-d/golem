#include <golem/use_cases/GetPatterns.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

GetPatterns::GetPatterns(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

const std::vector<domain::Pattern>& GetPatterns::operator()()
{
    const auto& patterns = m_project_repository.getProject().getPatterns();

    return patterns;
}

}
