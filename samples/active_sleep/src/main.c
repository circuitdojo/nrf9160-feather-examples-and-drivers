/*
 * Copyright (c) 2021 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <modem/lte_lc.h>
#include <devicetree.h>
#include <drivers/sensor.h>

static void setup_accel(void)
{
	const struct device *sensor = DEVICE_DT_GET(DT_INST(0, st_lis2dh));

	if (sensor == NULL || !device_is_ready(sensor))
	{
		printk("Could not get %s device\n",
			   DT_LABEL(DT_INST(0, st_lis2dh)));
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

void main(void)
{

	/* Disable accel */
	setup_accel();

	/* Power saving */
	lte_lc_psm_req(true);

	/* Init modem library */
	lte_lc_init_and_connect();

	while (1)
	{
		k_sleep(K_SECONDS(60));
	}
}