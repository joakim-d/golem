#include <golem/use_cases/AddNote.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

#include <cmath>

namespace use_cases
{

AddNote::AddNote(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

void AddNote::execute(
    size_t pattern_index,
    size_t note_index,
    domain::Note note)
{
    auto& project = m_project_repository.getProject();

    auto pattern = project.pattern(pattern_index);
    if (pattern == nullptr)
    {
        return;
    }

    // Cleaning any notes that are overlapping with the new
    // one
    const auto duration = note.duration();
    for (auto i = note_index + 1; i < note_index + duration; ++i)
    {
        pattern->clearNote(i);
    }

    // Reduce the duration of the note before the inserted one
    for (int i = int(note_index) - 1; i >= 0; --i)
    {
        auto note = pattern->note(i);
        if (note.has_value() && note->duration() + i > note_index)
        {
            note->setDuration(note_index - i);
            pattern->setNote(i, *note);
        }
    }

    pattern->setNote(note_index, note);
}

}
