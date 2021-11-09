#ifndef __HYPER_DEVICE_DATA_UTILS_H__
#define __HYPER_DEVICE_DATA_UTILS_H__

#include <stdint.h>

typedef struct
{
    uint8_t device_id[6];
    uint8_t (*get_data)(uint8_t *data);
    void (*set_data)(uint8_t *data, uint8_t data_len);
} hyper_device_reg_t;

#define HYPER_EXTENSIONS_REGISTRY_MAX 5

void hyper_extensions_registry_insert(hyper_device_reg_t *reg, uint8_t *device_id,
                                      uint8_t (*get_data)(uint8_t *data),
                                      void (*set_data)(uint8_t *data, uint8_t data_len));
int hyper_msgpack_extract_device_id(uint8_t *device_id, uint8_t *data, uint8_t data_len);

#endif