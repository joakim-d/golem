#include <golem/use_cases/StopNote.h>

#include <golem/core/NotePlayer.h>

namespace use_cases
{

StopNote::StopNote(
    domain::IProjectRepository& project_repository,
    core::NotePlayer& note_player)
    : m_project_repository(project_repository)
    , m_note_player(note_player)
{
}

void StopNote::execute(domain::Channel channel)
{
    m_note_player.stop(channel);
}

}
