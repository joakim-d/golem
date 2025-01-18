#pragma once

#include <model/Note.h>

#include <array>
#include <optional>

namespace model {

class Phrase {
public:
    static constexpr size_t NOTE_COUNT { 16u };

    void setNote(size_t index, Note note);
    void clearNote(size_t index);
    std::optional<Note> note(size_t index) const;

private:
    std::array<std::optional<Note>, NOTE_COUNT> m_notes;
};

}
