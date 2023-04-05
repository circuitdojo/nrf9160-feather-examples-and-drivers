/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include "../include/sensor.h"

/* Extern of sample_start_chan */
ZBUS_CHAN_DECLARE(sample_start_chan);

/* Polling on a fixed interval */
#ifndef CONFIG_LIS2DH_TRIGGER
/* Timer handler*/
static void sample_timer_handler(struct k_timer *timer)
{
    struct start_data start = {0};
    zbus_chan_pub(&sample_start_chan, &start, K_MSEC(500));
}

K_TIMER_DEFINE(sample_timer, sample_timer_handler, NULL);

/* Init function */
int timer_start_init(const struct device *dev)
{
    ARG_UNUSED(dev);

    k_timer_start(&sample_timer, K_SECONDS(1), K_SECONDS(1));

    return 0;
}

SYS_INIT(timer_start_init, APPLICATION, 99);
#endif

static void listener_cb(const struct zbus_channel *chan)
{
    const struct sensor_data *data = zbus_chan_const_msg(chan);

    LOG_INF("Got data! ts: %lli - x: %lf , y: %lf , z: %lf",
            data->ts,
            sensor_value_to_double(&data->value[0]),
            sensor_value_to_double(&data->value[1]),
            sensor_value_to_double(&data->value[2]));
}

ZBUS_LISTENER_DEFINE(sample_data_lis, listener_cb);
