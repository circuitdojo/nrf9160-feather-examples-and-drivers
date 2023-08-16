/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_event_manager);

/* Nordic deps */
#include <date_time.h>

/* Project deps */
#include <app_backend.h>
#include <app_battery.h>
#include <app_codec.h>
#include <app_event_manager.h>

/* Static flags */
static bool m_boot_message = false;

/* Define message queue */
K_MSGQ_DEFINE(app_event_msq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);

/* Static lookup table */
static char *event_manager_events[] = {
    "APP_EVENT_CELLULAR_DISCONNECT",
    "APP_EVENT_CELLULAR_CONNECTED",
    "APP_EVENT_BACKEND_CONNECTED",
    "APP_EVENT_BACKEND_ERROR",
    "APP_EVENT_BACKEND_DISCONNECTED",
    "APP_EVENT_GPS_ACTIVE",
    "APP_EVENT_GPS_INACTIVE",
    "APP_EVENT_GPS_DATA",
    "APP_EVENT_GPS_TIMEOUT",
    "APP_EVENT_GPS_STARTED",
    "APP_EVENT_MOTION_EVENT",
    "APP_EVENT_ACTIVITY_TIMEOUT",
    "APP_EVENT_UNKNOWN"};

int app_event_manager_push(struct app_event *p_evt)
{
    return k_msgq_put(&app_event_msq, p_evt, K_NO_WAIT);
}

char *app_event_type_to_string(enum app_event_type type)
{

    if (type <= APP_EVENT_END)
    {
        return event_manager_events[type];
    }
    else
    {
        return event_manager_events[APP_EVENT_END];
    }
}

void event_manager_thread(void *, void *, void *)
{

    for (;;)
    {
        int err = 0;
        struct app_event evt = {0};
        k_msgq_get(&app_event_msq, &evt, K_FOREVER);

        LOG_INF("Evt: %s", app_event_type_to_string(evt.type));

        switch (evt.type)
        {
        case APP_EVENT_CELLULAR_DISCONNECT:
            break;
        case APP_EVENT_CELLULAR_CONNECTED:

            /* Connect */
            app_backend_connect();

            break;
        case APP_EVENT_BACKEND_CONNECTED:
        {

            uint8_t buf[512];
            size_t size = 0;
            struct app_modem_info modem_info;

            if (!m_boot_message)
            {

                /* Get current time */
                err = date_time_now(&modem_info.ts);
                if (err)
                {
                    LOG_ERR("Unable to get current date/time. Err: %i", err);
                    break;
                }

                /* Config modem info params */
                err = modem_info_params_init(&modem_info.data);
                if (err)
                {
                    LOG_ERR("Could not initialize modem info parameters, error: %d", err);
                    break;
                }

                /* Get modem information */
                err = modem_info_params_get(&modem_info.data);
                if (err)
                {
                    LOG_ERR("Unable to get modem info. Err %i", err);
                    break;
                }

                /* Get battery voltage in mV */
                app_battery_measure_enable(true);
                int sample = app_battery_sample();
                app_battery_measure_enable(false);

                if (sample > 0)
                    modem_info.data.device.battery.value = sample;
                else
                    LOG_WRN("Unable to get battery measurement!");

                /* Set app version */
                modem_info.data.device.app_version = CONFIG_APP_VERSION;

                /* Encode */
                err =
                    app_codec_device_info_encode(&modem_info, buf, sizeof(buf), &size);
                if (err < 0)
                {
                    LOG_ERR("Unable to encode device info. Err: %i", err);
                    break;
                }

                /* Publish */
                err = app_backend_publish("boot", buf, size);
                if (err)
                {
                    LOG_ERR("Unable to publish. Err: %i", err);
                    break;
                }

                /* Set flag */
                m_boot_message = true;
            }

            /* Start GPS operations */
            err = app_gps_start();
            if (err)
                LOG_ERR("Unable to start GPS. Err: %i", err);

            break;
        }
        case APP_EVENT_GPS_DATA:
        {
            struct app_gps_data gps_data = {0};
            uint8_t buf[256];
            size_t size = 0;

#ifdef CONFIG_USE_LED_INDICATION
            /* Solid LED */
            app_indication_set(app_indication_solid);
#endif

            /* Set motion time to now -- avoids motion trigger */
            app_motion_set_trigger_time(k_uptime_get());

            /* Get last available */
            err = app_gps_get_last_fix(&gps_data);
            if (err < 0)
            {
                LOG_ERR("Unable to get last GPS data. Err: %i", err);
                break;
            }

            /* Encode CBOR data */
            err = app_codec_gps_encode(&gps_data, buf, sizeof(buf), &size);
            if (err < 0)
            {
                LOG_ERR("Unable to encode data. Err: %i", err);
                break;
            }

            LOG_INF("Data size: %i", size);

            /* Publish gps data */
            err = app_backend_publish("gps", buf, size);
            if (err)
            {
                LOG_ERR("Unable to publish. Err: %i", err);
            }

            /* Stream gps data */
            err = app_backend_stream("gps", buf, size);
            if (err)
            {
                LOG_ERR("Unable to stream. Err: %i", err);
            }

            break;
        }
        case APP_EVENT_MOTION_EVENT:
        {

            uint8_t buf[256];
            size_t size = 0;

            /* Create motion data event */
            struct app_motion_data motion_data;

            err = app_motion_sample_fetch(&motion_data);
            if (err)
                LOG_ERR("Unable to get motion sample: Err: %i", err);
            else
                LOG_INF("x: %i.%i y: %i.%i z: %i.%i", motion_data.x.val1,
                        abs(motion_data.x.val2), motion_data.y.val1,
                        abs(motion_data.y.val2), motion_data.z.val1,
                        abs(motion_data.z.val2));

            err = date_time_now(&motion_data.ts);
            if (err)
                LOG_WRN("Unable to get timestamp!");

            /* Encode CBOR dta */
            err = app_codec_motion_encode(&motion_data, buf, sizeof(buf), &size);
            if (err < 0)
            {
                LOG_ERR("Unable to encode data. Err: %i", err);
                break;
            }

            LOG_INF("Data size: %i", size);

            /* Publish gps data */
            err = app_backend_publish("motion", buf, size);
            if (err)
            {
                LOG_ERR("Unable to publish. Err: %i", err);
            }

            /* Also stream it */
            err = app_backend_stream("motion", buf, size);
            if (err)
            {
                LOG_ERR("Unable to publish. Err: %i", err);
            }

            /* (Re)start GPS operations */
            err = app_gps_start();
            if (err)
                LOG_ERR("Unable to start GPS. Err: %i", err);

            break;
        }
        case APP_EVENT_GPS_TIMEOUT:

            /* Stop GPS */
            // app_gps_stop();

            /* Reset count on motion */
            app_motion_reset_trigger_time();

            break;

        default:
            break;
        }
    }
}

#define EVENT_MANAGER_STACK_SIZE KB(8)
K_THREAD_DEFINE(event_manager_tid, EVENT_MANAGER_STACK_SIZE,
                event_manager_thread, NULL, NULL, NULL,
                K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);