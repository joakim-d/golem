#include <interfaces/ImGuiTools.h>

namespace gui
{

WindowDrawHelper::WindowDrawHelper(ImVec2 position)
    : m_draw_list(ImGui::GetWindowDrawList())
    , m_window_position(position)
{
}

void WindowDrawHelper::drawFilledRect(ImVec2 position, ImVec2 size, ImU32 color)
{
    m_draw_list->AddRectFilled(
        position + m_window_position, position + m_window_position + size,
        color);
}

void WindowDrawHelper::drawRect(ImVec2 position, ImVec2 size, ImU32 color)
{
    m_draw_list->AddRect(
        position + m_window_position, position + m_window_position + size,
        color);
}

void WindowDrawHelper::addText(ImVec2 position, ImU32 color, const char* text)
{
    m_draw_list->AddText(position + m_window_position, color, text);
}

ImVec2 operator+(ImVec2 l, ImVec2 r)
{
    return { l.x + r.x, l.y + r.y };
}

ImVec2 operator-(ImVec2 l, ImVec2 r)
{
    return { l.x - r.x, l.y - r.y };
}

bool IsMouseChildClicked(ImGuiMouseButton button)
{
    const auto window_position = ImGui::GetWindowPos();
    return ImGui::IsMouseClicked(button)
        && ImGui::IsMouseHoveringRect(
               window_position, window_position + ImGui::GetWindowSize());
}

bool IsMouseChildDown(ImGuiMouseButton button)
{
    const auto window_position = ImGui::GetWindowPos();
    return ImGui::IsMouseDown(button)
        && ImGui::IsMouseHoveringRect(
               window_position, window_position + ImGui::GetWindowSize());
}

ImVec2 GetMouseToWindowPosition()
{
    return ImGui::GetMousePos() - ImGui::GetWindowPos();
}

}
