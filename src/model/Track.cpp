#include <model/Track.h>

namespace model {

void Track::setPhraseIndex(size_t index, size_t chain_index)
{
    m_phrases[index] = chain_index;
}

void Track::clearPhraseIndex(size_t index)
{
    m_phrases[index].reset();
}

std::optional<size_t> Track::phraseIndex(size_t index) const
{
    return m_phrases[index];
}

}
