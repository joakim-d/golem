#include <golem/domain/Pattern.h>

namespace domain
{

Pattern::Pattern(size_t length)
    : m_notes(length)
{
}

size_t Pattern::noteCount() const
{
    return m_notes.size();
}

const std::optional<Note>& Pattern::note(size_t index) const
{
    return m_notes[index];
}

void Pattern::setNote(size_t index, Note note)
{
    m_notes[index] = note;
}

void Pattern::clearNote(size_t index)
{
    m_notes[index].reset();
}

}
