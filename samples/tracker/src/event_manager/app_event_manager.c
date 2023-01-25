/*
 * Copyright Circuit Dojo (c) 2021
 *
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#include <zephyr/kernel.h>

#include <app_event_manager.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_event_manager);

/* Define message queue */
K_MSGQ_DEFINE(app_event_msq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);

/* Static lookup table */
static char *event_manager_events[] = {
    "APP_EVENT_CELLULAR_DISCONNECT",
    "APP_EVENT_CELLULAR_CONNECTED",
    "APP_EVENT_BACKEND_ERROR",
    "APP_EVENT_BACKEND_CONNECTED",
    "APP_EVENT_BACKEND_DISCONNECTED",
    "APP_EVENT_AGPS_REQUEST",
    "APP_EVENT_GPS_ACTIVE",
    "APP_EVENT_GPS_INACTIVE",
    "APP_EVENT_GPS_DATA",
    "APP_EVENT_GPS_STARTED",
    "APP_EVENT_GPS_TIMEOUT",
    "APP_EVENT_MOTION_EVENT",
    "APP_EVENT_ACTIVITY_TIMEOUT",
    "APP_EVENT_MOTION_DATA",
    "APP_EVENT_UNKNOWN"};

int app_event_manager_push(struct app_event *p_evt)
{
    return k_msgq_put(&app_event_msq, p_evt, K_NO_WAIT);
}

int app_event_manager_get(struct app_event *p_evt)
{
    return k_msgq_get(&app_event_msq, p_evt, K_FOREVER);
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