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

    TrackView track_view { use_cases, song_index, 0 };
    // ImGui::Dummy({ -1, 48 });

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

    TrackView track_view_pulse_2 { use_cases, song_index, 1 };

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

    TrackView track_view_wave { use_cases, song_index, 2 };

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

    TrackView track_view_noise { use_cases, song_index, 3 };

    ImGui::EndChild();
}

}
