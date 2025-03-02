#include <golem/domain/Song.h>

namespace domain
{

Song::Song()
    : m_ticks_per_note(12)
    , m_notes_per_phrase(16)
{
}

void Song::visitTracks(const std::function<void(Track&)>& callback)
{
    for (auto& track : m_tracks)
    {
        callback(track);
    }
}

void Song::visitTracks(const std::function<void(const Track&)>& callback) const
{
    for (const auto& track : m_tracks)
    {
        callback(track);
    }
}

Track& Song::getTrack(size_t index)
{
    return m_tracks[index];
}

const Track& Song::getTrack(size_t index) const
{
    return m_tracks[index];
}

void Song::setNotesPerPhrase(unsigned int notes_per_phrase)
{
    m_notes_per_phrase = notes_per_phrase;
}

unsigned int Song::notesPerPhrase() const
{
    return m_notes_per_phrase;
}

void Song::setTicksPerNote(unsigned int ticks)
{
    m_ticks_per_note = ticks;
}

unsigned int Song::ticksPerNote() const
{
    return m_ticks_per_note;
}

size_t Song::notesCount() const
{
    return 256;
}

}
