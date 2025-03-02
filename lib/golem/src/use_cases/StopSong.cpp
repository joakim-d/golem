#include <golem/use_cases/StopSong.h>

#include <golem/core/SongPlayer.h>

namespace use_cases
{

StopSong::StopSong(core::SongPlayer& song_player)
    : m_song_player(song_player)
{
}

void StopSong::execute()
{
    m_song_player.stop();
}

}
