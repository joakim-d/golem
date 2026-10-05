add_executable(golem_core_tests
  apu_test.cpp
  golden_test.cpp
  notes_test.cpp
  player_test.cpp
  song_test.cpp
  song_binary_test.cpp
  song_text_test.cpp
  trace_test.cpp
  version_test.cpp)
target_link_libraries(golem_core_tests PRIVATE golem::core GTest::gtest_main)
target_compile_definitions(golem_core_tests
  PRIVATE GOLEM_TEST_SONGS_DIR="${PROJECT_SOURCE_DIR}/tests/songs")

gtest_discover_tests(golem_core_tests)
