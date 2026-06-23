cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
  Rally2D
  VERSION 1.0.0
  LANGUAGES C)

set(PROGRAM_NAME rally2d)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(raylib REQUIRED QUIET)

set(SOURCES
    src/main.c
    src/assets.c
    src/background.c
    src/collision.c
    src/game.c
    src/input.c
    src/render.c
    src/sprite.c)

add_executable(${PROGRAM_NAME} ${SOURCES})

target_include_directories(${PROGRAM_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

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

add_custom_command(
  TARGET ${PROGRAM_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets
          $<TARGET_FILE_DIR:${PROGRAM_NAME}>/assets
  COMMENT "Copyng assets...")

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
