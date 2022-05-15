#include <logging/log.h>
#include <kernel.h>
#include "axon.h"
#include "misc_utils.h"

#include "hyper_device_powermeter.h"

LOG_MODULE_REGISTER(hyper_device_powermeter, CONFIG_MAIN_LOG_LEVEL);
#include "hyper_device_18_v3.gen.h"

#include <modbus/modbus.h>

#define POWERMETER_DATA_READ_INTERVAL_MS 4000

typedef struct
{
	float data;
	bool is_set;
} powermeter_data_t;

powermeter_data_t voltage = {.is_set = 0};
powermeter_data_t current = {.is_set = 0};
powermeter_data_t power = {.is_set = 0};
powermeter_data_t apparent_power = {.is_set = 0};
powermeter_data_t reactive_power = {.is_set = 0};
powermeter_data_t power_factor = {.is_set = 0};
powermeter_data_t frequency = {.is_set = 0};
powermeter_data_t import_active_energy = {.is_set = 0};
powermeter_data_t export_active_energy = {.is_set = 0};
powermeter_data_t import_reactive_energy = {.is_set = 0};
powermeter_data_t export_reactive_energy = {.is_set = 0};
powermeter_data_t total_active_energy = {.is_set = 0};
powermeter_data_t total_reactive_energy = {.is_set = 0};

static int client_iface;

const static struct modbus_iface_param client_param = {
    .mode = MODBUS_MODE_RTU,
    .rx_timeout = 50000,
    .serial = {
	.baud = 9600,
	.parity = UART_CFG_PARITY_NONE,
    },
};

static float modbus_get_float_dcba(const uint16_t *src)
{
	float f;
	uint32_t i;
	uint8_t a, b, c, d;

	a = (src[0] >> 8) & 0xFF;
	b = (src[0] >> 0) & 0xFF;
	c = (src[1] >> 8) & 0xFF;
	d = (src[1] >> 0) & 0xFF;

	i = (a << 24) |
	    (b << 16) |
	    (c << 8) |
	    (d << 0);
	memcpy(&f, &i, 4);

	return f;
}

static int modbus_read_input_regs_float(float *data, const uint8_t unit_id, const uint16_t start_addr)
{
	uint16_t input_regs[2] = {0x00};

	int ret = modbus_read_input_regs(client_iface, unit_id, start_addr, input_regs, (sizeof(input_regs) / sizeof(uint16_t)));
	if (ret)
	{
		LOG_ERR("hyper_device_powermeter_get_data: failed to read 0x%x with %d", start_addr, ret);
	}
	else
	{
		LOG_HEXDUMP_DBG(input_regs, sizeof(input_regs),
				"WR|RD input register:");
		*data = modbus_get_float_dcba(input_regs);
	}

	return ret;
}

static int init_modbus_client(void)
{
	const char iface_name[] = {DT_PROP(DT_INST(0, zephyr_modbus_serial), label)};

	client_iface = modbus_iface_get_by_name(iface_name);

	return modbus_init_client(client_iface, client_param);
}

static int deinit_modbus_client(void)
{
	const char iface_name[] = {DT_PROP(DT_INST(0, zephyr_modbus_serial), label)};

	client_iface = modbus_iface_get_by_name(iface_name);

	return modbus_disable(client_iface);
}

static void powermeter_read_loop()
{
	int err;
	while (true)
	{
		err = init_modbus_client();
		if (err)
		{
			if (err != -22)
			{
				LOG_ERR("Modbus RTU client initialization failed err: %d", err);
				return;
			}
		}
		static uint8_t node = 0x02;

		// Voltage
		if (!modbus_read_input_regs_float(&(voltage.data), node, 0x0000))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fV", voltage.data);
			voltage.is_set = true;
		}
		else
		{
			voltage.is_set = false;
		}

		// Current
		if (!modbus_read_input_regs_float(&current.data, node, 0x0006))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fA", current.data);
			current.is_set = true;
		}
		else
		{
			current.is_set = false;
		}

		// power
		if (!modbus_read_input_regs_float(&power.data, node, 0x000C))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fW", power.data);
			power.is_set = true;
		}
		else
		{
			power.is_set = false;
		}

		// apparent_power
		if (!modbus_read_input_regs_float(&apparent_power.data, node, 0x0012))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fVA", apparent_power.data);
			apparent_power.is_set = true;
		}
		else
		{
			apparent_power.is_set = false;
		}

		// reactive_power
		if (!modbus_read_input_regs_float(&reactive_power.data, node, 0x0018))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fVAr", reactive_power.data);
			reactive_power.is_set = true;
		}
		else
		{
			reactive_power.is_set = false;
		}

		// power_factor
		if (!modbus_read_input_regs_float(&power_factor.data, node, 0x001E))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %f (Power Factor)", power_factor.data);
			power_factor.is_set = true;
		}
		else
		{
			power_factor.is_set = false;
		}

		// Frequency
		if (!modbus_read_input_regs_float(&frequency.data, node, 0x0046))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fHz", frequency.data);
			frequency.is_set = true;
		}
		else
		{
			frequency.is_set = false;
		}

		// import_active_energy
		if (!modbus_read_input_regs_float(&import_active_energy.data, node, 0x0048))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fkWh", import_active_energy.data);
			import_active_energy.is_set = true;
		}
		else
		{
			import_active_energy.is_set = false;
		}

		// export_active_energy
		if (!modbus_read_input_regs_float(&export_active_energy.data, node, 0x004A))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fkWh", export_active_energy.data);
			export_active_energy.is_set = true;
		}
		else
		{
			export_active_energy.is_set = false;
		}

		// import_reactive_energy
		if (!modbus_read_input_regs_float(&import_reactive_energy.data, node, 0x004C))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fkVArh", import_reactive_energy.data);
			import_reactive_energy.is_set = true;
		}
		else
		{
			import_reactive_energy.is_set = false;
		}

		// export_reactive_energy
		if (!modbus_read_input_regs_float(&export_reactive_energy.data, node, 0x004E))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fkVArh", export_reactive_energy.data);
			export_reactive_energy.is_set = true;
		}
		else
		{
			export_reactive_energy.is_set = false;
		}

		// total_active_energy
		if (!modbus_read_input_regs_float(&total_active_energy.data, node, 0x0156))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fkWh", total_active_energy.data);
			total_active_energy.is_set = true;
		}
		else
		{
			total_active_energy.is_set = false;
		}

		// total_reactive_energy
		if (!modbus_read_input_regs_float(&total_reactive_energy.data, node, 0x0158))
		{
			LOG_DBG("hyper_device_powermeter_get_data: %fkVArh", total_reactive_energy.data);
			total_reactive_energy.is_set = true;
		}
		else
		{
			total_reactive_energy.is_set = false;
		}

		deinit_modbus_client();
		k_msleep(POWERMETER_DATA_READ_INTERVAL_MS);
	}
}

