/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/*
 * Copyright Circuit Dojo (c) 2021
 *
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#include <date_time.h>
#include <modem/lte_lc.h>
#include <modem/modem_info.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem.h>
#include <nrf_modem_at.h>

#include <app_backend.h>
#include <app_battery.h>
#include <app_codec.h>
#include <app_event_manager.h>
#include <app_gps.h>
#include <app_indication.h>
#include <app_motion.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_main);

/* Flags */
static bool m_boot_message = false;

/* Imei */
static char imei[20];

int main(void)
{
    int err;

    /* Show version */
    LOG_INF("Tracker. Version: %s", CONFIG_APP_VERSION);

    /* Set up indication */
    err = app_indication_init();
    if (err)
        LOG_ERR("Unable to setup indication. Err: %i", err);

#ifdef CONFIG_USE_LED_INDICATION
    /* Glowing LED */
    app_indication_set(app_indication_glow);
#endif

    /* Init modem library */
    err = nrf_modem_lib_init();
    if (err < 0)
        __ASSERT_MSG_INFO("Unable to initialize modem lib. (err: %i)", err);

    /* Init lte_lc*/
    err = lte_lc_init();
    if (err < 0)
        __ASSERT_MSG_INFO("Failed to init. Err: %i", err);

    /* Power saving is turned on */
    err = lte_lc_psm_req(true);
    if (err < 0)
        __ASSERT_MSG_INFO("PSM request failed. (err: %i)", err);

    /* Connect to LTE */
    err = lte_lc_connect();
    if (err < 0)
        __ASSERT_MSG_INFO("LTE connect failed. (err: %i)", err);

    /* Setup gps */
    err = app_gps_setup();
    if (err)
        LOG_ERR("Unable to setup GPS. Err: %i", err);

    /* Configure modem info module*/
    err = modem_info_init();
    if (err)
    {
        LOG_ERR("Failed initializing modem info module, error: %d", err);
    }

    /* Get the IMEI (used for client ID)*/
    err = modem_info_string_get(MODEM_INFO_IMEI, imei, sizeof(imei));
    if (err < 0)
    {
        __ASSERT(false, "Unable to get IMEI. Err: %i", err);
    }
    else
    {
        imei[15] = '\0';
    }

    LOG_INF("IMEI: %s", (char *)(imei));

    /* Init backend */
    app_backend_init(imei, strlen(imei));

    /* Initialize motion */
    struct app_motion_config motion_config = {
        .trigger_interval = 600,
    };

    err = app_motion_init(motion_config);
    if (err)
        LOG_ERR("Unable to configure motion. Err: %i", err);

    /* Start GPS operations */
    err = app_gps_start();
    if (err)
        LOG_ERR("Unable to start GPS. Err: %i", err);

    while (true)
    {

        int err;
        struct app_event evt;

        /* Wait for the next event */
        err = app_event_manager_get(&evt);
        if (err)
        {
            LOG_WRN("Unable to get event. Err: %i", err);
            continue;
        }

        LOG_INF("Evt: %s", app_event_type_to_string(evt.type));

        switch (evt.type)
        {
        case APP_EVENT_CELLULAR_DISCONNECT:
            break;
        case APP_EVENT_CELLULAR_CONNECTED:
            break;
        case APP_EVENT_BACKEND_CONNECTED:
        {

            uint8_t buf[256];
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
                modem_info.data.device.battery.value = app_battery_sample();
                app_battery_measure_enable(false);

                /* Set app version */
                modem_info.data.device.app_version = CONFIG_APP_VERSION;

                /* Encode */
                err =
                    app_codec_device_info_encode(&modem_info, buf, sizeof(buf), &size);
                if (err)
                {
                    LOG_ERR("Unable to encode boot time. Err: %i", err);
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

            break;
        }
        case APP_EVENT_GPS_DATA:
        {
            uint8_t buf[256];
            size_t size = 0;

#ifdef CONFIG_USE_LED_INDICATION
            /* Solid LED */
            app_indication_set(app_indication_solid);
#endif

            /* Set motion time to now -- avoids motion trigger */
            app_motion_set_trigger_time(k_uptime_get());

            /* Save timestamp */
            if (evt.gps_data.ts == 0)
            {
                /* Get the current time */
                err = date_time_now(&evt.gps_data.ts);
                if (err)
                    LOG_WRN("Unable to get timestamp!");
            }

            /* Encode CBOR data */
            err = app_codec_gps_encode(&evt.gps_data, buf, sizeof(buf), &size);
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
        case APP_EVENT_MOTION_DATA:
        {

            uint8_t buf[256];
            size_t size = 0;

            /* Encode CBOR data */
            err = app_codec_motion_encode(&evt.motion_data, buf, sizeof(buf), &size);
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

            break;
        }
        case APP_EVENT_MOTION_EVENT:
        {

            /* Create motion data event */
            struct app_event out = {
                .type = APP_EVENT_MOTION_DATA,
            };

            err = app_motion_sample_fetch(&out.motion_data);
            if (err)
                LOG_ERR("Unable to get motion sample: Err: %i", err);
            else
                LOG_INF("x: %i.%i y: %i.%i z: %i.%i", out.motion_data.x.val1,
                        abs(out.motion_data.x.val2), out.motion_data.y.val1,
                        abs(out.motion_data.y.val2), out.motion_data.z.val1,
                        abs(out.motion_data.z.val2));

            err = date_time_now(&out.motion_data.ts);
            if (err)
                LOG_WRN("Unable to get timestamp!");

            /* (Re)start GPS operations */
            err = app_gps_start();
            if (err)
            {
                LOG_ERR("Unable to start GPS. Err: %i", err);
            }

            break;
        }
        case APP_EVENT_ACTIVITY_TIMEOUT:

            break;
        case APP_EVENT_GPS_TIMEOUT:

            /* Stop GPS */
            app_gps_stop();

            /* Reset count on motion */
            app_motion_reset_trigger_time();

            break;
        case APP_EVENT_GPS_STARTED:

            break;
        case APP_EVENT_BACKEND_DISCONNECTED:

            /* Disconnect */
            // activate_lte(false);

            break;
        case APP_EVENT_BACKEND_ERROR:
            /* TODO: fix this? */
            break;

        default:
            break;
        }
    }
}
