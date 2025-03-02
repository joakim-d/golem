#pragma once

#include <golem/domain/Track.h>
#include <golem/domain/instruments/Instrument.h>

#include <array>
#include <cstddef>
#include <functional>

namespace domain
{

class Song
{
public:
    static constexpr size_t TRACK_COUNT { 4 };

    Song();

    size_t notesCount() const;

    unsigned int notesPerPhrase() const;
    void setNotesPerPhrase(unsigned int notes_per_phrase);

    unsigned int ticksPerNote() const;
    void setTicksPerNote(unsigned int ticks);

    void visitTracks(const std::function<void(Track&)>& callback);
    void visitTracks(const std::function<void(const Track&)>& callback) const;
    Track& getTrack(size_t index);
    const Track& getTrack(size_t index) const;

private:
    size_t m_notes_count;
    unsigned int m_ticks_per_note;
    unsigned int m_notes_per_phrase;
    std::array<Track, TRACK_COUNT> m_tracks;
};

}
