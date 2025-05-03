#include <golem/use_cases/GetMinMaxTrackFrequencies.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>
#include <golem/domain/Track.h>

#include <limits>

namespace use_cases
{

GetMinMaxTrackFrequencies::GetMinMaxTrackFrequencies(
    domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

std::optional<std::pair<domain::NoteFrequency, domain::NoteFrequency>>
GetMinMaxTrackFrequencies::operator()(size_t song_index, size_t track_index)
{
    auto song = m_project_repository.getProject().song(song_index);

    if (song == nullptr || track_index >= domain::Song::TRACK_COUNT)
    {
        return {};
    }

    auto& track = song->getTrack(track_index);

    unsigned min_note = std::numeric_limits<unsigned>::max();
    unsigned max_note = std::numeric_limits<unsigned>::min();

    for (size_t i = 0; i < track.noteCount(); ++i)
    {
        const auto& note = track.note(i);
        if (!note.has_value())
        {
            continue;
        }
        const unsigned frequency
            = static_cast<unsigned>(note.value().frequency());

        if (frequency > max_note)
        {
            max_note = frequency;
        }
        if (frequency < min_note)
        {
            min_note = frequency;
        }
    }

    if (min_note == std::numeric_limits<unsigned>::max())
    {
        return {};
    }

    return { { static_cast<domain::NoteFrequency>(min_note),
               static_cast<domain::NoteFrequency>(max_note) } };
}

}
