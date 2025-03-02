#include <golem/use_cases/AddNote.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

#include <cmath>

namespace use_cases
{

AddNote::AddNote(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void AddNote::execute(
    size_t song_index,
    size_t track_index,
    size_t note_index,
    domain::Note note)
{
    auto& project = m_project_repository.getProject();

    auto song = project.song(song_index);
    if (song == nullptr)
    {
        return;
    }

    if (track_index >= domain::Song::TRACK_COUNT)
    {
        return;
    }

    auto& track = song->getTrack(track_index);

    // Cleaning any notes that are overlapping with the new
    // one
    const auto duration = note.duration();
    for (auto i = note_index + 1; i < note_index + duration; ++i)
    {
        track.clearNote(i);
    }

    // Reduce the duration of the note before the inserted one
    for (int i = int(note_index) - 1; i >= 0; --i)
    {
        auto note = track.note(i);
        if (note.has_value() && note->duration() + i > note_index)
        {
            note->setDuration(note_index - i);
            track.setNote(i, *note);
        }
    }

    track.setNote(note_index, note);
}

}
