#include "hyper_device_utils.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(hyper_device_utils, CONFIG_MAIN_LOG_LEVEL);

void hyper_extensions_registry_insert(hyper_device_reg_t *reg, uint8_t *device_id,
                                      hyper_result_t (*get_data)(uint8_t *data, uint8_t *data_len),
                                      hyper_result_t (*set_data)(uint8_t *data, uint8_t data_len))
{
    uint8_t zeros[6] = {0};
    for (int i = 0; i < HYPER_EXTENSIONS_REGISTRY_MAX; i++)
    {
        // Add only if empty
        if (!memcmp(reg[i].device_id, zeros, sizeof(reg[i].device_id)))
        {
            // Add only if it doesnt exist yet
            if (memcmp(reg[i].device_id, device_id, sizeof(reg[i].device_id)))
            {
                memcpy(reg[i].device_id, device_id, sizeof(reg[i].device_id));
                reg[i].get_data = get_data;
                reg[i].set_data = set_data;
                LOG_HEXDUMP_DBG(device_id, 6, "device added to hyper_extensions_registry:");
                LOG_DBG("reg[i].get_data: %p\n", reg[i].get_data);
                LOG_DBG("reg[i].set_data: %p\n", reg[i].set_data);
                break;
            }
            else
            {
                LOG_ERR("extension already exists in hyper_extensions_registry, pos: %d\n", i);
                LOG_HEXDUMP_DBG(reg[i].device_id, 6, "reg[i].device_id");
                LOG_HEXDUMP_DBG(device_id, 6, "device_id:");
            }
        }
    }
}