/*
 * Copyright (c) 2021 Hyper Collective LTD
 */

#include <drivers/i2c.h>
#include <sys/util.h>

#define LOG_LEVEL CONFIG_ADC_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(adc_mcp342x);

#include "mcp342x.h"



#define BIT_RDY BIT(7)
#define BIT_MODE BIT(4)

#define POS_CHAN 5
#define POS_SR 2
#define POS_GAIN 0

#define MASK_VAL 3

#define SIGN12 0xfffff000
#define SIGN14 0xffffc000
#define SIGN16 0xffff0000
#define SIGN18 0xfffc0000

#define CHECK(x)           \
	do                     \
	{                      \
		int __;            \
		if ((__ = x) != 0) \
			return __;     \
	} while (0)
#define CHECK_ARG(VAL) \
	do                 \
	{                  \
		if (!(VAL))    \
			return -1; \
	} while (0)

static const uint32_t sample_time[] = {
	[MCP342X_RES_12] = 4167,
	[MCP342X_RES_14] = 16667,
	[MCP342X_RES_16] = 66667,
	[MCP342X_RES_18] = 266667};

static const float lsb[] = {
	[MCP342X_RES_12] = 0.001,
	[MCP342X_RES_14] = 0.00025,
	[MCP342X_RES_16] = 0.0000625,
	[MCP342X_RES_18] = 0.000015625};

static const int gain_val[] = {
	[MCP342X_GAIN1] = 1,
	[MCP342X_GAIN2] = 2,
	[MCP342X_GAIN4] = 4,
	[MCP342X_GAIN8] = 8};

static void get_cfg(mcp342x_t *dev, uint8_t reg)
{
	dev->mode = (reg & BIT_MODE) ? MCP342X_CONTINUOUS : MCP342X_ONESHOT;
	dev->channel = (reg >> POS_CHAN) & MASK_VAL;
	dev->resolution = (reg >> POS_SR) & MASK_VAL;
	dev->gain = (reg >> POS_GAIN) & MASK_VAL;
}

static inline uint8_t get_reg(mcp342x_t *dev)
{
	return (dev->mode == MCP342X_CONTINUOUS ? BIT_MODE : 0) | ((dev->channel & MASK_VAL) << POS_CHAN) | ((dev->resolution & MASK_VAL) << POS_SR) | ((dev->gain & MASK_VAL) << POS_GAIN);
}

int mcp342x_init_desc(mcp342x_t *dev, const char *label, uint8_t addr)
{
	CHECK_ARG(dev && addr >= MCP342X_ADDR_MIN && addr <= MCP342X_ADDR_MAX);
	dev->i2c_dev = device_get_binding(label);
	dev->i2c_addr = addr;

	if (dev->i2c_dev == NULL)
	{
		LOG_ERR("No device \"%s\" found; did initialization fail?", label);
		return 1;
	}
	else
	{
		LOG_INF("Found device \"%s\"", label);
		return 0;
	}

	// TODO: check if device is available by reading from it

	return 0;
}

int mcp342x_get_data(mcp342x_t *dev, int32_t *data, bool *ready)
{
	CHECK_ARG(dev);
	*data = 0;
	uint8_t buf[4] = {0};

	int ret = i2c_read(dev->i2c_dev, buf, sizeof(buf), dev->i2c_addr);
	if (ret)
	{
		LOG_ERR("i2c_read err (mcp342x_get_data): %d", ret);
		return -1;
	}

	uint8_t reg = buf[(buf[3] & (MCP342X_RES_18 << POS_SR)) == (MCP342X_RES_18 << POS_SR) ? 3 : 2];
	get_cfg(dev, reg);
	if (ready)
		*ready = !(reg & BIT_RDY);

	if (!data)
		return 0;

	uint32_t r = 0;
	switch (dev->resolution)
	{
	case MCP342X_RES_12:
		r = (buf[0] << 8) | buf[1];
		if (r & BIT(11))
			r |= SIGN12;
		break;
	case MCP342X_RES_14:
		r = (buf[0] << 8) | buf[1];
		if (r & BIT(13))
			r |= SIGN14;
		break;
	case MCP342X_RES_16:
		r = (buf[0] << 8) | buf[1];
		if (r & BIT(15))
			r |= SIGN16;
		break;
	case MCP342X_RES_18:
		r = (buf[0] << 16) | (buf[1] << 8) | buf[2];
		if (r & BIT(17))
			r |= SIGN18;
		break;
	}

	*data = *((int32_t *)&r);

	return 0;
}

