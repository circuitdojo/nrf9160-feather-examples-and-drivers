/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_MOTION_H
#define _APP_MOTION_H

#include <zephyr/drivers/sensor.h>

/**
 * @brief Data obtained from the device when an event occurs.
 *
 */
struct app_motion_data
{
    /* Time when the sample occured */
    int64_t ts;

    /* Value of XYZ when event occured */
    struct sensor_value x, y, z;
};

/**
 * @brief
 *
 */
struct app_motion_config
{
    /* Interval in seconds between wake events */
    int trigger_interval;
};

/**
 * @brief Resets trigger mechanism so another event can be
 *  triggered before the next trigger interval
 *
 */
void app_motion_reset_trigger_time(void);

void app_motion_set_trigger_time(uint64_t val);

/**
 * @brief Gets a sample from the device
 *
 * @param p_data pointer to the data structure being filled
 * @return int 0 on success
 */
int app_motion_sample_fetch(struct app_motion_data *p_data);

#endif