
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/mfd/npm13xx.h>
LOG_MODULE_REGISTER(app_startup);

#define SYSREG_VBUSIN_BASE 0x02U

#define SYSREG_TASKUPDATEILIMSW 0x00U
#define SYSREG_VBUSINILIM0 0x01U
#define SYSREG_VBUSINILIM_1000MA 0x0AU

static int sysreg_setup(void)
{

    /* Get pmic */
    static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm1300_pmic));
    if (!pmic)
    {
        LOG_ERR("Failed to get PMIC device\n");
        return -ENODEV;
    }

    /* Write to MFD to set SYSREG current to 1000mA */
    int ret = mfd_npm13xx_reg_write(pmic, SYSREG_VBUSIN_BASE, SYSREG_VBUSINILIM0, SYSREG_VBUSINILIM_1000MA);
    if (ret < 0)
    {
        LOG_ERR("Failed to set VBUSINLIM. Err: %d", ret);
        return ret;
    }

    /* Save and update */
    ret = mfd_npm13xx_reg_write(pmic, SYSREG_VBUSIN_BASE, SYSREG_TASKUPDATEILIMSW, 0x01);
    if (ret < 0)
    {
        LOG_ERR("Failed to save settings. Err: %d", ret);
        return ret;
    }

    LOG_INF("*** Vsys Current Limit: %d mA ***", SYSREG_VBUSINILIM_1000MA * 100);

    /* Delay boot for programmer */
    k_sleep(K_SECONDS(2));

    return 0;
}

SYS_INIT(sysreg_setup, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
