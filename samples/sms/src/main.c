/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <device.h>
#include <string.h>
#include <zephyr.h>
#include <drivers/gpio.h>
#include <modem/at_cmd.h>

/* Switch */
#define SW0_NODE DT_ALIAS(sw0)
#define SW0_GPIO_LABEL DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios))

/* Callback data */
static struct gpio_callback button_cb_data;

/* Structures for work */
static struct k_delayed_work sms_work;

/**@brief Recoverable BSD library error. */
void bsd_recoverable_error_handler(uint32_t err)
{
	printk("bsdlib recoverable error: %u\n", err);
}

/*TODO: enter your contents of AT+CMGS below*/
const char *at_commands[] = {
	"AT+CNMI=3,2,0,1",
	"AT+CMGS=21\r0001000B912131445411F0000009C834888E2ECBCB21\x1A",
	/* Add more here if needed */
};

/*Callback for AT command*/
void at_cmd_handler(const char *response)
{
	printk("%s", response);
}

/* Work for starting sms commands*/
static void sms_work_fn(struct k_work *work)
{
	for (int i = 0; i < ARRAY_SIZE(at_commands); i++)
	{
		int err;

		printk("%s\r\n", at_commands[i]);

		if ((err = at_cmd_write_with_callback(at_commands[i],
											  at_cmd_handler)) != 0)
		{
			printk("Error sending: %s. Error: %i.", at_commands[i], err);
		}
	}

	/* Run SMS work */
	/* k_delayed_work_submit(&sms_work, K_SECONDS(300)); */
}

/* Button pressed handler */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());

	/* Run SMS work */
	k_delayed_work_submit(&sms_work, K_NO_WAIT);
}

static void button_init()
{
	const struct device *button;
	int ret;

	button = device_get_binding(SW0_GPIO_LABEL);
	if (button == NULL)
	{
		printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
		return;
	}

	ret = gpio_pin_configure(button, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
	if (ret != 0)
	{
		printk("Error %d: failed to configure %s pin %d\n", ret,
			   SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}

	ret = gpio_pin_interrupt_configure(button, SW0_GPIO_PIN,
									   GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0)
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			   ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(button, &button_cb_data);
	printk("Set up button at %s pin %d\n", SW0_GPIO_LABEL, SW0_GPIO_PIN);
}

void main(void)
{
	/* Init the button */
	button_init();

	/* Work init */
	k_delayed_work_init(&sms_work, sms_work_fn);

	/* Boot message */
	printk("SMS example started\n");
}