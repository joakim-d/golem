#include <golem/use_cases/GetNotesPerPhrase.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace use_cases
{

GetNotesPerPhrase::GetNotesPerPhrase(
    domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

size_t GetNotesPerPhrase::execute(size_t song_index)
{
    auto song = m_project_repository.getProject().song(song_index);
    if (song == nullptr)
    {
        return 0;
    }

    return song->notesPerPhrase();
}

}
