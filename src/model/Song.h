#pragma once

#include <model/Channel.h>
#include <model/Phrase.h>
#include <model/Track.h>
#include <model/instruments/Instrument.h>

#include <array>
#include <cstddef>
#include <functional>

namespace model {

static constexpr size_t TRACK_LENGTH { 64 };

class Song {
public:
    static constexpr size_t PHRASE_COUNT { 256 };
    static constexpr size_t INSTRUMENT_COUNT { 16 };
    static constexpr size_t TRACK_COUNT { 4 };
    Song();

    size_t beatsPerMinute() const;
    void setTicksPerNote(size_t ticks);
    size_t ticksPerNote() const;

    void visitInstruments(const std::function<void(Instrument&)>& callback);
    void visitInstruments(const std::function<void(const Instrument&)>& callback) const;
    Instrument& getInstrument(size_t index);
    const Instrument& getInstrument(size_t index) const;

    void visitPhrases(const std::function<void(Phrase&)>& callback);
    void visitPhrases(const std::function<void(const Phrase&)>& callback) const;
    Phrase& getPhrase(size_t index);
    const Phrase& getPhrase(size_t index) const;

    void visitTracks(const std::function<void(Track&)>& callback);
    void visitTracks(const std::function<void(const Track&)>& callback) const;
    Track& getTrack(size_t index);
    const Track& getTrack(size_t index) const;

private:
    size_t m_beats_per_minutes;
    size_t m_ticks_per_note;
    std::array<Instrument, INSTRUMENT_COUNT> m_instruments;
    std::array<Phrase, PHRASE_COUNT> m_phrases;
    std::array<Track, TRACK_COUNT> m_tracks;
};

}
