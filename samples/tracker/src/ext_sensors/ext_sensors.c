/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <drivers/sensor.h>
#include "ext_sensors.h"
#include <stdlib.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ext_sensors, CONFIG_EXTERNAL_SENSORS_LOG_LEVEL);

#if defined(CONFIG_BOARD_THINGY91_NRF9160NS) || defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160NS)
/* Convert to s/m2 depending on the maximum measured range used for adxl362 or lis2dh. */
#if defined(CONFIG_ADXL362_ACCEL_RANGE_2G) || defined(CONFIG_LIS2DH_ACCEL_RANGE_2G)
#define RANGE_MAX_M_S2 19.6133
#elif defined(CONFIG_ADXL362_ACCEL_RANGE_4G) || defined(CONFIG_LIS2DH_ACCEL_RANGE_4G)
#define RANGE_MAX_M_S2 39.2266
#elif defined(CONFIG_ADXL362_ACCEL_RANGE_8G) || defined(CONFIG_LIS2DH_ACCEL_RANGE_8G)
#define RANGE_MAX_M_S2 78.4532
#else
#error Unknown accel mode!
#endif

#endif

/* Max decimal place */
#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
#define THRESHOLD_RESOLUTION_DECIMAL_MAX 2048
#elif defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160NS)
#define THRESHOLD_RESOLUTION_DECIMAL_MAX 127
#else
#error Board unsupported!
#endif

/* Local accelerometer threshold value. Used to filter out unwanted values in
 * the callback from the accelerometer.
 */
double threshold = RANGE_MAX_M_S2;

struct env_sensor
{
	enum sensor_channel channel;
	const struct device *dev;
	struct k_spinlock lock;
};

#if defined(CONFIG_EXTERNAL_SENSOR_ENVIRONMENTAL)
static struct env_sensor temp_sensor = {
	.channel = SENSOR_CHAN_AMBIENT_TEMP,
	.dev = DEVICE_DT_GET(DT_ALIAS(temp_sensor)),

};

static struct env_sensor humid_sensor = {
	.channel = SENSOR_CHAN_HUMIDITY,
	.dev = DEVICE_DT_GET(DT_ALIAS(humidity_sensor)),
};
#endif

static struct env_sensor accel_sensor = {
	.channel = SENSOR_CHAN_ACCEL_XYZ,
	.dev = DEVICE_DT_GET(DT_ALIAS(accelerometer)),
};

static ext_sensor_handler_t evt_handler;

static void accelerometer_trigger_handler(const struct device *dev,
										  struct sensor_trigger *trig)
{
	int err = 0;
	struct sensor_value data[ACCELEROMETER_CHANNELS];
	struct ext_sensor_evt evt;
	static bool initial_trigger;

	switch (trig->type)
	{
	case SENSOR_TRIG_THRESHOLD:
	case SENSOR_TRIG_DELTA:

		/* Ignore the initial trigger after initialization of the
		 * accelerometer which always carries jibberish xyz values.
		 */
		if (!initial_trigger)
		{
			initial_trigger = true;
			break;
		}

		if (sensor_sample_fetch(dev) < 0)
		{
			LOG_ERR("Sample fetch error");
			return;
		}

		err = sensor_channel_get(dev, SENSOR_CHAN_ACCEL_X, &data[0]);
		err += sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Y, &data[1]);
		err += sensor_channel_get(dev, SENSOR_CHAN_ACCEL_Z, &data[2]);

		if (err)
		{
			LOG_ERR("sensor_channel_get, error: %d", err);
			return;
		}

		evt.value_array[0] = sensor_value_to_double(&data[0]);
		evt.value_array[1] = sensor_value_to_double(&data[1]);
		evt.value_array[2] = sensor_value_to_double(&data[2]);

		/* Do a soft filter here to avoid sending data triggered by
		 * the inactivity threshold.
		 */
		if ((abs(evt.value_array[0]) > threshold ||
			 (abs(evt.value_array[1]) > threshold) ||
			 (abs(evt.value_array[2]) > threshold)))
		{

			evt.type = EXT_SENSOR_EVT_ACCELEROMETER_TRIGGER;
			evt_handler(&evt);
		}

		break;
	default:
		LOG_ERR("Unknown trigger");
	}
}

int ext_sensors_init(ext_sensor_handler_t handler)
{
	if (handler == NULL)
	{
		LOG_INF("External sensor handler NULL!");
		return -EINVAL;
	}

#if defined(CONFIG_EXTERNAL_SENSOR_ENVIRONMENTAL)
	if (!device_is_ready(temp_sensor.dev))
	{
		LOG_ERR("Temperature sensor device is not ready");
		return -ENODEV;
	}

	if (!device_is_ready(humid_sensor.dev))
	{
		LOG_ERR("Humidity sensor device is not ready");
		return -ENODEV;
	}
#endif

	if (!device_is_ready(accel_sensor.dev))
	{
		LOG_ERR("Accelerometer device is not ready");
		return -ENODEV;
	}

	struct sensor_trigger trig = {.chan = SENSOR_CHAN_ACCEL_XYZ};

#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
	trig.type = SENSOR_TRIG_THRESHOLD;
#elif defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160NS)
	trig.type = SENSOR_TRIG_DELTA;
#endif

	if (sensor_trigger_set(accel_sensor.dev, &trig,
						   accelerometer_trigger_handler))
	{
		LOG_ERR("Could not set trigger for device %s",
				accel_sensor.dev->name);
		return -ENODATA;
	}

	evt_handler = handler;

	return 0;
}

