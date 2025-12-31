add_library(hawthorn_parser STATIC parser.c)
add_library(hawthorn_compiler STATIC compiler.c)
target_include_directories(hawthorn_parser PUBLIC "${DEFAULT_INCLUDE_DIR}")
target_include_directories(hawthorn_compiler PUBLIC "${DEFAULT_INCLUDE_DIR}")
target_link_libraries(
  hawthorn_parser PRIVATE hawthorn_value hawthorn_lexer hawthorn_chunk
                          hawthorn_table hawthorn_compiler)
