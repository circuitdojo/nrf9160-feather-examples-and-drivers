/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/http/client.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(cloud);

#include <modem/modem_key_mgmt.h>
#include <cJSON.h>

#include "cloud.h"

/* CA Certificate */
static const char cert[] = {
#include "isrg-root-x1.pem"
};

/* Variables */
const int32_t timeout = 3 * MSEC_PER_SEC;

/* Callback */
static void (*cloud_callback)(struct device_data *data);

/* Setup TLS options on a given socket */
int tls_setup(int fd)
{
    int err;
    int verify = TLS_PEER_VERIFY_REQUIRED;
    int session_cache = TLS_SESSION_CACHE_ENABLED;

    /* Security tag that we have provisioned the certificate with */
    const sec_tag_t tls_sec_tag[] = {
        CONFIG_CLOUD_TLS_SEC_TAG,
    };

    /* Cipher suite */
    // Cipher Suite: TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 (0xc02b)
    nrf_sec_cipher_t cipher_list[] = {0xc02b};

    /* Set options */
    err = setsockopt(fd, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));
    if (err)
    {
        err = -errno;
        LOG_ERR("Failed to setup peer verification, err %d", errno);
        return err;
    }

    /* Associate the socket with the security tag
     * we have provisioned the certificate with.
     */
    err = setsockopt(fd, SOL_TLS, TLS_SEC_TAG_LIST, tls_sec_tag, sizeof(tls_sec_tag));
    if (err)
    {
        err = -errno;
        LOG_ERR("Failed to setup TLS sec tag, err %d", errno);
        return err;
    }

    err = setsockopt(fd, SOL_TLS, TLS_CIPHERSUITE_LIST, cipher_list, sizeof(cipher_list));
    if (err)
    {
        err = -errno;
        LOG_ERR("Failed to setup TLS cipher, err %d", errno);
        return err;
    }

    err = setsockopt(fd, SOL_TLS, TLS_HOSTNAME, CONFIG_CLOUD_HOSTNAME, sizeof(CONFIG_CLOUD_HOSTNAME) - 1);
    if (err)
    {
        err = -errno;
        LOG_ERR("Failed to setup TLS hostname, err %d", errno);
        return err;
    }

    err = setsockopt(fd, SOL_TLS, TLS_SESSION_CACHE, &session_cache,
                     sizeof(session_cache));
    if (err)
    {
        err = -errno;
        LOG_ERR("Failed to setup session cache, err %d", errno);
        return err;
    }

    return 0;
}

static int socket_setup()
{
    int fd;
    int err = 0;
    struct addrinfo *res;

    /* Hints */
    struct addrinfo hints =
        {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        };

    LOG_INF("Looking up %s", CONFIG_CLOUD_HOSTNAME);
    err = getaddrinfo(CONFIG_CLOUD_HOSTNAME, NULL, &hints, &res);
    if (err)
    {
        LOG_ERR("getaddrinfo() failed. Err: %i", errno);
        return err;
    }

    ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(CONFIG_CLOUD_PORT);

    /* Create it */
    fd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TLS_1_2);
    if (fd == -1)
    {
        LOG_ERR("Failed to open socket!");
        err = -ECONNABORTED;
        goto clean_up;
    }

    struct timeval recv_timeout = {
        .tv_sec = 5};

    err = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout));
    if (err != 0)
    {
        err = -errno;
        LOG_ERR("Set receive timeout failed, error: %d, errno: %d", err, errno);
        goto clean_up;
    }

    struct timeval send_timeout = {
        .tv_sec = 5};

    err = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout));
    if (err)
    {
        err = -errno;
        LOG_ERR("Set transmit timeout failed, error: %d, errno: %d", err, errno);
        goto clean_up;
    }

    /* Setup TLS socket options */
    err = tls_setup(fd);
    if (err < 0)
    {
        LOG_ERR("Unable to setup TLS. Err: %i", err);
        goto clean_up;
    }

    /* Connect */
    err = connect(fd, res->ai_addr, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        err = -errno;
        LOG_ERR("Unable to connect. Err: %i - %s", err, strerror(errno));
    }
    else
        LOG_INF("Connected!");

clean_up:
    freeaddrinfo(res);
    if (err < 0 && fd > -1)
    {
        (void)close(fd);
        fd = -1;
    }

    /* Return error or socket */
    if (err < 0)
        return err;
    else
        return fd;
}

