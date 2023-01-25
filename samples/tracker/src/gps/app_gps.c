/*
 * Copyright (c) 2022 Circuit Dojo LLC
 */

/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/kernel.h>

#include <stdio.h>
#include <date_time.h>

#include <app_gps.h>
#include <app_event_manager.h>

#include <nrf_modem_gnss.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_gps);

/* Forward declarations. */
// static void time_set(struct gps_pvt *gps_data);
// static void data_send(struct gps_pvt *gps_data);

/* Semaphore/event handling */
K_MSGQ_DEFINE(nmea_queue, sizeof(struct nrf_modem_gnss_nmea_data_frame), 10, 4);
static K_SEM_DEFINE(pvt_data_sem, 0, 1);
static K_SEM_DEFINE(time_sem, 0, 1);

/* Tracking state */
static enum app_gps_state state = APP_GPS_STATE_STOPPED;

/* Tracking */
static struct nrf_modem_gnss_pvt_data_frame last_pvt;

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
        retval = nrf_modem_gnss_read(&last_pvt, sizeof(last_pvt), NRF_MODEM_GNSS_DATA_PVT);
        if (retval == 0)
        {
            k_sem_give(&pvt_data_sem);
        }
        break;

    case NRF_MODEM_GNSS_EVT_NMEA:
        retval = nrf_modem_gnss_read(&nmea_data,
                                     sizeof(struct nrf_modem_gnss_nmea_data_frame),
                                     NRF_MODEM_GNSS_DATA_NMEA);
        if (retval == 0)
        {
            retval = k_msgq_put(&nmea_queue, &nmea_data, K_NO_WAIT);
        }
        break;

    case NRF_MODEM_GNSS_EVT_AGPS_REQ:
        retval = nrf_modem_gnss_read(&last_agps,
                                     sizeof(last_agps),
                                     NRF_MODEM_GNSS_DATA_AGPS_REQ);
        if (retval == 0)
        {
            // TODO: send agps request
            // k_work_submit_to_queue(&gnss_work_q, &agps_data_get_work);
        }
        break;

    default:
        break;
    }
}

/* Static module functions. */
// static void data_send(struct gps_pvt *gps_data)
// {

//     int err;
//     int64_t message_ts = 0;
//     uint8_t nsat = 0;

//     err = date_time_now(&message_ts);
//     if (err)
//     {
//         LOG_ERR("date_time_now, error: %d", err);
//     }

//     /* Get satellite count */
//     for (int i = 0; i < GPS_PVT_MAX_SV_COUNT; i++)
//     {
//         LOG_DBG("sv: %i in fix: %i", i, gps_data->sv[i].in_fix);
//         if (gps_data->sv[i].in_fix)
//         {
//             nsat++;
//         }
//     }

//     struct app_event event = {
//         .type = EVENT_GPS_DATA};

//     /* Set GPS data */
//     event.gps_data->fix = 1;
//     event.gps_data->nsat = nsat;
//     event.gps_data->lat = gps_data->latitude;
//     event.gps_data->spkm = 0;
//     event.gps_data->lng = gps_data->longitude;
//     event.gps_data->alt = gps_data->altitude;
//     event.gps_data->hdop = gps_data->hdop;
//     event.gps_data->timestamp = message_ts;

//     err = app_event_manager_push(&event);
//     if (err)
//     {
//         LOG_ERR("Unable to send gps event. Err %i", err);
//     }
// }

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

// static void time_set(struct gps_pvt *gps_data)
// {
//     /* Change datetime.year and datetime.month to accommodate the
//      * correct input format.
//      */
//     struct tm gps_time = {
//         .tm_year = gps_data->datetime.year - 1900,
//         .tm_mon = gps_data->datetime.month - 1,
//         .tm_mday = gps_data->datetime.day,
//         .tm_hour = gps_data->datetime.hour,
//         .tm_min = gps_data->datetime.minute,
//         .tm_sec = gps_data->datetime.seconds,
//     };

//     date_time_set(&gps_time);
// }

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