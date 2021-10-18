/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file main.c
 * @author Jared Wolff (hello@jaredwolff.com)
 * @date 2021-07-20
 * 
 * @copyright Copyright Circuit Dojo (c) 2021
 * 
 */

#include <zephyr.h>
#include <stdio.h>

#if defined(CONFIG_NRF_MODEM_LIB)
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <modem/at_cmd.h>
#include <modem/at_notif.h>
#include <modem/modem_info.h>
#include <nrf_modem.h>
#include <date_time.h>
#endif

#include <power/reboot.h>

#include <net/coap.h>
#include <net/golioth/system_client.h>

#include <app_gps.h>
#include <app_codec.h>
#include <app_event_manager.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_main);

/* Used to prevent the app from running before connecting */
K_SEM_DEFINE(lte_connected, 0, 1);

static struct golioth_client *client = GOLIOTH_SYSTEM_CLIENT_GET();

#if defined(CONFIG_NRF_MODEM_LIB)
static void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
    case LTE_LC_EVT_NW_REG_STATUS:
        if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
            (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING))
        {
            /* Send disconnected event */
            struct app_event app_event = {
                .type = APP_EVENT_CELLULAR_DISCONNECT,
            };
            app_event_manager_push(&app_event);

            break;
        }

        /* Otherwise send connected event */
        struct app_event app_event = {
            .type = APP_EVENT_CELLULAR_CONNECTED,
        };
        app_event_manager_push(&app_event);

        LOG_INF("Network registration status: %s",
                evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home network" : "Connected - roaming");

        k_sem_give(&lte_connected);
        break;
    case LTE_LC_EVT_PSM_UPDATE:
        LOG_INF("PSM parameter update: TAU: %d, Active time: %d",
                evt->psm_cfg.tau, evt->psm_cfg.active_time);
        break;
    case LTE_LC_EVT_EDRX_UPDATE:
    {
        char log_buf[60];
        ssize_t len;

        len = snprintf(log_buf, sizeof(log_buf),
                       "eDRX parameter update: eDRX: %f, PTW: %f",
                       evt->edrx_cfg.edrx, evt->edrx_cfg.ptw);
        if (len > 0)
        {
            LOG_INF("%s", log_buf);
        }
        break;
    }
    case LTE_LC_EVT_RRC_UPDATE:
        LOG_INF("RRC mode: %s",
                evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ? "Connected" : "Idle");
        break;
    case LTE_LC_EVT_CELL_UPDATE:
        LOG_INF("LTE cell changed: Cell ID: %d, Tracking area: %d",
                evt->cell.id, evt->cell.tac);
        break;
    default:
        break;
    }
}

static void modem_configure(void)
{
    int err;

    if (IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT))
    {
        /* Do nothing, modem is already configured and LTE connected. */
    }
    else
    {
        err = lte_lc_init_and_connect_async(lte_handler);
        if (err)
        {
            LOG_ERR("Modem could not be configured, error: %d",
                    err);
            return;
        }
    }
}
#endif

#if defined(CONFIG_NRF_MODEM_LIB)
static void nrf_modem_lib_dfu_handler(void)
{
    int err;

    err = nrf_modem_lib_init(NORMAL_MODE);

    switch (err)
    {
    case MODEM_DFU_RESULT_OK:
        LOG_INF("Modem update suceeded, reboot");
        sys_reboot(SYS_REBOOT_COLD);
        break;
    case MODEM_DFU_RESULT_UUID_ERROR:
    case MODEM_DFU_RESULT_AUTH_ERROR:
        LOG_ERR("Modem update failed, error: %d", err);
        LOG_ERR("Modem will use old firmware");
        sys_reboot(SYS_REBOOT_COLD);
        break;
    case MODEM_DFU_RESULT_HARDWARE_ERROR:
    case MODEM_DFU_RESULT_INTERNAL_ERROR:
        LOG_ERR("Modem update malfunction, error: %d, reboot", err);
        sys_reboot(SYS_REBOOT_COLD);
        break;
    default:
        break;
    }
}
#endif

static void golioth_on_message(struct golioth_client *client,
                               struct coap_packet *rx)
{
    uint16_t payload_len;
    const uint8_t *payload;
    uint8_t type;

    type = coap_header_get_type(rx);
    payload = coap_packet_get_payload(rx, &payload_len);

