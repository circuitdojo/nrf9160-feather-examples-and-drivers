/*
 * Copyright (c) 2022 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/errno.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/mfd/npm13xx.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
LOG_MODULE_REGISTER(main);

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

/* Addresses */
#define NPM1300_BUCK_BASE 0x04U
#define NPM1300_BUCK_OFFSET_EN_CLR 0x01U
#define NPM1300_BUCK2_OFFSET_EN_CLR 0x03U
#define NPM1300_BUCK_BUCKCTRL0 0x15U
#define NPM1300_BUCK_STATUS 0x34U

/* Bits */
#define NPM1300_BUCK2_MODE_BIT BIT(0)
#define NPM1300_BUCK2_PULLDOWN_EN BIT(3)

/* Gpios */
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
#if defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec latch_en =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), latch_en_gpios);

static const struct gpio_dt_spec wp =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), wp_gpios);
static const struct gpio_dt_spec hold =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), hold_gpios);
#endif

static void setup_accel(void)
{
  const struct device *sensor = DEVICE_DT_GET(DT_ALIAS(accel0));

  if (!device_is_ready(sensor))
  {
    LOG_ERR("Could not get accel0 device");
    return;
  }

  // Disable the device
  struct sensor_value odr = {
      .val1 = 0,
  };

  int rc = sensor_attr_set(sensor, SENSOR_CHAN_ACCEL_XYZ,
                           SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);
  if (rc != 0)
  {
    LOG_ERR("Failed to set odr: %d", rc);
    return;
  }
}

static int setup_gpio(void)
{

  gpio_pin_configure_dt(&sw0, GPIO_DISCONNECTED);
#if defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160)
  gpio_pin_configure_dt(&led0, GPIO_DISCONNECTED);
  gpio_pin_configure_dt(&latch_en, GPIO_DISCONNECTED);

  gpio_pin_configure_dt(&wp, GPIO_INPUT | GPIO_PULL_UP);
  gpio_pin_configure_dt(&hold, GPIO_INPUT | GPIO_PULL_UP);
#endif

  return 0;
}

int setup_uart()
{

  static const struct device *const console_dev =
      DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

  /* Disable console UART */
  int err = pm_device_action_run(console_dev, PM_DEVICE_ACTION_SUSPEND);
  if (err < 0)
  {
    LOG_ERR("Unable to suspend console UART. (err: %d)", err);
    return err;
  }

  /* Turn off to save power */
  NRF_CLOCK->TASKS_HFCLKSTOP = 1;

  return 0;
}

static int setup_pmic()
{

#if defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9161) || defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9151)

    int err;

    /* Get pmic */
    static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm1300_pmic));
    if (!pmic)
    {
        LOG_ERR("Failed to get PMIC device\n");
        return -ENODEV;
    }

    /* Bypass regulator framework and disable BUCK2 directly via I2C.
       The Zephyr refcount can be inflated by a POF event at low VBAT,
       preventing regulator_disable() from issuing the hardware command. */
    err = mfd_npm13xx_reg_write(pmic, NPM1300_BUCK_BASE,
                    NPM1300_BUCK2_OFFSET_EN_CLR, 0x01);
    if (err < 0) {
        LOG_ERR("Failed to disable BUCK2: %d", err);
    }

    /* Enable pulldown to discharge BUCK2 output */
    uint8_t reg = 0;

    err = mfd_npm13xx_reg_read(pmic, NPM1300_BUCK_BASE,
                   NPM1300_BUCK_BUCKCTRL0, &reg);
    if (err < 0) {
        LOG_ERR("Failed to read BUCKCTRL0: %d", err);
    } else if (!(reg & NPM1300_BUCK2_PULLDOWN_EN)) {
        err = mfd_npm13xx_reg_write(pmic, NPM1300_BUCK_BASE,
                        NPM1300_BUCK_BUCKCTRL0,
                        NPM1300_BUCK2_PULLDOWN_EN);
        if (err < 0)
            LOG_ERR("Failed to enable BUCK2 pulldown: %d", err);
    }

#endif

    return 0;
}

int nor_storage_init(void)
{

  static const struct device *spi_nor = DEVICE_DT_GET(DT_ALIAS(ext_flash));

  /* Disable external flash */
  int err = pm_device_action_run(spi_nor, PM_DEVICE_ACTION_SUSPEND);
  if (err < 0)
  {
    LOG_ERR("Unable to suspend SPI NOR flash. (err: %d)", err);
    return err;
  }

  return 0;
}

int main(void)
{
  int err = 0;

  LOG_INF("Active Sleep Sample");

  /* Setup GPIO */
  setup_gpio();

  /* Disable accel */
  setup_accel();

  /* Init NOR */
  nor_storage_init();

  /* Init modem */
  err = nrf_modem_lib_init();
  if (err < 0)
    LOG_ERR("Unable to initialize nRF Modem lib. Err: %i", err);

  /* Peripherals */
  setup_uart();

  /* Disable regulator */
  setup_pmic();

  return 0;
}
