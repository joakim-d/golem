cmake_minimum_required(VERSION 3.16)

project(sdl_ecs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(
    sdl_ecs
    ${SOURCES}
)

target_include_directories(
    sdl_ecs
    PRIVATE
    src
)

target_link_libraries(
    sdl_ecs
    PRIVATE
    SDL2::Main
    SDL2::Image
    SDL2::TTF
)

include(GNUInstallDirs)
install(TARGETS sdl_ecs
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
