
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define POWER_MODE_PIN 13

const static struct device *gpio;

static int gps_sample_setup(const struct device *arg)
{

	/* Gpio pin */
	gpio = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	__ASSERT(gpio, "Failed to get the gpio0 device");

    /* Set low */
    gpio_pin_configure(gpio, POWER_MODE_PIN, GPIO_OUTPUT_LOW);

    return 0;
}

SYS_INIT(gps_sample_setup, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);