int mcp342x_get_voltage(mcp342x_t *dev, float *volts, bool *ready)
{
	CHECK_ARG(volts);

	int32_t raw = 0;
	CHECK(mcp342x_get_data(dev, &raw, ready));
	*volts = lsb[dev->resolution] * raw / gain_val[dev->gain];

	return 0;
}

float mcp342x_raw_to_voltage(mcp342x_t *dev, int32_t raw)
{
	return lsb[dev->resolution] * raw / gain_val[dev->gain];
}

int mcp342x_get_sample_time_us(mcp342x_t *dev, uint32_t *us)
{
	CHECK_ARG(dev && us && dev->resolution <= MCP342X_RES_18);

	*us = sample_time[dev->resolution];
	return 0;
}

int mcp342x_set_config(mcp342x_t *dev)
{
	CHECK_ARG(dev);

	uint8_t r = get_reg(dev);

	int ret = i2c_write(dev->i2c_dev, &r, 1, dev->i2c_addr);
	if (ret)
	{
		LOG_ERR("i2c_write err (mcp342x_set_config): %d", ret);
		return -1;
	}

	return 0;
}

int mcp342x_get_config(mcp342x_t *dev)
{
	return mcp342x_get_data(dev, NULL, NULL);
}

int mcp342x_start_conversion(mcp342x_t *dev)
{
	CHECK_ARG(dev);

	uint8_t r = get_reg(dev) | BIT_RDY;

	int ret = i2c_write(dev->i2c_dev, &r, 1, dev->i2c_addr);
	if (ret)
	{
		LOG_ERR("i2c_write err (mcp342x_start_conversion): %d", ret);
		return -1;
	}

	return 0;
}

int mcp342x_oneshot_conversion(mcp342x_t *dev, int32_t *data)
{
	CHECK_ARG(dev && data);
	*data = 0;

	dev->mode = MCP342X_ONESHOT;

	uint32_t st;
	CHECK(mcp342x_get_sample_time_us(dev, &st));
	CHECK(mcp342x_start_conversion(dev));

	k_usleep(st + 1);
	bool ready;
	CHECK(mcp342x_get_data(dev, data, &ready));
	if (!ready)
	{
		LOG_ERR("Data not ready");
		return -1;
	}

	return 0;
}

int mcp342x_average_oneshot_conversion(mcp342x_t *dev, int32_t *data, int32_t num_readings)
{
	*data = 0;

	int32_t readings[num_readings]; // the readings from the analog input
	int32_t total = 0;				// the running total
	int ret = 0;

	for (int i = 0; i < num_readings; i++)
	{
		readings[i] = 0;
	}

	for (int i = 0; i < num_readings; i++)
	{
		ret = mcp342x_oneshot_conversion(dev, &readings[i]);
		if (ret)
		{
			LOG_ERR("mcp342x_oneshot_conversion err: %d", ret);
			return ret;
		}
		total = total + readings[i];
	}

	// write the average
	*data = total / num_readings;

	return ret;
}

int mcp342x_average_oneshot_conversion_volt(mcp342x_t *dev, float *adc_volt, int32_t num_readings)
{
	int ret = 0;
	int32_t adc_raw_data;

	ret = mcp342x_average_oneshot_conversion(dev, &adc_raw_data, num_readings);
	if (ret)
	{
		LOG_ERR("mcp342x_average_oneshot_conversion err: %d", ret);
		return -1;
	}
	else
	{
		LOG_INF("mcp342x_oneshot_conversion: %d", adc_raw_data);
		*adc_volt = mcp342x_raw_to_voltage(dev, adc_raw_data);
		LOG_INF("mcp342x_oneshot_conversion (Volts): %f", *adc_volt);
		return ret;
	}
}
