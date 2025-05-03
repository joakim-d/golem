#include <golem/use_cases/SetTicksPerNote.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

SetTicksPerNote::SetTicksPerNote(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void SetTicksPerNote::operator()(size_t song_index, unsigned ticks_per_note)
{
    auto song = m_project_repository.getProject().song(song_index);
    if (song == nullptr)
    {
        return;
    }

    song->setTicksPerNote(ticks_per_note);
}

}
