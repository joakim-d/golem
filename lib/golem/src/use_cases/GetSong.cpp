#include <golem/use_cases/GetSong.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

GetSong::GetSong(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

const domain::Song* GetSong::operator()(size_t song_index)
{
    return m_project_repository.getProject().song(song_index);
}

}
