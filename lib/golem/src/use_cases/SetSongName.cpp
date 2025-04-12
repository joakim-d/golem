#include <golem/use_cases/SetSongName.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

SetSongName::SetSongName(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void SetSongName::operator()(size_t song_index, std::string name)
{
    auto& project = m_project_repository.getProject();

    auto song = project.song(song_index);
    if (song == nullptr)
    {
        return;
    }

    song->setName(name);
}

}
