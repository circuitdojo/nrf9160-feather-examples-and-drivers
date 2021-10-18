/**
 * @file app_event_manager.h
 * @author Jared Wolff (hello@jaredwolff.com)
 * @date 2021-07-20
 * 
 * @copyright Copyright Circuit Dojo (c) 2021
 * 
 */

#ifndef _APP_EVENT_MANAGER_H
#define _APP_EVENT_MANAGER_H

#include <drivers/gps.h>

/**
 * @brief Used to interact with different functionality
 * in this application
 */
enum app_event_type
{

    APP_EVENT_CELLULAR_DISCONNECT,
    APP_EVENT_CELLULAR_CONNECTED,
    APP_EVENT_GOLIOTH_CONNECTED,
    APP_EVENT_AGPS_REQUEST,
    APP_EVENT_GPS_DATA
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
        struct gps_pvt *gps_data;
        struct gps_agps_request *agps_request;
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