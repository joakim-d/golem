#pragma once

#include <cstddef>

namespace core
{
class PatternPlayer;
}

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class PlayPattern
{
public:
    PlayPattern(
        domain::IProjectRepository& project_repository,
        core::PatternPlayer& pattern_player);

    void
    execute(size_t ticks_per_note, size_t track_index, size_t pattern_index);

private:
    domain::IProjectRepository& m_project_repository;
    core::PatternPlayer& m_pattern_player;
};

}
