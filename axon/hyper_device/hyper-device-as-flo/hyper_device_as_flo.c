#include <logging/log.h>
#include <kernel.h>
#include "axon.h"
#include "misc_utils.h"
#include "hyper_device_as_flo.h"

LOG_MODULE_REGISTER(hyper_device_as_flo, CONFIG_MAIN_LOG_LEVEL);
#include "hyper_device_20_v3.gen.h"

// https://files.atlas-scientific.com/flow_EZO_Datasheet.pdf
#define AS_EZO_FLO_I2C_ADDR 0x68
#define AS_EZO_FLO_READ_CMD "R"
#define AS_EZO_FLO_FIND_CMD "Find"
#define AS_EZO_FLO_OUTPUT_STR_GET_CMD "O,?"
#define AS_EZO_FLO_OUTPUT_STR_ENABLE_FR_CMD "O,FR,1"
// Largest string 32 characters (from the datasheet)
#define AS_EZO_FLO_MAX_READ_BUF 32

static hyper_device_20_t hyper_device_20 = {0};

static int as_ezo_cmd(char *cmd, uint8_t cmd_len, uint32_t proc_delay_ms, uint8_t i2c_addr, uint8_t *buff, uint8_t buff_len, char *ezo_str)
{
	int ret = 0;
	ret = hyper_extension_bus_i2c_write(cmd, cmd_len, i2c_addr);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_write err: %d\n", ret);

		return -1;
	}
	k_sleep(K_MSEC(proc_delay_ms));

	ret = hyper_extension_bus_i2c_read(buff, buff_len, i2c_addr);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_read err: %d\n", ret);

		return -1;
	}

	// first byte is response code
	uint8_t ezo_flo_res_code = buff[0];
	switch (ezo_flo_res_code)
	{
	case 2:
		LOG_ERR("[%s] (response code '%u'): syntax error\n", ezo_str, ezo_flo_res_code);
		return ezo_flo_res_code;
	case 254:
		LOG_ERR("[%s] (response code '%u'): still processing, not ready\n", ezo_str, ezo_flo_res_code);
		return ezo_flo_res_code;
	case 255:
		LOG_ERR("[%s] (response code '%u'): no data to send\n", ezo_str, ezo_flo_res_code);
		return ezo_flo_res_code;
	}

	return ret;
}

static void as_ezo_find(bool value)
{
	uint8_t read_buff[1] = {0};
	if (value)
	{
		as_ezo_cmd(AS_EZO_FLO_FIND_CMD, sizeof(AS_EZO_FLO_FIND_CMD), 300,
				  AS_EZO_FLO_I2C_ADDR, read_buff, sizeof(read_buff), "EZO-FLO");
	}
	return;
}

static int as_ezo_flo_flow_rate_enable(void)
{
	int ret;
	uint8_t read_buff[AS_EZO_FLO_MAX_READ_BUF] = {0};
	// check if flow rate is already enabled
	ret = as_ezo_cmd(AS_EZO_FLO_OUTPUT_STR_GET_CMD, sizeof(AS_EZO_FLO_OUTPUT_STR_GET_CMD),
			 300, AS_EZO_FLO_I2C_ADDR, read_buff, sizeof(read_buff), "EZO-FLO");
	if (ret != 0)
	{
		return ret;
	}

	// enable flow rate if not enabled
	if (strstr(read_buff, "FR") == NULL)
	{
		LOG_INF("Flow rate not enabled! enabling...");
		ret = as_ezo_cmd(AS_EZO_FLO_OUTPUT_STR_ENABLE_FR_CMD, sizeof(AS_EZO_FLO_OUTPUT_STR_ENABLE_FR_CMD),
				 300, AS_EZO_FLO_I2C_ADDR, read_buff, sizeof(read_buff), "EZO-FLO");
	}

	return ret;
}

static int hyper_device_as_ezo_flo_read(float *flow_rate, float *total_flow)
{
	int ret;
	uint8_t read_buff[AS_EZO_FLO_MAX_READ_BUF] = {0};

	// enable flow rate
	ret = as_ezo_flo_flow_rate_enable();
	if (ret != 0)
	{
		return ret;
	}

	ret = as_ezo_cmd(AS_EZO_FLO_READ_CMD, sizeof(AS_EZO_FLO_READ_CMD), 300,
			 AS_EZO_FLO_I2C_ADDR, read_buff, sizeof(read_buff), "EZO-FLO");
	if (ret != 0)
	{
		return ret;
	}

	char flo_data[sizeof(read_buff) - 1];
	memcpy(flo_data, &read_buff[1], sizeof(flo_data));

	LOG_DBG("[EZO-FLO] recieved: '%s'", flo_data);

	char *total_flow_ptr;
	char *flow_rate_ptr;

	total_flow_ptr = strtok(flo_data, ",");
	flow_rate_ptr = strtok(NULL, ",");

	// Convert strings to float
	*total_flow = atof(total_flow_ptr);
	*flow_rate = atof(flow_rate_ptr);

	// if "Find" functionality is activated, re-enable it as it is disabled by the previous commands
	if ((hyper_device_20.ezo_flo_find_set) && (hyper_device_20.ezo_flo_find))
	{
		as_ezo_find(true);
	}

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
	hyper_device_20_bind_ezo_flo_find(&hyper_device_20, as_ezo_find);
	hyper_extensions_registry_insert(reg, extension_uid, hyper_device_as_flo_get_data, NULL);

	return 0;
};
