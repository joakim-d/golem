#include <interfaces/TrackView.h>

#include <golem/use_cases/ProjectUseCases.h>

#include <golem/domain/Track.h>
#include <golem/utils/scope_guard.h>

#include <interfaces/GuiState.h>
#include <interfaces/ImGuiTools.h>

#include <imgui.h>

namespace gui
{

constexpr auto SPACING = 1;
constexpr auto PHRASE_WIDTH = 260;
constexpr auto VIEW_HEIGHT = 48.f;

static void drawPhrase()
{
}

TrackView::TrackView(
    use_cases::ProjectUseCases& use_cases,
    size_t song_index,
    size_t track_index)
{
    char child_name[32];
    snprintf(
        child_name, sizeof(child_name), "TrackView%u",
        static_cast<unsigned>(track_index));

    ImGui::BeginChild(
        child_name, ImVec2 { -1, VIEW_HEIGHT }, 0,
        ImGuiWindowFlags_NoScrollbar);

    utils::ScopeGuard scope_guard { ImGui::EndChild };

    auto track = use_cases.get_track(song_index, track_index);

    const auto notes_per_phrase = use_cases.get_notes_per_phrase(song_index);
    // ImGui::

    if (IsMouseChildClicked(ImGuiMouseButton_Left))
    {
        GuiState::setTrackIndex(track_index);
    }

    if (notes_per_phrase == 0)
    {
        return;
    }

    // const auto min_max
    //     = use_cases.get_min_max_track_frequencies(song_index, track_index);

    // if (!min_max)
    // {
    //     return;
    // }

    // auto delta = static_cast<unsigned>(min_max->second)
    //     - static_cast<unsigned>(min_max->first) + 1;

    // const auto note_height = VIEW_HEIGHT / delta > 0 ? VIEW_HEIGHT / delta :
    // 1; const auto note_width = PHRASE_WIDTH / notes_per_phrase;

    // const auto note_count = track->noteCount();
    // const auto phrases = note_count / notes_per_phrase;

    // size_t note_index = 0;
    // WindowDrawHelper draw_helper;
    // for (size_t phrase_index = 0; phrase_index < phrases; ++phrase_index)
    // {
    //     draw_helper.drawRect(
    //         { static_cast<float>(phrase_index * (PHRASE_WIDTH + SPACING)),
    //           0.f },
    //         { PHRASE_WIDTH, VIEW_HEIGHT }, ImColor { 255, 0, 0 });

    //     for (size_t current_note = phrase_index * notes_per_phrase,
    //                 end_note = current_note + notes_per_phrase;
    //          current_note != end_note; current_note++)
    //     {
    //         auto note = track->note(current_note);
    //         if (!note.has_value())
    //         {
    //             continue;
    //         }

    //         const auto position = static_cast<unsigned>(note->frequency())
    //             - static_cast<unsigned>(min_max->first);

    //         draw_helper.drawFilledRect(
    //             { float(
    //                   (current_note - (phrase_index * notes_per_phrase))
    //                       * note_width
    //                   + (phrase_index * (PHRASE_WIDTH + SPACING))),
    //               VIEW_HEIGHT - ((position + 1) * note_height) },
    //             { float(note_width), float(note_height) },
    //             ImColor { 255, 0, 0 });
    //     }
    // }

    ImGui::Dummy({ -1, VIEW_HEIGHT });
}

}
