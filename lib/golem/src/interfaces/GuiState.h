#pragma once

#include <cstddef>
#include <imgui.h>

#include <optional>

namespace gui {

class GuiState {
public:
	static GuiState& instance();

	GuiState(const GuiState&) = delete;
	GuiState& operator=(const GuiState&) = delete;
	GuiState(GuiState&&) = delete;
	GuiState& operator=(GuiState&&) = delete;

	ImColor selected_color;
	ImColor idle_color;
	ImColor hovered_color;

	size_t song_index;
	size_t track_index;
	size_t instrument_index;
	size_t pattern_index;

	struct DraggedPattern {
		ImVec2 offset_position; // Relative to the mouse position
		size_t pattern_index;
	};

	std::optional<DraggedPattern> dragged_pattern;

private:
	GuiState();
};

}
