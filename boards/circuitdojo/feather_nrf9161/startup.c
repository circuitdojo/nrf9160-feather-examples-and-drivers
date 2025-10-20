
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/mfd/npm13xx.h>
LOG_MODULE_REGISTER(app_startup);

#define SYSREG_VBUSIN_BASE 0x02

#define SYSREG_TASKUPDATEILIMSW 0x00
#define SYSREG_VBUSINILIM0 0x01
#define SYSREG_VBUSINILIM_1000MA 0x0a

static int sysreg_setup(void)
{

    /* Get pmic */
    static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm1300_pmic));
    if (!pmic)
    {
        LOG_ERR("Failed to get PMIC device\n");
        return -ENODEV;
    }

    uint8_t data = 0;
    int ret = mfd_npm13xx_reg_read_burst(pmic, SYSREG_VBUSIN_BASE, SYSREG_VBUSINILIM0, &data, 1);
    if (ret < 0)
    {
        printk("Failed to read VBUSINLIM. Err: %d", ret);
        return ret;
    }
    else
    {
        if (data == 0)
        {
            data = 5;

            printk("*** Vsys Current Limit: %d mA ***\n", data * 100);

            return 0;
        }
    }

    /* Write to MFD to set SYSREG current to 500mA */
    ret = mfd_npm13xx_reg_write(pmic, SYSREG_VBUSIN_BASE, SYSREG_VBUSINILIM0, 0);
    if (ret < 0)
    {
        printk("Failed to set VBUSINLIM. Err: %d\n", ret);
        return ret;
    }

    /* Save and update */
    ret = mfd_npm13xx_reg_write(pmic, SYSREG_VBUSIN_BASE, SYSREG_TASKUPDATEILIMSW, 0x01);
    if (ret < 0)
    {
        printk("Failed to save settings. Err: %d\n", ret);
        return ret;
    }

    /* Delay boot for programmer */
    k_sleep(K_SECONDS(2));

    return 0;
}

SYS_INIT(sysreg_setup, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
