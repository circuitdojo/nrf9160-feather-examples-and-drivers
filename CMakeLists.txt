#
# Copyright (c) 2020 Circuit Dojo
#
# SPDX-License-Identifier: Apache-2.0
#

# Apply bootloader patch 
execute_process(COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/patch/mcuboot.diff --directory=bootloader/mcuboot WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../ OUTPUT_QUIET ERROR_QUIET)

# Point to nRF9160 Feather root directory.
set(NRF9160_FEATHER_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "nRF9160 Feather root directory")

add_subdirectory(drivers)
