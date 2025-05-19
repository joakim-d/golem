#include <interfaces/SongView.h>

#include <interfaces/GuiState.h>
#include <interfaces/ImGuiTools.h>
#include <interfaces/PatternView.h>
#include <interfaces/TrackView.h>

#include <golem/use_cases/ProjectUseCases.h>

#include <imgui.h>

namespace gui {

constexpr auto FIRST_COLUMN_WIDTH = 120.f;
constexpr auto FIRST_COLOR = ImColor {0x11, 0x11, 0x11};
constexpr auto SECOND_COLOR = ImColor {0x22, 0x22, 0x22};

SongView::SongView(
	use_cases::ProjectUseCases& use_cases,
	size_t song_index)
{
    const float cellHeight = 64.f;
	ImGui::BeginChild("SongView", ImVec2 {-1, cellHeight * 4 + 4}, 0, ImGuiWindowFlags_NoScrollbar);

	const ImVec2 cursor_screen_pos = {0, 0};

    const auto track_index = GuiState::trackIndex();

    const auto selected_color = GuiState::selectedColor();
    const auto idle_color = GuiState::idleColor();

    ImGui::BeginChild(
		"Pulse 1 Label",
		ImVec2 {64, cellHeight},
		ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
		ImGui::Dummy({64, cellHeight});
        WindowDrawHelper draw_helper;
        draw_helper.drawFilledRect(
			{0, 0}, {64, cellHeight}, getClickableItemColor(GuiState::trackIndex() == 0));
		draw_helper.addText({1, 0}, 0xFFFFFFFF, "Pulse 1");

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            GuiState::setTrackIndex(0);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

	TrackView track_view {use_cases, song_index, 0};
    // ImGui::Dummy({ -1, 48 });

    ImGui::BeginChild(
		"Pulse 2 Label",
		ImVec2 {64, cellHeight},
		ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
		ImGui::Dummy({64, cellHeight});
        draw_helper.drawFilledRect(
			{0, 0}, {64, cellHeight}, getClickableItemColor(GuiState::trackIndex() == 1));
		draw_helper.addText({1, 0}, 0xFFFFFFFF, "Pulse 2");

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            GuiState::setTrackIndex(1);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

	TrackView track_view_pulse_2 {use_cases, song_index, 1};

    ImGui::BeginChild(
		"Wave Label",
		ImVec2 {64, cellHeight},
		ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
		ImGui::Dummy({64, cellHeight});
        draw_helper.drawFilledRect(
			{0, 0}, {64, cellHeight}, getClickableItemColor(GuiState::trackIndex() == 2));
		draw_helper.addText({1, 0}, 0xFFFFFFFF, "Wave");

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            GuiState::setTrackIndex(2);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

	TrackView track_view_wave {use_cases, song_index, 2};

    ImGui::BeginChild(
		"Noise Label",
		ImVec2 {64, cellHeight},
		ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
		ImGui::Dummy({64, cellHeight});
        draw_helper.drawFilledRect(
			{0, 0}, {64, cellHeight}, getClickableItemColor(GuiState::trackIndex() == 3));
		draw_helper.addText({1, 0}, 0xFFFFFFFF, "Noise");

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            GuiState::setTrackIndex(3);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

	TrackView track_view_noise {use_cases, song_index, 3};

    ImGui::EndChild();

    ImGui::BeginChild(
		"Patterns", ImVec2 {64, cellHeight}, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);
    {
        WindowDrawHelper draw_helper;
		draw_helper.drawFilledRect({0, 0}, {64, cellHeight}, SECOND_COLOR);
		draw_helper.addText({1, 0}, 0xFFFFFFFF, "Patterns");
		ImGui::Dummy({64, cellHeight});
    }
    ImGui::EndChild();

	char pattern_id[32];
	for (size_t pattern_index = 0; pattern_index < 32; ++pattern_index) {
		snprintf(pattern_id, sizeof(pattern_id), "%u_%u", 4u, static_cast<unsigned>(pattern_index));
        ImGui::SameLine();
		PatternView pattern_view {use_cases, pattern_index, pattern_id};
    }
}

}
