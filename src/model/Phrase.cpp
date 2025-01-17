#include <model/Phrase.h>

namespace model {

std::optional<Note> Phrase::note(size_t index) const
{
    return m_elements[index].note;
}

void Phrase::setNote(size_t index, Note note)
{
    m_elements[index].note = note;
}

void Phrase::clearNote(size_t index)
{
    m_elements[index].note.reset();
}

std::optional<size_t> Phrase::instrumentIndex(size_t index) const
{
    return m_elements[index].instrument_index;
}

void Phrase::setInstrumentIndex(size_t index, size_t instrument_index)
{
    m_elements[index].instrument_index = instrument_index;
}

void Phrase::clearInstrumentIndex(size_t index)
{
    m_elements[index].instrument_index.reset();
}

}
