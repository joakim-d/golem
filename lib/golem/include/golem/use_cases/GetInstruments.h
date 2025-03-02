#pragma once

#include <vector>

namespace domain
{
class Instrument;
class IProjectRepository;
}

namespace use_cases
{

class GetInstruments
{
public:
    GetInstruments(domain::IProjectRepository& project_repository);

    const std::vector<domain::Instrument>& operator()();

private:
    domain::IProjectRepository& m_project_repository;
};

}
