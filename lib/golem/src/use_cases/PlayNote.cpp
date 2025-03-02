#include <golem/use_cases/PlayNote.h>

#include <golem/core/NotePlayer.h>

namespace use_cases
{

PlayNote::PlayNote(
    domain::IProjectRepository& project_repository,
    core::NotePlayer& note_player)
    : m_project_repository(project_repository)
    , m_note_player(note_player)
{
}

void PlayNote::execute(
    size_t instrument_index,
    domain::Channel channel,
    domain::NoteFrequency note)
{
    m_note_player.play(instrument_index, channel, note);
}

}
