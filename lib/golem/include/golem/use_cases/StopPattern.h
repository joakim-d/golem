#pragma once

namespace core
{
class PatternPlayer;
}

namespace use_cases
{

class StopPattern
{
public:
    StopPattern(core::PatternPlayer& song_player);

    void execute();

private:
    core::PatternPlayer& m_pattern_player;
};

}
