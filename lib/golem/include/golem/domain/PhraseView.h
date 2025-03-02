#pragma once

#include <cstddef>
#include <optional>

namespace domain
{
class Note;

class PhraseView
{
public:
    PhraseView(const std::optional<Note>* begin, size_t size);

    const std::optional<Note>& note(size_t index) const;
    size_t size() const;

private:
    const std::optional<Note>* m_begin;
    const size_t m_size;
};

}
