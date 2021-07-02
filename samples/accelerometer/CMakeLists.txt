#
# Copyright (c) 2018, Yannis Damigos
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

# Adding custom overlay to add settings that aren't included in the SDK (yet) for the nRF9160 Feather
if(${BOARD} STREQUAL circuitdojo_feather_nrf9160ns)
message(STATUS "Adding .overlay for mcuboot.")
list(APPEND mcuboot_DTC_OVERLAY_FILE
  "${CMAKE_CURRENT_LIST_DIR}/../../shared/conf/mcuboot/circuitdojo_feather_nrf9160ns.overlay"
  )
endif()

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(accel)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
