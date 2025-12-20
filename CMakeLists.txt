cmake_minimum_required(VERSION 3.15)
project(
  hawthorn
  VERSION 0.0.1
  LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Build type")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_FLAGS_RELWITHDEBINFO
    "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -fno-omit-frame-pointer -O2")

if(MSVC)
  set(CMAKE_C_STANDARD 17)
else()
  set(CMAKE_C_STANDARD 23)
endif()

set(CMAKE_C_STANDARD_REQUIRED ON)

set(DEFAULT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/includes")

file(
  GLOB_RECURSE
  SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.c"
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.cc"
  "${CMAKE_SOURCE_DIR}/src/*.cxx"
  "${CMAKE_SOURCE_DIR}/src/*.c++")

add_subdirectory(src/share)
add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/interpreter)
add_subdirectory(src/value)

add_executable(hawthorn src/main.c)

target_include_directories(hawthorn PRIVATE ${DEFAULT_INCLUDE_DIR})

target_link_libraries(
  hawthorn
  PRIVATE megonil_error
          megonil_array
          megonil_string
          hawthorn_interpreter
          hawthorn_lexer
          hawthorn_parser
          m)
