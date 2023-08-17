/*
 * Copyright (c) 2022 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/pm/device.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

/* Gpios */
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec latch_en = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), latch_en_gpios);

static void setup_accel(void)
{
	const struct device *sensor = DEVICE_DT_GET(DT_ALIAS(accel0));

	if (!device_is_ready(sensor))
	{
		printk("Could not get accel0 device\n");
		return;
	}

	// Disable the device
	struct sensor_value odr = {
		.val1 = 0,
	};

	int rc = sensor_attr_set(sensor, SENSOR_CHAN_ACCEL_XYZ,
							 SENSOR_ATTR_SAMPLING_FREQUENCY,
							 &odr);
	if (rc != 0)
	{
		printk("Failed to set odr: %d\n", rc);
		return;
	}
}

static int setup_gpio(void)
{

	gpio_pin_configure(sw0.port,
					   sw0.pin,
					   GPIO_DISCONNECTED);

	gpio_pin_configure(led0.port,
					   led0.pin,
					   GPIO_DISCONNECTED);

	gpio_pin_configure(latch_en.port,
					   latch_en.pin,
					   GPIO_DISCONNECTED);

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
		printk("Unable to suspend console UART. (err: %d)\n", err);
		return err;
	}

	/* Turn off to save power */
	NRF_CLOCK->TASKS_HFCLKSTOP = 1;

	return 0;
}

int main(void)
{
	printk("active_sleep\n");

	/* Setup GPIO */
	setup_gpio();

	/* Disable accel */
	setup_accel();

	/* Init modem */
	nrf_modem_lib_init();
	lte_lc_init();

	/* Peripherals */
	setup_uart();

	return 0;
}
