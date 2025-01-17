#pragma once

#include <model/Note.h>

#include <array>
#include <optional>

namespace model {

class Track {
public:
    static constexpr size_t PHRASE_COUNT { 256u };

    void setPhraseIndex(size_t index, size_t chain_index);
    void clearPhraseIndex(size_t index);
    std::optional<size_t> phraseIndex(size_t index) const;

private:
    std::array<std::optional<size_t>, PHRASE_COUNT> m_phrases;
};

}
