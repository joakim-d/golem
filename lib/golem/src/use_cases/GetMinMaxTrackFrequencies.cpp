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
GetMinMaxTrackFrequencies::operator()(size_t pattern_index)
{
    auto pattern = m_project_repository.getProject().pattern(pattern_index);

    if (pattern == nullptr)
    {
        return {};
    }

    unsigned min_note = std::numeric_limits<unsigned>::max();
    unsigned max_note = std::numeric_limits<unsigned>::min();

    for (size_t i = 0; i < pattern->noteCount(); ++i)
    {
        const auto& note = pattern->note(i);
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
