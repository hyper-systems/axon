#include <logging/log.h>
#include "hyper_device_axon.h"
#include "axon.h"
#include "misc_utils.h"
#include "hyper-device-class-11-axon_board_v1_0.V2.gen.h"
#include "hyper-device-class-12-axon_board_v1_3_0.V2.gen.h"

LOG_MODULE_REGISTER(hyper_device_axon, CONFIG_MAIN_LOG_LEVEL);

static hyper_device_11_t hyper_device_11 = {0};
__unused static hyper_device_12_t hyper_device_12 = {0};

static void on_publish_interval_update(uint16_t value) {
	if(value > 0) {
		LOG_INF(">>> ACTION: on_publish_interval_update call with value %u", value);
		axon_publish_interval_set(value);
	}
	else {
		LOG_ERR(">>> ACTION: on_publish_interval_update call with invalid value 0!");
	}
}

static void on_trigger_reboot(bool value) {
	LOG_INF(">>> ACTION: triggering reboot... value: %u", value);
	if(value) {
		axon_trigger_cold_reboot();
	}
}

static void hyper_device_axon_set_data(uint8_t *data, uint8_t data_len){
    LOG_INF("raw_msg_without_header (len: %u): ", data_len);
    hyper_hexdump(data, data_len);

    hyper_device_11_decode(&hyper_device_11, data, data_len);
    hyper_device_11_dispatch(&hyper_device_11);
	LOG_INF("after update:");
	hyper_device_11_pp(&hyper_device_11);
}

static uint8_t hyper_device_axon_get_data(uint8_t *data){
    // [UPDATE] Set some attributes.
	if (!axon_sensors_sample_fetch()) {
		hyper_device_11_set_environment_temperature(&hyper_device_11, axon_temp_get());
		hyper_device_11_set_humidity(&hyper_device_11, axon_humid_get());
		hyper_device_11_set_atmospheric_pressure(&hyper_device_11, axon_press_get());
		hyper_device_11_set_illuminance(&hyper_device_11, axon_luminosity_get());
	}

    hyper_device_11_set_firmware_version(&hyper_device_11, HYPER_FIRMWARE_VER, sizeof(HYPER_FIRMWARE_VER));
	hyper_device_11_set_uptime(&hyper_device_11, axon_uptime_get());

    hyper_device_11_set_publish_interval(&hyper_device_11, axon_publish_interval_get());

    // Pretty-print device.
	hyper_device_11_pp(&hyper_device_11);

    // [SEND] Encode device.
	uint8_t message_len = hyper_device_11_encode(&hyper_device_11, data);
	LOG_INF("Encoding device data (size=%d)", message_len);

    return message_len;
}

int hyper_device_axon_init(hyper_device_reg_t *reg){
    // Axon Device Hardware is already inited at this stage

    //TODO: check which axon version and init accordingly

    // use macaddress as device_id
	uint8_t device_id[8];
	axon_macaddr_get(device_id);

	hyper_device_11_init(&hyper_device_11, device_id);

    hyper_device_11_bind_publish_interval(&hyper_device_11, on_publish_interval_update);

	hyper_device_11_bind_reboot(&hyper_device_11, on_trigger_reboot);

    hyper_extensions_registry_insert(reg, device_id, hyper_device_axon_get_data, hyper_device_axon_set_data);

    return 0;
};
