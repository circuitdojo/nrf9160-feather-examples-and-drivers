#
# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mfw_update)

# NORDIC SDK APP START
target_sources(app PRIVATE ${NRF_DIR}/samples/nrf9160/fmfu_smp_svr/src/main.c)
# NORDIC SDK APP END
