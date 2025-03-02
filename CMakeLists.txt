cmake_minimum_required(VERSION 3.16)

project(golem LANGUAGES CXX)

option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_UNIT_TESTS "Enable unit test project" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(DirectoryCopy)

add_subdirectory(
    submodules/json
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
file(GLOB_RECURSE RESOURCES "resources/*")

if(ENABLE_ASAN)
    add_compile_options("-fsanitize=address")
    add_link_options("-fsanitize=address")
endif()

add_subdirectory(lib/golem)

add_executable(
   golem
   ${SOURCES}
   ${RESOURCES}
)

target_include_directories(
    golem
    PRIVATE
    src
)

target_link_libraries(
    golem
    PRIVATE
    golem_lib
)

if(ENABLE_UNIT_TESTS)
    add_subdirectory(test/unit)
endif()

directory_copy(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_BINARY_DIR}/resources
)

include(GNUInstallDirs)
# install(TARGETS golem
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
