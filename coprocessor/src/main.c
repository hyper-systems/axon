/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(coprocessor_sample, CONFIG_OT_COPROCESSOR_LOG_LEVEL);

#include <devicetree.h>
#include <drivers/led.h>

#if DT_NODE_HAS_STATUS(DT_INST(0, pwm_leds), okay)
#define LED_PWM_NODE_ID DT_INST(0, pwm_leds)
#define LED_PWM_DEV_NAME DEVICE_DT_NAME(LED_PWM_NODE_ID)
#else
#error "No LED PWM device found"
#endif

#define LED_PWM_LABEL(led_node_id) DT_PROP_OR(led_node_id, label, NULL),

const struct device *led_pwm;

#define PWMLED1 0

#define WELCOME_TEXT                                                    \
	"\n\r"                                                          \
	"\n\r"                                                          \
	"=========================================================\n\r" \
	"OpenThread Coprocessor application is now running on NCS.\n\r" \
	"=========================================================\n\r"

void main(void)
{
	int ret = 0;

	led_pwm = device_get_binding(LED_PWM_DEV_NAME);
	if (led_pwm)
	{
		LOG_INF("Found device %s", LED_PWM_DEV_NAME);
	}
	else
	{
		LOG_ERR("Device %s not found", LED_PWM_DEV_NAME);
		return;
	}

	ret = led_blink(led_pwm, PWMLED1, 30, 130);
	if (ret < 0)
	{
		LOG_ERR("led_blink() failed with exit code: %d\n", ret);
		return;
	}
	LOG_INF(WELCOME_TEXT);
}
