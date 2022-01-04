/*
 * Copyright (c) 2021 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <modem/lte_lc.h>
#include <devicetree.h>

void main(void)
{

	/* Init modem library */
	lte_lc_init();
	lte_lc_power_off();

	while (1)
	{
		k_sleep(K_SECONDS(60));
	}
}