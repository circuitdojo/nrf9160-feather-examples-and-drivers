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

ZBUS_CHAN_DEFINE(sample_start_chan,                /* Name */
                 struct start_data,                /* Message type */
                 NULL,                             /* Validator */
                 NULL,                             /* User data */
                 ZBUS_OBSERVERS(sample_start_sub), /* observers */
                 ZBUS_MSG_INIT(false)              /* Initial value */
);

ZBUS_CHAN_DEFINE(sample_data_chan,                /* Name */
                 struct sensor_data,              /* Message type */
                 NULL,                            /* Validator */
                 NULL,                            /* User data */
                 ZBUS_OBSERVERS(sample_data_lis), /* observers */
                 ZBUS_MSG_INIT(0)                 /* Initial value is 0 */
);

void main(void)
{
    int err = 0;

    /* Init sensor */
    err = sensor_init();
    if (err < 0)
        LOG_ERR("Unable to init sensor. Err: %i", err);

/* Setup timer if not using trigger  */
#ifndef CONFIG_LIS2DH_TRIGGER
    LOG_INF("Polling at 0.5 Hz");

    struct start_data start = {0};

    while (1)
    {
        zbus_chan_pub(&sample_start_chan, &start, K_MSEC(500));
        k_sleep(K_SECONDS(1));
    }
#endif
}

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
