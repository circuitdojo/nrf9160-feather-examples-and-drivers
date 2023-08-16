# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND git describe --tags --long
    WORKING_DIRECTORY                ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE                  version
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE                   stderr
    RESULT_VARIABLE                  return_code
  )
  if(return_code)
    message(STATUS "git describe failed: ${return_code}")
  elseif(CMAKE_VERBOSE_MAKEFILE)
    message(STATUS "git describe stderr: ${stderr}")
  endif()
endif()

if(version)
  # Create version.conf
  file(WRITE version.conf "CONFIG_APP_VERSION=\"${version}\"\n")
  message(STATUS "Version: ${version}")
endif()


# Load NCS/Zephyr build system
set(CONF_FILE prj.conf)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf)
  list(APPEND CONF_FILE boards/${BOARD}.conf)
endif()

# TODO: change to EXTRA_CONF_FILE >= Zephyr 3.4
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.conf)
  list(APPEND CONF_FILE version.conf)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tracker)

target_sources(app PRIVATE src/main.c)

# Application directories
add_subdirectory(src/backend)
add_subdirectory(src/battery)
add_subdirectory(src/codec)
add_subdirectory(src/event_manager)
add_subdirectory(src/gps)
add_subdirectory(src/modem)
add_subdirectory(src/motion)
add_subdirectory(src/shell)
add_subdirectory(src/storage)