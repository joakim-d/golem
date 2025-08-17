#pragma once

#include <golem/domain/Pattern.h>

#include <vector>

namespace domain
{

class Track
{
public:
    Track();

    // const std::optional<Note>& note(size_t index) const;
    // void setNote(size_t index, Note note);
    // void clearNote(size_t index);
    // size_t noteCount() const;

    void setPatternIndex(size_t offset, size_t pattern_index);
    void clearPatternIndex(size_t offset);

    const std::vector<std::optional<size_t>>& patternIndexes() const;

private:
    std::vector<std::optional<size_t>> m_pattern_indexes;
};

}
