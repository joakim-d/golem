#pragma once

#include <golem/domain/Note.h>

#include <cstddef>
#include <optional>
#include <vector>

namespace domain
{

class Pattern
{
public:
    Pattern(size_t length);

    const std::optional<Note>& note(size_t index) const;
    void setNote(size_t index, Note note);
    void clearNote(size_t index);
    size_t noteCount() const;

private:
    std::vector<std::optional<Note>> m_notes;
};

}
