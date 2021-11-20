#include <logging/log.h>
#include "axon.h"
#include "controller.h"
#include "transport.h"
#include "hyper_device_axon.h"
#include "hyper_device_mcu.h"
#include "hyper_device_hydrokit.h"

#include "hyper_device_utils.h"

LOG_MODULE_REGISTER(controller, CONFIG_MAIN_LOG_LEVEL);

static hyper_device_reg_t hyper_extensions_registry[HYPER_EXTENSIONS_REGISTRY_MAX] = {0};

static uint8_t hyper_message_buffer[256] = {0};

static void hyper_controller_on_message_per_device_id_cb(uint8_t *device_id, uint8_t *data, uint8_t data_len)
{
    for (int i = 0; i < HYPER_EXTENSIONS_REGISTRY_MAX; i++)
    {
        if (!memcmp(hyper_extensions_registry[i].device_id, device_id, sizeof(device_id)))
        {
            if (hyper_extensions_registry[i].set_data != NULL)
            {
                if (hyper_extensions_registry[i].set_data(data, data_len) != HYPER_OK)
                {
                    return;
                }
                LOG_HEXDUMP_INF(device_id, 6, "Calling callback for:");
                return;
            }
            else
            {
                LOG_HEXDUMP_ERR(device_id, 6, "Callback is not set for:");
                return;
            }
        }
    }
    LOG_HEXDUMP_ERR(device_id, 6, "No callback found for:");
}

static void hyper_controller_on_message_handler(uint8_t *data, uint8_t data_len)
{

    // Only decode data if data received.
    if (data_len != 0)
    {
        uint8_t device_id[6];
        if (hyper_msgpack_decode_device_id(device_id, data, data_len))
        {
            LOG_ERR("hyper_msgpack_decode_device_id() failed! Ignoring...");
            return;
        }
        else
        {
            hyper_controller_on_message_per_device_id_cb(device_id, data, data_len);
        }
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
        if (hyper_device_hydrokit_is_hydrokit(extension_class_id))
        {
            ret = hyper_device_hydrokit_init(hyper_extensions_registry);
            if (ret)
            {
                LOG_ERR("hyper_device_hydrokit_init() failed with exit code: %d\n", ret);
                return ret;
            }
        }
        else
        {

            LOG_ERR("Extension detection error!");
            ret = 1;
        }
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

    // publish all extensions in the registry
    for (int i = 0; i < HYPER_EXTENSIONS_REGISTRY_MAX; i++)
    {
        if (hyper_extensions_registry[i].get_data != NULL)
        {
            // reset buffer
            memset(hyper_message_buffer, 0, sizeof(hyper_message_buffer));
            uint8_t data_len;
            if (hyper_extensions_registry[i].get_data(hyper_message_buffer, &data_len) == HYPER_OK)
            {
                if (data_len > 0)
                {
                    transport_publish(hyper_message_buffer, data_len);
                }
            }
        }
    }
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