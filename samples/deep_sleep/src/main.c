/*
 * Copyright (c) 2024 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/drivers/mfd/npm1300.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define SHIP_BASE 0x0BU
#define SHIP_OFFSET_CONFIG 0x04U
#define SHIP_OFFSET_LPCONFIG 0x06U
#define SHIP_OFFSET_CFGSTROBE 0x01U
#define SHIP_OFFSET_TASKENTERSHIPMODE 0x02U

/* Addresses */
#define NPM1300_BUCK_BASE 0x04U
#define NPM1300_BUCK_OFFSET_EN_CLR 0x01U
#define NPM1300_BUCK_BUCKCTRL0 0x15U
#define NPM1300_BUCK_STATUS 0x34U

int main(void) {

  printk("Deep sleep sample\n");

#if defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9161) || defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9151)
  static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm1300_pmic));

  int ret;
  uint8_t reg = 0;

  /* See if pulldown is not already enabled */
  ret = mfd_npm1300_reg_read(pmic, NPM1300_BUCK_BASE, NPM1300_BUCK_BUCKCTRL0,
                             &reg);
  if (ret < 0)
    printk("Failed to set VBUSINLIM. Err: %d\n", ret);

  if ((reg & 0x08) == 0) {

    /* Write to MFD to enable pulldown for both Bucks */
    ret = mfd_npm1300_reg_write(pmic, NPM1300_BUCK_BASE, NPM1300_BUCK_BUCKCTRL0,
                                0x08 + 0x04);
    if (ret < 0)
      printk("Failed to set VBUSINLIM. Err: %d\n", ret);
  }

    // Initializing here to ensure Hibernate mode works as expected
    ret = mfd_npm1300_reg_write(pmic, SHIP_BASE, SHIP_OFFSET_CONFIG, 3U);
    if (ret < 0) {
      printk("Failed to write config. Err: %d\n", ret);
      return ret;
    }
    
    ret = mfd_npm1300_reg_write(pmic, SHIP_BASE, SHIP_OFFSET_LPCONFIG, 0U);
    if (ret < 0) {
      printk("Failed to write config lp config. Err: %d\n", ret);
      return ret;
    }
    
    ret = mfd_npm1300_reg_write(pmic, SHIP_BASE, SHIP_OFFSET_CFGSTROBE, 1U);
    if (ret < 0) {
      printk("Failed to write cfg strobe. Err: %d\n", ret);
      return ret;
    }
    

#if true
    /* set hibernate mode and power down */
    ret = mfd_npm1300_hibernate(pmic, 60000);
    if (ret < 0) {
      printk("Failed to hibernate. Err: %d\n", ret);
      return ret;
    }
#else
  /* Put into ship mode */
  ret =
      mfd_npm1300_reg_write(pmic, SHIP_BASE, SHIP_OFFSET_TASKENTERSHIPMODE, 1U);
  if (ret < 0) {
    printk("Failed to go into ship mode. Err: %i\n", ret);
    return ret;
  }
#endif

#else
#error Sample not supported on this board
#endif

  return 0;
}
