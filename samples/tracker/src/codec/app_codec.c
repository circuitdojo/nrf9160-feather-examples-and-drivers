#include <app_codec.h>

#include <qcbor/qcbor_encode.h>

int app_codec_gps_encode(struct app_codec_gps_payload *p_payload, uint8_t *p_buf, size_t buf_len, size_t *p_size)
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
    QCBOREncode_AddDoubleToMap(&ec, "lat", p_payload->p_gps_data->latitude);
    QCBOREncode_AddDoubleToMap(&ec, "lng", p_payload->p_gps_data->longitude);

    /* Timestamp */
    if (p_payload->timestamp > 0)
        QCBOREncode_AddUInt64ToMap(&ec, "ts", p_payload->timestamp);

    /* Close map */
    QCBOREncode_CloseMap(&ec);

    /* Finish things up */
    return QCBOREncode_FinishGetSize(&ec, p_size);
}

int app_codec_boot_time_encode(uint64_t time, uint8_t *p_buf, size_t buf_len, size_t *p_size)
{
    // Setup of the goods
    UsefulBuf buf = {
        .ptr = p_buf,
        .len = buf_len};
    QCBOREncodeContext ec;
    QCBOREncode_Init(&ec, buf);

    /* Create over-arching map */
    QCBOREncode_OpenMap(&ec);

    /* Add timetstamp */
    QCBOREncode_AddUInt64ToMap(&ec, "ts", time);

    /* Close map */
    QCBOREncode_CloseMap(&ec);

    /* Finish things up */
    return QCBOREncode_FinishGetSize(&ec, p_size);
}