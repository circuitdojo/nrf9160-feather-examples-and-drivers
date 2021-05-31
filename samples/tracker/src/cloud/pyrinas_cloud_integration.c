#include "cloud/cloud_wrapper.h"
#include <zephyr.h>
#include <pyrinas_cloud/pyrinas_cloud.h>
#include <modem/at_cmd.h>

#define MODULE pyrinas_cloud_integration

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_CLOUD_INTEGRATION_LOG_LEVEL);

static cloud_wrap_evt_handler_t wrapper_evt_handler;

static void cloud_wrapper_notify_event(const struct cloud_wrap_event *evt)
{
    if ((wrapper_evt_handler != NULL) && (evt != NULL))
    {
        wrapper_evt_handler(evt);
    }
    else
    {
        LOG_ERR("Library event handler not registered, or empty event");
    }
}

void pyrinas_cloud_event_handler(const struct pyrinas_cloud_evt *const evt)
{
    struct cloud_wrap_event cloud_wrap_evt = {0};
    bool notify = false;

    switch (evt->type)
    {
    case PYRINAS_CLOUD_EVT_CONNECTING:
        LOG_DBG("PYRINAS_CLOUD_EVT_CONNECTING");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_CONNECTING;
        notify = true;
        break;
    case PYRINAS_CLOUD_EVT_CONNECTED:
        LOG_DBG("PYRINAS_CLOUD_EVT_CONNECTED");
        break;
    case PYRINAS_CLOUD_EVT_READY:
        LOG_DBG("PYRINAS_CLOUD_EVT_READY");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_CONNECTED;
        notify = true;
        break;
    case PYRINAS_CLOUD_EVT_DISCONNECTED:
        LOG_DBG("PYRINAS_CLOUD_EVT_DISCONNECTED");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_DISCONNECTED;
        notify = true;
        break;
    case PYRINAS_CLOUD_EVT_DATA_RECIEVED:
        LOG_DBG("PYRINAS_CLOUD_EVT_DATA_RECIEVED");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_DATA_RECEIVED;
        cloud_wrap_evt.data.buf = evt->data.msg.data;
        cloud_wrap_evt.data.len = evt->data.msg.data_len;
        notify = true;
        break;
    case PYRINAS_CLOUD_EVT_FOTA_START:
        LOG_DBG("PYRINAS_CLOUD_EVT_FOTA_START");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_FOTA_START;
        notify = true;
        break;
    case PYRINAS_CLOUD_EVT_FOTA_DONE:
        LOG_DBG("PYRINAS_CLOUD_EVT_FOTA_DONE");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_FOTA_DONE;
        notify = true;
        break;
    case PYRINAS_CLOUD_EVT_FOTA_ERROR:
        LOG_DBG("PYRINAS_CLOUD_EVT_FOTA_ERROR");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_FOTA_ERROR;
        notify = true;
        break;
    case PYRINAS_CLOUD_EVT_ERROR:
        LOG_DBG("PYRINAS_CLOUD_EVT_ERROR");
        cloud_wrap_evt.type = CLOUD_WRAP_EVT_ERROR;
        cloud_wrap_evt.err = evt->data.err;
        notify = true;
        break;
    default:
        LOG_WRN("Unknown Pyrinas Cloud event type: %d", evt->type);
        break;
    }

    if (notify)
    {
        cloud_wrapper_notify_event(&cloud_wrap_evt);
    }
}

int cloud_wrap_init(cloud_wrap_evt_handler_t event_handler)
{

    /* Configuration */
    struct pyrinas_cloud_config config = {
        .evt_cb = pyrinas_cloud_event_handler,
    };

    pyrinas_cloud_init(&config);

    LOG_INF("********************************************");
    LOG_INF(" The Asset Tracker v2 has started");
    LOG_INF(" Version:     %s",
            CONFIG_ASSET_TRACKER_V2_APP_VERSION);
    LOG_INF(" Cloud:       %s", "Pyrinas Cloud");
    LOG_INF(" Endpoint:    %s",
            CONFIG_PYRINAS_CLOUD_MQTT_BROKER_HOSTNAME);
    LOG_INF("********************************************");

    wrapper_evt_handler = event_handler;

    return 0;
}

int cloud_wrap_connect(void)
{
    int err;

    /* Then connect */
    err = pyrinas_cloud_connect();
    if (err)
    {
        LOG_ERR("pyrinas_cloud_connect, error: %d", err);
        return err;
    }

    return 0;
}

int cloud_wrap_disconnect(void)
{
    int err;

    err = pyrinas_cloud_disconnect();
    if (err)
    {
        LOG_ERR("cloud_wrap_disconnect, error: %d", err);
        return err;
    }

    return 0;
}

int cloud_wrap_state_get(void)
{

    /* TODO */
    return 0;
}

int cloud_wrap_state_send(char *buf, size_t len)
{
    int err;

    err = pyrinas_cloud_publish("state", buf, len);
    if (err)
    {
        LOG_ERR("pyrinas_cloud_publish, error: %d", err);
        return err;
    }

    return 0;
}

int cloud_wrap_data_send(char *buf, size_t len)
{
    int err;

    err = pyrinas_cloud_publish("data", buf, len);
    if (err)
    {
        LOG_ERR("pyrinas_cloud_publish, error: %d", err);
        return err;
    }

    return 0;
}

int cloud_wrap_batch_send(char *buf, size_t len)
{
    int err;

    err = pyrinas_cloud_publish("batch", buf, len);
    if (err)
    {
        LOG_ERR("pyrinas_cloud_publish, error: %d", err);
        return err;
    }

    return 0;
}

int cloud_wrap_ui_send(char *buf, size_t len)
{
    return 0;
}
