/*
 * Copyright Circuit Dojo (c) 2022
 *
 * SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/pwm.h>

#include <app_indication.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_indication);

/* PWM node */
#define PWM_LED_NODE DT_NODELABEL(pwm_led0)
#define PWM_CTLR DT_PWMS_CTLR(PWM_LED_NODE)
#define PWM_CHAN DT_PWMS_CHANNEL(PWM_LED_NODE)
#define PWM_FLAGS DT_PWMS_FLAGS(PWM_LED_NODE)

/* Led PWM period, calculated for 50 Hz signal - in microseconds. */
#define LED_PWM_PERIOD_US (USEC_PER_SEC / 50U)

/* Static bits. */
static const struct device *led_pwm_dev = DEVICE_DT_GET(PWM_CTLR);
static uint32_t pattern_index = 0;
static enum app_indication_mode current_mode = app_indication_none;

/* Patterns */
/* Note: output is inverted */
const uint8_t app_indication_pattern_glow[] = {255, 225, 200, 175, 150, 125, 100, 50, 100, 125, 150, 175, 200, 225};
const uint8_t app_indication_pattern_error[] = {150, 255, 150, 255, 150, 255, 255, 150, 150, 150, 255, 150, 150, 150, 255, 150, 150, 150, 255, 255};
const uint8_t app_indication_pattern_fast_blink[] = {200, 255};

/* Timer function */
void pwm_change_fn(struct k_timer *dummy)
{
    uint32_t pulse = LED_PWM_PERIOD_US;

    switch (current_mode)
    {
    case app_indication_solid:
        pulse = 0;
        break;
    case app_indication_glow:
        pulse = app_indication_pattern_glow[pattern_index++ % sizeof(app_indication_pattern_glow)] * LED_PWM_PERIOD_US / 255U;
        break;
    case app_indication_error:
        pulse = app_indication_pattern_error[pattern_index++ % sizeof(app_indication_pattern_error)] * LED_PWM_PERIOD_US / 255U;
        break;
    case app_indication_fast_blink:
        pulse = app_indication_pattern_fast_blink[pattern_index++ % sizeof(app_indication_pattern_fast_blink)] * LED_PWM_PERIOD_US / 255U;
        break;
    case app_indication_none:
    default:
        break;
    };

    int err = pwm_pin_set_usec(led_pwm_dev, PWM_CHAN,
                               LED_PWM_PERIOD_US, pulse, PWM_FLAGS);
    if (err)
    {
        LOG_ERR("Pwm set fail. Err: %i", err);
    }
}

/* Timers */
K_TIMER_DEFINE(pwm_change_timer, pwm_change_fn, NULL);

int app_indication_init(void)
{

    if (led_pwm_dev == NULL || !device_is_ready(led_pwm_dev))
    {
        LOG_ERR("Error: PWM device is not ready");
        return -EINVAL;
    }

    return app_indication_set(app_indication_none);
}

int app_indication_set(enum app_indication_mode mode)
{

    int err = 0;

    /* Set current mode */
    current_mode = mode;

    /* Reset index */
    pattern_index = 0;

    /* Pulse*/
    uint32_t pulse = LED_PWM_PERIOD_US;

    switch (mode)
    {
    case app_indication_solid:
        pulse = 0;
        break;
    case app_indication_glow:
        pulse = app_indication_pattern_glow[pattern_index++] * LED_PWM_PERIOD_US / 255U;
        break;
    case app_indication_error:
        pulse = app_indication_pattern_error[pattern_index++] * LED_PWM_PERIOD_US / 255U;
        break;
    case app_indication_fast_blink:
        pulse = app_indication_pattern_fast_blink[pattern_index++] * LED_PWM_PERIOD_US / 255U;
        break;
    case app_indication_none:
    default:
        break;
    }

    err = pwm_pin_set_usec(led_pwm_dev, PWM_CHAN,
                           LED_PWM_PERIOD_US, pulse, PWM_FLAGS);
    if (err)
    {
        LOG_ERR("Pwm set fail. Err: %i", err);
        return err;
    }

    if (mode != app_indication_none)
    {
        /* Start repeat timer.. */
        k_timer_start(&pwm_change_timer, K_MSEC(200), K_MSEC(200));
    }
    else
    {
        /* Stop timer */
        k_timer_stop(&pwm_change_timer);
    }

    return 0;
}
