#include <golem/use_cases/AddSong.h>

#include <golem/domain/IProjectRepository.h>

#include <golem/domain/Project.h>

namespace use_cases {

AddSong::AddSong(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void AddSong::execute()
{
    m_project_repository.getProject().addSong(domain::Song {});
}

}
