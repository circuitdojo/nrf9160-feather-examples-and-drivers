#
# Copyright (c) 2021 Circuit Dojo LLC
#
# SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
#

cmake_minimum_required(VERSION 3.13.4)

# Determine the build type
if (NOT BUILD_TYPE)
  set(BUILD_TYPE debug)
elseif(BUILD_TYPE STREQUAL "release")
  add_compile_definitions( BUILD_TYPE_RELEASE )
else()
  message( FATAL_ERROR "Unknown build type: ${BUILD_TYPE}" )
endif()

# Determine the backend type
if (NOT BACKEND)
  set(BACKEND golioth)
endif()

# Print out build type
message(STATUS "Build type: ${BUILD_TYPE} 🚀")
message(STATUS "Backend: ${BACKEND} 🎒")

# Set environment variables
set(ZEPHYR_TOOLCHAIN_VARIANT gnuarmemb)
# set(GNUARMEMB_TOOLCHAIN_PATH "C:\\Program Files (x86)\\GNU Tools Arm Embedded\\9 2019-q4-major")
# set(GNUARMEMB_TOOLCHAIN_PATH /opt/nordic/ncs/v1.5.1/toolchain)

# Get the version from Git
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
    message(STATUS "git describe failed: ${stderr}; ${KERNEL_VERSION_STRING} will be used instead")
  elseif(CMAKE_VERBOSE_MAKEFILE)
    message(STATUS "git describe stderr: ${stderr}")
  endif()
endif()

if(version)
  string(REGEX REPLACE "^([0-9]+).*$" "\\1" version_major ${version})
  string(REGEX REPLACE "^[0-9]+\.([0-9]+).*$" "\\1" version_minor "${version}")
  string(REGEX REPLACE "^[0-9]+\.[0-9]+\.([0-9]+).*$" "\\1" version_patch "${version}")
  string(REGEX REPLACE "^[0-9]+\.[0-9]+\.[0-9]+-([0-9]+)-.*$" "\\1" version_commit "${version}")
  string(REGEX REPLACE "^[0-9]+\.[0-9]+\.[0-9]+-[0-9]+-(.*)$" "\\1" version_hash "${version}")

  # Create version.conf
  file(WRITE conf/version.conf "CONFIG_APP_VERSION_MAJOR=${version_major}\n")
  file(APPEND conf/version.conf "CONFIG_APP_VERSION_MINOR=${version_minor}\n")
  file(APPEND conf/version.conf "CONFIG_APP_VERSION_PATCH=${version_patch}\n")
  file(APPEND conf/version.conf "CONFIG_APP_VERSION_COMMIT=${version_commit}\n")
  file(APPEND conf/version.conf "CONFIG_APP_VERSION_HASH=\"${version_hash}\"\n")
  file(APPEND conf/version.conf "CONFIG_APP_VERSION=\"${version}\"\n")

  message(STATUS "Version: ${version_major}.${version_minor}.${version_patch}-${version_commit}-${version_hash}")

endif()

# Define configuration files.
list(APPEND CONF_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/prj.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/backends/${BACKEND}.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/${BUILD_TYPE}.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/version.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf
  )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(tracker-demo)

# Application directories
add_subdirectory(src/storage)
add_subdirectory(src/gps)
add_subdirectory(src/event_manager)
add_subdirectory(src/codec)
add_subdirectory(src/backend)
add_subdirectory(src/battery)
add_subdirectory(src/motion)

target_sources(app PRIVATE src/main.c)