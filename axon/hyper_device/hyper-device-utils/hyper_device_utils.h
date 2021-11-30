#ifndef __HYPER_DEVICE_UTILS__
#define __HYPER_DEVICE_UTILS__

#include <stdint.h>
#include "hyper_device_core_v3.h"

typedef struct
{
	uint8_t device_id[6];
	hyper_result_t (*get_data)(uint8_t *data, uint8_t *data_len);
	hyper_result_t (*set_data)(uint8_t *data, uint8_t data_len);
} hyper_device_reg_t;

#define HYPER_EXTENSIONS_REGISTRY_MAX 5

void hyper_extensions_registry_insert(hyper_device_reg_t *reg, uint8_t *device_id,
				      hyper_result_t (*get_data)(uint8_t *data, uint8_t *data_len),
				      hyper_result_t (*set_data)(uint8_t *data, uint8_t data_len));

#endif
