# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

message(STATUS "Applying lis2dh diff")
execute_process(COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/patch/lis2dh.diff --directory=zephyr WORKING_DIRECTORY ${NRF9160_FEATHER_ROOT_DIR}/../ OUTPUT_QUIET ERROR_QUIET)



find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(deep_sleep)

target_sources(app PRIVATE src/main.c)
