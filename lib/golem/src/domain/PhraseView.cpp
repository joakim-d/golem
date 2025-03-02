#include <golem/domain/PhraseView.h>

#include <golem/domain/Note.h>

namespace domain
{

PhraseView::PhraseView(const std::optional<Note>* begin, size_t size)
    : m_begin(begin)
    , m_size(size)
{
}

const std::optional<Note>& PhraseView::note(size_t index) const
{
    return *(m_begin + index);
}

size_t PhraseView::size() const
{
    return m_size;
}

}
