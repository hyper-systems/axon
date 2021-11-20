#include <logging/log.h>
#include "hyper_device_axon.h"
#include "axon.h"
#include "misc_utils.h"

LOG_MODULE_REGISTER(hyper_device_axon, CONFIG_MAIN_LOG_LEVEL);

#if defined(CONFIG_BOARD_AXON_V1_0_NRF52840) || defined(CONFIG_BOARD_AXON_V0_2_NRF52840)
#include "hyper_device_11_v3.gen.h"
#define AXON_HYPER_DEVICE_11
static hyper_device_11_t hyper_device = {0};
#else
#include "hyper_device_12_v3.gen.h"
#define AXON_HYPER_DEVICE_12
static hyper_device_12_t hyper_device = {0};
#endif

static void on_publish_interval_update(uint16_t value)
{
	if (value > 0)
	{
		LOG_INF(">>> ACTION: on_publish_interval_update call with value %u", value);
		axon_publish_interval_set(value);
	}
	else
	{
		LOG_ERR(">>> ACTION: on_publish_interval_update call with invalid value 0!");
	}
}

static void on_trigger_reboot(bool value)
{
	LOG_INF(">>> ACTION: triggering reboot... value: %u", value);
	if (value)
	{
		axon_trigger_cold_reboot();
	}
}

static hyper_result_t hyper_device_axon_set_data(uint8_t *data, uint8_t data_len)
{
	LOG_INF("raw_msg_without_header (len: %u): ", data_len);
	hyper_hexdump(data, data_len);
#ifdef AXON_HYPER_DEVICE_11
	hyper_result_t ret = hyper_device_11_decode(&hyper_device, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_11_encode() failed with error %d", ret);
		return ret;
	}
	hyper_device_11_dispatch(&hyper_device);
	LOG_INF("after update:");
	hyper_device_11_print(&hyper_device);
#else
	hyper_result_t ret = hyper_device_12_decode(&hyper_device, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_12_decode() failed with error %d", ret);
		return ret;
	}
	hyper_device_12_dispatch(&hyper_device);
	LOG_INF("after update:");
	hyper_device_12_print(&hyper_device);
#endif

	return ret;
}

static hyper_result_t hyper_device_axon_get_data(uint8_t *data, uint8_t *data_len)
{
	// [UPDATE] Set some attributes.
	if (!axon_sensors_sample_fetch())
	{
#ifdef AXON_HYPER_DEVICE_11
		hyper_device_11_set_environment_temperature(&hyper_device, axon_temp_get());
		hyper_device_11_set_humidity(&hyper_device, axon_humid_get());
		hyper_device_11_set_atmospheric_pressure(&hyper_device, axon_press_get());
		hyper_device_11_set_illuminance(&hyper_device, axon_luminosity_get());
	}

	// do not include Null terminator in version string
	hyper_device_11_set_firmware_version(&hyper_device, HYPER_FIRMWARE_VER, sizeof(HYPER_FIRMWARE_VER) - 1);
	hyper_device_11_set_uptime(&hyper_device, axon_uptime_get());

	hyper_device_11_set_publish_interval(&hyper_device, axon_publish_interval_get());

	// Pretty-print device.
	hyper_device_11_print(&hyper_device);

	// [SEND] Encode device.
	hyper_result_t ret = hyper_device_11_encode(&hyper_device, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_11_encode failed with error %d", ret);
#else
		hyper_device_12_set_environment_temperature(&hyper_device, axon_temp_get());
		hyper_device_12_set_humidity(&hyper_device, axon_humid_get());
		hyper_device_12_set_illuminance(&hyper_device, axon_luminosity_get());
	}

	// do not include Null terminator in version string
	hyper_device_12_set_firmware_version(&hyper_device, HYPER_FIRMWARE_VER, sizeof(HYPER_FIRMWARE_VER) - 1);
	hyper_device_12_set_uptime(&hyper_device, axon_uptime_get());

	hyper_device_12_set_publish_interval(&hyper_device, axon_publish_interval_get());

	// Pretty-print device.
	hyper_device_12_print(&hyper_device);

	// [SEND] Encode device.
	hyper_result_t ret = hyper_device_12_encode(&hyper_device, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_12_encode failed with error %d", ret);
#endif
		return ret;
	}
	LOG_INF("Encoding device data (size=%d)", *data_len);
	return ret;
}

int hyper_device_axon_init(hyper_device_reg_t *reg)
{
	// Axon Device Hardware is already inited at this stage

	//TODO: check which axon version and init accordingly

	// use macaddress as device_id
	uint8_t device_id[8];
	axon_macaddr_get(device_id);

#ifdef AXON_HYPER_DEVICE_11
	hyper_device_11_init(&hyper_device, device_id);
	hyper_device_11_bind_publish_interval(&hyper_device, on_publish_interval_update);
	hyper_device_11_bind_reboot(&hyper_device, on_trigger_reboot);
#else
	hyper_device_12_init(&hyper_device, device_id);
	hyper_device_12_bind_publish_interval(&hyper_device, on_publish_interval_update);
	hyper_device_12_bind_reboot(&hyper_device, on_trigger_reboot);
#endif

	hyper_extensions_registry_insert(reg, device_id, hyper_device_axon_get_data, hyper_device_axon_set_data);

	return 0;
};
