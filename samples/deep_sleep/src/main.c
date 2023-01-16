/*
 * Copyright (c) 2020 Circuit Dojo LLC
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>

#define SW0_NODE DT_ALIAS(sw0)
#define SW0_GPIO_LABEL DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS (GPIO_INPUT | GPIO_PULL_UP)

#define LED0_NODE DT_ALIAS(led0)
#define LED0_GPIO_LABEL DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_GPIO_PIN DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_GPIO_FLAGS (GPIO_OUTPUT)

#define GPIO0 DT_LABEL(DT_NODELABEL(gpio0))
#define POWER_LATCH_PIN 31

/* Static stuff */
const struct device *button;
const struct device *led;
const struct device *gpio;

/* Worker for checking button after 2 seconds */
static struct k_work_delayable button_press_work;

/* LED helpers, which use the led0 devicetree alias if it's available. */
static const struct device *initialize_led(void);

static struct gpio_callback button_cb_data;

static void button_press_work_fn(struct k_work *item)
{
	bool pressed = false;

	if (gpio_pin_get(button, SW0_GPIO_PIN) == 0)
		pressed = true;

	printk("Still pressed: %s\n", pressed ? "true" : "false");

	if (!pressed)
		return;

	/* Turn off LED */
	gpio_pin_set(led, LED0_GPIO_PIN, 1);

	/* Wait until it's  not pressed */
	while (gpio_pin_get(button, SW0_GPIO_PIN) == 0)
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
	gpio_pin_set(led, LED0_GPIO_PIN, 0);
}

void main(void)
{

	int ret;

	gpio = device_get_binding(GPIO0);
	if (gpio == NULL)
	{
		printk("Error: unable to get gpio device binding.\n");
		return;
	}

	button = device_get_binding(SW0_GPIO_LABEL);
	if (button == NULL)
	{
		printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
		return;
	}

	ret = gpio_pin_configure(button, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
	if (ret != 0)
	{
		printk("Error %d: failed to configure %s pin %d\n",
			   ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}

	ret = gpio_pin_interrupt_configure(button,
									   SW0_GPIO_PIN,
									   GPIO_INT_EDGE_TO_INACTIVE);
	if (ret != 0)
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			   ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(button, &button_cb_data);
	printk("Set up button at %s pin %d\n", SW0_GPIO_LABEL, SW0_GPIO_PIN);

	led = initialize_led();

	/* Init work function */
	k_work_init_delayable(&button_press_work, button_press_work_fn);

	printk("Press the button\n");
	while (1)
	{
		k_cpu_idle();
	}
}

static const struct device *initialize_led(void)
{
	const struct device *led;
	int ret;

	led = device_get_binding(LED0_GPIO_LABEL);
	if (led == NULL)
	{
		printk("Didn't find LED device %s\n", LED0_GPIO_LABEL);
		return NULL;
	}

	ret = gpio_pin_configure(led, LED0_GPIO_PIN, LED0_GPIO_FLAGS);
	if (ret != 0)
	{
		printk("Error %d: failed to configure LED device %s pin %d\n",
			   ret, LED0_GPIO_LABEL, LED0_GPIO_PIN);
		return NULL;
	}

	printk("Set up LED at %s pin %d\n", LED0_GPIO_LABEL, LED0_GPIO_PIN);

	return led;
}