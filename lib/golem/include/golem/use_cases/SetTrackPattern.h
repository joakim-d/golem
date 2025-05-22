#pragma once

#include <cstddef>

namespace domain {
class IProjectRepository;
class Track;
}

namespace use_cases {

class SetTrackPattern {
public:
	SetTrackPattern(domain::IProjectRepository& project_repository);

	void operator()(size_t song_index,
					size_t track_index,
					size_t pattern_offset,
					size_t pattern_index);

private:
	domain::IProjectRepository& m_project_repository;
};

}
