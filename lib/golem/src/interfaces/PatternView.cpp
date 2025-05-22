#include <interfaces/PatternView.h>

#include <golem/domain/Pattern.h>
#include <golem/use_cases/ProjectUseCases.h>
#include <golem/utils/scope_guard.h>
#include <interfaces/GuiState.h>
#include <interfaces/ImGuiTools.h>

#include <imgui.h>

#include <cstdio>
#include <cstring>

namespace gui {

PatternView::PatternView(
    use_cases::ProjectUseCases& project_use_cases,
	std::optional<size_t> pattern_index,
	const char* pattern_id,
	bool track_pattern,
	std::function<void()> on_drag)
{
    char child_name[32];
	snprintf(child_name, sizeof(child_name), "PatternView%s", pattern_id);

    constexpr float pattern_width = 80.f;
    constexpr float pattern_height = 64.f;

    ImGui::BeginChild(
		child_name,
		ImVec2 {pattern_width, pattern_height},
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImGui::GetWindowSize());
	utils::ScopeGuard scope_guard {ImGui::EndChild};

	auto& gui_state = GuiState::instance();

	ImColor background_color = getClickableItemColor(pattern_index == gui_state.pattern_index);

	auto mouse_to_window = GetMouseToWindowPosition();
	auto window_size = ImGui::GetWindowSize();

	bool mouse_over = mouse_to_window.x < window_size.x
				   && mouse_to_window.y < window_size.y
				   && mouse_to_window.x >= 0
				   && mouse_to_window.y >= 0;

	if (track_pattern && gui_state.dragged_pattern.has_value() && mouse_over) {
		background_color = gui_state.selected_color;
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			on_drag();
		}
	}

	WindowDrawHelper window_draw_helper;

	char text_buffer[32];
	if (pattern_index) {
		snprintf(
			text_buffer, sizeof(text_buffer), "Pattern %u", static_cast<unsigned>(*pattern_index));
	} else {
		text_buffer[0] = 0;
	}

	if (ImGui::IsItemHovered()) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	}

	window_draw_helper.addText({0, 0}, ImColor {0xff, 0xff, 0xff}, text_buffer);

	window_draw_helper.drawFilledRect({0, 0}, {pattern_width, pattern_height}, background_color);

	if (!pattern_index.has_value()) {
        return;
    }

	auto pattern = project_use_cases.get_pattern(*pattern_index);

	if (pattern == nullptr) {
		return;
	}

	float note_width = pattern_width / static_cast<float>(pattern->noteCount());

	const ImColor note_color {0x11, 0xff, 0x11};

	for (size_t note_index = 0; note_index < pattern->noteCount(); ++note_index) {
        const auto& note = pattern->note(note_index);

		if (!note) {
            continue;
        }

        window_draw_helper.drawFilledRect(
			{static_cast<float>(note_index) * note_width,
			 63.f - static_cast<float>(note->frequency())},
			{note_width, 1.f},
			note_color);
    }

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		gui_state.pattern_index = *pattern_index;

		gui_state.dragged_pattern = GuiState::DraggedPattern {};
		gui_state.dragged_pattern->pattern_index = *pattern_index;

		gui_state.dragged_pattern->offset_position = ImGui::GetWindowPos() - ImGui::GetMousePos();
	}
}

}
