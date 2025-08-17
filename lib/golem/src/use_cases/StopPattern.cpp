#include <golem/use_cases/StopPattern.h>

#include <golem/core/PatternPlayer.h>

namespace use_cases
{

StopPattern::StopPattern(core::PatternPlayer& pattern_player)
    : m_pattern_player(pattern_player)
{
}

void StopPattern::execute()
{
    m_pattern_player.stop();
}

}
