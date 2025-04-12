#pragma once

#include <cstddef>

#include <string>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class SetSongName
{
public:
    SetSongName(domain::IProjectRepository& project_repository);

    void operator()(size_t song_index, std::string name);

private:
    domain::IProjectRepository& m_project_repository;
};

}
