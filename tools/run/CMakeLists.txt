# Peanut-GB glue, built apart so that third-party warnings do not hit our warning flags.
add_library(golem_gb_core STATIC src/gb_core.c)
target_link_libraries(golem_gb_core PRIVATE peanut_gb::peanut_gb)

add_library(golem_run
  src/rom_runner.cpp)
add_library(golem::run ALIAS golem_run)

target_include_directories(golem_run PUBLIC include)
target_link_libraries(golem_run PUBLIC golem::core PRIVATE golem_gb_core)

# SameBoy glue, when SameBoy is built (GCC or Clang): the cross-checking emulator.
if(GOLEM_HAS_SAMEBOY)
  add_library(golem_sameboy_core STATIC src/sameboy_core.c)
  set_target_properties(golem_sameboy_core PROPERTIES C_EXTENSIONS ON)
  target_link_libraries(golem_sameboy_core PRIVATE sameboy::core)
  target_link_libraries(golem_run PRIVATE golem_sameboy_core)
  target_compile_definitions(golem_run PRIVATE GOLEM_HAS_SAMEBOY)
endif()

if(MSVC)
  target_compile_options(golem_run PRIVATE /W4 /WX)
else()
  target_compile_options(golem_run PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(BUILD_TESTING)
  add_executable(golem_run_tests tests/rom_runner_test.cpp)
  target_link_libraries(golem_run_tests PRIVATE golem::run GTest::gtest_main)
  if(GOLEM_HAS_SAMEBOY)
    target_compile_definitions(golem_run_tests PRIVATE GOLEM_HAS_SAMEBOY)
  endif()
  gtest_discover_tests(golem_run_tests)
endif()
