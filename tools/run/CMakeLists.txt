# Peanut-GB glue, built apart so that third-party warnings do not hit our warning flags.
add_library(golem_gb_core STATIC src/gb_core.c)
target_link_libraries(golem_gb_core PRIVATE peanut_gb::peanut_gb)

add_library(golem_run
  src/rom_runner.cpp)
add_library(golem::run ALIAS golem_run)

target_include_directories(golem_run PUBLIC include)
target_link_libraries(golem_run PUBLIC golem::core PRIVATE golem_gb_core)

if(MSVC)
  target_compile_options(golem_run PRIVATE /W4 /WX)
else()
  target_compile_options(golem_run PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(BUILD_TESTING)
  add_executable(golem_run_tests tests/rom_runner_test.cpp)
  target_link_libraries(golem_run_tests PRIVATE golem::run GTest::gtest_main)
  gtest_discover_tests(golem_run_tests)
endif()
