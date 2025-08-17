#include <golem/domain/Track.h>

namespace domain {

Track::Track()
{
	m_pattern_indexes.resize(8);
}

void Track::setPatternIndex(
	size_t offset,
	size_t pattern_index)
{
	if (m_pattern_indexes.size() < offset) {
        m_pattern_indexes.resize(offset + 1);
    }
    m_pattern_indexes[offset] = pattern_index;
}

void Track::clearPatternIndex(size_t offset)
{
	if (offset >= m_pattern_indexes.size()) {
        return;
    }
    m_pattern_indexes[offset].reset();
}

const std::vector<std::optional<size_t>>& Track::patternIndexes() const
{
    return m_pattern_indexes;
}

}
