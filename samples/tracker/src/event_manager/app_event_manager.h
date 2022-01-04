/*
 * Copyright Circuit Dojo (c) 2021
 * 
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#ifndef _APP_EVENT_MANAGER_H
#define _APP_EVENT_MANAGER_H

#include <drivers/gps.h>

#include <app_motion.h>
#include <app_gps.h>

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
    APP_EVENT_AGPS_REQUEST,
    APP_EVENT_GPS_DATA,
    APP_EVENT_GPS_TIMEOUT,
    APP_EVENT_GPS_STARTED,
    APP_EVENT_MOTION_EVENT,
    APP_EVENT_MOTION_DATA,
    APP_EVENT_ACTIVITY_TIMEOUT
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
        struct app_gps_data gps_data;
        struct gps_agps_request agps_request;
        struct app_motion_data motion_data;
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

/**
 * @brief Gets an event from the message queue
 * 
 * @param p_evt pointer where the data will be copied to.
 * @return int 
 */
int app_event_manager_get(struct app_event *p_evt);

#endif