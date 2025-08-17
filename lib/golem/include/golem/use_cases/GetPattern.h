#pragma once

#include <cstddef>

namespace domain
{
class Pattern;
class IProjectRepository;
}

namespace use_cases
{

class GetPattern
{
public:
    GetPattern(domain::IProjectRepository& project_repository);

    domain::Pattern* operator()(size_t index);

private:
    domain::IProjectRepository& m_project_repository;
};

}
