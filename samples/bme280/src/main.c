/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#define BME280 DT_INST(0, bosch_bme280)

#if !DT_NODE_HAS_STATUS(BME280, okay)
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#endif

const struct device *dev = DEVICE_DT_GET(BME280);

void main(void)
{
	if (!device_is_ready(dev))
	{
		printk("No device \"%s\" found; did initialization fail?\n",
			   dev->name);
		return;
	}
	else
	{
		printk("Found device \"%s\"\n", dev->name);
	}

	while (1)
	{
		struct sensor_value temp, press, humidity;

		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);
		sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);

		printk("temp: %d.%06d; press: %d.%06d; humidity: %d.%06d\n",
			   temp.val1, temp.val2, press.val1, press.val2,
			   humidity.val1, humidity.val2);

		k_sleep(K_MSEC(1000));
	}
}
