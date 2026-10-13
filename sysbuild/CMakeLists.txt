# Copyright (c) 2026 Circuit Dojo LLC
# SPDX-License-Identifier: Apache-2.0

set(NFED_UICR_DEBUG_ACCESS_OPEN_HEX
  ${SYSBUILD_CURRENT_MODULE_DIR}/boards/circuitdojo/feather_nrf9151/uicr_debug_access_open.hex
  CACHE INTERNAL "UICR words that keep the nRF9151 Feather debug port open")

# Partition manager defines the merged.hex command in nrf's post-CMake hook.
# This module depends on nrf, so this hook runs after it and can append the
# UICR merge to that command. mergehex's default overlap mode fails the build
# if an image already writes these words. mergehex truncates its output before
# reading its inputs, so merge into a temporary file and rename it.
function(${SYSBUILD_CURRENT_MODULE_NAME}_post_cmake)
  if(NOT SB_CONFIG_FEATHER_NRF9151_DEBUG_ACCESS_OPEN)
    return()
  endif()

  if(NOT TARGET merged_hex)
    message(FATAL_ERROR
      "FEATHER_NRF9151_DEBUG_ACCESS_OPEN needs the partition manager's merged.hex")
  endif()

  set(merged_hex ${CMAKE_BINARY_DIR}/merged.hex)
  add_custom_command(
    OUTPUT ${merged_hex}
    APPEND
    COMMAND
    ${PYTHON_EXECUTABLE}
    ${ZEPHYR_BASE}/scripts/build/mergehex.py
    -o ${merged_hex}.uicr
    ${merged_hex}
    ${NFED_UICR_DEBUG_ACCESS_OPEN_HEX}
    COMMAND ${CMAKE_COMMAND} -E rename ${merged_hex}.uicr ${merged_hex}
    DEPENDS ${NFED_UICR_DEBUG_ACCESS_OPEN_HEX}
  )
endfunction()
