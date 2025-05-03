#pragma once

#include <cstddef>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class SetTicksPerNote
{
public:
    SetTicksPerNote(domain::IProjectRepository& project_repository);

    void operator()(size_t song_index, unsigned ticks_per_note);

private:
    domain::IProjectRepository& m_project_repository;
};

}
