/*
 * Copyright 2023 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/init.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(modem);

/* nRF Libraries */
#include <modem/lte_lc.h>

/* Event manager */
#include <app_event_manager.h>

atomic_t is_tau = ATOMIC_INIT(0);

static void lte_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
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
    case LTE_LC_EVT_PSM_UPDATE:
        LOG_INF("PSM parameter update: TAU: %d, Active time: %d",
                evt->psm_cfg.tau, evt->psm_cfg.active_time);
        break;
    case LTE_LC_EVT_TAU_PRE_WARNING:
        LOG_INF("TAU Pre-Warning");

        /* Set current tau*/
        atomic_set(&is_tau, 1);
        break;
    case LTE_LC_EVT_NW_REG_STATUS:
        if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
            (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING))
        {
            LOG_INF("Disconnected");

            APP_EVENT_MANAGER_PUSH(APP_EVENT_CELLULAR_DISCONNECT);
            break;
        }

        LOG_INF("Network registration status: %s",
                evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home network" : "Connected - roaming");

        APP_EVENT_MANAGER_PUSH(APP_EVENT_CELLULAR_CONNECTED);

        break;
    case LTE_LC_EVT_CELL_UPDATE:
        LOG_INF("LTE cell changed: Cell ID: %d, Tracking area: %d",
                evt->cell.id, evt->cell.tac);

        LOG_DBG("RSRP: %i", evt->cell.rsrp);
        break;
    case LTE_LC_EVT_MODEM_SLEEP_ENTER:
        LOG_INF("Sleep enter");

        break;
    case LTE_LC_EVT_MODEM_SLEEP_EXIT:
        LOG_INF("Sleep exit");
        break;
    default:
        break;
    }
}

static int modem_monitor_init(void)
{
    lte_lc_register_handler(lte_handler);

    return 0;
}

SYS_INIT(modem_monitor_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);