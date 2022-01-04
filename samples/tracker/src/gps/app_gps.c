/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
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
#include <date_time.h>
#include <modem/agps.h>
#include <drivers/gpio.h>

#include <app_gps.h>
#include <app_event_manager.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_gps);

/* Power supply mode control */
#define PSCTL_LABEL DT_NODELABEL(psctl)
#define MODE_PIN DT_GPIO_PIN_BY_IDX(PSCTL_LABEL, mode_gpios, 0)
#define MODE_FLAGS DT_GPIO_FLAGS_BY_IDX(PSCTL_LABEL, mode_gpios, 0)

/* GPS device. Used to identify the GPS driver in the sensor API. */
static const struct device *gps_dev, *gpio;

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
    {
        LOG_DBG("GPS_EVT_SEARCH_STARTED");
        state = APP_GPS_STATE_STARTED;

#if defined(CONFIG_GPS_POWER_SUPPLY_MODE_OVERRIDE)
        gpio_pin_set(gpio, MODE_PIN, true);
#endif

        /* Push event */
        APP_EVENT_MANAGER_PUSH(APP_EVENT_GPS_STARTED);
    }
    break;
    case GPS_EVT_SEARCH_STOPPED:
        LOG_DBG("GPS_EVT_SEARCH_STOPPED");
        state = APP_GPS_STATE_STOPPED;

#if defined(CONFIG_GPS_POWER_SUPPLY_MODE_OVERRIDE)
        gpio_pin_set(gpio, MODE_PIN, false);
#endif
        break;
    case GPS_EVT_SEARCH_TIMEOUT:
    {
        LOG_DBG("GPS_EVT_SEARCH_TIMEOUT");

#if defined(CONFIG_GPS_POWER_SUPPLY_MODE_OVERRIDE)
        gpio_pin_set(gpio, MODE_PIN, false);
#endif

        /* Push event */
        APP_EVENT_MANAGER_PUSH(APP_EVENT_GPS_TIMEOUT);
    }
    break;
    case GPS_EVT_PVT:
        /* Don't spam logs */
        break;
    case GPS_EVT_PVT_FIX:
        LOG_DBG("GPS_EVT_PVT_FIX");
        time_set(&evt->pvt);
        data_send(&evt->pvt);

#if defined(CONFIG_GPS_POWER_SUPPLY_MODE_OVERRIDE)
        gpio_pin_set(gpio, MODE_PIN, false);
#endif
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
        memcpy(&app_event.agps_request, &evt->agps_request, sizeof(struct gps_agps_request));

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

    /* Copy the data contents */
    memcpy(&app_event.gps_data.data, gps_data, sizeof(struct gps_pvt));

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

    /* Get GPIO */
    gpio = DEVICE_DT_GET(DT_NODELABEL(gpio0));
    if (gpio == NULL || !device_is_ready(gpio))
    {
        LOG_ERR("Error: unable to get gpio0 device binding.\n");
        return -EEXIST;
    }

#if defined(CONFIG_GPS_POWER_SUPPLY_MODE_OVERRIDE)
    gpio_pin_configure(gpio, MODE_PIN, GPIO_OUTPUT_INACTIVE | MODE_FLAGS);
#endif

    return 0;
}

int app_gps_start(void)
{
    int err;

    /* Return already started error */
    if (state == APP_GPS_STATE_STARTED)
    {
        return -EALREADY;
    }

    /* Config w/ timeout */
    struct gps_config gps_cfg = {
        .nav_mode = GPS_NAV_MODE_SINGLE_FIX,
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

/* Converts the A-GPS data request from GPS driver to GNSS API format. */
static void agps_request_convert(
    struct nrf_modem_gnss_agps_data_frame *dest,
    const struct gps_agps_request *src)
{
    dest->sv_mask_ephe = src->sv_mask_ephe;
    dest->sv_mask_alm = src->sv_mask_alm;
    dest->data_flags = 0;
    if (src->utc)
    {
        dest->data_flags |= NRF_MODEM_GNSS_AGPS_GPS_UTC_REQUEST;
    }
    if (src->klobuchar)
    {
        dest->data_flags |= NRF_MODEM_GNSS_AGPS_KLOBUCHAR_REQUEST;
    }
    if (src->nequick)
    {
        dest->data_flags |= NRF_MODEM_GNSS_AGPS_NEQUICK_REQUEST;
    }
    if (src->system_time_tow)
    {
        dest->data_flags |= NRF_MODEM_GNSS_AGPS_SYS_TIME_AND_SV_TOW_REQUEST;
    }
    if (src->position)
    {
        dest->data_flags |= NRF_MODEM_GNSS_AGPS_POSITION_REQUEST;
    }
    if (src->integrity)
    {
        dest->data_flags |= NRF_MODEM_GNSS_AGPS_INTEGRITY_REQUEST;
    }
}

int app_gps_agps_request(struct gps_agps_request *req)
{
    struct nrf_modem_gnss_agps_data_frame agps_request;

    agps_request_convert(&agps_request, req);

    int err = agps_request_send(agps_request, AGPS_SOCKET_NOT_PROVIDED);

    if (err)
    {
        LOG_ERR("Failed to request A-GPS data, error: %d", err);
        return err;
    }

    return 0;
}
