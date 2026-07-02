cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
  PathfindingGame
  VERSION 1.0.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(ONLY_SHAPE "Hide images if loaded" OFF)

find_package(raylib REQUIRED QUIET)

# Each name here becomes its own executable (game_v1, game_v2, ...), built from
# src/main_<version>.c, so both experiments can be run and compared side by side
# without reconfiguring anything.
set(GAME_VERSIONS v1 v2)

function(add_game_variant version)
  set(target game_${version})
  set(source ${CMAKE_CURRENT_SOURCE_DIR}/src/main_${version}.c)

  if(NOT EXISTS ${source})
    message(
      FATAL_ERROR
        "GAME_VERSIONS lists '${version}' but ${source} does not exist")
  endif()

  add_executable(${target} ${source})

  target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic
                                           -Wconversion)

  target_link_libraries(
    ${target}
    PRIVATE raylib
            wayland-client
            wayland-egl
            EGL
            xkbcommon
            m
            pthread
            dl)

  if(ONLY_SHAPE)
    target_compile_definitions(${target} PRIVATE ONLY_SHAPE)
  endif()

  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources
      $<TARGET_FILE_DIR:${target}>/resources
    COMMENT "Copying resources for ${target}...")
endfunction()

foreach(version IN LISTS GAME_VERSIONS)
  add_game_variant(${version})
endforeach()

if(ONLY_SHAPE)
  message(STATUS "ONLY_SHAPE: images disabled for this build")
endif()
