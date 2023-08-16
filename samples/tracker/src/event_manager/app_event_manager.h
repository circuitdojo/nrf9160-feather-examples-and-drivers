/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_EVENT_MANAGER_H
#define _APP_EVENT_MANAGER_H

#include <app_motion.h>

/**
 * @brief Max size of event queue
 *
 */
#define APP_EVENT_QUEUE_SIZE 24

/**
 * @brief Simplified macro for pushing an app event without data
 *
 */
#define APP_EVENT_MANAGER_PUSH(x)  \
    struct app_event app_event = { \
        .type = x,                 \
    };                             \
    app_event_manager_push(&app_event);

/**
 * @brief Used to interact with different functionality
 * in this application
 */
enum app_event_type
{
    APP_EVENT_CELLULAR_DISCONNECT,
    APP_EVENT_CELLULAR_CONNECTED,
    APP_EVENT_BACKEND_CONNECTED,
    APP_EVENT_BACKEND_ERROR,
    APP_EVENT_BACKEND_DISCONNECTED,
    APP_EVENT_GPS_ACTIVE,
    APP_EVENT_GPS_INACTIVE,
    APP_EVENT_GPS_DATA,
    APP_EVENT_GPS_TIMEOUT,
    APP_EVENT_GPS_STARTED,
    APP_EVENT_MOTION_EVENT,
    APP_EVENT_ACTIVITY_TIMEOUT,
    APP_EVENT_END
};

/**
 * @brief Application event that can be passed back to the main
 * context
 *
 */
struct app_event
{
    enum app_event_type type;
    union
    {
        int err;
    };
};

/**
 * @brief Get the string representation of the Application event
 *
 * @param type app event type enum
 * @return char* NULL if error
 */
char *app_event_type_to_string(enum app_event_type type);

/**
 * @brief Pushes event to message queue
 *
 * @param p_evt the event to be copied.
 * @return int
 */
int app_event_manager_push(struct app_event *p_evt);

#endif