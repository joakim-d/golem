#pragma once

#include <golem/domain/PhraseView.h>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class GetPhraseView
{
public:
    GetPhraseView(domain::IProjectRepository& project_repository);

    std::optional<domain::PhraseView>
    execute(size_t song_index, size_t track_index, size_t phrase_index);

private:
    domain::IProjectRepository& m_project_repository;
};

}
