/*
 * Copyright (c) 2018-2019 Peter Bigot Consulting, LLC
 * Copyright (c) 2019-2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT voltage_divider

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr.h>
#include <init.h>
#include <drivers/gpio.h>
#include <drivers/adc.h>
#include <drivers/sensor.h>

#include "battery.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(battery);

#define VBATT DT_PATH(vbatt)

#ifdef CONFIG_BOARD_THINGY52_NRF52832
/* This board uses a divider that reduces max voltage to
 * reference voltage (600 mV).
 */
#define BATTERY_ADC_GAIN ADC_GAIN_1
#else
/* Other boards may use dividers that only reduce battery voltage to
 * the maximum supported by the hardware (3.6 V)
 */
#define BATTERY_ADC_GAIN ADC_GAIN_1_6
#endif

struct io_channel_config
{
	uint8_t channel;
};

struct gpio_channel_config
{
	const char *label;
	uint8_t pin;
	uint8_t flags;
};

struct divider_config
{
	struct io_channel_config io_channel;
	struct gpio_channel_config power_gpios;
	/* output_ohm is used as a flag value: if it is nonzero then
	 * the battery is measured through a voltage divider;
	 * otherwise it is assumed to be directly connected to Vdd.
	 */
	uint32_t output_ohm;
	uint32_t full_ohm;
};

static const struct divider_config divider_config = {
#if DT_NODE_HAS_STATUS(VBATT, okay)
	.io_channel = {
		DT_IO_CHANNELS_INPUT(VBATT),
	},
#if DT_NODE_HAS_PROP(VBATT, power_gpios)
	.power_gpios = {
		DT_GPIO_LABEL(VBATT, power_gpios),
		DT_GPIO_PIN(VBATT, power_gpios),
		DT_GPIO_FLAGS(VBATT, power_gpios),
	},
#endif
	.output_ohm = DT_PROP(VBATT, output_ohms),
	.full_ohm = DT_PROP(VBATT, full_ohms),
#else  /* /vbatt exists */
	.io_channel = {
		DT_LABEL(DT_NODELABEL(adc)),
	},
#endif /* /vbatt exists */
};

struct divider_data
{
	const struct device *adc;
	const struct device *gpio;
	struct adc_channel_cfg adc_cfg;
	struct adc_sequence adc_seq;
	int16_t raw;
};
static struct divider_data divider_data = {
	.adc = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR(VBATT)),
};

static int
divider_setup(void)
{
	const struct divider_config *cfg = &divider_config;
	const struct io_channel_config *iocp = &cfg->io_channel;
	const struct gpio_channel_config *gcp = &cfg->power_gpios;
	struct divider_data *ddp = &divider_data;
	struct adc_sequence *asp = &ddp->adc_seq;
	struct adc_channel_cfg *accp = &ddp->adc_cfg;
	int rc;

	if (!device_is_ready(ddp->adc))
	{
		LOG_ERR("ADC device is not ready %s", ddp->adc->name);
		return -ENOENT;
	}

	if (gcp->label)
	{
		ddp->gpio = device_get_binding(gcp->label);
		if (ddp->gpio == NULL)
		{
			LOG_ERR("Failed to get GPIO %s", gcp->label);
			return -ENOENT;
		}
		rc = gpio_pin_configure(ddp->gpio, gcp->pin,
								GPIO_OUTPUT_INACTIVE | gcp->flags);
		if (rc != 0)
		{
			LOG_ERR("Failed to control feed %s.%u: %d",
					gcp->label, gcp->pin, rc);
			return rc;
		}
	}

	*asp = (struct adc_sequence){
		.channels = BIT(0),
		.buffer = &ddp->raw,
		.buffer_size = sizeof(ddp->raw),
		.oversampling = 4,
		.calibrate = true,
	};

#ifdef CONFIG_ADC_NRFX_SAADC
	*accp = (struct adc_channel_cfg){
		.gain = BATTERY_ADC_GAIN,
		.reference = ADC_REF_INTERNAL,
		.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
	};

	if (cfg->output_ohm != 0)
	{
		accp->input_positive = SAADC_CH_PSELP_PSELP_AnalogInput0 + iocp->channel;
	}
	else
	{
		accp->input_positive = SAADC_CH_PSELP_PSELP_VDD;
	}

	asp->resolution = 14;
#else /* CONFIG_ADC_var */
#error Unsupported ADC
#endif /* CONFIG_ADC_var */

	rc = adc_channel_setup(ddp->adc, accp);
	LOG_INF("Setup AIN%u got %d", iocp->channel, rc);

	return rc;
}

