#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.13.1)

# Adding custom overlay to add settings that aren't included in the SDK (yet) for the nRF9160 Feather
if(${BOARD} STREQUAL circuitdojo_feather_nrf9160ns)
message(STATUS "Adding .overlay for mcuboot.")
list(APPEND mcuboot_DTC_OVERLAY_FILE
  "${CMAKE_CURRENT_LIST_DIR}/../../shared/conf/mcuboot/circuitdojo_feather_nrf9160ns.overlay"
  )
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gps_socket_sample)

zephyr_library_sources(${NRF_DIR}/samples/nrf9160/gps/src/main.c)

zephyr_library_sources_ifdef(
  CONFIG_SUPL_CLIENT_LIB
  ${NRF_DIR}/samples/nrf9160/gps/src/supl_support.c
)
