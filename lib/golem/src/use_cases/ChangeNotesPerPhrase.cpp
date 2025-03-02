#include <golem/use_cases/ChangeNotesPerPhrase.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>
#include <golem/domain/Song.h>

namespace use_cases {

ChangeNotesPerPhrase::ChangeNotesPerPhrase(domain::IProjectRepository& repository)
    : m_project_repository(repository)
{
}

void ChangeNotesPerPhrase::execute(size_t song_index, unsigned int notes_per_phrase)
{
    auto song = m_project_repository.getProject().song(song_index);
    if (song == nullptr) {
        return;
    }
    song->setNotesPerPhrase(notes_per_phrase);
}

}
