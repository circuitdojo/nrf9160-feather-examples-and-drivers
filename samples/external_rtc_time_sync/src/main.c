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
#include <date_time.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <drivers/counter/pcf85063a.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

K_SEM_DEFINE(date_time_ready, 0, 1);

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

	int err = counter_start(rtc);
	if (err)
	{
		LOG_ERR("Unable to start RTC. Err: %i", err);
	}
}

static void print_date_time(const struct tm *time)
{
	LOG_INF("%i:%i:%i %i/%i/%i - %i", time->tm_hour, time->tm_min, time->tm_sec, time->tm_mon + 1, time->tm_mday, 1900 + time->tm_year, time->tm_isdst);
	LOG_INF("yday %i", time->tm_yday);
}

static void date_time_event_handler(const struct date_time_evt *evt)
{
	switch (evt->type)
	{
	case DATE_TIME_OBTAINED_MODEM:
	case DATE_TIME_OBTAINED_NTP:
	case DATE_TIME_OBTAINED_EXT:
		LOG_INF("Date & time obtained.");
		k_sem_give(&date_time_ready);
		break;
	case DATE_TIME_NOT_OBTAINED:
		LOG_INF("Date & time not obtained.");
		break;
	default:
		break;
	}
}

void main(void)
{
	int err = 0;

	/* Init RTC */
	rtc_init();

	/* Init lte_lc*/
	err = lte_lc_init_and_connect();
	if (err)
		LOG_ERR("Failed to init and connect. Err: %i", err);

	/* Force time update */
	err = date_time_update_async(date_time_event_handler);
	if (err)
		LOG_ERR("Unable to update time with date_time_update_async. Err: %i", err);

	/* Wait for time */
	k_sem_take(&date_time_ready, K_FOREVER);

	/* Get the time */
	uint64_t ts = 0;
	err = date_time_now(&ts);
	if (err)
	{
		LOG_ERR("Unable to get date & time. Err: %i", err);
		return;
	}

	/* Convert to seconds */
	ts = ts / 1000;

	LOG_INF("UTC Unix Epoc: %lld", ts);

	/* Convert time to struct tm */
	struct tm *tm_p = gmtime(&ts);
	struct tm time = {0};
	struct tm future_time = {0};

	if (tm_p != NULL)
	{
		memcpy(&time, tm_p, sizeof(struct tm));
	}
	else
	{
		LOG_ERR("Unable to convert to broken down UTC");
		return;
	}

	/* Print out the current time */
	print_date_time(&time);

	/* Set time */
	err = pcf85063a_set_time(rtc, &time);
	if (err)
	{
		LOG_ERR("Unable to set time. Err: %i", err);
		return;
	}

	k_sleep(K_SECONDS(2));

	/* Get current time from device */
	err = pcf85063a_get_time(rtc, &future_time);
	if (err)
	{
		LOG_ERR("Unable to get time. Err: %i", err);
	}

	/* Print out the current time */
	print_date_time(&future_time);

	/* Convert back to timestamp */
	uint64_t future_ts = mktime(&future_time);

	LOG_INF("UTC Unix Epoc: %lld", future_ts);
}
