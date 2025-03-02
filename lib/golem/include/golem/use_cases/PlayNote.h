#pragma once

#include <golem/domain/Channel.h>
#include <golem/domain/Note.h>

namespace core
{
class NotePlayer;
}

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class PlayNote
{
public:
    PlayNote(
        domain::IProjectRepository& project_repository,
        core::NotePlayer& note_player);

    void execute(
        size_t instrument_index,
        domain::Channel channel,
        domain::NoteFrequency note);

private:
    domain::IProjectRepository& m_project_repository;
    core::NotePlayer& m_note_player;
};

}
