
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define POWER_MODE_PIN 13

const struct device *gpio = DEVICE_DT_GET(DT_NODELABEL(gpio0));

static int gps_sample_setup(const struct device *arg)
{

    /* Gpio pin */
    if (!device_is_ready(gpio))
        __ASSERT(gpio, "Failed to get the gpio0 device");

    /* Set low */
    gpio_pin_configure(gpio, POWER_MODE_PIN, GPIO_OUTPUT_LOW);

    return 0;
}

SYS_INIT(gps_sample_setup, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);