K_THREAD_DEFINE(powermeter_get_data_tid, 500,
		powermeter_read_loop, NULL, NULL, NULL,
		5, 0, -1);

static hyper_device_18_t hyper_device_18 = {0};

static hyper_result_t hyper_device_powermeter_get_data(uint8_t *data, uint8_t *data_len)
{
	bool has_data = false;
	if (voltage.is_set)
	{
		hyper_device_18_set_voltage(&hyper_device_18, voltage.data);
		has_data = true;
	}

	if (current.is_set)
	{
		hyper_device_18_set_current(&hyper_device_18, current.data);
		has_data = true;
	}

	if (power.is_set)
	{
		hyper_device_18_set_power(&hyper_device_18, power.data);
		has_data = true;
	}

	if (apparent_power.is_set)
	{
		hyper_device_18_set_apparent_power(&hyper_device_18, apparent_power.data);
		has_data = true;
	}

	if (reactive_power.is_set)
	{
		hyper_device_18_set_reactive_power(&hyper_device_18, reactive_power.data);
		has_data = true;
	}

	if (power_factor.is_set)
	{
		hyper_device_18_set_power_factor(&hyper_device_18, power_factor.data);
		has_data = true;
	}

	if (frequency.is_set)
	{
		hyper_device_18_set_frequency(&hyper_device_18, frequency.data);
		has_data = true;
	}

	if (import_active_energy.is_set)
	{
		hyper_device_18_set_import_active_energy(&hyper_device_18, import_active_energy.data);
		has_data = true;
	}

	if (export_active_energy.is_set)
	{
		hyper_device_18_set_export_active_energy(&hyper_device_18, export_active_energy.data);
		has_data = true;
	}

	if (import_reactive_energy.is_set)
	{
		hyper_device_18_set_import_reactive_energy(&hyper_device_18, import_reactive_energy.data);
		has_data = true;
	}

	if (export_reactive_energy.is_set)
	{
		hyper_device_18_set_export_reactive_energy(&hyper_device_18, export_reactive_energy.data);
		has_data = true;
	}

	if (total_active_energy.is_set)
	{
		hyper_device_18_set_total_active_energy(&hyper_device_18, total_active_energy.data);
		has_data = true;
	}

	if (total_reactive_energy.is_set)
	{
		hyper_device_18_set_total_reactive_energy(&hyper_device_18, total_reactive_energy.data);
		has_data = true;
	}

	if (!has_data)
	{
		return 1;
	}

	// Pretty-print device.
	hyper_device_18_print(&hyper_device_18);

	// [SEND] Encode device.
	hyper_result_t res;

	res = hyper_device_18_encode(&hyper_device_18, data, data_len);
	if (res != HYPER_OK)
	{
		LOG_ERR("hyper_device_18_encode failed with error %d", res);
		return res;
	}
	LOG_INF("Encoding device data (size=%d)", *data_len);

	return res;
}

bool hyper_device_powermeter_is_powermeter(uint32_t class_id)
{

	if (class_id == HYPER_DEVICE_POWERMETER)
	{
		return true;
	};
	return false;
}

int hyper_device_powermeter_init(hyper_device_reg_t *reg)
{
	int ret = 0;
	uint8_t extension_uid[6] = {0};

	hyper_extension_eui48_read(extension_uid);
	if (ret < 0)
	{
		LOG_ERR("hyper_extension_eui48_read() failed with exit code: %d\n", ret);
		return ret;
	}

	hyper_device_18_init(&hyper_device_18, extension_uid);

	k_thread_start(powermeter_get_data_tid);

	hyper_extensions_registry_insert(reg, extension_uid, hyper_device_powermeter_get_data, NULL);

	return 0;
};
