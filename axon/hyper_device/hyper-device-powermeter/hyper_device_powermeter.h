#ifndef __HYPER_DEVICE_POWERMETER_H__
#define __HYPER_DEVICE_POWERMETER_H__

#include <stdbool.h>
#include <stdint.h>
#include "hyper_device_utils.h"

bool hyper_device_powermeter_is_powermeter(uint32_t class_id);
int hyper_device_powermeter_init(hyper_device_reg_t *reg);

#define HYPER_DEVICE_POWERMETER 18

#endif
