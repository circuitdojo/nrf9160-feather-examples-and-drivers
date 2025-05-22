# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(battery)

# If board is nRF9160 add the source files 
if (CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160)
    FILE(GLOB app_sources ${ZEPHYR_BASE}/samples/boards/nrf/battery/src/*.c)
    target_sources(app PRIVATE ${app_sources})
elseif(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9161 OR CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9151)
    target_sources(app PRIVATE src/main.c)
else()
    message(FATAL_ERROR "Unsupported board")
endif()
