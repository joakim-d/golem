#pragma once

#include <imgui.h>

namespace gui
{

class WindowDrawHelper
{
public:
    WindowDrawHelper(ImVec2 offset = ImGui::GetWindowPos());

    void drawRect(ImVec2 position, ImVec2 size, ImU32 color);
    void drawFilledRect(ImVec2 position, ImVec2 size, ImU32 color);
    void addText(ImVec2 position, ImU32 color, const char* text);

private:
    ImDrawList* m_draw_list;
    const ImVec2 m_window_position;
};

ImVec2 operator+(ImVec2 l, ImVec2 r);
ImVec2 operator-(ImVec2 l, ImVec2 r);

bool IsMouseChildClicked(ImGuiMouseButton button);
bool IsMouseChildDown(ImGuiMouseButton button);
ImVec2 GetMouseToWindowPosition();

}
