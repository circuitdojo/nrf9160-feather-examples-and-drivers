/*
 * Copyright (c) 2024 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(temperature_sensor, LOG_LEVEL_INF);

/* Check if STS4x sensor exists in devicetree */
#if !DT_NODE_EXISTS(DT_NODELABEL(sts4x))
#error "STS4x temperature sensor not found in devicetree. This sample requires nRF9151 Feather v2."
#endif

static const struct device *temp_sensor = DEVICE_DT_GET(DT_NODELABEL(sts4x));

int main(void)
{
	LOG_INF("STS4x Temperature Sensor Sample");
	LOG_INF("Board: nRF9151 Feather v2");

	/* Check if device is ready */
	if (!device_is_ready(temp_sensor)) {
		LOG_ERR("Temperature sensor not ready");
		return -ENODEV;
	}

	LOG_INF("Temperature sensor initialized successfully");

	/* Main loop - read temperature every second */
	while (1) {
		struct sensor_value temp;
		int rc;

		/* Fetch sample from sensor */
		rc = sensor_sample_fetch(temp_sensor);
		if (rc < 0) {
			LOG_ERR("Failed to fetch temperature sample: %d", rc);
			k_sleep(K_MSEC(1000));
			continue;
		}

		/* Get temperature channel value */
		rc = sensor_channel_get(temp_sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		if (rc < 0) {
			LOG_ERR("Failed to get temperature channel: %d", rc);
			k_sleep(K_MSEC(1000));
			continue;
		}

		/* Display temperature */
		LOG_INF("Temperature: %d.%06d °C", temp.val1, temp.val2);

		k_sleep(K_MSEC(1000));
	}

	return 0;
}
