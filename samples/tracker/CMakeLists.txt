#
# Copyright (c) 2021 Circuit Dojo LLC
#

cmake_minimum_required(VERSION 3.13.4)

# Determine the build type
if (NOT BUILD_TYPE)
  set(BUILD_TYPE debug)
endif()

# Print out build type
message(STATUS "Build type: ${BUILD_TYPE} 🚀")

# Set environment variables
set(ZEPHYR_TOOLCHAIN_VARIANT gnuarmemb)
# set(GNUARMEMB_TOOLCHAIN_PATH "C:\\Program Files (x86)\\GNU Tools Arm Embedded\\9 2019-q4-major")
set(GNUARMEMB_TOOLCHAIN_PATH /opt/nordic/ncs/v1.5.1/toolchain)

# Define configuration files.
list(APPEND CONF_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/prj.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/golioth.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/backend.conf
  ${CMAKE_CURRENT_SOURCE_DIR}/conf/${BUILD_TYPE}.conf
  )

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

message(STATUS "Adding nRF9160 Feather .conf overlay.")
list(APPEND CONF_FILE
  ${CMAKE_CURRENT_SOURCE_DIR}/boards/${BOARD}.conf
)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(golioth-demo)

# Application directories
add_subdirectory(src/storage)
add_subdirectory(src/gps)
add_subdirectory(src/event_manager)
add_subdirectory(src/codec)

# External libraries
add_subdirectory(lib)

target_sources(app PRIVATE src/main.c)