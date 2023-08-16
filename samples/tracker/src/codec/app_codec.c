/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app_codec.h>

#include <zcbor_decode.h>
#include <zcbor_encode.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_codec);

int app_codec_motion_encode(struct app_motion_data *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size)
{

    // Setup of the goods
    ZCBOR_STATE_E(es, 0, p_buf, buf_len, 0);

    /* Create over-arching map */
    bool ok = zcbor_map_start_encode(es, 4);
    if (!ok)
    {
        LOG_ERR("Did not start CBOR map correctly. Err: %i", zcbor_peek_error(es));
        return -ENOMEM;
    }

    /* Add coordinates */
    zcbor_tstr_put_lit(es, "x");
    zcbor_float64_put(es, sensor_value_to_double(&p_payload->x));

    zcbor_tstr_put_lit(es, "y");
    zcbor_float64_put(es, sensor_value_to_double(&p_payload->y));

    zcbor_tstr_put_lit(es, "z");
    zcbor_float64_put(es, sensor_value_to_double(&p_payload->z));

    /* Timestamp */
    if (p_payload->ts > 0)
    {
        zcbor_tstr_put_lit(es, "ts");
        zcbor_uint64_put(es, p_payload->ts);
    }

    /* Close map */
    ok = zcbor_map_end_encode(es, 4);
    if (!ok)
    {
        LOG_ERR("Did not encode CBOR map correctly. Err: %i", zcbor_peek_error(es));
        return -ENOMEM;
    }

    *p_size = es->payload - p_buf;
    LOG_INF("Size: %i", *p_size);

    /* Finish things up */
    return 0;
}

int app_codec_gps_encode(struct app_gps_data *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size)
{
    // Setup of the goods
    ZCBOR_STATE_E(es, 0, p_buf, buf_len, 0);

    /* Create over-arching map */
    bool ok = zcbor_map_start_encode(es, 4); // Assuming 4 key-value pairs like the previous example, adjust accordingly
    if (!ok)
    {
        LOG_ERR("Did not start CBOR map correctly. Err: %i", zcbor_peek_error(es));
        return -ENOMEM;
    }

    /* Add coordinates */
    zcbor_tstr_put_lit(es, "lat");
    zcbor_float64_put(es, p_payload->data.latitude);

    zcbor_tstr_put_lit(es, "lng");
    zcbor_float64_put(es, p_payload->data.longitude);

    /* Add remaining parameters */
    zcbor_tstr_put_lit(es, "alt");
    zcbor_float64_put(es, p_payload->data.altitude);

    /* Timestamp */
    if (p_payload->ts > 0)
    {
        zcbor_tstr_put_lit(es, "ts");
        zcbor_uint64_put(es, p_payload->ts);
    }

    /* Close map */
    ok = zcbor_map_end_encode(es, 4);
    if (!ok)
    {
        LOG_ERR("Did not encode CBOR map correctly. Err: %i", zcbor_peek_error(es));
        return -ENOMEM;
    }

    *p_size = es->payload - p_buf; // Use a pointer to modify the value
    LOG_INF("Size: %i", *p_size);

    /* Finish things up */
    return 0;
}

int app_codec_device_info_encode(struct app_modem_info *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size)
{
    // Setup of the goods
    ZCBOR_STATE_E(es, 0, p_buf, buf_len, 0);

    /* Create over-arching map */
    bool ok = zcbor_map_start_encode(es, 6); // There are 6 top-level key-value pairs
    if (!ok)
    {
        LOG_ERR("Did not start CBOR map correctly. Err: %i", zcbor_peek_error(es));
        return -ENOMEM;
    }

    /* Battery voltage */
    zcbor_tstr_put_lit(es, "vbat");
    zcbor_uint64_put(es, p_payload->data.device.battery.value);

    /* Network stuff */
    zcbor_tstr_put_lit(es, "nw");
    zcbor_map_start_encode(es, 11); // There are 11 key-value pairs in "nw" map
    zcbor_tstr_put_lit(es, "rsrp");
    zcbor_uint64_put(es, p_payload->rsrp);
    zcbor_tstr_put_lit(es, "area");
    zcbor_uint64_put(es, p_payload->data.network.area_code.value);
    zcbor_tstr_put_lit(es, "mnc");
    zcbor_uint64_put(es, p_payload->data.network.mnc.value);
    zcbor_tstr_put_lit(es, "mcc");
    zcbor_uint64_put(es, p_payload->data.network.mcc.value);
    zcbor_tstr_put_lit(es, "cell");
    zcbor_uint64_put(es, p_payload->data.network.cellid_hex.value);
    zcbor_tstr_put_lit(es, "ip");
    zcbor_tstr_put_term(es, p_payload->data.network.ip_address.value_string);
    zcbor_tstr_put_lit(es, "band");
    zcbor_uint64_put(es, p_payload->data.network.current_band.value);
    zcbor_tstr_put_lit(es, "m_gps");
    zcbor_uint64_put(es, p_payload->data.network.gps_mode.value);
    zcbor_tstr_put_lit(es, "m_lte");
    zcbor_uint64_put(es, p_payload->data.network.lte_mode.value);
    zcbor_tstr_put_lit(es, "m_nb");
    zcbor_uint64_put(es, p_payload->data.network.nbiot_mode.value);
    zcbor_map_end_encode(es, 11);

    /* SIM Stuff*/
    zcbor_tstr_put_lit(es, "sim");
    zcbor_map_start_encode(es, 1); // There is 1 key-value pair in "sim" map
    zcbor_tstr_put_lit(es, "iccid");
    zcbor_tstr_put_term(es, p_payload->data.sim.iccid.value_string);
    zcbor_map_end_encode(es, 1);

    /* Versions/board info */
    zcbor_tstr_put_lit(es, "inf");
    zcbor_map_start_encode(es, 3); // There are 3 key-value pairs in "inf" map
    zcbor_tstr_put_lit(es, "modv");
    zcbor_tstr_put_term(es, p_payload->data.device.modem_fw.value_string);
    zcbor_tstr_put_lit(es, "brdv");
    zcbor_tstr_put_term(es, p_payload->data.device.board);
    zcbor_tstr_put_lit(es, "appv");
    zcbor_tstr_put_term(es, p_payload->data.device.app_version);
    zcbor_map_end_encode(es, 3);

    /* Add timestamp */
    zcbor_tstr_put_lit(es, "ts");
    zcbor_uint64_put(es, p_payload->ts);

    /* Close map */
    ok = zcbor_map_end_encode(es, 6);
    if (!ok)
    {
        LOG_ERR("Did not encode CBOR map correctly. Err: %i", zcbor_peek_error(es));
        return -ENOMEM;
    }

    *p_size = es->payload - p_buf;
    LOG_INF("Size: %i", *p_size);

    /* Finish things up */
    return 0;
}
