#include <lib/codec/example_json_codec.h>

static const struct json_obj_descr sensor2_value_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct sensor2_value, x_value, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct sensor2_value, y_value, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct sensor2_value, z_value, JSON_TOK_NUMBER),
};

static const struct json_obj_descr example_json_payload_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct example_json_payload, timestamp, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct example_json_payload, sensor1_value, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_OBJECT(struct example_json_payload, sensor2_value,
                          sensor2_value_descr),
};

int example_json_codec_encode(struct example_json_payload *payload, char *buf, size_t buf_len)
{

    /* Calculate the encoded length */
    ssize_t len = json_calc_encoded_len(example_json_payload_descr, ARRAY_SIZE(example_json_payload_descr), &payload);

    /* Return error if buffer isn't correctly sized */
    if (buf_len < len)
    {
        return -ENOMEM;
    }

    /* Encode */
    int ret = json_obj_encode_buf(example_json_payload_descr, ARRAY_SIZE(example_json_payload_descr), payload, buf, buf_len);

    /* Return the length if possible*/
    if (ret == 0)
        return len;
    else
        return ret;
}