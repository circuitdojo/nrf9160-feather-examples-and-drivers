/*
 * Copyright 2023 Circuit Dojo LLC
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BACKEND_H
#define _BACKEND_H

#include <zephyr/kernel.h>

/**
 * @brief Initialize the backend
 * 
 * @param client_id ID of this client. 
 * @param client_id_len Length of the ID string
 * @return int 0 on success
 */
int app_backend_init(char *client_id, size_t client_id_len);

/**
 * @brief Publish to the backend
 * 
 * @param topic topic string used
 * @param p_data pointer to data structure
 * @param len length of data
 * @return int 0 on success
 */
int app_backend_publish(char *topic, uint8_t *p_data, size_t len);

/**
 * @brief Publish to the backend stream  (if possible)
 * 
 * @param topic topic string used
 * @param p_data pointer to data structure
 * @param len length of data
 * @return int 0 on success
 */
int app_backend_stream(char *topic, uint8_t *p_data, size_t len);

/**
 * @brief Connect to the backend
 * 
 * @return int 0 on success
 */
int app_backend_connect(void);

/**
 * @brief Disconnect from the backend
 * 
 * @return int 0 on success
 */
int app_backend_disconnect(void);

/**
 * @brief Checks if connected to the backend
 * 
 * @return true is connected
 * @return false is not connected
 */
bool app_backend_is_connected(void);

#endif