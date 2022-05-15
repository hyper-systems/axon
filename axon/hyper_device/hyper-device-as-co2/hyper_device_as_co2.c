#include <logging/log.h>
#include <kernel.h>
#include "axon.h"
#include "misc_utils.h"
#include "hyper_device_as_co2.h"

LOG_MODULE_REGISTER(hyper_device_as_co2, CONFIG_MAIN_LOG_LEVEL);
#include "hyper_device_14_v3.gen.h"

#define AS_EZO_CO2_I2C_ADDR 0x69
#define AS_EZO_CO2_READ_CMD "R"

static hyper_device_14_t hyper_device_14 = {0};

// https://files.atlas-scientific.com/EZO_CO2_Datasheet.pdf
static int hyper_device_as_ezo_co2_read(uint16_t *co2_ppm)
{
	int ret = 0;
	ret = hyper_extension_bus_i2c_write(AS_EZO_CO2_READ_CMD, sizeof(AS_EZO_CO2_READ_CMD) - 1, AS_EZO_CO2_I2C_ADDR);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_write err: %d\n", ret);

		return -1;
	}
	k_sleep(K_MSEC(250));

	uint8_t read_buff[20];
	ret = hyper_extension_bus_i2c_read(read_buff, sizeof(read_buff), AS_EZO_CO2_I2C_ADDR);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_read err: %d\n", ret);

		return -1;
	}

	// first byte is response code
	uint8_t ezo_co2_res_code = read_buff[0];
	switch (ezo_co2_res_code)
	{
	case 2:
		LOG_ERR("EZO-CO2 (response code '%u'): syntax error\n", ezo_co2_res_code);
		return ezo_co2_res_code;
	case 254:
		LOG_ERR("EZO-CO2 (response code '%u'): still processing, not ready\n", ezo_co2_res_code);
		return ezo_co2_res_code;
	case 255:
		LOG_ERR("EZO-CO2 (response code '%u'): no data to send\n", ezo_co2_res_code);
		return ezo_co2_res_code;
	}

	char co2_data[sizeof(read_buff) - 1];
	memcpy(co2_data, &read_buff[1], sizeof(co2_data));

	if (!strcmp("*WARM", co2_data))
	{
		LOG_ERR("EZO-CO2: warming up, cannot take reading at the moment!");
		return -1;
	}

	// Convert string to int
	*co2_ppm = atoi(co2_data);

	return ret;
}

static hyper_result_t hyper_device_as_co2_get_data(uint8_t *data, uint8_t *data_len)
{
	uint16_t co2_ppm;
	int ret = 0;

	ret = hyper_device_as_ezo_co2_read(&co2_ppm);
	if (ret)
	{
		LOG_ERR("hyper_device_as_ezo_co2_read() for pH failed with exit code: %d\n", ret);
	}
	else
	{
		hyper_device_14_set_co2(&hyper_device_14, co2_ppm);
	}

	// Pretty-print device.
	hyper_device_14_print(&hyper_device_14);

	// [SEND] Encode device.
	hyper_result_t res;

	res = hyper_device_14_encode(&hyper_device_14, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_14_encode failed with error %d", ret);
		return ret;
	}
	LOG_INF("Encoding device data (size=%d)", *data_len);

	return res;
}

bool hyper_device_as_co2_is_as_co2(uint32_t class_id)
{

	if (class_id == HYPER_DEVICE_AS_CO2)
	{
		return true;
	};
	return false;
}

int hyper_device_as_co2_init(hyper_device_reg_t *reg)
{
	int ret = 0;
	uint8_t extension_uid[6] = {0};

	hyper_extension_eui48_read(extension_uid);
	if (ret < 0)
	{
		LOG_ERR("hyper_extension_eui48_read() failed with exit code: %d\n", ret);
		return ret;
	}

	hyper_device_14_init(&hyper_device_14, extension_uid);

	hyper_extensions_registry_insert(reg, extension_uid, hyper_device_as_co2_get_data, NULL);

	return 0;
};
