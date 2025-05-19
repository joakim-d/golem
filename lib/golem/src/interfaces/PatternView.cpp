#include <interfaces/PatternView.h>

#include <golem/domain/Pattern.h>
#include <golem/use_cases/ProjectUseCases.h>
#include <golem/utils/scope_guard.h>
#include <interfaces/GuiState.h>
#include <interfaces/ImGuiTools.h>

#include <imgui.h>

#include <cstdio>
#include <cstring>

namespace gui
{

PatternView::PatternView(
    use_cases::ProjectUseCases& project_use_cases,
    size_t pattern_index)
{
    char child_name[32];
    snprintf(
        child_name, sizeof(child_name), "PatternView%u",
        static_cast<unsigned>(pattern_index));

    constexpr float pattern_width = 80.f;
    constexpr float pattern_height = 64.f;

    ImGui::BeginChild(
        child_name, ImVec2 { pattern_width, pattern_height },
        ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

    ImGui::Dummy(ImGui::GetWindowSize());
    utils::ScopeGuard scope_guard { ImGui::EndChild };

    auto pattern = project_use_cases.get_pattern(pattern_index);

    if (pattern == nullptr)
    {
        return;
    }

    float note_width = pattern_width / static_cast<float>(pattern->noteCount());

    const ImColor color { 0x11, 0xff, 0x11 };

    const ImColor background_color = ImGui::IsItemHovered()
        ? GuiState::hoveredColor()
        : pattern_index == GuiState::patternIndex() ? GuiState::selectedColor()
                                                    : GuiState::idleColor();

    WindowDrawHelper window_draw_helper;

    char text_buffer[32];
    snprintf(
        text_buffer, sizeof(text_buffer), "Pattern %u",
        static_cast<unsigned>(pattern_index));

    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    window_draw_helper.addText(
        { 0, 0 }, ImColor { 0xff, 0xff, 0xff }, text_buffer);

    window_draw_helper.drawFilledRect(
        { 0, 0 }, { pattern_width, pattern_height }, background_color);

    for (size_t note_index = 0; note_index < pattern->noteCount(); ++note_index)
    {
        const auto& note = pattern->note(note_index);

        if (!note)
        {
            continue;
        }

        window_draw_helper.drawFilledRect(
            { static_cast<float>(note_index) * note_width,
              63.f - static_cast<float>(note->frequency()) },
            { note_width, 1.f }, color);
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        GuiState::setPatternIndex(pattern_index);
    }
}

}
