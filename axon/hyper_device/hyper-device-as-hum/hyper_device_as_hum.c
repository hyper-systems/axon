#include <logging/log.h>
#include <kernel.h>
#include "axon.h"
#include "misc_utils.h"
#include "hyper_device_as_hum.h"

LOG_MODULE_REGISTER(hyper_device_as_hum, CONFIG_MAIN_LOG_LEVEL);
#include "hyper_device_16_v3.gen.h"

#define AS_EZO_HUM_I2C_ADDR 0x6F
#define AS_EZO_HUM_READ_CMD "R"

static hyper_device_16_t hyper_device_16 = {0};

// https://files.atlas-scientific.com/EZO-HUM-Datasheet.pdf
static int hyper_device_as_ezo_hum_read(float *temp, float *hum, float *dew)
{
	int ret = 0;
	ret = hyper_extension_bus_i2c_write(AS_EZO_HUM_READ_CMD, sizeof(AS_EZO_HUM_READ_CMD) - 1, AS_EZO_HUM_I2C_ADDR);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_write err: %d\n", ret);

		return -1;
	}
	k_sleep(K_MSEC(250));

	// Largest string 22 characters (from the datasheet)
	uint8_t read_buff[22];
	ret = hyper_extension_bus_i2c_read(read_buff, sizeof(read_buff), AS_EZO_HUM_I2C_ADDR);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_read err: %d\n", ret);

		return -1;
	}

	// first byte is response code
	uint8_t ezo_hum_res_code = read_buff[0];
	switch (ezo_hum_res_code)
	{
	case 2:
		LOG_ERR("EZO-HUM (response code '%u'): syntax error\n", ezo_hum_res_code);
		return ezo_hum_res_code;
	case 254:
		LOG_ERR("EZO-HUM (response code '%u'): still processing, not ready\n", ezo_hum_res_code);
		return ezo_hum_res_code;
	case 255:
		LOG_ERR("EZO-HUM (response code '%u'): no data to send\n", ezo_hum_res_code);
		return ezo_hum_res_code;
	}

	char hum_data[sizeof(read_buff) - 1];
	memcpy(hum_data, &read_buff[1], sizeof(hum_data));

	char *hum_ptr;
	char *temp_ptr;
	char *nul_ptr;
	char *dew_ptr;

	hum_ptr = strtok(hum_data, ",");
	temp_ptr = strtok(NULL, ",");
	nul_ptr = strtok(NULL, ",");
	dew_ptr = strtok(NULL, ",");

	// Convert strings to float
	*temp = atof(temp_ptr);
	*hum = atof(hum_ptr);
	*dew = atof(dew_ptr);

	return ret;
}

static hyper_result_t hyper_device_as_hum_get_data(uint8_t *data, uint8_t *data_len)
{
	float temp, hum, dew;
	int ret = 0;

	ret = hyper_device_as_ezo_hum_read(&temp, &hum, &dew);
	if (ret)
	{
		LOG_ERR("hyper_device_as_ezo_hum_readh() for pH failed with exit code: %d\n", ret);
	}
	else
	{
		hyper_device_16_set_environment_temperature(&hyper_device_16, temp);
		hyper_device_16_set_humidity(&hyper_device_16, hum);
		hyper_device_16_set_dew_point_temperature(&hyper_device_16, dew);
	}

	// Pretty-print device.
	hyper_device_16_print(&hyper_device_16);

	// [SEND] Encode device.
	hyper_result_t res;

	res = hyper_device_16_encode(&hyper_device_16, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_16_encode failed with error %d", ret);
		return ret;
	}
	LOG_INF("Encoding device data (size=%d)", *data_len);

	return res;
}

bool hyper_device_as_hum_is_as_hum(uint32_t class_id)
{

	if (class_id == HYPER_DEVICE_AS_HUM)
	{
		return true;
	};
	return false;
}

int hyper_device_as_hum_init(hyper_device_reg_t *reg)
{
	int ret = 0;
	uint8_t extension_uid[6] = {0};

	hyper_extension_eui48_read(extension_uid);
	if (ret < 0)
	{
		LOG_ERR("hyper_extension_eui48_read() failed with exit code: %d\n", ret);
		return ret;
	}

	hyper_device_16_init(&hyper_device_16, extension_uid);

	hyper_extensions_registry_insert(reg, extension_uid, hyper_device_as_hum_get_data, NULL);

	return 0;
};
