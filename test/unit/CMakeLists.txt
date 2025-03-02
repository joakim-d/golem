project(golem_unit_test)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(
    golem_unit_test
    ${SOURCES}
)

target_link_libraries(
    golem_unit_test
    PRIVATE
    golem_lib
    GTest::gtest
    GTest::gmock
)

target_include_directories(
    golem_unit_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(golem_unit_test)
