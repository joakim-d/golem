add_library(golem_core
  src/apu.cpp
  src/edit.cpp
  src/instrument_fields.cpp
  src/notes.cpp
  src/player.cpp
  src/song.cpp
  src/song_binary.cpp
  src/song_text.cpp
  src/trace.cpp
  src/version.cpp)
add_library(golem::core ALIAS golem_core)

target_include_directories(golem_core PUBLIC include)
target_compile_definitions(golem_core PRIVATE GOLEM_VERSION="${PROJECT_VERSION}")

if(MSVC)
  target_compile_options(golem_core PRIVATE /W4 /WX)
else()
  target_compile_options(golem_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
