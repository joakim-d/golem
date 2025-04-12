file(GLOB
    IMGUI_SRC
    "imgui/*.cpp"
    "imgui/*.h"
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_opengl3.cpp"
)

add_library(
    imgui
    ${IMGUI_SRC}
)

target_include_directories(
    imgui
    PUBLIC
    imgui
)
