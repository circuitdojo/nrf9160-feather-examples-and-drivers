/*
 * Copyright Circuit Dojo (c) 2021
 * 
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#include <app_codec.h>

#include <qcbor/qcbor_encode.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_codec);

int app_codec_motion_encode(struct app_motion_data *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size)
{

    // Setup of the goods
    UsefulBuf buf = {
        .ptr = p_buf,
        .len = buf_len};
    QCBOREncodeContext ec;
    QCBOREncode_Init(&ec, buf);

    /* Create over-arching map */
    QCBOREncode_OpenMap(&ec);

    /* Add coordinates */
    QCBOREncode_AddDoubleToMap(&ec, "x", sensor_value_to_double(&p_payload->x));
    QCBOREncode_AddDoubleToMap(&ec, "y", sensor_value_to_double(&p_payload->y));
    QCBOREncode_AddDoubleToMap(&ec, "z", sensor_value_to_double(&p_payload->z));

    /* Timestamp */
    if (p_payload->ts > 0)
        QCBOREncode_AddUInt64ToMap(&ec, "ts", p_payload->ts);

    /* Close map */
    QCBOREncode_CloseMap(&ec);

    /* Finish things up */
    return QCBOREncode_FinishGetSize(&ec, p_size);
}

int app_codec_gps_encode(struct app_gps_data *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size)
{

    // Setup of the goods
    UsefulBuf buf = {
        .ptr = p_buf,
        .len = buf_len};
    QCBOREncodeContext ec;
    QCBOREncode_Init(&ec, buf);

    /* Create over-arching map */
    QCBOREncode_OpenMap(&ec);

    /* Add coordinates */
    QCBOREncode_AddDoubleToMap(&ec, "lat", p_payload->data.latitude);
    QCBOREncode_AddDoubleToMap(&ec, "lng", p_payload->data.longitude);

    /* Add remaining parameters */
    QCBOREncode_AddDoubleToMap(&ec, "acc", p_payload->data.accuracy);
    QCBOREncode_AddDoubleToMap(&ec, "alt", p_payload->data.altitude);
    QCBOREncode_AddDoubleToMap(&ec, "spd", p_payload->data.speed);
    QCBOREncode_AddDoubleToMap(&ec, "hdg", p_payload->data.heading);

    /* Timestamp */
    if (p_payload->ts > 0)
        QCBOREncode_AddUInt64ToMap(&ec, "ts", p_payload->ts);

    /* Close map */
    QCBOREncode_CloseMap(&ec);

    /* Finish things up */
    return QCBOREncode_FinishGetSize(&ec, p_size);
}

int app_codec_device_info_encode(struct app_modem_info *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size)
{

    // Setup of the goods
    UsefulBuf buf = {
        .ptr = p_buf,
        .len = buf_len};
    QCBOREncodeContext ec;
    QCBOREncode_Init(&ec, buf);

    /* Create over-arching map */
    QCBOREncode_OpenMap(&ec);

    /* Battery voltage */
    QCBOREncode_AddUInt64ToMap(&ec, "vbat", p_payload->data.device.battery.value);

    /* Network stuff */
    QCBOREncode_OpenMapInMap(&ec, "nw");
    QCBOREncode_AddUInt64ToMap(&ec, "rsrp", p_payload->rsrp);
    QCBOREncode_AddUInt64ToMap(&ec, "area", p_payload->data.network.area_code.value);
    QCBOREncode_AddUInt64ToMap(&ec, "mnc", p_payload->data.network.mnc.value);
    QCBOREncode_AddUInt64ToMap(&ec, "mcc", p_payload->data.network.mcc.value);
    QCBOREncode_AddUInt64ToMap(&ec, "cell", p_payload->data.network.cellid_hex.value);
    QCBOREncode_AddSZStringToMap(&ec, "ip", p_payload->data.network.ip_address.value_string);
    QCBOREncode_AddUInt64ToMap(&ec, "band", p_payload->data.network.current_band.value);
    QCBOREncode_AddUInt64ToMap(&ec, "m_gps", p_payload->data.network.gps_mode.value);
    QCBOREncode_AddUInt64ToMap(&ec, "m_lte", p_payload->data.network.lte_mode.value);
    QCBOREncode_AddUInt64ToMap(&ec, "m_nb", p_payload->data.network.nbiot_mode.value);
    QCBOREncode_CloseMap(&ec);

    /* SIM Stuff*/
    QCBOREncode_OpenMapInMap(&ec, "sim");
    QCBOREncode_AddSZStringToMap(&ec, "iccid", p_payload->data.sim.iccid.value_string);
    QCBOREncode_CloseMap(&ec);

    // LOG_INF("sim");

    /* Versions/board info */
    QCBOREncode_OpenMapInMap(&ec, "inf");
    QCBOREncode_AddSZStringToMap(&ec, "modv", p_payload->data.device.modem_fw.value_string);
    QCBOREncode_AddSZStringToMap(&ec, "brdv", p_payload->data.device.board);
    QCBOREncode_AddSZStringToMap(&ec, "appv", p_payload->data.device.app_version);
    QCBOREncode_CloseMap(&ec);

    /* Add timetstamp */
    QCBOREncode_AddUInt64ToMap(&ec, "ts", p_payload->ts);

    /* Close map */
    QCBOREncode_CloseMap(&ec);

    /* Finish things up */
    return QCBOREncode_FinishGetSize(&ec, p_size);
}