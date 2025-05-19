#pragma once

#include <vector>

namespace domain
{
class Pattern;
class IProjectRepository;
}

namespace use_cases
{

class GetPatterns
{
public:
    GetPatterns(domain::IProjectRepository& project_repository);

    const std::vector<domain::Pattern>& operator()();

private:
    domain::IProjectRepository& m_project_repository;
};

}
