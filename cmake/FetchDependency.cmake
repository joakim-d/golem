# Generic helper to pull a third-party C++ library with FetchContent.
#
# golem_fetch_dependency(<name>
#   URL <url> [URL_HASH <algo>=<hash>]
#   | GIT_REPOSITORY <repo> GIT_TAG <tag>
#   [SOURCE_SUBDIR <dir>]
#   [OPTIONS "<var> <value>" ...]
#   [FIND_PACKAGE_ARGS <args>...])
#
# OPTIONS are set as variables before the dependency is configured, which is how
# its option() switches get overridden. When FIND_PACKAGE_ARGS is given, an installed
# package is tried first via find_package(<name> <args>) before downloading (see
# FETCHCONTENT_TRY_FIND_PACKAGE_MODE). Dependencies are added as SYSTEM so their
# headers do not trigger warnings in our targets.

include_guard(GLOBAL)

include(FetchContent)

function(golem_fetch_dependency name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG
    ""
    "URL;URL_HASH;GIT_REPOSITORY;GIT_TAG;SOURCE_SUBDIR"
    "OPTIONS;FIND_PACKAGE_ARGS")

  if(ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "golem_fetch_dependency(${name}): unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
  endif()

  set(declare_args)
  if(ARG_URL)
    list(APPEND declare_args URL "${ARG_URL}" DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    if(ARG_URL_HASH)
      list(APPEND declare_args URL_HASH "${ARG_URL_HASH}")
    endif()
  elseif(ARG_GIT_REPOSITORY)
    if(NOT ARG_GIT_TAG)
      message(FATAL_ERROR "golem_fetch_dependency(${name}): GIT_REPOSITORY requires GIT_TAG")
    endif()
    list(APPEND declare_args
      GIT_REPOSITORY "${ARG_GIT_REPOSITORY}"
      GIT_TAG "${ARG_GIT_TAG}"
      GIT_SHALLOW TRUE)
  else()
    message(FATAL_ERROR "golem_fetch_dependency(${name}): URL or GIT_REPOSITORY is required")
  endif()

  if(ARG_SOURCE_SUBDIR)
    list(APPEND declare_args SOURCE_SUBDIR "${ARG_SOURCE_SUBDIR}")
  endif()

  if(ARG_FIND_PACKAGE_ARGS OR "FIND_PACKAGE_ARGS" IN_LIST ARG_KEYWORDS_MISSING_VALUES)
    list(APPEND declare_args FIND_PACKAGE_ARGS ${ARG_FIND_PACKAGE_ARGS})
  endif()

  # Let normal variables override option() in the dependency (policy CMP0077).
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  foreach(option IN LISTS ARG_OPTIONS)
    string(REGEX MATCH "^([^ ]+) (.*)$" _ "${option}")
    if(NOT CMAKE_MATCH_1)
      message(FATAL_ERROR "golem_fetch_dependency(${name}): option must be \"<var> <value>\", got \"${option}\"")
    endif()
    set(${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
  endforeach()

  FetchContent_Declare(${name} ${declare_args} SYSTEM)
  FetchContent_MakeAvailable(${name})
endfunction()
