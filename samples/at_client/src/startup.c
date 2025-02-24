
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(startup);

#include <modem/nrf_modem_lib.h>
#include <nrf_modem_at.h>

/* Initialization of AUX pin */
#if defined(CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9151)
#define AUXANTCFG_ENABLE "AT\%XANTCFG=1"

NRF_MODEM_LIB_ON_INIT(aux_init_hook, on_modem_lib_init, NULL);

static void on_modem_lib_init(int ret, void *ctx)
{
    ARG_UNUSED(ctx);

    if (ret != 0)
    {
        return;
    }

    printk("*** Setting configuration: %s ***\n", AUXANTCFG_ENABLE);
    int err = nrf_modem_at_printf("%s", AUXANTCFG_ENABLE);
    if (err)
    {
        LOG_ERR("Failed to set configuration (err: %d)", err);
    }
}
#endif