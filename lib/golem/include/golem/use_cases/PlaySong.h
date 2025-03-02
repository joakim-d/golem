#pragma once

#include <cstddef>

namespace core
{
class SongPlayer;
}

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class PlaySong
{
public:
    PlaySong(
        domain::IProjectRepository& project_repository,
        core::SongPlayer& song_player);

    void execute(size_t song_index);

private:
    domain::IProjectRepository& m_project_repository;
    core::SongPlayer& m_song_player;
};

}
