#include <model/Phrase.h>

namespace model {

std::optional<Note> Phrase::note(size_t index) const
{
    return m_notes[index];
}

void Phrase::setNote(size_t index, Note note)
{
    m_notes[index] = note;
}

void Phrase::clearNote(size_t index)
{
    m_notes[index].reset();
}

}
