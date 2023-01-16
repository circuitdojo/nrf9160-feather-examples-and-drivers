/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include <drivers/counter.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/* RTC control */
const struct device *rtc;

static void rtc_init()
{

	/* Get the device */
	rtc = device_get_binding("PCF85063A");
	if (rtc == NULL)
	{
		LOG_ERR("Failed to get RTC device binding");
		return;
	}

	LOG_INF("device is %p, name is %s", rtc, log_strdup(rtc->name));

	/* 2 seconds */
	const struct counter_alarm_cfg cfg = {
		.ticks = 10,
	};

	/* Set the alarm */
	int ret = counter_set_channel_alarm(rtc, 0, &cfg);
	if (ret)
	{
		LOG_ERR("Unable to set alarm");
	}
}

static bool timer_flag = false;

void main(void)
{

	/* Init RTC */
	rtc_init();

	while (true)
	{
		if (!timer_flag)
		{
			int ret = counter_get_pending_int(rtc);

			if (ret == 1)
			{
				LOG_INF("Timer event!");

				timer_flag = true;

				int ret = counter_cancel_channel_alarm(rtc, 0);
				if (ret)
				{
					LOG_ERR("Unable to cancel channel alarm!");
				}
			}
		}

		k_sleep(K_MSEC(100));
	}
}
