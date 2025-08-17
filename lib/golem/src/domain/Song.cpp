#include <golem/domain/Song.h>

namespace domain {

Song::Song()
    : m_ticks_per_note(12)
    , m_notes_per_pattern(16)
    , m_patterns_per_track(8)
{
}

void Song::visitTracks(const std::function<void(Track&)>& callback)
{
	for (auto& track : m_tracks) {
        callback(track);
    }
}

void Song::visitTracks(const std::function<void(const Track&)>& callback) const
{
	for (const auto& track : m_tracks) {
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

void Song::setNotesPerPattern(unsigned int notes_per_phrase)
{
    m_notes_per_pattern = notes_per_phrase;
}

unsigned int Song::notesPerPattern() const
{
    return m_notes_per_pattern;
}

void Song::setTicksPerNote(unsigned int ticks)
{
	if (ticks == 0) {
        return;
    }
    m_ticks_per_note = ticks;
}

unsigned int Song::ticksPerNote() const
{
    return m_ticks_per_note;
}

const std::string& Song::name() const
{
    return m_name;
}

void Song::setName(std::string name)
{
    m_name = std::move(name);
}

size_t Song::notesCount() const
{
    return 256;
}

unsigned int Song::patternsPerTrack() const
{
    return m_patterns_per_track;
}

}
