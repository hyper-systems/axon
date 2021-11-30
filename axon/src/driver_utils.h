#ifndef __DRIVER_UTILS_H__
#define __DRIVER_UTILS_H__

#include <device.h>

int hyper_dev_init(struct device **devc, const char *label);
void hyper_macaddr_get(uint8_t *macaddr);
bool hyper_i2c_device_is_present(struct device *i2c_dev, uint16_t i2c_addr);
#endif
