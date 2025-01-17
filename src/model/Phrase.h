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

    void setInstrumentIndex(size_t index, size_t instrument_index);
    void clearInstrumentIndex(size_t index);
    std::optional<size_t> instrumentIndex(size_t index) const;

private:
    struct PhraseElement {
        std::optional<Note> note;
        std::optional<size_t> instrument_index;
    };
    std::array<PhraseElement, NOTE_COUNT> m_elements;
};

}
