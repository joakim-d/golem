project(golem_lib)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
file(GLOB_RECURSE HEADERS "include/*.h")

add_library(
    ${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    SDL2::Main
    SDL2::Image
    SDL2::TTF
    nlohmann_json::nlohmann_json
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
