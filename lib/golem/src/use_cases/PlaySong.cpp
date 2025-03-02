#include <golem/use_cases/PlaySong.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>
#include <golem/domain/Song.h>

#include <golem/core/SongPlayer.h>

namespace use_cases
{

PlaySong::PlaySong(
    domain::IProjectRepository& project_repository,
    core::SongPlayer& song_player)
    : m_project_repository(project_repository)
    , m_song_player(song_player)
{
}

void PlaySong::execute(size_t song_index)
{
    m_song_player.playSong(song_index);
}

}
