cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
  MovementFunctions
  VERSION 1.0.0
  LANGUAGES C)

set(PROGRAM_NAME test)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(raylib REQUIRED QUIET)

set(SOURCES src/main.c)

add_executable(${PROGRAM_NAME} ${SOURCES})

target_link_libraries(
  ${PROGRAM_NAME}
  PRIVATE raylib
          wayland-client
          wayland-egl
          EGL
          xkbcommon
          m
          pthread
          dl)

target_compile_options(${PROGRAM_NAME} PRIVATE -Wall -Wextra -Wpedantic
                                               -Wconversion)
