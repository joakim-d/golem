#pragma once

#include <cstddef>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class CopyNotes
{
public:
    CopyNotes(domain::IProjectRepository& project_repository);

    void execute(
        size_t song_index,
        size_t track_index,
        size_t index_from,
        size_t copy_length,
        size_t index_to);

private:
    domain::IProjectRepository& m_project_repository;
};

} // namespace use_cases
