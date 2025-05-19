#include <interfaces/TrackView.h>

#include <golem/use_cases/ProjectUseCases.h>

#include <golem/domain/Track.h>
#include <golem/utils/scope_guard.h>

#include <interfaces/GuiState.h>
#include <interfaces/ImGuiTools.h>
#include <interfaces/PatternView.h>

#include <imgui.h>

namespace gui {

constexpr auto SPACING = 1;
constexpr auto PHRASE_WIDTH = 260;
constexpr auto VIEW_HEIGHT = 64.f;

static void drawPhrase()
{
}

TrackView::TrackView(
    use_cases::ProjectUseCases& use_cases,
    size_t song_index,
    size_t track_index)
{
    char child_name[32];
	snprintf(child_name, sizeof(child_name), "TrackView%u", static_cast<unsigned>(track_index));

	ImGui::BeginChild(child_name, ImVec2 {-1, VIEW_HEIGHT}, 0, ImGuiWindowFlags_NoScrollbar);

	utils::ScopeGuard scope_guard {ImGui::EndChild};

    auto track = use_cases.get_track(song_index, track_index);

	if (IsMouseChildClicked(ImGuiMouseButton_Left)) {
        GuiState::setTrackIndex(track_index);
    }
	char pattern_id[32];

	unsigned pattern_position = 0;
	for (const auto& pattern_index : track->patternIndexes()) {
		snprintf(
			pattern_id,
			sizeof(pattern_id),
			"%u_%u",
			static_cast<unsigned>(track_index),
			static_cast<unsigned>(pattern_position++));

		PatternView pattern_view {use_cases, pattern_index, pattern_id};
		ImGui::SameLine();
	}

	ImGui::Dummy({-1, VIEW_HEIGHT});
}

}
