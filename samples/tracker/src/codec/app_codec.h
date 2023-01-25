/*
 * Copyright Circuit Dojo (c) 2021
 *
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#ifndef _APP_CODEC_H
#define _APP_CODEC_H

#include <zephyr/kernel.h>

#include <modem/modem_info.h>

#include <app_gps.h>
#include <app_motion.h>

struct app_modem_info
{
    struct modem_param_info data;
    uint8_t rsrp;
    uint64_t ts;
};

/**
 * @brief Encodes a app_codec_gps_payload struct to binary CBOR
 *
 * @param p_payload the data structure we're working with
 * @param p_buf where the encoded data will be stored (destination buffer)
 * @param buf_len size of the destination buffer
 * @param p_size actual written size
 * @return int 0 or QCBORError
 */
int app_codec_gps_encode(struct app_gps_data *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size);

/**
 * @brief Encodes the device info
 *
 * @param p_payload the data structure we're working with
 * @param p_buf where the encoded data will be stored (destination buffer)
 * @param buf_len size of the destination buffer
 * @param p_size actual written size
 * @return int 0 or QCBORError
 */
int app_codec_device_info_encode(struct app_modem_info *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size);

/**
 * @brief Encodes motion event
 *
 * @param p_payload the data structure we're working with
 * @param p_buf where the encoded data will be stored (destination buffer)
 * @param buf_len size of the destination buffer
 * @param p_size actual written size
 * @return int 0 on success
 */
int app_codec_motion_encode(struct app_motion_data *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size);

#endif /*_APP_CODEC_H*/