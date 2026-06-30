cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
  Spaceship
  VERSION 1.0.0
  LANGUAGES C)

set(PROGRAM_NAME game)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(AUDIO_MUTED "Silence all audio (useful during development/testing)" OFF)
option(ONLY_SHAPE "Hide images if loaded (useful during development/testing)"
       OFF)

find_package(raylib REQUIRED QUIET)

set(SOURCES src/main.c)

add_executable(${PROGRAM_NAME} ${SOURCES})

if(AUDIO_MUTED)
  target_compile_definitions(${PROGRAM_NAME} PRIVATE AUDIO_MUTED)
  message(STATUS "AUDIO_MUTED: audio disabled for this build")
endif()

if(ONLY_SHAPE)
  target_compile_definitions(${PROGRAM_NAME} PRIVATE ONLY_SHAPE)
  message(STATUS "ONLY_SHAPE: images disabled for this build")
endif()

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
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROGRAM_NAME}>/resources
  COMMENT "Copyng resources...")

target_compile_options(${PROGRAM_NAME} PRIVATE -Wall -Wextra -Wpedantic
                                               -Wconversion)
