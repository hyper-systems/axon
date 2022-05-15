#include <logging/log.h>
#include <kernel.h>
#include "axon.h"
#include "misc_utils.h"
#include "hyper_device_as_flo.h"

LOG_MODULE_REGISTER(hyper_device_as_flo, CONFIG_MAIN_LOG_LEVEL);
#include "hyper_device_20_v3.gen.h"

#define AS_EZO_FLO_I2C_ADDR 0x68
#define AS_EZO_FLO_READ_CMD "R"
#define AS_EZO_FLO_FIND_CMD "Find"

static hyper_device_20_t hyper_device_20 = {0};

// https://files.atlas-scientific.com/flow_EZO_Datasheet.pdf
static int hyper_device_as_ezo_flo_read(float *flow_rate, float *total_flow)
{
	int ret = 0;
	ret = hyper_extension_bus_i2c_write(AS_EZO_FLO_READ_CMD, sizeof(AS_EZO_FLO_READ_CMD) - 1, AS_EZO_FLO_I2C_ADDR);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_write err: %d\n", ret);

		return -1;
	}
	k_sleep(K_MSEC(250));

	// Largest string 32 characters (from the datasheet)
	uint8_t read_buff[32];
	ret = hyper_extension_bus_i2c_read(read_buff, sizeof(read_buff), AS_EZO_FLO_I2C_ADDR);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_read err: %d\n", ret);

		return -1;
	}

	// first byte is response code
	uint8_t ezo_flo_res_code = read_buff[0];
	switch (ezo_flo_res_code)
	{
	case 2:
		LOG_ERR("EZO-FLO (response code '%u'): syntax error\n", ezo_flo_res_code);
		return ezo_flo_res_code;
	case 254:
		LOG_ERR("EZO-FLO (response code '%u'): still processing, not ready\n", ezo_flo_res_code);
		return ezo_flo_res_code;
	case 255:
		LOG_ERR("EZO-FLO (response code '%u'): no data to send\n", ezo_flo_res_code);
		return ezo_flo_res_code;
	}

	char flo_data[sizeof(read_buff) - 1];
	memcpy(flo_data, &read_buff[1], sizeof(flo_data));

	printk("==== DABAG: '%s'\n", flo_data);

	char *flow_rate_ptr;
	char *total_flow_ptr;
	// char *nul_ptr;

	flow_rate_ptr = strtok(flo_data, ",");
	total_flow_ptr = strtok(NULL, ",");
	// nul_ptr = strtok(NULL, ",");

	// Convert strings to float
	*flow_rate = atof(flow_rate_ptr);
	*total_flow = atof(total_flow_ptr);

	return ret;
}

static hyper_result_t hyper_device_as_flo_get_data(uint8_t *data, uint8_t *data_len)
{
	float flow_rate, total_flow;
	int ret = 0;

	ret = hyper_device_as_ezo_flo_read(&flow_rate, &total_flow);
	if (ret)
	{
		LOG_ERR("hydrokit_read_ph() for pH failed with exit code: %d\n", ret);
	}
	else
	{
		hyper_device_20_set_ezo_flo_flow_rate(&hyper_device_20, flow_rate);
		hyper_device_20_set_ezo_flo_total_flow(&hyper_device_20, total_flow);
	}

	// Pretty-print device.
	hyper_device_20_print(&hyper_device_20);

	// [SEND] Encode device.
	hyper_result_t res;

	res = hyper_device_20_encode(&hyper_device_20, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_20_encode failed with error %d", ret);
		return ret;
	}
	LOG_INF("Encoding device data (size=%d)", *data_len);

	return res;
}

bool hyper_device_as_flo_is_as_flo(uint32_t class_id)
{

	if (class_id == HYPER_DEVICE_AS_FLO)
	{
		return true;
	};
	return false;
}

int hyper_device_as_flo_init(hyper_device_reg_t *reg)
{
	int ret = 0;
	uint8_t extension_uid[6] = {0};

	hyper_extension_eui48_read(extension_uid);
	if (ret < 0)
	{
		LOG_ERR("hyper_extension_eui48_read() failed with exit code: %d\n", ret);
		return ret;
	}

	hyper_device_20_init(&hyper_device_20, extension_uid);

	hyper_extensions_registry_insert(reg, extension_uid, hyper_device_as_flo_get_data, NULL);

	return 0;
};
