#include <golem/domain/Note.h>

#include <array>

namespace domain {

static std::array<std::string, 12> NOTE_PREFIXES = {
    "C",
    "C",
    "D",
    "D",
    "E",
    "F",
    "F",
    "G",
    "G",
    "A",
    "A",
    "B"
};

static std::string getNotePrefix(NoteFrequency note)
{
    unsigned index = static_cast<unsigned>(note) % 12;
    return NOTE_PREFIXES[index];
}

static std::string getOctave(NoteFrequency note)
{
    unsigned index = static_cast<unsigned>(note) / 12;
    return std::to_string(index + 2);
}

static std::string getNoteSuffix(NoteFrequency note)
{
    unsigned index = static_cast<unsigned>(note) % 12;
    if (index == 1 || index == 3 || index == 6 || index == 8 || index == 10) {
        return "#";
    }
    return "";
}

std::string to_string(NoteFrequency note)
{
    return getNotePrefix(note) + getOctave(note) + getNoteSuffix(note);
}

Note::Note(
    NoteFrequency frequency,
    size_t duration,
    size_t instrument_index)
    : m_frequency(frequency)
    , m_duration(duration)
    , m_instrument_index(instrument_index)
{
}

NoteFrequency Note::frequency() const
{
    return m_frequency;
}

size_t Note::duration() const
{
    return m_duration;
}

size_t Note::instrumentIndex() const
{
    return m_instrument_index;
}

void Note::setFrequency(NoteFrequency frequency)
{
    m_frequency = frequency;
}

void Note::setDuration(size_t duration)
{
    m_duration = duration;
}

void Note::setInstrumentIndex(size_t instrument_index)
{
    m_instrument_index = instrument_index;
}

}
