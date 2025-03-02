#pragma once

#include <cstddef>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class GetNotesPerPhrase
{
public:
    GetNotesPerPhrase(domain::IProjectRepository& project_repository);

    size_t execute(size_t song_index);

private:
    domain::IProjectRepository& m_project_repository;
};

} // namespace use_cases
