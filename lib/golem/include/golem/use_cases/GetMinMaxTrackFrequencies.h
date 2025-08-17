#pragma once

#include <golem/domain/Note.h>

#include <cstddef>

#include <optional>
#include <tuple>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class GetMinMaxTrackFrequencies
{
public:
    GetMinMaxTrackFrequencies(domain::IProjectRepository& project_repository);

    std::optional<std::pair<domain::NoteFrequency, domain::NoteFrequency>>
    operator()(size_t pattern_index);

private:
    domain::IProjectRepository& m_project_repository;
};

}
