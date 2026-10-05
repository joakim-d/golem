add_executable(golem-trace trace_main.cpp)
target_link_libraries(golem-trace PRIVATE golem::core)

add_executable(golem-tracediff tracediff_main.cpp)
target_link_libraries(golem-tracediff PRIVATE golem::core)
