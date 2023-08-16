/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_GPS_H
#define _APP_GPS_H

#include <nrf_modem_gnss.h>

/* GPS Event */
struct app_gps_data
{
    int64_t ts;
    struct nrf_modem_gnss_pvt_data_frame data;
};

enum app_gps_state
{
    APP_GPS_STATE_STOPPED,
    APP_GPS_STATE_ACTIVE,
};

/**
 * @brief Sets up GPS module
 *
 * @return int returns error code. 0 on success.
 */
int app_gps_setup(void);

/**
 * @brief Starts GPS request
 *
 * @return int 0 on success. Otherwise returns error code.
 */
int app_gps_start(void);

/**
 * @brief Stops automated GPS requests
 *
 * @return int 0 on success. Otherwise returns error code.
 */
int app_gps_stop(void);

/**
 * @brief Set GPS period (in seconds)
 *
 * @param seconds period for getting GPS data
 * @return int 0 on success. Otherwise returns error code.
 */
int app_gps_set_period(int seconds);

int app_gps_get_last_fix(struct app_gps_data *data);

#endif