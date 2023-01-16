/*
 * Copyright Circuit Dojo (c) 2021
 * 
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#include <zephyr/kernel.h>
#include <power/reboot.h>

#include <pyrinas_cloud/pyrinas_cloud.h>

#include <app_event_manager.h>
#include <app_backend.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(backend_pyrinas);

void pyrinas_cloud_evt_handler(const struct pyrinas_cloud_evt *const p_evt)
{

    switch (p_evt->type)
    {
    case PYRINAS_CLOUD_EVT_ERROR:
    {

        /* Send error and the error code */
        struct app_event app_event = {
            .type = APP_EVENT_BACKEND_ERROR,
            .err = p_evt->data.err,
        };

        app_event_manager_push(&app_event);
    }

    break;
    case PYRINAS_CLOUD_EVT_READY:
    {
        APP_EVENT_MANAGER_PUSH(APP_EVENT_BACKEND_CONNECTED);
    }

    break;
    case PYRINAS_CLOUD_EVT_DISCONNECTED:
    {
        APP_EVENT_MANAGER_PUSH(APP_EVENT_BACKEND_DISCONNECTED);
    }
    break;
    case PYRINAS_CLOUD_EVT_FOTA_DONE:
        sys_reboot(0);
        break;
    default:
        break;
    }
}

/* Public functions*/
int app_backend_publish(char *p_topic, uint8_t *p_data, size_t len)
{
    return pyrinas_cloud_publish(p_topic, p_data, len);
}

int app_backend_connect(void)
{
    return pyrinas_cloud_connect();
}

int app_backend_disconnect(void)
{
    return pyrinas_cloud_disconnect();
}

int app_backend_init(char *client_id, size_t len)
{

    /* Init Pyrinas Cloud */
    struct pyrinas_cloud_config config = {
        .evt_cb = pyrinas_cloud_evt_handler,
        .client_id = {
            .str = client_id,
            .len = len,
        },
    };

    pyrinas_cloud_init(&config);

    return 0;
}

bool app_backend_is_connected(void)
{
    return pyrinas_cloud_is_connected();
}