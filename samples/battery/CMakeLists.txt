# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)



find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(battery)

FILE(GLOB app_sources ${ZEPHYR_BASE}/samples/boards/nrf/battery/src/*.c)
target_sources(app PRIVATE ${app_sources})
