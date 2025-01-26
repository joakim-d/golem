#include <model/Song.h>

namespace model {

Song::Song()
    : m_ticks_per_note(12)
{
}

void Song::visitPhrases(const std::function<void(Phrase&)>& callback)
{
    for (auto& phrase : m_phrases) {
        callback(phrase);
    }
}

void Song::visitPhrases(const std::function<void(const Phrase&)>& callback) const
{
    for (const auto& phrase : m_phrases) {
        callback(phrase);
    }
}

void Song::visitInstruments(const std::function<void(Instrument&)>& callback)
{
    for (auto& instrument : m_instruments) {
        callback(instrument);
    }
}

void Song::visitInstruments(const std::function<void(const Instrument&)>& callback) const
{
    for (const auto& instrument : m_instruments) {
        callback(instrument);
    }
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

Phrase& Song::getPhrase(size_t index)
{
    return m_phrases[index];
}

Instrument& Song::getInstrument(size_t index)
{
    return m_instruments[index];
}

Track& Song::getTrack(size_t index)
{
    return m_tracks[index];
}

const Phrase& Song::getPhrase(size_t index) const
{
    return m_phrases[index];
}

const Instrument& Song::getInstrument(size_t index) const
{
    return m_instruments[index];
}

const Track& Song::getTrack(size_t index) const
{
    return m_tracks[index];
}

size_t Song::ticksPerNote() const
{
    return m_ticks_per_note;
}

}
