#pragma once

#include <golem/domain/PhraseView.h>

namespace domain
{
class IProjectRepository;
class Track;
}

namespace use_cases
{

class GetTrack
{
public:
    GetTrack(domain::IProjectRepository& project_repository);

    const domain::Track* operator()(size_t song_index, size_t track_index);

private:
    domain::IProjectRepository& m_project_repository;
};

}
