#include <interfaces/MainView.h>

#include <interfaces/GuiState.h>
#include <interfaces/ImGuiTools.h>
#include <interfaces/PianoRollView.h>
#include <interfaces/SequencerHeaderView.h>
#include <interfaces/SequencerView.h>

#include <golem/use_cases/ProjectUseCases.h>

#include <golem/utils/scope_guard.h>

#include <imgui.h>

namespace gui
{
MainView::MainView(
    use_cases::ProjectUseCases& use_cases,
    size_t song_index,
    size_t track_index)
{
    ImGui::BeginChild(
        "MainView", ImVec2 { -1, -1 }, 0, ImGuiWindowFlags_NoScrollbar);

    utils::ScopeGuard scope_guard { ImGui::EndChild };

    static ImVec2 previous_mouse_pos;
    static ImVec2 offset { 0, PianoRollView::viewHeight() / 2 };

    if (IsMouseChildClicked(ImGuiMouseButton_Middle))
    {
        previous_mouse_pos = ImGui::GetMousePos();
    }

    if (IsMouseChildDown(ImGuiMouseButton_Middle))
    {
        auto current_mouse_pos = ImGui::GetMousePos();
        // offset.x += previous_mouse_pos.x - current_mouse_pos.x;
        offset.y += previous_mouse_pos.y - current_mouse_pos.y;

        const auto window_size = ImGui::GetWindowSize();

        const float max_y_offset = PianoRollView::viewHeight() - window_size.y
            + SequencerHeaderView::viewHeight()
            + 10; // I don't know why we need to add 10 pixels here...

        const float max_x_offset = SequencerView::viewWidth(use_cases);

        if (offset.y > max_y_offset)
        {
            offset.y = max_y_offset;
        }

        if (offset.y < 0.f)
        {
            offset.y = 0.f;
        }

        previous_mouse_pos = current_mouse_pos;
    }

    SequencerHeaderView header_view { offset.x - PianoRollView::viewWidth() };

    PianoRollView piano_roll_view { use_cases, offset.y };

    ImGui::SameLine();

    SequencerView sequencer_view { use_cases, offset };

    if (ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        use_cases.get_playback_info.execute().stopped
            ? use_cases.play_song.execute(GuiState::songIndex())
            : use_cases.stop_song.execute();
    }
}
}