static void response_cb(struct http_response *rsp,
                        enum http_final_call final_data,
                        void *user_data)
{

    LOG_INF("HTTP Status %d", rsp->http_status_code);

    /* Check status */
    if (rsp->http_status_code != 200 && rsp->http_status_code != 201)
    {
        return;
    }

    if (final_data == HTTP_DATA_FINAL)
    {
        LOG_HEXDUMP_INF(rsp->recv_buf, rsp->recv_buf_len, "Response data");

        if (!rsp->body_found)
        {
            LOG_ERR("Body not found");
            return;
        }

        /* TODO: Decode and do something! */
        /* Start of body is rsp->body_frag_start */
        struct device_data data = {
            .do_something = false,
        };

        /* Callback */
        if (cloud_callback != NULL)
            cloud_callback(&data);
    }
}

int cloud_publish(struct device_data *data)
{
    int fd = -1;
    int ret = 0;
    uint8_t recv_buf_ipv4[256] = {0};

    LOG_INF("Publish path: %s%s", CONFIG_CLOUD_HOSTNAME, CONFIG_CLOUD_PUBLISH_PATH);

    /* Create JSON */
    cJSON *obj = cJSON_CreateObject();

    cJSON_AddBoolToObject(obj, "do_something", data->do_something);

    char *msg = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);

    if (msg == NULL)
    {
        LOG_ERR("Unable to encode JSON");
        return -ENOMEM;
    }
    else
    {
        LOG_INF("Payload: %s", msg);
    }

    /* Setup socket */
    fd = socket_setup();
    if (fd < 0)
    {
        k_free(msg);

        LOG_ERR("Unable to setup socket. Err: %i", ret);
        return ret;
    }

    LOG_INF("Socket setup complete");

    /* POST */
    struct http_request req;

    memset(&req, 0, sizeof(req));

    /* Don't keep connection open.. */
    char *const headers[] = {
        "Connection: close\r\n",
        NULL};

    req.method = HTTP_POST;
    req.url = CONFIG_CLOUD_PUBLISH_PATH;
    req.host = CONFIG_CLOUD_HOSTNAME;
    req.protocol = "HTTP/1.1";
    req.payload = msg;
    req.payload_len = strlen(msg);
    req.response = response_cb;
    req.recv_buf = recv_buf_ipv4;
    req.recv_buf_len = sizeof(recv_buf_ipv4);
    req.content_type_value = "application/json";
    req.header_fields = (const char **)headers;

    ret = http_client_req(fd, &req, timeout, NULL);
    if (ret < 0)
        LOG_ERR("Unable to send data to cloud. Err: %i", ret);

    /* Close connection */
    (void)close(fd);

    /* Free data */
    cJSON_free(msg);

    LOG_INF("Data sent to cloud");

    return 0;
}

/* Provision certificate to modem */
int cert_provision(void)
{
    int err;
    bool exists;
    int mismatch;

    /* Check to see if it exists first .. */
    err = modem_key_mgmt_exists(CONFIG_CLOUD_TLS_SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, &exists);
    if (err)
    {
        LOG_INF("Failed to check for certificates err %d", err);
        return err;
    }

    /* Compare if it does */
    if (exists)
    {
        mismatch = modem_key_mgmt_cmp(CONFIG_CLOUD_TLS_SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, cert,
                                      strlen(cert));
        if (!mismatch)
        {
            LOG_INF("Certificate match");
            return 0;
        }

        LOG_INF("Certificate mismatch");
        err = modem_key_mgmt_delete(CONFIG_CLOUD_TLS_SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN);
        if (err)
        {
            LOG_INF("Failed to delete existing certificate, err %d", err);
        }
    }

    LOG_INF("Provisioning certificate");

    /*  Provision certificate to the modem */
    err = modem_key_mgmt_write(CONFIG_CLOUD_TLS_SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, cert,
                               sizeof(cert) - 1);
    if (err)
    {
        LOG_INF("Failed to provision certificate, err %d", err);
        return err;
    }

    return 0;
}

int cloud_init(void (*callback)(struct device_data *data))
{
    int err;

    if (callback == NULL)
        return -EINVAL;

    cloud_callback = callback;

    /* Provision certificates before connecting to the LTE network */
    err = cert_provision();
    if (err)
        LOG_ERR("Unable to provision certificate! Err: %i", err);

    return 0;
}
