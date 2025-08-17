#include <golem/use_cases/SetTrackPattern.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Track.h>

#include <golem/domain/Project.h>
#include <golem/domain/Song.h>

namespace use_cases {

SetTrackPattern::SetTrackPattern(domain::IProjectRepository& project_repository)
	: m_project_repository(project_repository)
{
}

void SetTrackPattern::operator()(
	size_t song_index,
	size_t track_index,
	size_t pattern_offset,
	size_t pattern_index)
{
	auto song = m_project_repository.getProject().song(song_index);

	if (song == nullptr) {
		return;
	}

	if (track_index >= domain::Song::TRACK_COUNT) {
		return;
	}

	auto& track = song->getTrack(track_index);

	track.setPatternIndex(pattern_offset, pattern_index);
}

}
