#ifndef __HYPER_DEVICE_HYDROKIT_H__
#define __HYPER_DEVICE_HYDROKIT_H__

#include <stdbool.h>
#include <stdint.h>
#include "hyper_device_utils.h"

bool hyper_device_hydrokit_is_hydrokit(uint32_t class_id);
int hyper_device_hydrokit_init(hyper_device_reg_t *reg);

#define HYPER_DEVICE_HYDROKIT 12

#endif