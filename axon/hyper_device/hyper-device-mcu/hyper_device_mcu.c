#include <logging/log.h>
#include <kernel.h>
#include "hyper_device_mcu.h"
#include "axon.h"
#include "misc_utils.h"

LOG_MODULE_REGISTER(hyper_device_mcu, CONFIG_MAIN_LOG_LEVEL);

#define HYPER_EXTENSION_MAX_READ_RETRIES 3
#define HYPER_EXTENSION_I2C_ADDR 0x05
#define HYPER_EXTENSION_MAX_SIZE 128
typedef enum
{
	HYPER_EXTENSION_RES_ERROR = 0x00,
	HYPER_EXTENSION_RES_ERROR_NOT_READY,
} hyper_extension_control_reg_result_t;

typedef enum
{
	HYPER_EXTENSION_BUS_CMD_GET_DATA = 0x01,
	HYPER_EXTENSION_BUS_CMD_SET_DATA = 0x02,
} hyper_extension_cmds_t;

#define HYPER_EXTENSION_BUS_CONTROL_REG_ADDR 0x01
#define HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR 0x02
#define HYPER_EXTENSION_DATA_REG_START_ADDR 0x0A
typedef struct
{
	uint8_t read_retries;
	uint8_t data[HYPER_EXTENSION_MAX_SIZE];
	uint8_t data_size;
} hyper_extension_t;

hyper_extension_t hyper_extension = {0};

static int extension_bus_get_data(uint8_t *data, uint8_t *data_len)
{
	int ret;
	ret = hyper_extension_bus_i2c_reg_write_byte(HYPER_EXTENSION_I2C_ADDR,
						     HYPER_EXTENSION_BUS_CONTROL_REG_ADDR,
						     HYPER_EXTENSION_BUS_CMD_GET_DATA);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_reg_write_byte: %d", ret);
		return -1;
	}
	// give it a bit of time for get_data
	k_sleep(K_MSEC(200));

	hyper_result_t hres;
	ret = hyper_extension_bus_i2c_reg_read_byte(HYPER_EXTENSION_I2C_ADDR,
						    HYPER_EXTENSION_BUS_CONTROL_REG_ADDR,
						    &hres);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_reg_read_byte: %d", ret);
		return -1;
	}

	LOG_INF("HYPER_EXTENSION_BUS_CONTROL_REG_ADDR: %x", hres);
	if (hres != HYPER_OK)
	{
		LOG_ERR("HYPER_EXTENSION_BUS_CONTROL_REG_ADDR hyper_result return code: %d", hres);
		return -1;
	}

	uint8_t extension_data_size;
	ret = hyper_extension_bus_i2c_reg_read_byte(HYPER_EXTENSION_I2C_ADDR,
						    HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR,
						    &extension_data_size);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_reg_read_byte: %d", ret);
		return -1;
	}
	LOG_INF("HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR: 0x%x (%u)", extension_data_size, extension_data_size);
	if (extension_data_size == 0)
	{
		LOG_ERR("HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR value is 0");
		return -1;
	}
	if (extension_data_size > HYPER_EXTENSION_MAX_SIZE)
	{
		LOG_ERR("HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR value (%u) is bigger than HYPER_EXTENSION_MAX_SIZE(%u)",
			extension_data_size, HYPER_EXTENSION_MAX_SIZE);
		return -1;
	}

	// read data in chunks of 32bytes
	uint8_t r_buff[extension_data_size];
	uint8_t reg_addr = HYPER_EXTENSION_DATA_REG_START_ADDR;
	uint8_t data_len_remaining = extension_data_size;
	uint8_t data_ptr = 0;
	while (data_len_remaining > 32)
	{
		ret = hyper_extension_bus_i2c_burst_read(HYPER_EXTENSION_I2C_ADDR, reg_addr, &r_buff[data_ptr], 32);
		if (ret != 0)
		{
			LOG_ERR("hyper_extension_bus_i2c_burst_read: %d", ret);
			return -1;
		}
		reg_addr += 32;
		data_ptr += 32;
		data_len_remaining -= 32;
	}
	ret = hyper_extension_bus_i2c_burst_read(HYPER_EXTENSION_I2C_ADDR, reg_addr, &r_buff[data_ptr], data_len_remaining);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_burst_read: %d", ret);
		return -1;
	}

	// after data read is successful, copy data back
	memcpy(data, r_buff, extension_data_size);
	*data_len = extension_data_size;

	// LOG_INF("data: ");
	// hyper_hexdump(data, *data_len);

	return ret;
}

// add retries to the extension_bus_get_data function
static int hyper_device_mcu_extension_bus_get_data(hyper_extension_t *ext)
{
	int ret;
	ext->read_retries = 0;

	ret = extension_bus_get_data(ext->data, &(ext->data_size));
	while ((ret != 0) && (ext->read_retries < HYPER_EXTENSION_MAX_READ_RETRIES))
	{
		LOG_ERR("hyper_device_mcu_extension_bus_get_data: %d", ret);
		k_sleep(K_MSEC(500));
		LOG_INF("Retrying to read extension: retry %u of %u", ext->read_retries, HYPER_EXTENSION_MAX_READ_RETRIES);
		ret = extension_bus_get_data(ext->data, &(ext->data_size));
		if (ret != 0)
		{
			LOG_ERR("hyper_device_mcu_extension_bus_get_data: %d", ret);
		}
		ext->read_retries++;
	}

	return ret;
}

