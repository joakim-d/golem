#pragma once

#include <golem/domain/PlaybackInfo.h>

namespace core
{
class SongPlayer;
}

namespace use_cases
{

class GetPlaybackInfo
{
public:
    GetPlaybackInfo(core::SongPlayer& song_player);

    const domain::PlaybackInfo& execute();

private:
    core::SongPlayer& m_song_player;
};

}
