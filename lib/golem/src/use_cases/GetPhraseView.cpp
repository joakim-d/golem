#include <golem/use_cases/GetPhraseView.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>
#include <golem/domain/Song.h>

namespace use_cases
{

GetPhraseView::GetPhraseView(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

std::optional<domain::PhraseView> GetPhraseView::execute(
    size_t song_index,
    size_t track_index,
    size_t phrase_index)
{
    auto song = m_project_repository.getProject().song(song_index);

    if (song == nullptr)
    {
        return {};
    }

    if (track_index >= domain::Song::TRACK_COUNT)
    {
        return {};
    }

    const auto notes_per_phrase = song->notesPerPhrase();
    const auto& track = song->getTrack(track_index);
    const auto note_count = track.noteCount();

    if ((notes_per_phrase) * (phrase_index + 1) > note_count)
    {
        return {};
    }

    return domain::PhraseView {
        &track.note((notes_per_phrase) * (phrase_index)), notes_per_phrase
    };
}

}
