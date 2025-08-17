#include <golem/use_cases/PlayPattern.h>

#include <golem/core/PatternPlayer.h>

namespace use_cases
{

PlayPattern::PlayPattern(
    domain::IProjectRepository& project_repository,
    core::PatternPlayer& pattern_player)
    : m_project_repository(project_repository)
    , m_pattern_player(pattern_player)
{
}

void PlayPattern::execute(
    size_t ticks_per_note,
    size_t track_index,
    size_t pattern_index)
{
    m_pattern_player.play(ticks_per_note, track_index, pattern_index);
}

}
