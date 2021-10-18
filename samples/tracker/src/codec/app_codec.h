/**
 * @file app_codec.h
 * @author Jared Wolff (hello@jaredwolff.com)
 * @date 2021-07-25
 * 
 * @copyright Copyright Circuit Dojo (c) 2021
 * 
 */

#ifndef _APP_CODEC_H
#define _APP_CODEC_H

#include <zephyr.h>
#include <drivers/gps.h>

/**
 * @brief GPS payload with included timestamp from date_time
 * 
 */
struct app_codec_gps_payload
{
    struct gps_pvt *p_gps_data;
    uint64_t timestamp;
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
int app_codec_gps_encode(struct app_codec_gps_payload *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size);

/**
 * @brief Encodes boot message
 * 
 * @param time current UTC time in seconds
 * @param p_buf where the encoded data will be stored (destination buffer)
 * @param buf_len size of the destination buffer
 * @param p_size actual written size 
 * @return int 0 or QCBORError
 */
int app_codec_boot_time_encode(uint64_t time, uint8_t *p_buf, size_t buf_len, size_t *p_size);

#endif /*_APP_CODEC_H*/