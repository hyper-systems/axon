#include <zephyr.h>
#include <logging/log.h>
#include <drivers/i2c.h>
#include "driver_utils.h"

LOG_MODULE_REGISTER(driver_utils, CONFIG_COAP_CLIENT_LOG_LEVEL);

int hyper_dev_init(struct device **devc, const char *label)
{
	*devc = (struct device *)device_get_binding(label);

	if (*devc == NULL)
	{
		LOG_ERR("No device \"%s\" found; did initialization fail?", label);
		return 1;
	}
	else
	{
		LOG_INF("Found device \"%s\"", label);
		return 0;
	}
}

#if defined(CONFIG_SOC_SERIES_NRF52X)
static void nrf5_ieee_eui64_read(uint8_t *ieee_eui64)
{
	uint64_t factory_address;
	uint32_t index = 0;
	// By default, the device uses Nordic Semiconductor's MA-L (fa-ce-36). It can be modified by overwriting
	const uint32_t nordic_oui = 0xf4ce36;

	// Set the MAC Address Block Larger (MA-L) formerly called OUI.
	ieee_eui64[index++] = (nordic_oui >> 16) & 0xff;
	ieee_eui64[index++] = (nordic_oui >> 8) & 0xff;
	ieee_eui64[index++] = nordic_oui & 0xff;

	// Use device identifier assigned during the production.
	factory_address = (uint64_t)NRF_FICR->DEVICEID[0] << 32;
	factory_address |= NRF_FICR->DEVICEID[1];
	memcpy(ieee_eui64 + index, &factory_address, sizeof(factory_address) - index);
}
#endif

void hyper_macaddr_get(uint8_t *macaddr)
{
	uint8_t eui64[8];
#if defined(CONFIG_SOC_SERIES_NRF52X)
	nrf5_ieee_eui64_read(eui64);
	// get the last 6 bytes, ignoring the first two bytes of Nordic Semiconductor's MA-L
	memcpy(macaddr, &eui64[2], 6);
#else
#error hyper_macaddress_read unsupported by this target device
#endif
}

bool hyper_i2c_device_is_present(struct device *i2c_dev, uint16_t i2c_addr)
{
	struct i2c_msg msgs[1];
	uint8_t dst;
	/* Send the address to read from */
	msgs[0].buf = &dst;
	msgs[0].len = 0U;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;
	if (i2c_transfer(i2c_dev, &msgs[0], 1, i2c_addr) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
