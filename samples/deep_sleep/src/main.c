/*
 * Copyright (c) 2020 Circuit Dojo LLC
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>

#define POWER_LATCH_PIN 31

/* Gpios */
struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* Static stuff */
const struct device *gpio = DEVICE_DT_GET(DT_NODELABEL(gpio0));

/* Worker for checking button after 2 seconds */
static struct k_work_delayable button_press_work;

static struct gpio_callback button_cb_data;

static void button_press_work_fn(struct k_work *item)
{
	bool pressed = false;

	if (gpio_pin_get_dt(&sw0) == 0)
		pressed = true;

	printk("Still pressed: %s\n", pressed ? "true" : "false");

	if (!pressed)
		return;

	/* Turn off LED */
	gpio_pin_set_dt(&led0, 0);

	/* Wait until it's  not pressed */
	while (gpio_pin_get_dt(&sw0) == 0)
		;

	/*Shut er down.*/
	if (pressed)
	{

		/* Turn off latch pin */
		gpio_pin_set_raw(gpio, POWER_LATCH_PIN, 0);

		/* Wait infinitely */
		for (;;)
			;
	}
}

void button_pressed(const struct device *dev, struct gpio_callback *cb,
					uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());

	/* Schedule work for 2 seconds to see if the button is still pressed */
	k_work_schedule(&button_press_work, K_SECONDS(2));

	/*Set LED on*/
	gpio_pin_set_dt(&led0, 1);
}

int main(void)
{

	int ret;

	if (!device_is_ready(gpio))
	{
		printk("Error: unable to get gpio device binding.\n");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&sw0, GPIO_INPUT);
	if (ret != 0)
	{
		printk("Error %d: failed to configure sw0 on pin %d\n",
			   ret, sw0.pin);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&sw0,
										  GPIO_INT_EDGE_TO_INACTIVE);
	if (ret != 0)
	{
		printk("Error %d: failed to configure interrupt on sw0 pin %d\n",
			   ret, sw0.pin);
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(sw0.pin));
	gpio_add_callback(sw0.port, &button_cb_data);
	printk("Set up button at pin %d\n", sw0.pin);

	/* Initialize LED */
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT);
	if (ret != 0)
	{
		printk("Error %d: failed to configure LED on pin %d\n",
			   ret, led0.pin);
		return ret;
	}

	/* Init work function */
	k_work_init_delayable(&button_press_work, button_press_work_fn);

	printk("Press the button\n");
	while (1)
	{
		k_cpu_idle();
	}

	return 0;
}