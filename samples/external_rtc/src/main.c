/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/counter.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/* RTC control */
static const struct device *rtc = DEVICE_DT_GET(DT_ALIAS(rtc0));

static void rtc_init()
{

	/* Check if ready*/
	if (!device_is_ready(rtc))
	{
		LOG_ERR("RTC is not ready!");
	}

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
