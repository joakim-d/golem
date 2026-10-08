cmake_minimum_required(VERSION 3.25)

project(golem VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include(CTest)

if(BUILD_TESTING)
  include(gtest)
endif()
include(peanut_gb)
include(sameboy)

add_subdirectory(core)
add_subdirectory(driver)
add_subdirectory(tools)

# clang-format targets: `format` rewrites sources, `format-check` fails on diffs.
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  file(GLOB_RECURSE GOLEM_FORMAT_SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/core/*.cpp
    ${PROJECT_SOURCE_DIR}/core/*.h
    ${PROJECT_SOURCE_DIR}/tools/*.cpp
    ${PROJECT_SOURCE_DIR}/tools/*.h)
  add_custom_target(format
    COMMAND ${CLANG_FORMAT} -i ${GOLEM_FORMAT_SOURCES}
    COMMENT "Running clang-format")
  add_custom_target(format-check
    COMMAND ${CLANG_FORMAT} --dry-run --Werror ${GOLEM_FORMAT_SOURCES}
    COMMENT "Checking clang-format")
endif()
