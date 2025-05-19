#include <golem/interfaces/gui/MainWindow.h>

#include <GL/glew.h> // Initialize with glewInit()

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#include <iostream>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <interfaces/ConfigurationView.h>
#include <interfaces/MainView.h>
#include <interfaces/MenuBar.h>
#include <interfaces/SongView.h>

namespace gui {

// Process all input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// GLFW: whenever the window size changed (by OS or user resize) this callback
// function executes
void framebuffer_size_callback(
	GLFWwindow* window,
	int width,
	int height)
{
    // Make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}

std::optional<MainWindow> MainWindow::create(
	int width,
	int height,
	const char* title)
{
    // Initialize GLFW
	if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return {};
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFWwindow object
	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return {};
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Set up ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW and OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    MainWindow result;

    result.m_window = window;

    return result;
}

int MainWindow::execute(
    use_cases::ProjectUseCases& use_cases,
    domain::events::ProjectDomainEventProcessor& event_processor)
{
    MenuBar menu;
    // Render loop

	size_t song_index {0};
	size_t track_index {0};
	while (!glfwWindowShouldClose(m_window)) {
        // Input
        processInput(m_window);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		menu.render(m_window);
        auto io = ImGui::GetIO();

		ImGui::GetStyle().ItemSpacing = ImVec2 {0, 0};
		ImGui::GetStyle().ItemInnerSpacing = ImVec2 {0, 0};
		const auto background_color = ImColor {0x33, 0x33, 0x33};
        ImGui::PushStyleColor(ImGuiCol_WindowBg, background_color.Value);
        // ImGui::SetNextWindowPos({ 0, menuHeight });
		ImGui::SetNextWindowSize(ImVec2 {io.DisplaySize.x, io.DisplaySize.y});
		ImGui::Begin(
			"Main Content Area",
			nullptr,
			ImGuiWindowFlags_NoDecoration
				| ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoMove);

		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

        const auto window_size = ImGui::GetWindowSize();
		ImGui::BeginChild("SongAndMainView", {window_size.x - 256, -1});
		SongView song_view {use_cases, song_index};
		MainView main_view {use_cases, song_index, track_index};
        ImGui::EndChild();
        ImGui::SameLine();
		ConfigurationView configuration_view {use_cases};

        ImGui::End();
        ImGui::PopStyleColor();

        // Render ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        // glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();

    return 0;
}

}
