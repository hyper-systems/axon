#include <logging/log.h>
#include <kernel.h>
#include "hyper_device_mcu.h"
#include "axon.h"
#include "misc_utils.h"

LOG_MODULE_REGISTER(hyper_device_mcu, CONFIG_MAIN_LOG_LEVEL);

#define EXT_I2C_ADDR 0x05
#define EXT_MEASURE_CMD 0x01
#define EXT_GET_DATA_CMD 0x02

#define EXT_MEASURE_NOT_READY 0x01
#define EXT_MEASURE_READY 0x02
#define EXT_READ_RETRIES 3

#define HYPER_EXTENSION_MAX_SIZE 64

typedef enum
{
	EXT_STATUS_READY,
	EXT_STATUS_MEASURING_READY,
	EXT_STATUS_MEASURING_WAITING,
	EXT_STATUS_READ,
	EXT_STATUS_MEASURING_ERROR,
	EXT_STATUS_READ_ERROR,
	EXT_STATUS_WRITE_ERROR,
	EXT_STATUS_BUS_ERROR,
	EXT_STATUS_NOT_PRESENT
} ext_ret_t;

typedef struct
{
	int status;
	uint8_t read_retries;
	uint8_t data[HYPER_EXTENSION_MAX_SIZE];
	uint8_t data_size;
} hyper_extension_t;

hyper_extension_t hyper_extension = {0};

static int ext_measure_status_get()
{

	uint8_t measure_status = 0;
	uint8_t w_buff = EXT_MEASURE_CMD;
	int ret;

	// Send measure command to get extension Hyper data size
	ret = hyper_extension_bus_i2c_write(&w_buff, 1, EXT_I2C_ADDR);
	if (ret != 0)
	{
		LOG_DBG("measure i2c write error: %d", ret);
		return -1;
	}
	// Read
	ret = hyper_extension_bus_i2c_read(&measure_status, 1, EXT_I2C_ADDR);
	if (ret != 0)
	{
		LOG_DBG("measure i2c read error: %d", ret);
		return -1;
	}

	if (measure_status == EXT_MEASURE_READY)
	{
		return measure_status;
	}
	else if (measure_status == EXT_MEASURE_NOT_READY)
	{
		LOG_ERR("EXT_MEASURE_NOT_READY measure status: %02x", measure_status);
		return measure_status;
	}
	else
	{
		LOG_ERR("invalid measure status: %02x", measure_status);
		return -1;
	}
}

static int ext_data_size_get()
{

	uint8_t ext_dev_size = 0;
	uint8_t w_buff = EXT_GET_DATA_CMD;
	int ret;

	// Send measure command to get extension Hyper data size
	ret = hyper_extension_bus_i2c_write(&w_buff, 1, EXT_I2C_ADDR);
	if (ret != 0)
	{
		LOG_DBG("WTF write: %d", ret);
		return -1;
	}
	// Read
	ret = hyper_extension_bus_i2c_read(&ext_dev_size, 1, EXT_I2C_ADDR);
	if (ret != 0)
	{
		LOG_DBG("WTF read: %d", ret);
		return -1;
	}

	if ((ext_dev_size > HYPER_EXTENSION_MAX_SIZE) || (ext_dev_size == 0))
	{
		LOG_ERR("invalid ext_data_size: %d", ext_dev_size);
		return -1;
	}
	else
	{
		LOG_DBG("ext_data_size: %d", ext_dev_size);
	}

	return ext_dev_size;
}

static int ext_data_get(uint8_t *data, uint8_t size)
{
	int ret;
	// Return dev data as a command to read extension data
	ret = hyper_extension_bus_i2c_write(&size, 1, EXT_I2C_ADDR);
	if (ret != 0)
	{
		return -1;
	}
	ret = hyper_extension_bus_i2c_read(data, size, EXT_I2C_ADDR);
	if (ret != 0)
	{
		return -1;
	}

	return 0;
}

