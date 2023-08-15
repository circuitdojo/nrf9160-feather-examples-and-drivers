/*
 * Copyright (c) 2020 Circuit Dojo LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/device.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <nrf_modem_at.h>

/* Pins */
struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

/* Callback data */
static struct gpio_callback button_cb_data;

/* Structures for work */
static struct k_work_delayable sms_work;

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

		if ((err = nrf_modem_at_cmd_async(at_cmd_handler, at_commands[i])) != 0)
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
	k_work_schedule(&sms_work, K_NO_WAIT);
}

static void button_init()
{
	int ret;

	ret = gpio_pin_configure_dt(&sw0, GPIO_INPUT);
	if (ret != 0)
	{
		printk("Error %d: failed to configure sw0 on pin %d\n", ret,
			   sw0.pin);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&sw0,
										  GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0)
	{
		printk("Error %d: failed to configure interrupt on sw0 pin %d\n",
			   ret, sw0.pin);
		return;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(sw0.pin));
	gpio_add_callback(sw0.port, &button_cb_data);
	printk("Set up button at sw0 pin %d\n", sw0.pin);
}

int main(void)
{
	/* Init the button */
	button_init();

	/* Work init */
	k_work_init_delayable(&sms_work, sms_work_fn);

	/* Boot message */
	printk("SMS example started\n");

	return 0;
}