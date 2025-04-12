#include <golem/use_cases/GetTicksPerNote.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

GetTicksPerNote::GetTicksPerNote(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

unsigned int GetTicksPerNote::execute(size_t song_index)
{
    auto& project = m_project_repository.getProject();

    auto song = project.song(song_index);
    if (song == nullptr)
    {
        return 0;
    }

    return song->ticksPerNote();
}

}
