#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sensor);

#include "../include/sensor.h"

const struct device *sensor = DEVICE_DT_GET(DT_ALIAS(accel0));

/* Zbus channels */
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

#ifdef CONFIG_LIS2DH_TRIGGER
ZBUS_CHAN_DECLARE(sample_start_chan);

static void trigger_handler(const struct device *dev,
                            const struct sensor_trigger *trig)
{
    struct start_data start = {0};
    zbus_chan_pub(&sample_start_chan, &start, K_MSEC(500));
}
#endif

int sensor_init(const struct device *dev)
{
    ARG_UNUSED(dev);

    if (!device_is_ready(sensor))
    {
        LOG_ERR("Could not get accel0 device");
        return -EIO;
    }

#if CONFIG_LIS2DH_TRIGGER
    {
        struct sensor_trigger trig;
        int rc;

        trig.type = SENSOR_TRIG_DATA_READY;
        trig.chan = SENSOR_CHAN_ACCEL_XYZ;

        if (IS_ENABLED(CONFIG_LIS2DH_ODR_RUNTIME))
        {
            struct sensor_value odr = {
                .val1 = 1,
            };

            rc = sensor_attr_set(sensor, trig.chan,
                                 SENSOR_ATTR_SAMPLING_FREQUENCY,
                                 &odr);
            if (rc != 0)
            {
                LOG_ERR("Failed to set odr: %d", rc);
                return rc;
            }
            LOG_INF("Sampling at %u Hz", odr.val1);
        }

        rc = sensor_trigger_set(sensor, &trig, trigger_handler);
        if (rc != 0)
        {
            LOG_ERR("Failed to set trigger: %d", rc);
            return rc;
        }

        LOG_INF("Waiting for triggers");
    }
#endif

    return 0;
}

SYS_INIT(sensor_init, APPLICATION, 90);

ZBUS_SUBSCRIBER_DEFINE(sample_start_sub, 4);

static void sensor_task(void)
{
    const struct zbus_channel *chan;

    while (!zbus_sub_wait(&sample_start_sub, &chan, K_FOREVER))
    {
        LOG_INF("Get sample data!");

        int rc = 0;

        /* Get sample */
        rc = sensor_sample_fetch(sensor);
        if (rc == -EBADMSG)
        {
            /* Sample overrun.  Ignore in polled mode. */
            if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER))
            {
                LOG_WRN("Overrun!\r\n");
            }
            rc = 0;
        }

        /* Continue if we fetched ok */
        if (rc == 0)
        {

            /* Get timestamp */
            struct sensor_data data = {.ts = k_uptime_get(), .value = {{0}}};

            rc = sensor_channel_get(sensor,
                                    SENSOR_CHAN_ACCEL_XYZ,
                                    data.value);
            if (rc == 0)
            {
                LOG_INF("Publishing sensor data!");
                zbus_chan_pub(&sample_data_chan, &data, K_MSEC(250));
            }
        }
    }
}

K_THREAD_DEFINE(sensor_task_id, 512, sensor_task, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);
