#pragma once

#include <string>

namespace domain {

enum class NoteFrequency {
    C2,
    C2Sharp,
    D2,
    D2Sharp,
    E2,
    F2,
    F2Sharp,
    G2,
    G2Sharp,
    A2,
    A2Sharp,
    B2,
    C3,
    C3Sharp,
    D3,
    D3Sharp,
    E3,
    F3,
    F3Sharp,
    G3,
    G3Sharp,
    A3,
    A3Sharp,
    B3,
    C4,
    C4Sharp,
    D4,
    D4Sharp,
    E4,
    F4,
    F4Sharp,
    G4,
    G4Sharp,
    A4,
    A4Sharp,
    B4,
    C5,
    C5Sharp,
    D5,
    D5Sharp,
    E5,
    F5,
    F5Sharp,
    G5,
    G5Sharp,
    A5,
    A5Sharp,
    B5,
    C6,
    C6Sharp,
    D6,
    D6Sharp,
    E6,
    F6,
    F6Sharp,
    G6,
    G6Sharp,
    A6,
    A6Sharp,
    B6,
};

std::string to_string(NoteFrequency note);

class Note {
public:
    Note() = default;
    Note(
        NoteFrequency frequency,
        size_t duration,
        size_t instrument_index);

    NoteFrequency frequency() const;
    size_t duration() const;
    size_t instrumentIndex() const;

    void setFrequency(NoteFrequency frequency);
    void setDuration(size_t duration);
    void setInstrumentIndex(size_t instrument_index);

private:
    NoteFrequency m_frequency;
    size_t m_duration;
    size_t m_instrument_index;
};

}
