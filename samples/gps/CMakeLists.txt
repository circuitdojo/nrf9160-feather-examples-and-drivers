#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gnss_sample)

zephyr_library_sources(src/startup.c)
zephyr_library_sources(${NRF_DIR}/samples/cellular/gnss/src/main.c)

zephyr_library_sources_ifdef(CONFIG_GNSS_SAMPLE_ASSISTANCE_NRF_CLOUD ${NRF_DIR}/samples/cellular/gnss/src/assistance.c)
zephyr_library_sources_ifdef(CONFIG_GNSS_SAMPLE_ASSISTANCE_SUPL ${NRF_DIR}/samples/cellular/gnss/src/assistance_supl.c)
zephyr_library_sources_ifdef(CONFIG_GNSS_SAMPLE_ASSISTANCE_MINIMAL ${NRF_DIR}/samples/cellular/gnss/src/assistance_minimal.c)
zephyr_library_sources_ifdef(CONFIG_GNSS_SAMPLE_ASSISTANCE_MINIMAL ${NRF_DIR}/samples/cellular/gnss/src/mcc_location_table.c)

