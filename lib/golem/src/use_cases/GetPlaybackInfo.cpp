#include <golem/use_cases/GetPlaybackInfo.h>

#include <golem/core/SongPlayer.h>

namespace use_cases
{

GetPlaybackInfo::GetPlaybackInfo(core::SongPlayer& song_player)
    : m_song_player(song_player)
{
}

const domain::PlaybackInfo& GetPlaybackInfo::execute()
{
    return m_song_player.getPlaybackInfo();
}

}
