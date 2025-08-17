#include <golem/use_cases/GetPattern.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

GetPattern::GetPattern(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

domain::Pattern* GetPattern::operator()(size_t pattern_index)
{
    return m_project_repository.getProject().pattern(pattern_index);
}

}
