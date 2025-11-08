add_library(hawthorn_interpreter STATIC interpreter.c)
target_include_directories(hawthorn_interpreter PUBLIC "${DEFAULT_INCLUDE_DIR}")
