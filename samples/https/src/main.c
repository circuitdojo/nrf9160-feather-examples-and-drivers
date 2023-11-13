/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

/* nRF Libraries */
#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>
#include <modem/modem_info.h>

/* Local */
#include "cloud/cloud.h"

/* Timer */
static void timeout_handler(struct k_timer *timer_id);
K_TIMER_DEFINE(timer, timeout_handler, NULL);

/* Thread control */
K_SEM_DEFINE(thread_sem, 0, 1);

/* Variables */
static struct device_data data = {
    .do_something = true,
};

void cloud_cb(struct device_data *p_data)
{
    LOG_INF("Cloud callback");

    /* TODO: handle data here */
}

static void timeout_handler(struct k_timer *timer_id)
{
    LOG_INF("Timeout");
    k_sem_give(&thread_sem);
}

int main(void)
{
    int err;

    LOG_INF("HTTPS Sample. Board: %s", CONFIG_BOARD);

    /* Init modem lib */
    err = nrf_modem_lib_init();
    if (err < 0)
    {
        LOG_ERR("Failed to init modem lib. (err: %i)", err);
        return err;
    }

    /* Cloud init */
    err = cloud_init(cloud_cb);
    if (err < 0)
    {
        LOG_ERR("Unable to set callback. Err: %i", err);
        return err;
    }

    /* Init lte_lc*/
    err = lte_lc_init();
    if (err < 0)
    {
        LOG_ERR("Failed to init. Err: %i", err);
        return err;
    }

    /* Power saving is turned on */
    lte_lc_psm_req(true);

    /* Connect */
    err = lte_lc_connect();
    if (err < 0)
    {
        LOG_ERR("Failed to connect. Err: %i", err);
        return err;
    }

    /* Start timer */
    k_timer_start(&timer, K_MINUTES(CONFIG_DEFAULT_DELAY), K_MINUTES(CONFIG_DEFAULT_DELAY));

    /* Allow for instant publish */
    k_sem_give(&thread_sem);

    while (1)
    {
        k_sem_take(&thread_sem, K_FOREVER);

        /* Publish and sleep .. */
        err = cloud_publish(&data);
        if (err < 0)
            LOG_ERR("Unable to publish. Err: %i", err);
    }
}
