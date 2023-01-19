/*
 * Copyright (c) 2022 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <modem/lte_lc.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/pm/device.h>

/* Devices */
static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));
static const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
static const struct device *spi3_dev = DEVICE_DT_GET(DT_NODELABEL(spi3));

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

	if (!device_is_ready(sw0.port))
	{
		printk("GPIO device not ready\n");
		return -EIO;
	}

	gpio_pin_configure(sw0.port,
					   sw0.pin,
					   GPIO_DISCONNECTED);

	gpio_pin_configure(led0.port,
					   led0.pin,
					   GPIO_OUTPUT_LOW);

	gpio_pin_configure(latch_en.port,
					   latch_en.pin,
					   GPIO_DISCONNECTED);

	return 0;
}

int setup_uart()
{
	int err;

	/* Initialize the UART module */
	if (!device_is_ready(uart_dev))
	{
		printk("Cannot bind UART device\n");
		return -EIO;
	}

	/* Power off UART module */
	uart_rx_disable(uart_dev);
	k_sleep(K_MSEC(100));
	err = pm_device_action_run(uart_dev, PM_DEVICE_ACTION_SUSPEND);
	if (err)
	{
		printk("Can't power off uart: %d\n", err);
		return err;
	}

	return 0;
}

int setup_i2c()
{
	int err;

	if (!device_is_ready(i2c_dev))
	{
		printk("Cannot bind I2C device\n");
		return -EIO;
	}

	err = pm_device_action_run(i2c_dev, PM_DEVICE_ACTION_SUSPEND);
	if (err)
	{
		printk("Can't power off I2C: %d\n", err);
		return err;
	}

	return 0;
}

int setup_spi()
{
	int err;

	if (!device_is_ready(spi3_dev))
	{
		printk("Cannot bind SPI device\n");
		return -EIO;
	}

	err = pm_device_action_run(spi3_dev, PM_DEVICE_ACTION_SUSPEND);
	if (err)
	{
		printk("Can't power off spi: %d\n", err);
		return err;
	}

	return 0;
}

void main(void)
{
	printk("active_sleep\n");

	/* Setup GPIO */
	setup_gpio();

	/* Disable accel */
	setup_accel();

	/* Power saving */
	lte_lc_psm_req(true);
	lte_lc_edrx_req(false);

	/* Uart */
	setup_uart();

	/* Init modem library */
	lte_lc_init();
	lte_lc_power_off();
}