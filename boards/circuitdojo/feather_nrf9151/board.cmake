# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9151_NRF9151_NS)
  set(TFM_PUBLIC_KEY_FORMAT "full")
endif()

if(CONFIG_TFM_FLASH_MERGED_BINARY)
  set_property(TARGET runners_yaml_props_target PROPERTY hex_file tfm_merged.hex)
endif()

# TODO: change to nRF9151_xxAA when such device is available in JLink
board_runner_args(jlink "--device=nRF9160_xxAA" "--speed=4000")
board_runner_args(pyocd "--target=nRF9160_xxAA" "--frequency=4000000")
board_runner_args(probe-rs "--chip=nRF9151_xxAA")
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/probe-rs.board.cmake)
