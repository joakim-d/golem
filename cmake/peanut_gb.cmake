# Peanut-GB: single-header Game Boy emulator, used headless to log the driver's APU writes.
# Provides the header-only target peanut_gb::peanut_gb; one C file must include it with the
# implementation (see tools/run/src/gb_core.c).

include_guard(GLOBAL)

include(FetchDependency)

golem_fetch_dependency(peanut_gb
  URL https://github.com/deltabeard/Peanut-GB/archive/refs/tags/v1.3.0.tar.gz
  URL_HASH SHA256=a45ee6d15a1951e2573c9398d05266beff074ee01987af93cd55a71b5baaa48e)

add_library(peanut_gb INTERFACE)
add_library(peanut_gb::peanut_gb ALIAS peanut_gb)
target_include_directories(peanut_gb SYSTEM INTERFACE "${peanut_gb_SOURCE_DIR}")
