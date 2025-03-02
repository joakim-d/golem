#pragma once

namespace core
{
class SongPlayer;
}

namespace use_cases
{

class StopSong
{
public:
    StopSong(core::SongPlayer& song_player);

    void execute();

private:
    core::SongPlayer& m_song_player;
};

}