#if defined(CONFIG_EXTERNAL_SENSOR_ENVIRONMENTAL)
int ext_sensors_temperature_get(double *ext_temp)
{
	int err;
	struct sensor_value data;

	err = sensor_sample_fetch_chan(temp_sensor.dev, SENSOR_CHAN_ALL);
	if (err)
	{
		LOG_ERR("Failed to fetch data from %s, error: %d",
				temp_sensor.dev->name, err);
		return -ENODATA;
	}

	err = sensor_channel_get(temp_sensor.dev, temp_sensor.channel, &data);
	if (err)
	{
		LOG_ERR("Failed to fetch data from %s, error: %d",
				temp_sensor.dev->name, err);
		return -ENODATA;
	}

	k_spinlock_key_t key = k_spin_lock(&(temp_sensor.lock));
	*ext_temp = sensor_value_to_double(&data);
	k_spin_unlock(&(temp_sensor.lock), key);

	return 0;
}

int ext_sensors_humidity_get(double *ext_hum)
{
	int err;
	struct sensor_value data;

	err = sensor_sample_fetch_chan(humid_sensor.dev, SENSOR_CHAN_ALL);
	if (err)
	{
		LOG_ERR("Failed to fetch data from %s, error: %d",
				humid_sensor.dev->name, err);
		return -ENODATA;
	}

	err = sensor_channel_get(humid_sensor.dev, humid_sensor.channel, &data);
	if (err)
	{
		LOG_ERR("Failed to fetch data from %s, error: %d",
				humid_sensor.dev->name, err);
		return -ENODATA;
	}

	k_spinlock_key_t key = k_spin_lock(&(humid_sensor.lock));
	*ext_hum = sensor_value_to_double(&data);
	k_spin_unlock(&(humid_sensor.lock), key);

	return 0;
}
#endif

int ext_sensors_mov_thres_set(double threshold_new)
{
	int input_value;
	double range_max_m_s2 = RANGE_MAX_M_S2;
	double threshold_new_copy;

	if (threshold_new > range_max_m_s2)
	{
		LOG_ERR("Invalid threshold value");
		return -ENOTSUP;
	}

	threshold_new_copy = threshold_new;

	/* Convert threshold value into 11-bit decimal value relative
	 * to the configured measuring range of the accelerometer.
	 */
	threshold_new = (threshold_new *
					 (THRESHOLD_RESOLUTION_DECIMAL_MAX / range_max_m_s2));

	/* Add 0.5 to ensure proper conversion from double to int. */
	threshold_new = threshold_new + 0.5;
	input_value = (int)threshold_new;

	if (input_value >= THRESHOLD_RESOLUTION_DECIMAL_MAX)
	{
		input_value = THRESHOLD_RESOLUTION_DECIMAL_MAX - 1;
	}
	else if (input_value < 0)
	{
		input_value = 0;
	}

	int err;

#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)

	const struct sensor_value data = {
		.val1 = input_value};

	err = sensor_attr_set(accel_sensor.dev, SENSOR_CHAN_ACCEL_X,
						  SENSOR_ATTR_UPPER_THRESH, &data);
	if (err)
	{
		LOG_ERR("Failed to set accelerometer x-axis threshold value");
		LOG_ERR("Device: %s, error: %d",
				accel_sensor.dev->name, err);
		return err;
	}

	err = sensor_attr_set(accel_sensor.dev, SENSOR_CHAN_ACCEL_Y,
						  SENSOR_ATTR_UPPER_THRESH, &data);
	if (err)
	{
		LOG_ERR("Failed to set accelerometer y-axis threshold value");
		LOG_ERR("Device: %s, error: %d",
				accel_sensor.dev->name, err);
		return err;
	}

	err = sensor_attr_set(accel_sensor.dev, SENSOR_CHAN_ACCEL_Z,
						  SENSOR_ATTR_UPPER_THRESH, &data);
	if (err)
	{
		LOG_ERR("Failed to set accelerometer z-axis threshold value");
		LOG_ERR("Device: %s, error: %d",
				accel_sensor.dev->name, err);
		return err;
	}
#elif defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160NS)

	struct sensor_value data = {
		.val1 = input_value,
		.val2 = 0};

	/* The LIS2DH has only one threshold register per interrupt  */
	err = sensor_attr_set(accel_sensor.dev, SENSOR_CHAN_ACCEL_XYZ,
						  SENSOR_ATTR_SLOPE_TH, &data);
	if (err)
	{
		LOG_ERR("Failed to set accelerometer threshold value");
		LOG_ERR("Device: %s, error: %d",
				accel_sensor.dev->name, err);
		return err;
	}

	data.val1 = 0;

	err = sensor_attr_set(accel_sensor.dev, SENSOR_CHAN_ACCEL_XYZ,
						  SENSOR_ATTR_SLOPE_DUR, &data);
	if (err)
	{
		LOG_ERR("Failed to set accelerometer duration value");
		LOG_ERR("Device: %s, error: %d",
				accel_sensor.dev->name, err);
		return err;
	}

#endif

	threshold = threshold_new_copy;

	return 0;
}
