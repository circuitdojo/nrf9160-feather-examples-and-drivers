# Copyright (c) 2026 Circuit Dojo LLC
# SPDX-License-Identifier: Apache-2.0

if(SB_CONFIG_FEATHER_NRF9151_DEBUG_ACCESS_OPEN)
  # The UICR data is flashed as its own sysbuild image (an image flasher), so
  # neither the application nor an update image ever carries UICR words.
  # With a bootloader the flasher takes that image's board and configuration
  # (BASE_IMAGE); without one it uses the default image's.
  if(SB_CONFIG_SECURE_BOOT_APPCORE)
    set(uicr_base_image_arg BASE_IMAGE b0)
  elseif(SB_CONFIG_BOOTLOADER_MCUBOOT)
    set(uicr_base_image_arg BASE_IMAGE mcuboot)
  else()
    set(uicr_base_image_arg)
  endif()

  # NCS defines add_image_flasher() here and only includes it for some
  # configurations (e.g. b0), so include it rather than rely on that.
  include(${ZEPHYR_NRF_MODULE_DIR}/sysbuild/image_flasher.cmake)
  add_image_flasher(NAME uicr_debug_access_open
    HEX_FILE ${SYSBUILD_CURRENT_MODULE_DIR}/boards/circuitdojo/feather_nrf9151/uicr_debug_access_open.hex
    ${uicr_base_image_arg}
  )
endif()
