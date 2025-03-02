#include <golem/domain/Track.h>

namespace domain
{

Track::Track()
{
    m_notes.resize(256);
}

const std::optional<Note>& Track::note(size_t index) const
{
    return m_notes[index];
}

void Track::setNote(size_t index, Note note)
{
    if (index > m_notes.size())
    {
        return;
    }
    m_notes[index] = std::move(note);
}

void Track::clearNote(size_t index)
{
    if (index > m_notes.size())
    {
        return;
    }
    m_notes[index].reset();
}

size_t Track::noteCount() const
{
    return m_notes.size();
}

}
