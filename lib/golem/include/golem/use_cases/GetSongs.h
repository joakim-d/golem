#pragma once

#include <vector>

namespace domain
{
class IProjectRepository;
class Song;
}

namespace use_cases
{

class GetSongs
{
public:
    GetSongs(domain::IProjectRepository& project_repository);

    const std::vector<domain::Song>& operator()();

private:
    domain::IProjectRepository& m_project_repository;
};

}
