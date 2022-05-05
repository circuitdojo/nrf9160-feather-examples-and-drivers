/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/i2c.h>

#define LIS2DH_ADDR 0x18
#define LIS2DH_REG_WAI 0x0f

#define LIS2DH_REG_CTRL0 0x1e
#define LIS2DH_REG_CTRL1 0x20

#define LIS2DH_ACCEL_X_EN_BIT BIT(0)

#define LIS2Dh_REG_CTRL0_DEF 0b00010000

void main(void)
{
    uint8_t who_am_i = 0;
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));

    if (i2c_dev == NULL || !device_is_ready(i2c_dev))
    {
        printf("Could not get I2C device\n");
        return;
    }

    /* Read the Who Am I register */
    int ret = i2c_reg_read_byte(i2c_dev, LIS2DH_ADDR,
                                LIS2DH_REG_WAI, &who_am_i);
    if (ret)
    {
        printf("Unable get WAI data. (err %i)\n", ret);
        return;
    }

    printf("Who am I: 0x%x\n", who_am_i);

    /* Write to CTRL0 */
    ret = i2c_reg_write_byte(i2c_dev, LIS2DH_ADDR,
                             LIS2DH_REG_CTRL0, LIS2Dh_REG_CTRL0_DEF);
    if (ret)
    {
        printf("Unable write to CTRL0. (err %i)\n", ret);
        return;
    }

    printf("CTRL10 set to default.\n");

    /* Update a bit in CTRL1 */
    /* (This will read, update and write back)*/
    ret = i2c_reg_update_byte(i2c_dev, LIS2DH_ADDR,
                              LIS2DH_REG_CTRL1, LIS2DH_ACCEL_X_EN_BIT, LIS2DH_ACCEL_X_EN_BIT);
    if (ret)
    {
        printf("Unable write to CTRL1. (err %i)\n", ret);
        return;
    }

    printf("CTRL1 LIS2DH_ACCEL_X_EN_BIT set.\n");

    /* Burst Read */
    uint8_t ctrl0_ctrl1[2] = {0};
    ret = i2c_burst_read(i2c_dev, LIS2DH_ADDR, LIS2DH_REG_CTRL0, ctrl0_ctrl1, sizeof(ctrl0_ctrl1));
    if (ret)
    {
        printf("Unable to burst read CTRL0 + CTRL1. (err %i)\n", ret);
        return;
    }

    printf("CTRL0: 0x%x CTRL1: 0x%x\n", ctrl0_ctrl1[0], ctrl0_ctrl1[1]);

    /* Burst Write */
    ret = i2c_burst_write(i2c_dev, LIS2DH_ADDR, LIS2DH_REG_CTRL0, ctrl0_ctrl1, sizeof(ctrl0_ctrl1));
    if (ret)
    {
        printf("Unable to burst write CTRL0 + CTRL1. (err %i)\n", ret);
        return;
    }

    printf("CTRL0 + CTRL1 burst write complete!\n");
}
