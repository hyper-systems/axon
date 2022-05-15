#ifndef __HYPER_DEVICE_AS_FLO_H__
#define __HYPER_DEVICE_AS_FLO_H__

#include <stdbool.h>
#include <stdint.h>
#include "hyper_device_utils.h"

bool hyper_device_as_flo_is_as_flo(uint32_t class_id);
int hyper_device_as_flo_init(hyper_device_reg_t *reg);

#define HYPER_DEVICE_AS_FLO 20

#endif
