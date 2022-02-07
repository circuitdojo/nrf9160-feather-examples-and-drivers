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

#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <modem/at_cmd.h>
#include <modem/at_notif.h>
#include <modem/modem_info.h>
#include <nrf_modem.h>
#include <date_time.h>
#include <power/reboot.h>

#include <app_gps.h>
#include <app_codec.h>
#include <app_event_manager.h>
#include <app_indication.h>
#include <app_backend.h>
#include <app_motion.h>
#include <app_battery.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_main);

/* Tracking state */
bool m_cellular_connected = false;
bool m_boot_message = false;
bool m_initial_fix = false;

/* Tracking time */
int64_t m_last_publish = 0;

/* Imei */
static char imei[20];

/* RSRP */
static uint8_t rsrp = 0;

/* Saving output if not connected */
K_MSGQ_DEFINE(outgoing_msgq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);

void activity_expiry_function(struct k_timer *dummy)
{
    APP_EVENT_MANAGER_PUSH(APP_EVENT_ACTIVITY_TIMEOUT);
}

void gps_expiry_function(struct k_timer *dummy)
{
    APP_EVENT_MANAGER_PUSH(APP_EVENT_GPS_TIMEOUT);
}

/* Timers */
K_TIMER_DEFINE(activity_timer, activity_expiry_function, NULL);
K_TIMER_DEFINE(gps_search_timer, gps_expiry_function, NULL);

/**
 * @brief LTE event handler
 *
 * @param evt
 */
static void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
    case LTE_LC_EVT_NW_REG_STATUS:
        LOG_DBG("LTE Status: %i", evt->nw_reg_status);

        if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
            (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING))
        {
            /* Not connected. Send event. */
            APP_EVENT_MANAGER_PUSH(APP_EVENT_CELLULAR_DISCONNECT);
            break;
        }

        /* Otherwise send connected event */
        APP_EVENT_MANAGER_PUSH(APP_EVENT_CELLULAR_CONNECTED);

        LOG_DBG("Network registration status: %s",
                evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home network" : "Connected - roaming");

        break;
    default:
        break;
    }
}

static void nrf_modem_lib_dfu_handler(void)
{
    int err;

    err = nrf_modem_lib_init(NORMAL_MODE);

    switch (err)
    {
    case MODEM_DFU_RESULT_OK:
        LOG_INF("Modem update suceeded, reboot");
        sys_reboot(SYS_REBOOT_COLD);
        break;
    case MODEM_DFU_RESULT_UUID_ERROR:
    case MODEM_DFU_RESULT_AUTH_ERROR:
        LOG_ERR("Modem update failed, error: %d", err);
        LOG_ERR("Modem will use old firmware");
        sys_reboot(SYS_REBOOT_COLD);
        break;
    case MODEM_DFU_RESULT_HARDWARE_ERROR:
    case MODEM_DFU_RESULT_INTERNAL_ERROR:
        LOG_ERR("Modem update malfunction, error: %d, reboot", err);
        sys_reboot(SYS_REBOOT_COLD);
        break;
    default:
        break;
    }
}

static int activate_lte(bool activate)
{
    int err;

    if (activate)
    {
        err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_LTE);
        if (err)
        {
            LOG_ERR("Failed to activate LTE, error: %d", err);
            return err;
        }
    }
    else
    {
        err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_DEACTIVATE_LTE);
        if (err)
        {
            LOG_ERR("Failed to deactivate LTE, error: %d", err);
            return err;
        }
    }

    return 0;
}

