/*
 * Copyright (c) 2022 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/errno.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/mfd/npm1300.h>
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
#define NPM1300_BUCK_BUCKCTRL0 0x15U
#define NPM1300_BUCK_STATUS 0x34U

/* Bits */
#define NPM1300_BUCK2_MODE_BIT BIT(0)
#define NPM1300_BUCK2_PULLDOWN_EN BIT(3)

/* Check board support */
#if !defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9151)
#error "Unsupported board"
#endif

/* GPIOs */
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback sw0_cb_data;

/* Devices */
static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm1300_pmic));
static const struct device *buck2 = DEVICE_DT_GET(DT_NODELABEL(npm1300_buck2));
static const struct device *charger = DEVICE_DT_GET(DT_NODELABEL(npm1300_charger));
static const struct device *const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/* Semaphore */
K_SEM_DEFINE(usb_detect_sem, 0, 1);

/* Static */
static volatile bool vbus_connected;

int uart_set_mode(bool enabled)
{

  if (!device_is_ready(console_dev))
  {
    return -ENODEV;
  }

  if (enabled)
  {
    /* Enable console UART */
    int err = pm_device_action_run(console_dev, PM_DEVICE_ACTION_RESUME);
    if (err < 0)
    {
      LOG_ERR("Unable to suspend console UART. (err: %d)", err);
      return err;
    }
  }
  else
  {
    /* Disable console UART */
    int err = pm_device_action_run(console_dev, PM_DEVICE_ACTION_SUSPEND);
    if (err < 0)
    {
      LOG_ERR("Unable to suspend console UART. (err: %d)", err);
      return err;
    }

    /* Turn off to save power */
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
  }

  return 0;
}

static int buck2_set_mode(bool enabled)
{

  int err;

  /* Get pmic */
  if (!device_is_ready(pmic))
  {
    LOG_ERR("Failed to get PMIC device\n");
    return -ENODEV;
  }

  if (enabled)
  {

    err = regulator_enable(buck2);
    if (err < 0)
    {
      LOG_ERR("Failed to enable buck2: %d", err);
      return err;
    }

    err = mfd_npm1300_reg_update(pmic, NPM1300_BUCK_BASE, NPM1300_BUCK_BUCKCTRL0,
                                 0, NPM1300_BUCK2_PULLDOWN_EN);
    if (err < 0)
    {
      LOG_ERR("Failed to set buck2 pulldown. Err: %d", err);
      return err;
    }
  }
  else
  {

    err = regulator_disable(buck2);
    if (err < 0)
    {
      LOG_ERR("Failed to disable buck2: %d", err);
      return err;
    }
    err = mfd_npm1300_reg_update(pmic, NPM1300_BUCK_BASE, NPM1300_BUCK_BUCKCTRL0,
                                 NPM1300_BUCK2_PULLDOWN_EN, NPM1300_BUCK2_PULLDOWN_EN);
    if (err < 0)
    {
      LOG_ERR("Failed to set buck2 pulldown. Err: %d", err);
      return err;
    }
  }

  return 0;
}

static void event_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
  if (pins & BIT(NPM1300_EVENT_VBUS_DETECTED))
  {
    LOG_DBG("Vbus connected");
    vbus_connected = true;
    k_sem_give(&usb_detect_sem);
  }

  if (pins & BIT(NPM1300_EVENT_VBUS_REMOVED))
  {
    LOG_DBG("Vbus removed");
    vbus_connected = false;
    k_sem_give(&usb_detect_sem);
  }
}

void sw0_pressed(const struct device *dev, struct gpio_callback *cb,
                 uint32_t pins)
{
  printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

int switch_init()
{
  int ret = 0;

  if (!gpio_is_ready_dt(&sw0))
  {
    printk("Error: sw0 device %s is not ready\n",
           sw0.port->name);
    return -ENODEV;
  }

  ret = gpio_pin_configure_dt(&sw0, GPIO_INPUT);
  if (ret != 0)
  {
    printk("Error %d: failed to configure %s pin %d\n",
           ret, sw0.port->name, sw0.pin);
    return ret;
  }

  ret = gpio_pin_interrupt_configure_dt(&sw0,
                                        GPIO_INT_EDGE_TO_ACTIVE);
  if (ret != 0)
  {
    printk("Error %d: failed to configure interrupt on %s pin %d\n",
           ret, sw0.port->name, sw0.pin);
    return ret;
  }

  gpio_init_callback(&sw0_cb_data, sw0_pressed, BIT(sw0.pin));
  gpio_add_callback(sw0.port, &sw0_cb_data);

  return 0;
}

int main(void)
{
  int err = 0;

  LOG_INF("USB Detect Sample");

  /* Init switch */
  switch_init();

  /* Init modem */
  err = nrf_modem_lib_init();
  if (err < 0)
    LOG_ERR("Unable to initialize nRF Modem lib. Err: %i", err);

  /* Setup callback for PMIC events */
  static struct gpio_callback event_cb;

  gpio_init_callback(&event_cb, event_callback,
                     BIT(NPM1300_EVENT_VBUS_DETECTED) |
                         BIT(NPM1300_EVENT_VBUS_REMOVED));

  mfd_npm1300_add_callback(pmic, &event_cb);

  /* Initialise vbus detection status */
  struct sensor_value val;
  int ret = sensor_attr_get(charger, SENSOR_CHAN_CURRENT, SENSOR_ATTR_UPPER_THRESH, &val);

  if (ret < 0)
  {
    return false;
  }

  vbus_connected = (val.val1 != 0) || (val.val2 != 0);

  for (;;)
  {

    // Wait for semaphore
    k_sem_take(&usb_detect_sem, K_FOREVER);

    // Check if USB is connected
    if (vbus_connected)
    {
      LOG_INF("USB connected");
      // Enable regulator
      buck2_set_mode(true);
      // Set mode
      uart_set_mode(true);
    }
    else
    {
      LOG_INF("USB disconnected");
      // Disable regulator
      buck2_set_mode(false);
      // Set mode
      uart_set_mode(false);
    }
  }

  return 0;
}
