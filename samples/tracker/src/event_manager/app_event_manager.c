
/**
 * @file app_event_manager.c
 * @author Jared Wolff (hello@jaredwolff.com)
 * @date 2021-07-20
 * 
 * @copyright Copyright Circuit Dojo (c) 2021
 * 
 */

#include <zephyr.h>

#include <app_event_manager.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_event_manager);

#define APP_EVENT_QUEUE_SIZE 24

/* Define message queue */
K_MSGQ_DEFINE(app_event_msq, sizeof(struct app_event), APP_EVENT_QUEUE_SIZE, 4);

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
    switch (type)
    {
    case APP_EVENT_CELLULAR_DISCONNECT:
        return "APP_EVENT_CELLULAR_DISCONNECT";
    case APP_EVENT_CELLULAR_CONNECTED:
        return "APP_EVENT_CELLULAR_CONNECTED";
    case APP_EVENT_GOLIOTH_CONNECTED:
        return "APP_EVENT_GOLIOTH_CONNECTED";
    case APP_EVENT_AGPS_REQUEST:
        return "APP_EVENT_AGPS_REQUEST";
    case APP_EVENT_GPS_DATA:
        return "APP_EVENT_GPS_DATA";
    default:
        return "UNKNOWN";
    }
}