static int hyper_device_mcu_extension_bus_set_data(uint8_t *data, uint8_t data_len)
{

	if (data_len > HYPER_EXTENSION_MAX_SIZE)
	{
		LOG_ERR("data_len value (%u) is bigger than HYPER_EXTENSION_MAX_SIZE(%u)",
			data_len, HYPER_EXTENSION_MAX_SIZE);
		return -1;
	}

	int ret;

	// write data in chunks of 32bytes: starting registry address (1byte) + data (31 bytes)
	uint8_t w_buff[32];
	uint8_t reg_addr = HYPER_EXTENSION_DATA_REG_START_ADDR;
	uint8_t data_len_remaining = data_len;
	uint8_t data_ptr = 0;
	while (data_len_remaining > 31)
	{
		w_buff[0] = reg_addr;
		memcpy(&w_buff[1], &data[data_ptr], 31);
		LOG_INF("writting %u of %u: ", data_len_remaining, data_len);
		hyper_hexdump(&data[data_ptr], 31);
		ret = hyper_extension_bus_i2c_write(w_buff, sizeof(w_buff), HYPER_EXTENSION_I2C_ADDR);
		if (ret != 0)
		{
			LOG_ERR("hyper_extension_bus_i2c_write: %d", ret);
			return -1;
		}
		reg_addr += 31;
		data_ptr += 31;
		data_len_remaining -= 31;
	}
	LOG_INF("writting %u of %u: ", data_len_remaining, data_len);
	hyper_hexdump(&data[data_ptr], data_len_remaining);
	w_buff[0] = reg_addr;
	memcpy(&w_buff[1], &data[data_ptr], data_len_remaining);
	ret = hyper_extension_bus_i2c_write(w_buff, sizeof(w_buff), HYPER_EXTENSION_I2C_ADDR);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_burst_read: %d", ret);
		return -1;
	}

	// write data_len to HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR
	ret = hyper_extension_bus_i2c_reg_write_byte(HYPER_EXTENSION_I2C_ADDR,
						     HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR,
						     data_len);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_reg_write_byte: %d", ret);
		return -1;
	}

	// get data_len from HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR and confirm value
	uint8_t read_data_len;
	ret = hyper_extension_bus_i2c_reg_read_byte(HYPER_EXTENSION_I2C_ADDR,
						    HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR,
						    &read_data_len);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_reg_read_byte: %d", ret);
		return -1;
	}
	if (read_data_len != data_len)
	{
		LOG_ERR("value at HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR mismatch: %u (written) vs %u (read)",
			data_len, read_data_len);
		return -1;
	}
	LOG_INF("read HYPER_EXTENSION_BUS_DATA_SIZE_REG_ADDR:  %u", read_data_len);

	// request set_data
	ret = hyper_extension_bus_i2c_reg_write_byte(HYPER_EXTENSION_I2C_ADDR,
						     HYPER_EXTENSION_BUS_CONTROL_REG_ADDR,
						     HYPER_EXTENSION_BUS_CMD_SET_DATA);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_reg_write_byte: %d", ret);
		return -1;
	}

	// give it a bit of time for set_data
	k_sleep(K_MSEC(200));

	// get set_data status from HYPER_EXTENSION_BUS_CONTROL_REG_ADDR
	hyper_result_t hres;
	ret = hyper_extension_bus_i2c_reg_read_byte(HYPER_EXTENSION_I2C_ADDR,
						    HYPER_EXTENSION_BUS_CONTROL_REG_ADDR,
						    &hres);
	if (ret != 0)
	{
		LOG_ERR("hyper_extension_bus_i2c_reg_read_byte: %d", ret);
		return -1;
	}

	LOG_INF("HYPER_EXTENSION_BUS_CONTROL_REG_ADDR: %x", hres);
	if (hres != HYPER_OK)
	{
		LOG_ERR("HYPER_EXTENSION_BUS_CONTROL_REG_ADDR hyper_result return code: %d", hres);
		return -1;
	}

	LOG_INF("HYPER_EXTENSION_BUS_CONTROL_REG_ADDR set_data:  %d", hres);

	return ret;
}

hyper_result_t hyper_device_mcu_set_data(uint8_t *data, uint8_t data_len)
{
	int ret;
	ret = hyper_device_mcu_extension_bus_set_data(data, data_len);
	if (ret != 0)
	{
		LOG_ERR("hyper_device_mcu_extension_bus_set_data: %d", ret);
		return HYPER_ERR_ENCODE;
	}
	return HYPER_OK;
}

hyper_result_t hyper_device_mcu_get_data(uint8_t *data, uint8_t *data_len)
{

	int ret = hyper_device_mcu_extension_bus_get_data(&hyper_extension);
	if (ret != 0)
	{
		LOG_ERR("extension_bus_get_data: %d", ret);
		return HYPER_ERR_DECODE;
	}
	memcpy(data, hyper_extension.data, hyper_extension.data_size);
	LOG_INF("Got extension data (size=%u)!", hyper_extension.data_size);
	*data_len = hyper_extension.data_size;

	return HYPER_OK;
}

int hyper_device_mcu_init(hyper_device_reg_t *reg)
{
	int ret = 0;
	uint8_t device_id[8];

	// extract device id from extension payload
	hyper_device_mcu_extension_bus_get_data(&hyper_extension);

	ret = hyper_msgpack_decode_device_id(device_id, hyper_extension.data, hyper_extension.data_size);
	LOG_HEXDUMP_INF(device_id, 6, "hyper_msgpack_decode_device_id:");
	if (ret)
	{
		LOG_ERR("hyper_msgpack_decode_device_id() failed with exit code: %d\n", ret);
		return ret;
	}

	hyper_extensions_registry_insert(reg, device_id, hyper_device_mcu_get_data, hyper_device_mcu_set_data);

	return ret;
};
