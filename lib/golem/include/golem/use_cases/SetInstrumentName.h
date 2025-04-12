#pragma once

#include <cstddef>

#include <string>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class SetInstrumentName
{
public:
    SetInstrumentName(domain::IProjectRepository& project_repository);

    void operator()(size_t instrument_index, std::string name);

private:
    domain::IProjectRepository& m_project_repository;
};

}
