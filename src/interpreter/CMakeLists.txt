add_library(hawthorn_interpreter STATIC vm.c)

add_subdirectory(chunk)

target_include_directories(hawthorn_interpreter PUBLIC "${DEFAULT_INCLUDE_DIR}")
target_link_libraries(hawthorn_interpreter
                      PRIVATE hawthorn_value hawthorn_chunk hawthorn_table)
