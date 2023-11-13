/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _CLOUD_H
#define _CLOUD_H

struct device_data
{
    bool do_something;
};

/**
 * @brief Publish data to the cloud
 *
 * @param data Pointer to device data
 * @return int 0 if successful, negative errno otherwise
 *
 */
int cloud_publish(struct device_data *data);

/**
 * @brief Initialize the cloud
 *
 * @param callback Callback function to be called when data is received
 * @return int 0 if successful, negative errno otherwise
 *
 */
int cloud_init(void (*callback)(struct device_data *data));

#endif