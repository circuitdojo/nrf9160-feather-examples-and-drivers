# Copyright (c) 2026 Circuit Dojo LLC
# SPDX-License-Identifier: Apache-2.0

if(SB_CONFIG_FEATHER_NRF9151_DEBUG_ACCESS_OPEN)
  # Attach the UICR data to the bootloader image, as NCS does for
  # app_provision.hex, so it lands in the secure merged hex and the non-secure
  # application hex never touches UICR.
  if(SB_CONFIG_SECURE_BOOT_APPCORE)
    set(uicr_base_image b0)
  elseif(SB_CONFIG_BOOTLOADER_MCUBOOT)
    set(uicr_base_image mcuboot)
  else()
    message(FATAL_ERROR
      "FEATHER_NRF9151_DEBUG_ACCESS_OPEN needs b0 or MCUboot to carry the UICR hex")
  endif()

  add_image_flasher(NAME uicr_debug_access_open
    HEX_FILE ${SYSBUILD_CURRENT_MODULE_DIR}/boards/circuitdojo/feather_nrf9151/uicr_debug_access_open.hex
    BASE_IMAGE ${uicr_base_image}
  )
endif()