    if (!IS_ENABLED(CONFIG_LOG_BACKEND_GOLIOTH) && payload)
    {
        LOG_HEXDUMP_DBG(payload, payload_len, "Payload");
    }
}

void golioth_on_connect(struct golioth_client *client)
{

    struct app_event app_event = {
        .type = APP_EVENT_GOLIOTH_CONNECTED,
    };

    app_event_manager_push(&app_event);
}

int main(void)
{
    int err;

    err = app_gps_setup();
    if (err)
    {
        LOG_ERR("Unable to setup GPS. Err: %i", err);
    }

#if defined(CONFIG_NRF_MODEM_LIB)
    nrf_modem_lib_dfu_handler();

    modem_configure();

    err = modem_info_init();
    if (err)
    {
        LOG_ERR("Failed initializing modem info module, error: %d",
                err);
    }

    k_sem_take(&lte_connected, K_FOREVER);
#endif

    /*Setup and connect to Golioth*/
    client->on_message = golioth_on_message;
    client->on_connect = golioth_on_connect;
    golioth_system_client_start();

    while (true)
    {

        int err;
        struct app_event evt;

        err = app_event_manager_get(&evt);
        if (err)
        {
            LOG_WRN("Unable to get event. Err: %i", err);
            continue;
        }

        LOG_INF("Evt: %s", app_event_type_to_string(evt.type));

        switch (evt.type)
        {
        case APP_EVENT_CELLULAR_DISCONNECT:
            break;
        case APP_EVENT_CELLULAR_CONNECTED:
            /* Force time update */
            err = date_time_update_async(NULL);
            if (err)
            {
                LOG_ERR("Unable to update time with date_time_update_async. Err: %i", err);
            }

            /* Enable PSM mode */
#if defined(CONFIG_NRF_MODEM_LIB)
            err = lte_lc_psm_req(true);
            if (err)
            {
                LOG_ERR("Requesting PSM failed, error: %d", err);
            }
#endif

            /* Start GPS operations */
            err = app_gps_start();
            if (err)
            {
                LOG_ERR("Unable to start GPS. Err: %i", err);
            }

            break;
        case APP_EVENT_GOLIOTH_CONNECTED:
        {

            uint8_t buf[256];
            size_t size = 0;
            uint64_t ts = 0;

            err = date_time_now(&ts);
            if (err)
            {
                LOG_ERR("Unable to get current date/time. Err: %i", err);
            }

            /* Encode */
            err = app_codec_boot_time_encode(ts, buf, sizeof(buf), &size);
            if (err)
            {
                LOG_ERR("Unable to encode boot time. Err: %i", err);
                break;
            }

            err = golioth_lightdb_set(client,
                                      GOLIOTH_LIGHTDB_PATH("boot"),
                                      COAP_CONTENT_FORMAT_APP_CBOR,
                                      buf, size);
            if (err)
            {
                LOG_WRN("Failed to gps data: %d", err);
            }

            break;
        }
        case APP_EVENT_AGPS_REQUEST:

            /* Start AGPS request */
            err = app_gps_agps_request(evt.agps_request);
            if (err)
            {
                LOG_ERR("Unable to download AGPS data. Err: %i", err);
            }

            /* Free data allocated on heap */
            k_free(evt.agps_request);
            break;
        case APP_EVENT_GPS_DATA:
        {
            uint8_t buf[256];
            size_t size = 0;
            struct app_codec_gps_payload payload;
            payload.timestamp = 0;

            /* Get the current time */
            err = date_time_now(&payload.timestamp);
            if (err)
            {
                LOG_WRN("Unable to get timestamp!");
            }

            /* Set the data */
            payload.p_gps_data = evt.gps_data;

            /* Encode CBOR data */
            err = app_codec_gps_encode(&payload, buf, sizeof(buf), &size);
            if (err < 0)
            {
                LOG_ERR("Unable to encode data. Err: %i", err);
                goto app_event_gps_data_end;
            }

            LOG_INF("Data size: %i", size);

            /* Publish gps data */
            err = golioth_lightdb_set(client,
                                      GOLIOTH_LIGHTDB_PATH("gps"),
                                      COAP_CONTENT_FORMAT_APP_CBOR,
                                      buf, size);
            if (err)
            {
                LOG_WRN("Failed to gps data: %d", err);
            }

        app_event_gps_data_end:

            /* Free the data generated before */
            k_free(evt.gps_data);

            break;
        }
        }
    }
}