static int ext_read()
{
	hyper_extension.data_size = 0;

	switch (ext_measure_status_get())
	{
	case EXT_MEASURE_READY:
		hyper_extension.status = EXT_STATUS_MEASURING_READY;
		hyper_extension.data_size = ext_data_size_get();
		if (hyper_extension.data_size < 0)
		{
			LOG_ERR("Cannot init read extension '%u' size! Continuing...", hyper_extension.data_size);
			hyper_extension.status = EXT_STATUS_READ_ERROR;
		}
		else
		{
			if (ext_data_get(hyper_extension.data, hyper_extension.data_size))
			{
				LOG_ERR("Cannot init read extension data! Continuing...");
				hyper_extension.status = EXT_STATUS_READ_ERROR;
			}
			else
			{
				hyper_extension.status = EXT_STATUS_READ;
			}
		}
		break;
	case EXT_MEASURE_NOT_READY:
		hyper_extension.status = EXT_STATUS_MEASURING_WAITING;
		break;
	case -1:
		hyper_extension.status = EXT_STATUS_MEASURING_ERROR;
		break;
	default:
		break;
	}

	return hyper_extension.status;
}

static int ext_data_put(uint8_t *data, uint8_t size)
{
	int ret;
	// Return dev data as a command to read extension data
	ret = hyper_extension_bus_i2c_write(data, size, EXT_I2C_ADDR);
	if (ret != 0)
	{
		return -1;
	}

	return 0;
}

static void extension_read_task()
{
	if ((hyper_extension.read_retries < EXT_READ_RETRIES) && (hyper_extension.status < EXT_STATUS_READ))
	{
		int ret = ext_read();
		if (ret == EXT_STATUS_READ)
		{
			LOG_INF("EXT STATUS READ: %d", hyper_extension.status);
			hyper_extension.read_retries = EXT_READ_RETRIES;
		}
		else if (ret == EXT_STATUS_MEASURING_WAITING)
		{
			LOG_INF("EXT STATUS WAIT: %d", hyper_extension.status);
			hyper_extension.read_retries++;
		}
		else
		{
			// in case of error
			LOG_INF("EXT ERROR: %d", hyper_extension.status);
			hyper_extension.read_retries = EXT_READ_RETRIES;
		}
	}
}

static int extension_read_loop(void)
{

	hyper_extension.read_retries = 0;
	hyper_extension.status = EXT_STATUS_READY;

	while (hyper_extension.read_retries < EXT_READ_RETRIES)
	{
		extension_read_task(&hyper_extension);
		LOG_INF("LOOP: %u ext_read_retries:%u", hyper_extension.read_retries, EXT_READ_RETRIES);
		k_sleep(K_MSEC(500));
	}

	return hyper_extension.status;
}

void hyper_device_mcu_set_data(uint8_t *data, uint8_t data_len)
{
	LOG_INF("raw_msg_without_header (len: %u): ", data_len);
	hyper_hexdump(data, data_len);

	uint8_t ext_data_size = ext_data_size_get();
	if (ext_data_size <= 0)
	{
		LOG_ERR("Cannot init read extension size! Continuing...");
	}
	else
	{
		if (ext_data_put(data, data_len) < 0)
			LOG_ERR("Couldn't write extension data! Continuing...");
		else
		{
			LOG_INF("Data written to MCU extension!");
		}
	}
}

uint8_t hyper_device_mcu_get_data(uint8_t *data)
{
	uint8_t message_len = 0;

	extension_read_loop();

	if (hyper_extension.status == EXT_STATUS_READ)
	{
		memcpy(data, hyper_extension.data, hyper_extension.data_size);
		LOG_INF("Got extension data (size=%u)!", hyper_extension.data_size);
		message_len = hyper_extension.data_size;
	}

	return message_len;
}

int hyper_device_mcu_init(hyper_device_reg_t *reg)
{
	int ret = 0;
	uint8_t device_id[8];

	// extract device id from extension payload
	extension_read_loop();
	ret = hyper_msgpack_extract_device_id(device_id, hyper_extension.data, hyper_extension.data_size);
    LOG_HEXDUMP_INF(device_id, 6, "hyper_msgpack_extract_device_id:");
    if (ret)
    {
        LOG_ERR("hyper_msgpack_extract_device_id() failed with exit code: %d\n", ret);
        return ret;
    }

	hyper_extensions_registry_insert(reg, device_id, hyper_device_mcu_get_data, hyper_device_mcu_set_data);

	return ret;
};
