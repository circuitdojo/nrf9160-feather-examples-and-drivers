/*
 * Copyright Circuit Dojo (c) 2021
 * 
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#ifndef _APP_GPS_H
#define _APP_GPS_H

#include <drivers/gps.h>

struct app_gps_data
{
    struct gps_pvt data;
    int64_t ts;
};

/**
 * @brief Used to track GPS state
 * 
 */
enum app_gps_state
{
    APP_GPS_STATE_STOPPED,
    APP_GPS_STATE_STARTED,
};

/**
 * @brief Setup GPS peripheral
 * 
 * @return int 0 on success
 */
int app_gps_setup(void);

/**
 * @brief Start acquiring fix.
 * 
 * @return int 0 on success
 */
int app_gps_start(void);

/**
 * @brief Stop acquiring GPS fix
 * 
 * @return int 0 on success
 */
int app_gps_stop(void);

/**
 * @brief Downloads AGPS data via SUPL client
 * 
 * @param req request data from GPS
 * @return int 0 on success
 */
int app_gps_agps_request(struct gps_agps_request *req);

#endif /*_APP_GPS_H*/