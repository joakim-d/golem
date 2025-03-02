#pragma once

#include <golem/domain/Channel.h>

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

class StopNote
{
public:
    StopNote(
        domain::IProjectRepository& project_repository,
        core::NotePlayer& note_player);

    void execute(domain::Channel channel);

private:
    domain::IProjectRepository& m_project_repository;
    core::NotePlayer& m_note_player;
};

}
