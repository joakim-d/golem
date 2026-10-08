# golem-editor: the tracker window (docs/editor-steps/02-editor-window.md).

add_executable(golem-editor
  app.cpp
  keys.cpp
  main.cpp)
target_link_libraries(golem-editor PRIVATE golem::core golem::run imgui::imgui SDL3::SDL3)
if(WIN32)
  set_target_properties(golem-editor PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(MSVC)
  target_compile_options(golem-editor PRIVATE /W4 /WX)
else()
  target_compile_options(golem-editor PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Smoke test: the editor opens a song, plays it and draws frames headless, then quits.
# SDL's dummy video driver draws in a software framebuffer. The offscreen driver advertises
# OpenGL (through EGL), so on macOS SDL makes the window an OpenGL one, which fails there.
if(BUILD_TESTING)
  add_test(NAME editor.smoke
    COMMAND golem-editor ${PROJECT_SOURCE_DIR}/tests/songs/scale.gsong --play
      --quit-after-frames 60)
  set_tests_properties(editor.smoke PROPERTIES
    ENVIRONMENT "SDL_VIDEO_DRIVER=dummy;SDL_AUDIO_DRIVER=dummy")
endif()
