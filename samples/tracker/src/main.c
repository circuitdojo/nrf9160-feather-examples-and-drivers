/*
 * Copyright Circuit Dojo (c) 2023
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/device.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

/* nRF Libraries */
#include <modem/lte_lc.h>
#include <modem/modem_info.h>
#include <modem/nrf_modem_lib.h>

/* Local */
#include <app_backend.h>
#include <app_gps.h>

int main(void)
{
	int err;

	LOG_INF("Starting tracker on %s!", CONFIG_BOARD);

	/* Init modem library */
	err = nrf_modem_lib_init();
	if (err < 0)
		__ASSERT_MSG_INFO("Unable to initialize modem lib. (err: %i)", err);

#if IS_ENABLED(CONFIG_DISABLE_CONSOLE_ON_BOOT)

	/* MD button setup  */
	const struct gpio_dt_spec button =
		GPIO_DT_SPEC_GET_BY_IDX(DT_ALIAS(sw0), gpios, 0);

	/* Init button */
	gpio_pin_configure_dt(&button, GPIO_INPUT);

	/* Check button */
	k_sleep(K_SECONDS(2));

	/* Check if button is being pressed to remain active */
	if (!gpio_pin_get_dt(&button))
	{
		static const struct device *const console_dev =
			DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

		/* Disable console UART */
		err = pm_device_action_run(console_dev, PM_DEVICE_ACTION_SUSPEND);
		if (err)
		{
			LOG_ERR("Unable to suspend console UART. (err: %d)", err);
			return err;
		}
	}
#endif

	err = lte_lc_init();
	if (err < 0)
		__ASSERT_MSG_INFO("LTE init failed. (err: %i)", err);

	/* Power saving is turned on */
	err = lte_lc_psm_req(true);
	if (err < 0)
		__ASSERT_MSG_INFO("PSM request failed. (err: %i)", err);

	/* Edrx is turned on */
	err = lte_lc_edrx_req(true);
	if (err < 0)
		__ASSERT_MSG_INFO("eDRX request failed. (err: %i)", err);

	/* Connect to LTE */
	err = lte_lc_connect();
	if (err < 0)
		__ASSERT_MSG_INFO("LTE connect failed. (err: %i)", err);

	/* Set up modem info */
	err = modem_info_init();
	if (err < 0)
		__ASSERT_MSG_INFO("Unable to initialize modem info. (err: %i)", err);

	/* Get the IMEI (used for client ID)*/
	char imei[20] = {0};
	err = modem_info_string_get(MODEM_INFO_IMEI, imei, sizeof(imei));
	if (err < 0)
	{
		__ASSERT_MSG_INFO("Unable to get IMEI. Err: %i", err);
	}
	else
	{
		imei[15] = '\0';
	}

	LOG_INF("IMEI: %s", (char *)(imei));

	/* Init backend */
	err = app_backend_init(imei, strlen(imei));
	if (err < 0)
		__ASSERT_MSG_INFO("Unable to initialize backend. (err: %i)", err);

	/* Setup gps */
	err = app_gps_setup();
	if (err < 0)
		__ASSERT_MSG_INFO("Unable to setup GPS. Err: %i", err);

	return 0;
}