static bool battery_ok;

static int battery_measurement_init(const struct device *dev)
{
	int rc = divider_setup();

	battery_ok = (rc == 0);
	LOG_INF("Battery setup: %d %d", rc, battery_ok);
	return rc;
}

static int battery_measure_enable(bool enable)
{
	int rc = -ENOENT;

	if (battery_ok)
	{
		const struct divider_data *ddp = &divider_data;
		const struct gpio_channel_config *gcp = &divider_config.power_gpios;

		rc = 0;
		if (ddp->gpio)
		{
			rc = gpio_pin_set(ddp->gpio, gcp->pin, enable);
		}
	}
	return rc;
}

static int battery_sample(int *battery_voltage)
{
	int rc = -ENOENT;

	if (battery_ok)
	{
		struct divider_data *ddp = &divider_data;
		const struct divider_config *dcp = &divider_config;
		struct adc_sequence *sp = &ddp->adc_seq;

		rc = adc_read(ddp->adc, sp);
		if (rc)
		{
			LOG_ERR("Error reading ADC. Err: %i", rc);
			return rc;
		}

		sp->calibrate = false;

		int32_t val = ddp->raw;

		adc_raw_to_millivolts(adc_ref_internal(ddp->adc),
							  ddp->adc_cfg.gain,
							  sp->resolution,
							  &val);

		if (dcp->output_ohm != 0)
		{
			*battery_voltage = val * (uint64_t)dcp->full_ohm / dcp->output_ohm;
			LOG_INF("raw %u ~ %u mV => %d mV",
					ddp->raw, val, rc);
		}
		else
		{
			*battery_voltage = val;
			LOG_INF("raw %u ~ %u mV", ddp->raw, val);
		}
	}

	return rc;
}

unsigned int battery_level_pptt(unsigned int batt_mV,
								const struct battery_level_point *curve)
{
	const struct battery_level_point *pb = curve;

	if (batt_mV >= pb->lvl_mV)
	{
		/* Measured voltage above highest point, cap at maximum. */
		return pb->lvl_pptt;
	}
	/* Go down to the last point at or below the measured voltage. */
	while ((pb->lvl_pptt > 0) && (batt_mV < pb->lvl_mV))
	{
		++pb;
	}
	if (batt_mV < pb->lvl_mV)
	{
		/* Below lowest point, cap at minimum */
		return pb->lvl_pptt;
	}

	/* Linear interpolation between below and above points. */
	const struct battery_level_point *pa = pb - 1;

	return pb->lvl_pptt + ((pa->lvl_pptt - pb->lvl_pptt) * (batt_mV - pb->lvl_mV) / (pa->lvl_mV - pb->lvl_mV));
}

static int battery_measurement_sample_fetch(const struct device *dev,
											enum sensor_channel chan)
{
	int err = 0;

	struct battery_measurement_data *dat = (struct battery_measurement_data *)dev->data;

	switch (chan)
	{
	case SENSOR_CHAN_VOLTAGE:

		battery_measure_enable(true);
		k_sleep(K_USEC(100));
		err = battery_sample(&dat->voltage.val1);
		if (err)
		{
			LOG_WRN("Error getting battery sample. Err: %i", err);
		}
		battery_measure_enable(false);

		break;
	default:
		LOG_WRN("Unsupported channel: %i", chan);
		break;
	}

	return 0;
}

static int battery_measurement_channel_get(const struct device *dev,
										   enum sensor_channel chan,
										   struct sensor_value *val)
{

	struct battery_measurement_data *dat = (struct battery_measurement_data *)dev->data;

	switch (chan)
	{
	case SENSOR_CHAN_VOLTAGE:
		if (battery_ok)
		{
			memcpy(val, &dat->voltage, sizeof(struct sensor_value));
			break;
		}
		else
		{
			return -ENODEV;
		}
	default:
		LOG_WRN("Unsupported channel: %i", chan);
		return -EINVAL;
	}

	return 0;
}

static const struct sensor_driver_api battery_measurement_api = {
	.sample_fetch = &battery_measurement_sample_fetch,
	.channel_get = &battery_measurement_channel_get,
};

static struct battery_measurement_data battery_measurement_data;

DEVICE_DEFINE(voltage_divider, DT_INST_LABEL(0),
			  battery_measurement_init, NULL,
			  &battery_measurement_data, NULL, POST_KERNEL,
			  CONFIG_SENSOR_INIT_PRIORITY, &battery_measurement_api);
