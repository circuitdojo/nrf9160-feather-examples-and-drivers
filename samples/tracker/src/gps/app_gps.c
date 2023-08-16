/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_gps);

/* Nordic libs */
#include <nrf_modem_gnss.h>
#include <date_time.h>

/* Local deps */
#include <app_gps.h>
#include <app_event_manager.h>

/* Tracking state */
static enum app_gps_state state = APP_GPS_STATE_STOPPED;

/* Tracking */
static atomic_t has_data = ATOMIC_INIT(0);
static struct app_gps_data last_pvt;

/* AGPS */
static struct nrf_modem_gnss_agps_data_frame last_agps;

/* Handlers */
static void gnss_event_handler(int event)
{
    int retval;
    struct nrf_modem_gnss_nmea_data_frame nmea_data;

    switch (event)
    {
    case NRF_MODEM_GNSS_EVT_PVT:
        break;
    case NRF_MODEM_GNSS_EVT_FIX:
        retval = nrf_modem_gnss_read(&last_pvt.data, sizeof(last_pvt.data), NRF_MODEM_GNSS_DATA_PVT);
        if (retval == 0)
        {

            /* Get timestamp */
            int err = date_time_now(&last_pvt.ts);
            if (err < 0)
                LOG_WRN("date_time_now, error: %d", err);

            /* Set flag */
            atomic_set(&has_data, 1);

            /* Indicate ready */
            APP_EVENT_MANAGER_PUSH(APP_EVENT_GPS_DATA)
        }
        break;
    case NRF_MODEM_GNSS_EVT_NMEA:
        retval = nrf_modem_gnss_read(&nmea_data,
                                     sizeof(struct nrf_modem_gnss_nmea_data_frame),
                                     NRF_MODEM_GNSS_DATA_NMEA);
        if (retval == 0)
        {
            LOG_DBG("Got NMEA data");
        }
        break;

    case NRF_MODEM_GNSS_EVT_AGPS_REQ:
        retval = nrf_modem_gnss_read(&last_agps,
                                     sizeof(last_agps),
                                     NRF_MODEM_GNSS_DATA_AGPS_REQ);
        if (retval == 0)
        {
            LOG_INF("AGPS request!");
            /*TODO: send agps request*/
        }
        break;

    default:
        break;
    }
}

int app_gps_stop(void)
{
    int err;

    err = nrf_modem_gnss_stop();
    if (err)
    {
        LOG_WRN("Failed to stop GPS, error: %d", err);
        return err;
    }

    struct app_event event = {
        .type = APP_EVENT_GPS_INACTIVE,
    };
    app_event_manager_push(&event);

    return 0;
}

int app_gps_setup(void)
{
    int err;

    /* Configure GNSS. */
    err = nrf_modem_gnss_event_handler_set(gnss_event_handler);
    if (err != 0)
    {
        LOG_ERR("Could not initialize GPS, error: %d", err);
        return err;
    }

    /* Configure use case for GNSS */
    uint8_t use_case = NRF_MODEM_GNSS_USE_CASE_MULTIPLE_HOT_START;
    err = nrf_modem_gnss_use_case_set(use_case);
    if (err != 0)
    {
        LOG_ERR("Failed to set GNSS use case");
        return err;
    }

    /* Set power mode */
    uint8_t power_mode = NRF_MODEM_GNSS_PSM_DUTY_CYCLING_PERFORMANCE;
    err = nrf_modem_gnss_power_mode_set(power_mode);
    if (err != 0)
    {
        LOG_ERR("Failed to set GNSS power saving mode");
        return err;
    }

    if (nrf_modem_gnss_fix_retry_set(CONFIG_GNSS_SAMPLE_PERIODIC_TIMEOUT) != 0)
    {
        LOG_ERR("Failed to set GNSS fix retry");
        return -1;
    }

    if (nrf_modem_gnss_fix_interval_set(CONFIG_GNSS_SAMPLE_PERIODIC_INTERVAL) != 0)
    {
        LOG_ERR("Failed to set GNSS fix interval");
        return -1;
    }

    return 0;
}

int app_gps_start(void)
{
    int err;

    /* Return an error if it's already running */
    if (state == APP_GPS_STATE_ACTIVE)
    {
        return -EALREADY;
    }

    /* Start GPS work */
    err = nrf_modem_gnss_start();
    if (err)
    {
        LOG_WRN("Failed to start GPS, error: %d", err);
        return err;
    }

    /* Send status update to main */
    struct app_event event = {
        .type = APP_EVENT_GPS_ACTIVE,
    };
    app_event_manager_push(&event);

    return 0;
}

int app_gps_get_last_fix(struct app_gps_data *data)
{
    if (data == NULL)
        return -EINVAL;

    if (atomic_get(&has_data) == 0)
        return -ENODATA;

    /* Copy over */
    memcpy(data, &last_pvt, sizeof(struct app_gps_data));

    /* Reset flag */
    atomic_set(&has_data, 0);

    return 0;
}