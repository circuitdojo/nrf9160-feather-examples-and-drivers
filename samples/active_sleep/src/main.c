/*
 * Copyright (c) 2021 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <modem/lte_lc.h>
#include <devicetree.h>
#include <drivers/uart.h>

/* Static instance of uart */
const static struct device *uart;

void main(void)
{

	/* Get uart binding */
	uart = device_get_binding(DT_LABEL(DT_NODELABEL(uart0)));
	__ASSERT(uart, "Failed to get the uart device");

	/* Set power state */
	device_set_power_state(uart, DEVICE_PM_OFF_STATE, NULL, NULL);

	/* Init modem library */
	lte_lc_init();
	lte_lc_power_off();

	while (1)
	{
		k_sleep(K_SECONDS(60));
	}
}