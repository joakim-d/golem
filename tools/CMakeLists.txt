add_subdirectory(run)

add_executable(golem-trace trace_main.cpp)
target_link_libraries(golem-trace PRIVATE golem::core)

add_executable(golem-tracediff tracediff_main.cpp)
target_link_libraries(golem-tracediff PRIVATE golem::core)

add_executable(golem-run run_main.cpp)
target_link_libraries(golem-run PRIVATE golem::run)

add_executable(golem-encode encode_main.cpp)
target_link_libraries(golem-encode PRIVATE golem::core)

add_executable(golem-tables tables_main.cpp)
target_link_libraries(golem-tables PRIVATE golem::core)
