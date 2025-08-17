#pragma once

#include <cstddef>

namespace domain
{
class IProjectRepository;
class Song;
}

namespace use_cases
{

class GetSong
{
public:
    GetSong(domain::IProjectRepository& project_repository);

    const domain::Song* operator()(size_t song_index);

private:
    domain::IProjectRepository& m_project_repository;
};

}
