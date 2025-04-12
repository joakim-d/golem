#include <interfaces/SongView.h>

#include <interfaces/ImGuiTools.h>
#include <interfaces/TrackView.h>

#include <golem/use_cases/ProjectUseCases.h>

#include <imgui.h>

namespace gui
{

constexpr auto FIRST_COLUMN_WIDTH = 120.f;
constexpr auto FIRST_COLOR = ImColor { 0x11, 0x11, 0x11 };
constexpr auto SECOND_COLOR = ImColor { 0x22, 0x22, 0x22 };

SongView::SongView(use_cases::ProjectUseCases& use_cases, size_t song_index)
{
    ImGui::BeginChild(
        "SongView", ImVec2 { -1, 48 * 4 + 4 }, 0, ImGuiWindowFlags_NoScrollbar);

    // ImGui::Dummy({ 50 * 120, window_size.y });

    // ImVec2 mouse_pos = ImGui::GetMousePos();
    // static ImVec2 cursor_pos = ImVec2(0.f, 0.f); // Initial cursor position

    // Check if the left mouse button is being dragged
    // if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    // {
    //     // Get the mouse drag delta (how much the mouse has moved since the
    //     drag
    //     // started)
    //     ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

    //     // Update your cursor position based on the drag delta
    //     cursor_pos.x += drag_delta.x;
    //     // cursor_pos.y += drag_delta.y;

    //     // Reset the drag delta to avoid accumulating movement
    //     ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

    //     // Display the updated cursor position
    //     ImGui::Text(
    //         "Cursor Position: (%.1f, %.1f)", cursor_pos.x, cursor_pos.y);
    // }
    // ImGui::SetCursorPos(cursor_pos);

    // First Row

    // const ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    const ImVec2 cursor_screen_pos = { 0, 0 };

    ImGui::BeginChild(
        "Pulse 1 Label", ImVec2 { 64, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
        draw_helper.drawFilledRect({ 0, 0 }, { 64, 48 }, SECOND_COLOR);
        draw_helper.addText({ 1, 0 }, 0xFFFFFFFF, "Pulse 1");
        ImGui::Dummy({ 64, 48 });
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(
        "Pulse 1 Track", ImVec2 { -1, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        TrackView track_view { use_cases, song_index, 0 };
        WindowDrawHelper draw_helper;
        draw_helper.drawFilledRect({ 0, 0 }, { 64, 48 }, SECOND_COLOR);
        ImGui::Dummy({ -1, 48 });
    }
    ImGui::EndChild();

    ImGui::BeginChild(
        "Pulse 2 Label", ImVec2 { 64, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
        draw_helper.drawFilledRect({ 0, 0 }, { 64, 48 }, SECOND_COLOR);
        draw_helper.addText({ 1, 0 }, 0xFFFFFFFF, "Pulse 2");
        ImGui::Dummy({ 64, 48 });
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(
        "Pulse 2 Track", ImVec2 { -1, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        TrackView track_view { use_cases, song_index, 0 };
        ImGui::Dummy({ -1, 48 });
    }
    ImGui::EndChild();

    ImGui::BeginChild(
        "Wave Label", ImVec2 { 64, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
        draw_helper.drawFilledRect({ 0, 0 }, { 64, 48 }, SECOND_COLOR);
        draw_helper.addText({ 1, 0 }, 0xFFFFFFFF, "Wave");
        ImGui::Dummy({ 64, 48 });
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(
        "Wave Track", ImVec2 { -1, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        TrackView track_view { use_cases, song_index, 0 };
        ImGui::Dummy({ -1, 48 });
    }
    ImGui::EndChild();

    ImGui::BeginChild(
        "Noise Label", ImVec2 { 64, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
        draw_helper.drawFilledRect({ 0, 0 }, { 64, 48 }, SECOND_COLOR);
        draw_helper.addText({ 1, 0 }, 0xFFFFFFFF, "Noise");
        ImGui::Dummy({ 64, 48 });
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild(
        "Noise Track", ImVec2 { -1, 48 }, ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        TrackView track_view { use_cases, song_index, 0 };
        ImGui::Dummy({ -1, 48 });
    }
    ImGui::EndChild();

    ImGui::EndChild();
}

}
