add_library(hawthorn_compiler STATIC compiler.c)
target_include_directories(hawthorn_compiler PUBLIC "${DEFAULT_INCLUDE_DIR}")
