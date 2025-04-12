#include <interfaces/SequencerHeaderView.h>

#include <interfaces/ImGuiTools.h>

#include <golem/utils/scope_guard.h>

#include <imgui.h>

#include <cstdio>

namespace gui
{

constexpr auto PHRASE_COUNT = 16;
constexpr auto HEADER_WIDTH = 256;
constexpr auto HEADER_HEIGHT = 16;

SequencerHeaderView::SequencerHeaderView(float x_offset)
{
    ImGui::BeginChild(
        "SequencerHeaderView", ImVec2 { -1, HEADER_HEIGHT }, 0,
        ImGuiWindowFlags_NoScrollbar);

    utils::ScopeGuard scope_guard { ImGui::EndChild };

    WindowDrawHelper window_draw_helper;

    const auto window_size = ImGui::GetWindowSize();

    const ImVec2 size { HEADER_WIDTH, HEADER_HEIGHT };
    const ImColor color { 0x11, 0x11, 0x11 };
    ImVec2 position { 0, 0 };

    int index = (x_offset / HEADER_WIDTH) - 1;
    const int max_index = index + (window_size.x / HEADER_WIDTH) + 2;

    char buffer[32];
    for (; index < max_index; ++index)
    {
        position.x = (index * HEADER_WIDTH) - x_offset;
        window_draw_helper.drawFilledRect(position, size, color);

        snprintf(buffer, sizeof(buffer), "%d", index + 1);
        window_draw_helper.addText(
            position, ImColor { 0xFF, 0xFF, 0xFF }, buffer);
    }

    ImGui::Dummy({ -1, HEADER_HEIGHT });
}

float SequencerHeaderView::viewHeight()
{
    return HEADER_HEIGHT;
}

}
