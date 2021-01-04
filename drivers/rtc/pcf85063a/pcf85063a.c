/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <kernel.h>
#include <sys/byteorder.h>
#include <sys/__assert.h>
#include <sys/util.h>

#include <stdint.h>
#include <string.h>

#include <device.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <drivers/counter.h>

#include "pcf85063a.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(rtc);

#define DT_DRV_COMPAT nxp_pcf85063a

static const struct device *pcf85063a_dev;

static int pcf85063a_start(const struct device *dev)
{

	// Get the data pointer
	struct pcf85063a_data *data = pcf85063a_dev->data;

	// Turn it back on (active low)
	uint8_t reg = 0;
	uint8_t mask = PCF85063A_CTRL1_STOP;

	// Write back the updated register value
	int ret = i2c_reg_update_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
								  PCF85063A_CTRL1, mask, reg);
	if (ret)
	{
		LOG_ERR("Unable to stop RTC. (err %i)", ret);
		return ret;
	}

	return 0;
}

static int pcf85063a_stop(const struct device *dev)
{

	// Get the data pointer
	struct pcf85063a_data *data = pcf85063a_dev->data;

	// Turn it off
	uint8_t reg = PCF85063A_CTRL1_STOP;
	uint8_t mask = PCF85063A_CTRL1_STOP;

	// Write back the updated register value
	int ret = i2c_reg_update_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
								  PCF85063A_CTRL1, mask, reg);
	if (ret)
	{
		LOG_ERR("Unable to stop RTC. (err %i)", ret);
		return ret;
	}

	return 0;
}

static int pcf85063a_get_value(const struct device *dev, uint32_t *ticks)
{
	return 0;
}

static int pcf85063a_set_alarm(
	const struct device *dev, uint8_t chan_id, const struct counter_alarm_cfg *alarm_cfg)
{
	ARG_UNUSED(chan_id);

	uint8_t ticks = (uint8_t)alarm_cfg->ticks;

	// Get the data pointer
	struct pcf85063a_data *data = pcf85063a_dev->data;

	// Ret val for error checking
	int ret;

	// Clear any flags in CTRL2
	uint8_t reg = 0;
	uint8_t mask = PCF85063A_CTRL2_TF;

	ret = i2c_reg_update_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
							  PCF85063A_CTRL2, mask, reg);
	if (ret)
	{
		LOG_ERR("Unable to set RTC alarm. (err %i)", ret);
		return ret;
	}

	// Write the tick count. Ticks are 1 sec
	ret = i2c_reg_write_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
							 PCF85063A_TIMER_VALUE, ticks);
	if (ret)
	{
		LOG_ERR("Unable to set RTC timer value. (err %i)", ret);
		return ret;
	}

	// Set to 1 second mode
	reg = (PCF85063A_TIMER_MODE_FREQ_1 << PCF85063A_TIMER_MODE_FREQ_SHIFT) | PCF85063A_TIMER_MODE_EN | PCF85063A_TIMER_MODE_INT_EN;
	mask = PCF85063A_TIMER_MODE_FREQ_MASK | PCF85063A_TIMER_MODE_EN | PCF85063A_TIMER_MODE_INT_EN;

	LOG_INF("mode 0x%x", reg);

	// Write back the updated register value
	ret = i2c_reg_update_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
							  PCF85063A_TIMER_MODE, mask, reg);
	if (ret)
	{
		LOG_ERR("Unable to set RTC alarm. (err %i)", ret);
		return ret;
	}

	return 0;
}

static int pcf85063a_cancel_alarm(const struct device *dev, uint8_t chan_id)
{

	// Get the data pointer
	struct pcf85063a_data *data = pcf85063a_dev->data;

	// Ret val for error checking
	int ret;

	// Clear any flags in CTRL2
	uint8_t reg = 0;
	uint8_t mask = PCF85063A_CTRL2_TF;

	ret = i2c_reg_update_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
							  PCF85063A_CTRL2, mask, reg);
	if (ret)
	{
		LOG_ERR("Unable to set RTC alarm. (err %i)", ret);
		return ret;
	}

	// Turn off all itnerrupts/timer mode
	reg = 0;
	mask = PCF85063A_TIMER_MODE_EN | PCF85063A_TIMER_MODE_INT_EN | PCF85063A_TIMER_MODE_INT_TI_TP;

	LOG_INF("mode 0x%x", reg);

	// Write back the updated register value
	ret = i2c_reg_update_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
							  PCF85063A_TIMER_MODE, mask, reg);
	if (ret)
	{
		LOG_ERR("Unable to cancel RTC alarm. (err %i)", ret);
		return ret;
	}

	return 0;
}

static int pcf85063a_set_top_value(const struct device *dev, const struct counter_top_cfg *cfg)
{
	return 0;
}

static uint32_t pcf85063a_get_pending_int(const struct device *dev)
{

	// Get the data pointer
	struct pcf85063a_data *data = pcf85063a_dev->data;

	// Start with 0
	uint8_t reg = 0;

	// Write back the updated register value
	int ret = i2c_reg_read_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
								PCF85063A_CTRL2, &reg);
	if (ret)
	{
		LOG_ERR("Unable to get RTC CTRL2 reg. (err %i)", ret);
		return ret;
	}

	// Return 1 if interrupt. 0 if no flag.
	return (reg & PCF85063A_CTRL2_TF) ? 1U : 0U;
}

static uint32_t pcf85063a_get_top_value(const struct device *dev)
{
	return 0;
}

static uint32_t pcf85063a_get_max_relative_alarm(const struct device *dev)
{
	return 0;
}

static const struct counter_driver_api pcf85063a_driver_api = {
	.start = pcf85063a_start,
	.stop = pcf85063a_stop,
	.get_value = pcf85063a_get_value,
	.set_alarm = pcf85063a_set_alarm,
	.cancel_alarm = pcf85063a_cancel_alarm,
	.set_top_value = pcf85063a_set_top_value,
	.get_pending_int = pcf85063a_get_pending_int,
	.get_top_value = pcf85063a_get_top_value,
	.get_max_relative_alarm = pcf85063a_get_max_relative_alarm,
};

static const struct counter_config_info pcf85063_cfg_info = {
	.max_top_value = 0xff,
	.freq = 1,
	.channels = 1,
};

// ARG_UNUSED(dev);

static struct pcf85063a_data pcf85063a_data;

int pcf85063a_init(const struct device *dev)
{
	pcf85063a_dev = dev;

	/* Get the i2c device binding*/
	struct pcf85063a_data *data = pcf85063a_dev->data;
	data->i2c = device_get_binding(DT_BUS_LABEL(DT_DRV_INST(0)));

	// Set I2C Device.
	if (data->i2c == NULL)
	{
		LOG_ERR("Failed to get pointer to %s device!",
				DT_BUS_LABEL(DT_DRV_INST(0)));
		return -EINVAL;
	}

	// Check if it's alive.
	uint8_t reg;
	int ret = i2c_reg_read_byte(data->i2c, DT_REG_ADDR(DT_DRV_INST(0)),
								PCF85063A_CTRL1, &reg);
	if (ret)
	{
		LOG_ERR("Failed to read from PCF85063A! (err %i)", ret);
		return -EIO;
	}

	LOG_INF("%s is initialized!", pcf85063a_dev->name);

	return 0;
}

DEVICE_AND_API_INIT(pcf85063a, DT_INST_LABEL(0), pcf85063a_init, &pcf85063a_data,
					&pcf85063_cfg_info, POST_KERNEL, CONFIG_I2C_INIT_PRIORITY,
					&pcf85063a_driver_api);
