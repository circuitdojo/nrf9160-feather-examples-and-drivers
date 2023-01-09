#
# Copyright (c) 2020 Circuit Dojo
#
# SPDX-License-Identifier: Apache-2.0
#

# Point to nRF9160 Feather root directory.
set(NRF9160_FEATHER_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "nRF9160 Feather root directory")

zephyr_include_directories(include)

add_subdirectory(drivers)
add_subdirectory(lib)