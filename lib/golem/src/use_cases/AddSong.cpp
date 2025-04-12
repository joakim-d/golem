#include <golem/use_cases/AddSong.h>

#include <golem/domain/IProjectRepository.h>

#include <golem/domain/Project.h>

namespace use_cases
{

AddSong::AddSong(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void AddSong::execute()
{
    domain::Song song {};

    std::string name { "Song " };
    name += std::to_string(
        m_project_repository.getProject().getSongs().size() + 1);

    song.setName(name);
    m_project_repository.getProject().addSong(song);
}

}
