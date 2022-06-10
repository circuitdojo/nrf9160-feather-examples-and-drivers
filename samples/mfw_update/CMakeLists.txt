#
# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

list(APPEND CONF_FILE
  ${NRF_DIR}/samples/nrf9160/fmfu_smp_svr/prj.conf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fmfu_smp_svr)

# NORDIC SDK APP START
target_sources(app PRIVATE ${NRF_DIR}/samples/nrf9160/fmfu_smp_svr/src/main.c)
# NORDIC SDK APP END
