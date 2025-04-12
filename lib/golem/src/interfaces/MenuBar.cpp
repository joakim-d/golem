#include <interfaces/MenuBar.h>

#include <imgui.h>

#include <iostream>

#include <GLFW/glfw3.h>

namespace gui
{

void MenuBar::render(GLFWwindow* window)
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New", "Ctrl+O"))
        {
            std::cout << "New" << std::endl;
        }

        if (ImGui::MenuItem("Open"))
        {
        }

        if (ImGui::MenuItem("Exit"))
        {
            glfwSetWindowShouldClose(window, 1);
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

}
