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

# MCUboot related
list(APPEND mcuboot_OVERLAY_CONFIG
  "${CMAKE_CURRENT_SOURCE_DIR}/conf/mcuboot/mcuboot.conf"
  )

# Adding custom overlay to add settings that aren't included in the SDK (yet) for the nRF9160 Feather
if(${BOARD} STREQUAL circuitdojo_feather_nrf9160ns)
message(STATUS "Adding .overlay for mcuboot.")
list(APPEND mcuboot_DTC_OVERLAY_FILE
  "${CMAKE_CURRENT_SOURCE_DIR}/conf/mcuboot/circuitdojo_feather_nrf9160ns.overlay"
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
set(CONF_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/version.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/${BUILD_TYPE}.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/backend/${BACKEND}.conf
  )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(asset_tracker_v2)

# NORDIC SDK APP START
target_sources(app PRIVATE src/main.c)
# NORDIC SDK APP END

# Include source directory
target_include_directories(app PRIVATE src)

# Application directories
add_subdirectory(src/modules)
add_subdirectory(src/events)

add_subdirectory_ifdef(CONFIG_CLOUD_MODULE src/cloud)
add_subdirectory_ifdef(CONFIG_UI_MODULE src/led)
add_subdirectory_ifdef(CONFIG_SENSOR_MODULE src/ext_sensors)
add_subdirectory_ifdef(CONFIG_WATCHDOG_APPLICATION src/watchdog)

target_sources_ifdef(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160NS app PRIVATE
                        src/boards/circuitdojo_feather_nrf9160ns/*.c)
