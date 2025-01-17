#include <model/Note.h>

#include <array>

namespace model {

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

static std::string getNotePrefix(Note note)
{
    unsigned index = static_cast<unsigned>(note) % 12;
    return NOTE_PREFIXES[index];
}

static std::string getOctave(Note note)
{
    unsigned index = static_cast<unsigned>(note) / 12;
    return std::to_string(index + 2);
}

static std::string getNoteSuffix(Note note)
{
    unsigned index = static_cast<unsigned>(note) % 12;
    if (index == 1 || index == 3 || index == 6 || index == 8 || index == 10) {
        return "#";
    }
    return "";
}

std::string to_string(Note note)
{
    return getNotePrefix(note) + getOctave(note) + getNoteSuffix(note);
}

}