static bool check_and_establish_connection(void)
{

    /* Check if connected */
    if (!m_cellular_connected || !app_backend_is_connected())
    {

        if (!m_cellular_connected)
        {
            activate_lte(true);
        }
        else if (!app_backend_is_connected())
        {
            app_backend_connect();
        }

        /* Skip for now.. */
        return false;
    }

    return true;
}
static void rsrp_cb(char rsrp_value)
{
    rsrp = rsrp_value;
}

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

    /* Setup gps */
    err = app_gps_setup();
    if (err)
        LOG_ERR("Unable to setup GPS. Err: %i", err);

    /* Configure dfu handler*/
    nrf_modem_lib_dfu_handler();

    /* Register LTE handler */
    lte_lc_register_handler(lte_handler);

    /* Init lte_lc*/
    err = lte_lc_init_and_connect();
    if (err)
        LOG_ERR("Failed to init. Err: %i", err);

    /* Configure modem info module*/
    err = modem_info_init();
    if (err)
    {
        LOG_ERR("Failed initializing modem info module, error: %d",
                err);
    }

    /* RSRP value */
    err = modem_info_rsrp_register(rsrp_cb);

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

    LOG_INF("IMEI: %s", log_strdup(imei));

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
            m_cellular_connected = false;

            break;
        case APP_EVENT_CELLULAR_CONNECTED:

            /*Set the flag*/
            m_cellular_connected = true;

            /* Force time update */
            err = date_time_update_async(NULL);
            if (err)
                LOG_ERR("Unable to update time with date_time_update_async. Err: %i", err);

            /* Enable PSM mode */
            err = lte_lc_psm_req(true);
            if (err)
                LOG_ERR("Requesting PSM failed, error: %d", err);

            /* If there are outgoing messages, connect.. */
            if (k_msgq_num_used_get(&outgoing_msgq) > 0)
            {
                /* Connect to backend */
                err = app_backend_connect();
                if (err)
                    LOG_ERR("Unable to connect to backend. Err: %i", err);
            }

            break;
        case APP_EVENT_BACKEND_CONNECTED:
        {

            uint8_t buf[256];
            size_t size = 0;
            struct app_event outgoing;
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

                /* RSRP */
                modem_info.rsrp = rsrp;

                /* Get battery voltage in mV */
                app_battery_measure_enable(true);
                modem_info.data.device.battery.value = app_battery_sample();
                app_battery_measure_enable(false);

                /* Set app version */
                modem_info.data.device.app_version = CONFIG_APP_VERSION;

                /* Encode */
                err = app_codec_device_info_encode(&modem_info, buf, sizeof(buf), &size);
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

                /* Start (in)activity timer */
                k_timer_start(&activity_timer, K_SECONDS(10), K_NO_WAIT);

                /* Set flag */
                m_boot_message = true;
            }

            /* Move save messages */
            while (k_msgq_get(&outgoing_msgq, &outgoing, K_NO_WAIT) == 0)
            {
                app_event_manager_push(&outgoing);
            }

            break;
        }
        case APP_EVENT_AGPS_REQUEST:

            if (!m_cellular_connected)
            {
                /* Turn on LTE */
                activate_lte(true);

                /* Push event */
                app_event_manager_push(&evt);
            }
            else
            {
                /* Start AGPS request */
                err = app_gps_agps_request(&evt.agps_request);
                if (err)
                    LOG_ERR("Unable to download AGPS data. Err: %i", err);

                /* Disconnect from LTE after getting apgs data */
                if (!m_initial_fix)
                {
                    m_initial_fix = true;

                    // Disconnect only if it's enabled.
                    if (IS_ENABLED(CONFIG_DISCONNECT_FOR_FIRST_FIX))
                        activate_lte(false);
                }
            }

            break;
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

            /* Stop gps search timer */
            k_timer_stop(&gps_search_timer);

            /* Check if connected otherwise pop this into the outgoing msgq */
            if (!check_and_establish_connection())
            {
                /* Save msg */
                err = k_msgq_put(&outgoing_msgq, &evt, K_NO_WAIT);
                if (err)
                    LOG_ERR("Unable to queue to outgoing. Err: %i", err);

                break;
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

            /* Start (in)activity timer */
            k_timer_start(&activity_timer, K_SECONDS(10), K_NO_WAIT);

            break;

        case APP_EVENT_MOTION_DATA:
        {

            uint8_t buf[256];
            size_t size = 0;

            /* Check if connected otherwise pop this into the outgoing msgq */
            if (!app_backend_is_connected())
            {
                /* Save msg */
                err = k_msgq_put(&outgoing_msgq, &evt, K_NO_WAIT);
                if (err)
                    LOG_ERR("Unable to queue to outgoing. Err: %i", err);

                break;
            }

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

            /* Start (in)activity timer */
            k_timer_start(&activity_timer, K_SECONDS(1), K_NO_WAIT);

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
                LOG_INF("x: %i.%i y: %i.%i z: %i.%i", out.motion_data.x.val1, abs(out.motion_data.x.val2), out.motion_data.y.val1, abs(out.motion_data.y.val2), out.motion_data.z.val1, abs(out.motion_data.z.val2));

            err = date_time_now(&out.motion_data.ts);
            if (err)
                LOG_WRN("Unable to get timestamp!");

            /* Save msg */
            err = k_msgq_put(&outgoing_msgq, &evt, K_NO_WAIT);
            if (err)
                LOG_ERR("Unable to queue to outgoing. Err: %i", err);

            /* (Re)start GPS operations */
            err = app_gps_start();
            if (err)
            {
                LOG_ERR("Unable to start GPS. Err: %i", err);
            }

            break;
        }
        case APP_EVENT_ACTIVITY_TIMEOUT:

            /* Disconnect from backend */
            // app_backend_disconnect();

            break;
        case APP_EVENT_GPS_TIMEOUT:

            /* Stop timer */
            k_timer_stop(&gps_search_timer);

            /* Stop GPS */
            app_gps_stop();

            /* Reset count on motion */
            app_motion_reset_trigger_time();

            break;
        case APP_EVENT_GPS_STARTED:

            /* Start countdown if gps continues using CONFIG_GPS_CONTROL_FIX_TRY_TIME */
            k_timer_start(&gps_search_timer, K_SECONDS(CONFIG_GPS_CONTROL_FIX_TRY_TIME), K_NO_WAIT);

            break;
        case APP_EVENT_BACKEND_DISCONNECTED:

            /* Disconnect */
            // activate_lte(false);

            break;
        case APP_EVENT_BACKEND_ERROR:
            /* TODO: fix this? */
            break;
        }
        }
    }
}
