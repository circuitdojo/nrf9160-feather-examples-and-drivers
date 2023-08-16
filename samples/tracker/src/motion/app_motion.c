/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#include <app_motion.h>
#include <app_event_manager.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_motion);

static struct app_motion_config m_config = {
    .trigger_interval = 600,
};
static int64_t last_trigger = 0;
static const struct device *sensor = DEVICE_DT_GET(DT_ALIAS(accel0));

static void trigger_handler(const struct device *dev,
                            const struct sensor_trigger *trig)
{
    LOG_DBG("Accel trig");

    int64_t uptime = k_uptime_get();

    /* Prevent constant triggers */
    if (uptime > (last_trigger + m_config.trigger_interval * MSEC_PER_SEC) || last_trigger == 0)
    {

        /* TODO: do something.. */
        last_trigger = uptime;
    }
}

int app_motion_sample_fetch(struct app_motion_data *p_data)
{

    int err;
    struct sensor_value val[3];

    err = sensor_sample_fetch_chan(sensor, SENSOR_CHAN_ACCEL_XYZ);
    if (err)
    {
        LOG_ERR("Unable to fetch data. Err: %i", err);
        return err;
    }

    err = sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, val);
    if (err)
    {
        LOG_ERR("Unable to get data. Err: %i", err);
        return err;
    }

    p_data->x = val[0];
    p_data->y = val[1];
    p_data->z = val[2];

    /* Invert z-axis since it's on the bottom side of the board*/
    p_data->z.val1 *= -1;
    p_data->z.val2 *= -1;

    return 0;
}

void app_motion_reset_trigger_time(void)
{
    last_trigger = 0;
}

void app_motion_set_trigger_time(uint64_t val)
{
    last_trigger = val;
}

static int app_motion_init(void)
{

    /* Get accelerometer */
    if (!device_is_ready(sensor))
    {
        LOG_ERR("Could not get accel0 device");
        return -ENODEV;
    }

    struct sensor_trigger trig;
    struct sensor_value attr;
    int rc;

    /* Set to 1.5G in m/s^2 */
    attr.val1 = 0;
    attr.val2 = (int32_t)(SENSOR_G * 1.5);

    rc = sensor_attr_set(sensor, SENSOR_CHAN_ACCEL_XYZ,
                         SENSOR_ATTR_SLOPE_TH, &attr);
    if (rc < 0)
    {
        LOG_ERR("Cannot set slope threshold.");
        return rc;
    }

    /* Set trigger values */
    trig.type = SENSOR_TRIG_DELTA;
    trig.chan = SENSOR_CHAN_ACCEL_XYZ;

    rc = sensor_trigger_set(sensor, &trig, trigger_handler);
    if (rc != 0)
    {
        LOG_ERR("Failed to set trigger: %d", rc);
        return rc;
    }

    return 0;
}

SYS_INIT(app_motion_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);