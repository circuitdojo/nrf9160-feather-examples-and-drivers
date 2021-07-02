#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.13.1)

# Determine the build type
if (NOT BUILD_TYPE)
  set(BUILD_TYPE debug)
endif()

# Determine the backend environment
if (NOT BACKEND)
  set(BACKEND pyrinas)
endif()

# Print out build type
message(STATUS "Build type: ${BUILD_TYPE} 🚀")
message(STATUS "Backend: ${BACKEND} 🏓")

# Apply zephyr patch 
execute_process(COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/patch/zephyr.diff --directory=zephyr WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../../ OUTPUT_QUIET ERROR_QUIET)

# Adding custom overlay to add settings that aren't included in the SDK (yet) for the nRF9160 Feather
if(${BOARD} STREQUAL circuitdojo_feather_nrf9160ns)
message(STATUS "Adding .overlay for mcuboot.")
list(APPEND mcuboot_DTC_OVERLAY_FILE
  "${CMAKE_CURRENT_LIST_DIR}/../../shared/conf/mcuboot/circuitdojo_feather_nrf9160ns.overlay"
  )
endif()

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
  file(WRITE version.conf "CONFIG_PYRINAS_APP_VERSION_MAJOR=${version_major}\n")
  file(APPEND version.conf "CONFIG_PYRINAS_APP_VERSION_MINOR=${version_minor}\n")
  file(APPEND version.conf "CONFIG_PYRINAS_APP_VERSION_PATCH=${version_patch}\n")
  file(APPEND version.conf "CONFIG_PYRINAS_APP_VERSION_COMMIT=${version_commit}\n")
  file(APPEND version.conf "CONFIG_PYRINAS_APP_VERSION_HASH=\"${version_hash}\"\n")

  message(STATUS "Version: ${version_major}.${version_minor}.${version_patch}-${version_commit}-${version_hash}")

endif()

# Define configuration files.
list(APPEND CONF_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/version.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/${BUILD_TYPE}.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/backend/${BACKEND}.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf
  )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(asset_tracker_v2)

# Setting example directory path
set(ASSET_TRACKER_PATH ${NRF_DIR}/applications/asset_tracker_v2)

# NORDIC SDK APP START
target_sources(app PRIVATE ${ASSET_TRACKER_PATH}/src/main.c)
# NORDIC SDK APP END

# Include source directory
target_include_directories(app PRIVATE ${ASSET_TRACKER_PATH}/src)

# Application directories
add_subdirectory(src/modules)
add_subdirectory(src/boards)

add_subdirectory(src/cloud)
add_subdirectory(src/ext_sensors)

add_subdirectory(${ASSET_TRACKER_PATH}/src/events src/_events)
add_subdirectory(${ASSET_TRACKER_PATH}/src/cloud src/_cloud)
add_subdirectory(${ASSET_TRACKER_PATH}/src/led src/_led)
add_subdirectory(${ASSET_TRACKER_PATH}/src/watchdog src/_watchdog)
