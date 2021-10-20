#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gps_socket_sample)

zephyr_library_sources(${NRF_DIR}/samples/nrf9160/gps/src/main.c)
zephyr_library_sources(src/startup.c)

zephyr_library_sources_ifdef(
  CONFIG_SUPL_CLIENT_LIB
  ${NRF_DIR}/samples/nrf9160/gps/src/supl_support.c
)
