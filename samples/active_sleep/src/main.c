/*
 * Copyright (c) 2021 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <modem/lte_lc.h>
#include <devicetree.h>
#include <drivers/uart.h>
#include <comms/comms.h>
#include <drivers/gpio.h>

#define POWER_LATCH_PIN 31
#define LED_PIN 3
#define MODE_PIN 12

/* Static instance of uart */
const static struct device *uart;
const static struct device *gpio;
const static struct device *spi3;

void main(void)
{

	/* Get uart binding */
	uart = device_get_binding(DT_LABEL(DT_NODELABEL(uart0)));
	__ASSERT(uart, "Failed to get the uart device");

	/* Gpio pin */
	// gpio = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	// __ASSERT(uart, "Failed to get the gpio0 device");

	// gpio_pin_configure(gpio, POWER_LATCH_PIN, GPIO_DISCONNECTED);
	// gpio_pin_configure(gpio, LED_PIN, GPIO_DISCONNECTED);
	// gpio_pin_configure(gpio, MODE_PIN, GPIO_DISCONNECTED);

	/* Init modem library */
	lte_lc_init();
	lte_lc_power_off();

	/* Set power state */
	device_set_power_state(uart, DEVICE_PM_OFF_STATE, NULL, NULL);

	while (1)
	{
		k_sleep(K_SECONDS(60));
	}
}