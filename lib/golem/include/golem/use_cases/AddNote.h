#pragma once

#include <cstddef>

#include <golem/domain/Note.h>

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class AddNote
{
public:
    AddNote(domain::IProjectRepository& project_repository);

    void execute(size_t song_index, size_t track_index, size_t note_index, domain::Note note);

private:
    domain::IProjectRepository& m_project_repository;
};

}
