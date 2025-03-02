#pragma once

#include <cstddef>

namespace domain {
class IProjectRepository;
}

namespace use_cases {

class ChangeTicksPerNote {
public:
    ChangeTicksPerNote(domain::IProjectRepository& project_repository);

    void execute(size_t song_index, unsigned int notes_per_phrase);

private:
    domain::IProjectRepository& m_project_repository;
};

}
