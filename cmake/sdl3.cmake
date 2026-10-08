# SDL 3: window, input, audio and native file dialogs for the editor. Provides SDL3::SDL3.
# An installed SDL3 (CMake package) is used first; otherwise it is built as a static library.

include_guard(GLOBAL)

include(FetchDependency)

golem_fetch_dependency(SDL3
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18.tar.gz
  URL_HASH SHA256=9c75cf16330322c217dedd2e0609f1124f1b54b8633e763467b4684d0f4334a3
  OPTIONS
    "SDL_SHARED OFF"
    "SDL_STATIC ON"
    "SDL_TEST_LIBRARY OFF"
    "SDL_TESTS OFF"
    "SDL_EXAMPLES OFF"
    "SDL_INSTALL OFF"
  FIND_PACKAGE_ARGS 3 CONFIG)
