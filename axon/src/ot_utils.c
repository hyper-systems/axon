/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Hyper Collective LTD
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#include <zephyr.h>
#include <device.h>
#include <logging/log.h>
#include <net/openthread.h>
#include <net/socket.h>
#include <openthread/thread.h>
#include <stdio.h>

#include "ot_utils.h"
#include "axon.h"

LOG_MODULE_REGISTER(ot_utils, CONFIG_OT_UTILS_LOG_LEVEL);

#define RESPONSE_POLL_PERIOD 100

static uint32_t poll_period;

static bool is_connected;

static struct k_work toggle_MTD_SED_work;
static struct k_work on_connect_work;
static struct k_work on_disconnect_work;

/** @brief Function called when OpenThread connection
 *         is established.
 *
 * @param[in] item pointer to work item.
 */
static void on_ot_connect(struct k_work *item)
{
	ARG_UNUSED(item);
	axon_led_set_on();
}

/** @brief Function called when OpenThread connection is ended.
 *
 * @param[in] item pointer to work item.
 */
static void on_ot_disconnect(struct k_work *item)
{
	ARG_UNUSED(item);
	axon_led_set_blink(30, 130);
}

/** @brief Function called when the MTD modes are toggled.
 *
 * @param[in] val 1 if the MTD is in MED mode
 *                0 if the MTD is in SED mode
 */
void on_mtd_mode_toggle(uint32_t val)
{
	ARG_UNUSED(val);
};

static bool is_mtd_in_med_mode(otInstance *instance)
{
	return otThreadGetLinkMode(instance).mRxOnWhenIdle;
}

static void toggle_minimal_sleepy_end_device(struct k_work *item)
{
	otError error;
	otLinkModeConfig mode;
	struct openthread_context *context = openthread_get_default_context();

	__ASSERT_NO_MSG(context != NULL);

	openthread_api_mutex_lock(context);
	mode = otThreadGetLinkMode(context->instance);
	mode.mRxOnWhenIdle = !mode.mRxOnWhenIdle;
	error = otThreadSetLinkMode(context->instance, mode);
	openthread_api_mutex_unlock(context);

	if (error != OT_ERROR_NONE)
	{
		LOG_ERR("Failed to set MLE link mode configuration");
	}
	else
	{
		on_mtd_mode_toggle(mode.mRxOnWhenIdle);
	}
}

__unused static void poll_period_response_set(void)
{
	otError error;

	otInstance *instance = openthread_get_default_instance();

	if (is_mtd_in_med_mode(instance))
	{
		return;
	}

	if (!poll_period)
	{
		poll_period = otLinkGetPollPeriod(instance);

		error = otLinkSetPollPeriod(instance, RESPONSE_POLL_PERIOD);
		__ASSERT(error == OT_ERROR_NONE, "Failed to set pool period");

		LOG_INF("Poll Period: %dms set", RESPONSE_POLL_PERIOD);
	}
}

__unused static void poll_period_restore(void)
{
	otError error;
	otInstance *instance = openthread_get_default_instance();

	if (is_mtd_in_med_mode(instance))
	{
		return;
	}

	if (poll_period)
	{
		error = otLinkSetPollPeriod(instance, poll_period);
		__ASSERT_NO_MSG(error == OT_ERROR_NONE);

		LOG_INF("Poll Period: %dms restored", poll_period);
		poll_period = 0;
	}
}

static void update_device_state(void)
{
	struct otInstance *instance = openthread_get_default_instance();
	otLinkModeConfig mode = otThreadGetLinkMode(instance);
	on_mtd_mode_toggle(mode.mRxOnWhenIdle);
}

