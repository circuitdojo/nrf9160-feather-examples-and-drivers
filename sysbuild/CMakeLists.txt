# Copyright (c) 2026 Circuit Dojo LLC
# SPDX-License-Identifier: Apache-2.0

if(SB_CONFIG_FEATHER_NRF9151_DEBUG_ACCESS_OPEN)
  # With a bootloader, attach the UICR data to the bootloader image, as NCS
  # does for app_provision.hex, so it lands in the secure merged hex and the
  # non-secure application hex never touches UICR. Without one, the default
  # image's merged hex is the only one flashed and there is no update image to
  # keep clean, so attach it there.
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
