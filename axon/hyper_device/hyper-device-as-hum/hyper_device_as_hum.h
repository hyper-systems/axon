#ifndef __HYPER_DEVICE_AS_HUM_H__
#define __HYPER_DEVICE_AS_HUM_H__

#include <stdbool.h>
#include <stdint.h>
#include "hyper_device_utils.h"

bool hyper_device_as_hum_is_as_hum(uint32_t class_id);
int hyper_device_as_hum_init(hyper_device_reg_t *reg);

#define HYPER_DEVICE_AS_HUM 16

#endif