static void on_thread_state_changed(uint32_t flags, void *context)
{
	struct openthread_context *ot_context = context;

	if (flags & OT_CHANGED_THREAD_ROLE)
	{
		switch (otThreadGetDeviceRole(ot_context->instance))
		{
		case OT_DEVICE_ROLE_CHILD:
		case OT_DEVICE_ROLE_ROUTER:
		case OT_DEVICE_ROLE_LEADER:
			k_work_submit(&on_connect_work);
			is_connected = true;
			break;

		case OT_DEVICE_ROLE_DISABLED:
		case OT_DEVICE_ROLE_DETACHED:
		default:
			k_work_submit(&on_disconnect_work);
			is_connected = false;
			break;
		}
	}
}

int ot_utils_init(void)
{

	k_work_init(&on_connect_work, on_ot_connect);
	k_work_init(&on_disconnect_work, on_ot_disconnect);
	struct openthread_context *ot_context = openthread_get_default_context();

	__ASSERT_NO_MSG(ot_context != NULL);

	openthread_api_mutex_lock(ot_context);
	openthread_set_state_changed_cb(on_thread_state_changed);
	// set txpower to +8dB
	otPlatRadioSetTransmitPower(ot_context->instance, 8);
	openthread_api_mutex_unlock(ot_context);

#if defined(CONFIG_OPENTHREAD_MANUAL_START)
	openthread_start(ot_context);
#else
	// ugly hack to turn led on if OpenThread starts automatically and doesnt change role
	// happens when chaning from REED (FTD) to SED (MTD)
	switch (otThreadGetDeviceRole(ot_context->instance))
	{
	case OT_DEVICE_ROLE_CHILD:
	case OT_DEVICE_ROLE_ROUTER:
	case OT_DEVICE_ROLE_LEADER:
		k_work_submit(&on_connect_work);
		is_connected = true;
		break;
	default:
		k_work_submit(&on_disconnect_work);
		is_connected = false;
		break;
	}
#endif

	if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED))
	{
		k_work_init(&toggle_MTD_SED_work,
			    toggle_minimal_sleepy_end_device);
		k_work_submit(&toggle_MTD_SED_work);
	}

	// TODO: replace by net_if_get_link_addr
	// add manual ip, CUSTOM PREFIX + macaddress
	uint8_t macaddr[6];
	char manual_ip[46];
	axon_macaddr_get(macaddr);

	// TODO: add prefix as a KConfig symbol
	sprintf(manual_ip, "fdde:ad00:beef::%02x%02x:%02x%02x:%02x%02x",
		macaddr[0], macaddr[1], macaddr[2], macaddr[3],
		macaddr[4], macaddr[5]);

	LOG_INF("Adding manual ip: %s", manual_ip);

	otNetifAddress aAddress;

	otIp6AddressFromString(manual_ip, &aAddress.mAddress);
	aAddress.mPrefixLength = 64;
	aAddress.mPreferred = true;
	aAddress.mValid = true;
	aAddress.mAddressOrigin = OT_ADDRESS_ORIGIN_MANUAL;
	otIp6AddUnicastAddress(ot_context->instance, &aAddress);

	// add address with legacy prefix for retro compatibility
	sprintf(manual_ip, "fdaa:bb:1::%02x%02x:%02x%02x:%02x%02x",
		macaddr[0], macaddr[1], macaddr[2], macaddr[3],
		macaddr[4], macaddr[5]);

	LOG_INF("Adding manual ip: %s", manual_ip);

	otNetifAddress aAddress_old;

	otIp6AddressFromString(manual_ip, &aAddress_old.mAddress);
	aAddress_old.mPrefixLength = 64;
	aAddress_old.mPreferred = true;
	aAddress_old.mValid = true;
	aAddress_old.mAddressOrigin = OT_ADDRESS_ORIGIN_MANUAL;
	otIp6AddUnicastAddress(ot_context->instance, &aAddress_old);

	// TODO: get proper returns
	return 0;
}

void ot_utils_toggle_minimal_sleepy_end_device(void)
{
	if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED))
	{
		k_work_init(&toggle_MTD_SED_work,
			    toggle_minimal_sleepy_end_device);
		update_device_state();
	}
}

bool ot_is_connected()
{
	return is_connected;
}
