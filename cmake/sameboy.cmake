# SameBoy: an accurate Game Boy emulator, used to cross-check the driver tests run in
# Peanut-GB. Only its emulator core is built (no debugger, cheats or rewind).
# The core relies on GNU C extensions, so it is skipped with MSVC: GOLEM_HAS_SAMEBOY tells
# whether the target sameboy::core exists.

include_guard(GLOBAL)

option(GOLEM_SAMEBOY "Build SameBoy to cross-check the driver tests (GCC or Clang only)" ON)

set(GOLEM_HAS_SAMEBOY OFF)
if(MSVC)
  message(STATUS "SameBoy cross-check disabled: its core needs GCC or Clang")
  return()
endif()
if(NOT GOLEM_SAMEBOY)
  message(STATUS "SameBoy cross-check disabled (GOLEM_SAMEBOY=OFF)")
  return()
endif()

include(FetchDependency)

golem_fetch_dependency(sameboy
  URL https://github.com/LIJI32/SameBoy/archive/refs/tags/v1.0.3.tar.gz
  URL_HASH SHA256=7da338458e19396cb43dfe1a4df4555882ebea92540565dc993c1c706c981dc3)

file(GLOB sameboy_sources "${sameboy_SOURCE_DIR}/Core/*.c")
# The files of the features SameBoy's Makefile leaves out when they are disabled.
list(FILTER sameboy_sources EXCLUDE REGEX
  "/(cheats|cheat_search|debugger|rewind|sm83_disassembler|symbol_hash)\\.c$")

add_library(sameboy_core STATIC ${sameboy_sources})
add_library(sameboy::core ALIAS sameboy_core)
set_target_properties(sameboy_core PROPERTIES C_STANDARD 11 C_EXTENSIONS ON)
target_compile_definitions(sameboy_core
  PRIVATE
    GB_INTERNAL
    _GNU_SOURCE
    GB_VERSION="1.0.3"
    GB_COPYRIGHT_YEAR="2026"
  PUBLIC
    GB_DISABLE_DEBUGGER
    GB_DISABLE_CHEATS
    GB_DISABLE_CHEAT_SEARCH
    GB_DISABLE_REWIND)
# Third-party code: no warnings, and always optimized (it is slow unoptimized, even in Debug).
target_compile_options(sameboy_core PRIVATE -w -O2)
target_include_directories(sameboy_core SYSTEM PUBLIC "${sameboy_SOURCE_DIR}")
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(sameboy_core PUBLIC ${MATH_LIBRARY})
endif()

set(GOLEM_HAS_SAMEBOY ON)
