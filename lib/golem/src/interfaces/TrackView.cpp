#include <interfaces/TrackView.h>

#include <golem/use_cases/ProjectUseCases.h>

#include <golem/domain/Track.h>

#include <limits>

#include <imgui.h>

namespace gui
{

static void drawPhrase()
{
}

TrackView::TrackView(
    use_cases::ProjectUseCases& use_cases,
    size_t song_index,
    size_t track_index)
{
    auto track = use_cases.get_track(song_index, track_index);

    const auto notes_per_phrase = use_cases.get_notes_per_phrase(song_index);
    // ImGui::

    if (notes_per_phrase == 0)
    {
        return;
    }

    const auto min_max
        = use_cases.get_min_max_track_frequencies(song_index, track_index);

    if (!min_max)
    {
        return;
    }

    auto delta = static_cast<unsigned>(min_max->second)
        - static_cast<unsigned>(min_max->first);

    const auto note_count = track->noteCount();
    const auto phrases = note_count / notes_per_phrase;

    size_t note_index = 0;
    for (size_t phrase_index = 0; phrase_index < phrases; ++phrase_index)
    {
    }
}

}
