#ifndef __HYPER_DEVICE_AS_CO2_H__
#define __HYPER_DEVICE_AS_CO2_H__

#include <stdbool.h>
#include <stdint.h>
#include "hyper_device_utils.h"

bool hyper_device_as_co2_is_as_co2(uint32_t class_id);
int hyper_device_as_co2_init(hyper_device_reg_t *reg);

#define HYPER_DEVICE_AS_CO2 14

#endif
