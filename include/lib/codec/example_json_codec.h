#ifndef EXAMPLE_JSON_CODEC_H
#define EXAMPLE_JSON_CODEC_H

/* Zephyr requirements */
#include <zephyr/kernel.h>
#include <zephyr/data/json.h>

/**
 * @brief Nested struct
 *
 */
struct sensor2_value
{
    int32_t x_value;
    int32_t y_value;
    int32_t z_value;
};

/**
 * @brief Main JSON payload
 *
 */
struct example_json_payload
{
    uint32_t timestamp;
    int32_t sensor1_value;
    struct sensor2_value sensor2_value;
};

/**
 * @brief Example JSON payload encoding
 *
 * @param payload pointer to example payload
 * @param buf pointer to buffer to write encoded JSON
 * @param buf_len length of provided buffer
 * @return int number bytes written to buffer OR error code
 */
int example_json_codec_encode(struct example_json_payload *payload, char *buf, size_t buf_len);

#endif