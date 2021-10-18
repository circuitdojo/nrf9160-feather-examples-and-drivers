/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file app_gps.c
 * @author Jared Wolff (hello@jaredwolff.com)
 * @date 2021-07-20
 * 
 * @copyright Copyright Circuit Dojo (c) 2021
 * 
 */

#include <zephyr.h>
#include <stdio.h>
#include <date_time.h>

#include <app_gps.h>
#include <app_event_manager.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_gps);

/* GPS device. Used to identify the GPS driver in the sensor API. */
static const struct device *gps_dev;

/* Forward declarations. */
static void time_set(struct gps_pvt *gps_data);
static void data_send(struct gps_pvt *gps_data);

/* Tracking state */
static enum app_gps_state state = APP_GPS_STATE_STOPPED;

/* Handlers */
static void gps_event_handler(const struct device *dev, struct gps_event *evt)
{
    switch (evt->type)
    {
    case GPS_EVT_SEARCH_STARTED:
        LOG_DBG("GPS_EVT_SEARCH_STARTED");
        state = APP_GPS_STATE_STARTED;
        break;
    case GPS_EVT_SEARCH_STOPPED:
        LOG_DBG("GPS_EVT_SEARCH_STOPPED");
        state = APP_GPS_STATE_STOPPED;
        break;
    case GPS_EVT_SEARCH_TIMEOUT:
        LOG_DBG("GPS_EVT_SEARCH_TIMEOUT");
        break;
    case GPS_EVT_PVT:
        /* Don't spam logs */
        break;
    case GPS_EVT_PVT_FIX:
        LOG_DBG("GPS_EVT_PVT_FIX");
        time_set(&evt->pvt);
        data_send(&evt->pvt);
        break;
    case GPS_EVT_NMEA:
        /* Don't spam logs */
        break;
    case GPS_EVT_NMEA_FIX:
        LOG_DBG("Position fix with NMEA data");
        break;
    case GPS_EVT_OPERATION_BLOCKED:
        LOG_DBG("GPS_EVT_OPERATION_BLOCKED");
        break;
    case GPS_EVT_OPERATION_UNBLOCKED:
        LOG_DBG("GPS_EVT_OPERATION_UNBLOCKED");
        break;
    case GPS_EVT_AGPS_DATA_NEEDED:
    {
        LOG_DBG("GPS_EVT_AGPS_DATA_NEEDED");

        struct app_event app_event = {
            .type = APP_EVENT_AGPS_REQUEST,
        };

        /* Allocate on heap */
        app_event.agps_request = k_malloc(sizeof(struct gps_agps_request));
        memcpy(app_event.agps_request, &evt->agps_request, sizeof(struct gps_agps_request));

        /* Push it to the limit */
        app_event_manager_push(&app_event);
    }
    break;
    case GPS_EVT_ERROR:
        LOG_DBG("GPS_EVT_ERROR\n");
        break;
    default:
        break;
    }
}

/* Static module functions. */
static void data_send(struct gps_pvt *gps_data)
{

    int err;
    int64_t message_ts = 0;

    err = date_time_now(&message_ts);
    if (err)
    {
        LOG_ERR("date_time_now, error: %d", err);
    }

    struct app_event app_event = {
        .type = APP_EVENT_GPS_DATA};

    /* Allocate and set GPS data */
    app_event.gps_data = k_malloc(sizeof(struct gps_pvt));

    /* Copy the data contents */
    memcpy(app_event.gps_data, gps_data, sizeof(struct gps_pvt));

    err = app_event_manager_push(&app_event);
    if (err)
    {
        LOG_ERR("Unable to send gps event. Err %i", err);
    }
}

int app_gps_stop(void)
{
    int err;

    err = gps_stop(gps_dev);
    if (err)
    {
        LOG_WRN("Failed to stop GPS, error: %d", err);
        return err;
    }

    return 0;
}

static void time_set(struct gps_pvt *gps_data)
{
    /* Change datetime.year and datetime.month to accommodate the
	 * correct input format.
	 */
    struct tm gps_time = {
        .tm_year = gps_data->datetime.year - 1900,
        .tm_mon = gps_data->datetime.month - 1,
        .tm_mday = gps_data->datetime.day,
        .tm_hour = gps_data->datetime.hour,
        .tm_min = gps_data->datetime.minute,
        .tm_sec = gps_data->datetime.seconds,
    };

    date_time_set(&gps_time);
}

int app_gps_setup(void)
{
    int err;

    gps_dev = device_get_binding("NRF9160_GPS");
    if (gps_dev == NULL)
    {
        LOG_ERR("Could not get gps device");
        return -ENODEV;
    }

    err = gps_init(gps_dev, gps_event_handler);
    if (err)
    {
        LOG_ERR("Could not initialize GPS, error: %d", err);
        return err;
    }

    return 0;
}

int app_gps_start(void)
{
    int err;

    /* Return an error if it's already running */
    if (state == APP_GPS_STATE_STARTED)
    {
        return -EALREADY;
    }

    /* Config w/ timeout */
    struct gps_config gps_cfg = {
        .nav_mode = GPS_NAV_MODE_PERIODIC,
        .power_mode = GPS_POWER_MODE_DISABLED,
        .timeout = CONFIG_GPS_CONTROL_FIX_TRY_TIME,
        .interval =
            CONFIG_GPS_CONTROL_FIX_CHECK_INTERVAL,
        .priority = true,
    };

    /* Start GPS work */
    err = gps_start(gps_dev, &gps_cfg);
    if (err)
    {
        LOG_WRN("Failed to start GPS, error: %d", err);
        return err;
    }

    return 0;
}

int app_gps_agps_request(struct gps_agps_request *req)
{
    int err;

    err = gps_agps_request_send(*req, GPS_SOCKET_NOT_PROVIDED);
    if (err)
    {
        LOG_WRN("Failed to request A-GPS data, error: %d", err);
        LOG_WRN("This is expected to fail if we are not in a connected state");
        return err;
    }

    return 0;
}
