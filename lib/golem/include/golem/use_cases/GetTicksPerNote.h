#pragma once

#include <cstddef>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class GetTicksPerNote
{
public:
    GetTicksPerNote(domain::IProjectRepository& project_repository);

    unsigned int execute(size_t song_index);

private:
    domain::IProjectRepository& m_project_repository;
};

}
