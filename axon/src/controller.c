#include <logging/log.h>
#include "axon.h"
#include "controller.h"
#include "transport.h"
#include "hyper_device_axon.h"
#include "hyper_device_mcu.h"
#ifdef CONFIG_HYPER_DEVICE_HYDROKIT
#include "hyper_device_hydrokit.h"
#endif
#ifdef CONFIG_HYPER_DEVICE_AS_CO2
#include "hyper_device_as_co2.h"
#endif
#ifdef CONFIG_HYPER_DEVICE_AS_HUM
#include "hyper_device_as_hum.h"
#endif
#ifdef CONFIG_HYPER_DEVICE_AS_FLO
#include "hyper_device_as_flo.h"
#endif
#ifdef CONFIG_HYPER_DEVICE_POWERMETER
#include "hyper_device_powermeter.h"
#endif

#include "hyper_device_utils.h"

LOG_MODULE_REGISTER(controller, CONFIG_MAIN_LOG_LEVEL);

static hyper_device_reg_t hyper_extensions_registry[HYPER_EXTENSIONS_REGISTRY_MAX] = {0};

static uint8_t hyper_message_buffer[256] = {0};

static void hyper_controller_message_processor(uint32_t device_class_id,
					       uint8_t device_id[HYPER_DEVICE_ID_LENGTH], uint8_t *data, uint8_t data_len)
{

	// Only decode data if data received.
	if (data_len != 0)
	{
		for (int i = 0; i < HYPER_EXTENSIONS_REGISTRY_MAX; i++)
		{
			if (!memcmp(hyper_extensions_registry[i].device_id, device_id, HYPER_DEVICE_ID_LENGTH))
			{
				if (hyper_extensions_registry[i].set_data != NULL)
				{
					if (hyper_extensions_registry[i].set_data(data, data_len) != HYPER_OK)
					{
						return;
					}
					LOG_HEXDUMP_INF(device_id, 6, "Called callback for:");
					return;
				}
				else
				{
					LOG_HEXDUMP_ERR(device_id, 6, "Called is not set for:");
					return;
				}
			}
		}
		LOG_HEXDUMP_ERR(device_id, 6, "No callback found for:");
	}
}

static void hyper_controller_on_message_handler(uint8_t *data, uint8_t data_len)
{
	if (HYPER_OK !=
	    hyper_msgpack_process_multimessage(hyper_controller_message_processor, data, data_len))
	{
		LOG_ERR("hyper_msgpack_process_multimessage() failed!");
		return;
	}
}

static int hyper_controller_extensions_registry_update()
{
	int ret = 0;

	// reset hyper_extensions_registry after the first (Axon)
	for (int i = 1; i < HYPER_EXTENSIONS_REGISTRY_MAX; i++)
	{
		memset(hyper_extensions_registry[i].device_id, 0,
		       sizeof(hyper_extensions_registry[i].device_id));
		hyper_extensions_registry[i].get_data = NULL;
		hyper_extensions_registry[i].set_data = NULL;
	}

	// detect and init extensions
	hyper_extension_type_t hyper_extension_type = hyper_extension_get_type();
	if (hyper_extension_type == HYPER_EXTENSION_EEPROM)
	{
		uint32_t extension_class_id = 0;
		hyper_extension_class_id_read(&extension_class_id);
#ifdef CONFIG_HYPER_DEVICE_HYDROKIT
		if (hyper_device_hydrokit_is_hydrokit(extension_class_id))
		{
			ret = hyper_device_hydrokit_init(hyper_extensions_registry);
			if (ret)
			{
				LOG_ERR("hyper_device_hydrokit_init() failed with exit code: %d\n", ret);
				return ret;
			}
		}
#endif
#ifdef CONFIG_HYPER_DEVICE_AS_CO2
		if (hyper_device_as_co2_is_as_co2(extension_class_id))
		{
			ret = hyper_device_as_co2_init(hyper_extensions_registry);
			if (ret)
			{
				LOG_ERR("hyper_device_as_co2_init() failed with exit code: %d\n", ret);
				return ret;
			}
		}
#endif
#ifdef CONFIG_HYPER_DEVICE_AS_HUM
		if (hyper_device_as_hum_is_as_hum(extension_class_id))
		{
			ret = hyper_device_as_hum_init(hyper_extensions_registry);
			if (ret)
			{
				LOG_ERR("hyper_device_as_hum_init() failed with exit code: %d\n", ret);
				return ret;
			}
		}
#endif
		LOG_ERR("Extension detection error!");
		ret = 1;
#ifdef CONFIG_HYPER_DEVICE_AS_FLO
		else if (hyper_device_as_flo_is_as_flo(extension_class_id))
		{
			ret = hyper_device_as_flo_init(hyper_extensions_registry);
			if (ret)
			{
				LOG_ERR("hyper_device_as_flo_init() failed with exit code: %d\n", ret);
				return ret;
			}
		}
#endif
#ifdef CONFIG_HYPER_DEVICE_POWERMETER
		else if (hyper_device_powermeter_is_powermeter(extension_class_id))
		{
			ret = hyper_device_powermeter_init(hyper_extensions_registry);
			if (ret)
			{
				LOG_ERR("hyper_device_powermeter_init() failed with exit code: %d\n", ret);
				return ret;
			}
		}
#endif
	}
	if (hyper_extension_type == HYPER_EXTENSION_MCU)
	{
		ret = hyper_device_mcu_init(hyper_extensions_registry);
		if (ret)
		{
			LOG_ERR("hyper_device_mcu_init() failed with exit code: %d\n", ret);
			return ret;
		}
	}
	else
	{
		LOG_INF("No Extension detected!");
		ret = 0;
	}

	return ret;
}

static void hyper_controller_publish()
{
	// detect devices in bus and update registry
	hyper_controller_extensions_registry_update();
	uint8_t total_data_len = 0;
	// publish all extensions in the registry
	for (int i = 0; i < HYPER_EXTENSIONS_REGISTRY_MAX; i++)
	{
		if (hyper_extensions_registry[i].get_data != NULL)
		{
			uint8_t data_len = 0;
			if (hyper_extensions_registry[i].get_data(hyper_message_buffer + total_data_len, &data_len) == HYPER_OK)
			{
				if (data_len > 0)
				{
					total_data_len += data_len;
				}
			}
		}
	}
	transport_publish(hyper_message_buffer, total_data_len);
}

int hyper_controller_init()
{
	int ret = 0;
	// init transport
	ret = transport_init(hyper_controller_on_message_handler);
	if (ret)
	{
		LOG_ERR("transport_init() failed with exit code: %d\n", ret);
		return ret;
	}

	// init axon_publish mechanism
	axon_publish_interval_callback_set(hyper_controller_publish);
	axon_publish_interval_set(AXON_PUBLISH_INTERVAL_DEFAULT_SEC);

	// init hyper axon device
	ret = hyper_device_axon_init(hyper_extensions_registry);
	if (ret)
	{
		LOG_ERR("hyper_device_axon_init() failed with exit code: %d\n", ret);
		return ret;
	}

	// detect and init extensions
	ret = hyper_controller_extensions_registry_update();
	if (ret)
	{
		LOG_ERR("hyper_controller_extensions_registry_update() failed with exit code: %d\n", ret);
		return ret;
	}

	// TODO: init a task that pings the gateway each X minutes, reboots if not reachable

	return ret;
}
