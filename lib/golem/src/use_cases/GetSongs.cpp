#include <golem/use_cases/GetSongs.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>
namespace use_cases
{

GetSongs::GetSongs(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

const std::vector<domain::Song>& GetSongs::operator()()
{
    return m_project_repository.getProject().getSongs();
}

}
