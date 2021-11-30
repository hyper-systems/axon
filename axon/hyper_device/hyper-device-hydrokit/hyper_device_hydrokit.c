#include <logging/log.h>
#include "axon.h"
#include "hyper_device_hydrokit.h"
#include "mcp342x.h"
#include <math.h>

LOG_MODULE_REGISTER(hyper_device_hydrokit, CONFIG_MAIN_LOG_LEVEL);
#include "hyper_device_13_v3.gen.h"

static hyper_device_13_t hyper_device_13 = {0};

const uint32_t hydrokit_class_ids[] = {9, 13};

#define HYPER_EXTENSION_DATA_PH_OFFSET 0x04
#define HYPER_EXTENSION_DATA_ORP_OFFSET 0x08
#define HYPER_EXTENSION_DATA_EC_OFFSET 0x00
#define HYPER_EXTENSION_DATA_RTD_OFFSET 0x0c

#define ADC_GAIN MCP342X_GAIN1
#define ADC_RESOLUTION MCP342X_RES_12
#define ADC_READINGS 120

mcp342x_t adc_ph_dev = {
    .channel = MCP342X_CHANNEL1,
    .gain = MCP342X_GAIN1,
    .resolution = MCP342X_RES_12,
    .mode = MCP342X_ONESHOT};
mcp342x_t adc_ec_dev = {
    .channel = MCP342X_CHANNEL1,
    .gain = MCP342X_GAIN2,
    .resolution = MCP342X_RES_12,
    .mode = MCP342X_ONESHOT};
mcp342x_t adc_orp_dev = {
    .channel = MCP342X_CHANNEL1,
    .gain = MCP342X_GAIN1,
    .resolution = MCP342X_RES_12,
    .mode = MCP342X_ONESHOT};
mcp342x_t adc_temp_dev = {
    .channel = MCP342X_CHANNEL1,
    .gain = MCP342X_GAIN1,
    .resolution = MCP342X_RES_12,
    .mode = MCP342X_ONESHOT};

static int adc_init(mcp342x_t *adc_dev, uint8_t i2c_addr)
{
	int ret = 0;

	ret = mcp342x_init_desc(adc_dev, "I2C_1", i2c_addr);

	if (ret)
	{
		LOG_ERR("mcp342x_init_desc err: %d", ret);
		return -1;
	}
	ret = mcp342x_set_config(adc_dev);
	if (ret)
	{
		LOG_ERR("mcp342x_set_config err: %d", ret);
		return -1;
	}

	return ret;
}

static int hydrokit_ph_calib_data_read(float *data)
{
	return hyper_extension_data_read((uint8_t *)data,
					 sizeof(float), HYPER_EXTENSION_DATA_PH_OFFSET);
}

static int hydrokit_orp_calib_data_read(float *data)
{
	return hyper_extension_data_read((uint8_t *)data,
					 sizeof(float), HYPER_EXTENSION_DATA_ORP_OFFSET);
}

static int hydrokit_ec_calib_data_read(float *data)
{
	return hyper_extension_data_read((uint8_t *)data,
					 sizeof(float), HYPER_EXTENSION_DATA_EC_OFFSET);
}

__unused static int hydrokit_rtd_calib_data_read(float *data)
{
	return hyper_extension_data_read((uint8_t *)data,
					 sizeof(float), HYPER_EXTENSION_DATA_RTD_OFFSET);
}

static int hydrokit_read_ph(float *ph_volt)
{
	int ret = 0;
	ret = mcp342x_average_oneshot_conversion_volt(&adc_ph_dev, ph_volt, ADC_READINGS);
	if (ret)
	{
		LOG_ERR("mcp342x_average_oneshot_conversion_volt() for pH failed with exit code: %d\n", ret);
		return ret;
	}
	// Read calibration data stored in the eeprom
	float calib_data = 0;
	ret = hydrokit_ph_calib_data_read(&calib_data);
	if (ret)
	{
		LOG_ERR("hydrokit_ph_calib_data_read() failed with exit code: %d\n", ret);
		return ret;
	}
	LOG_INF("hydrokit_ph_calib_data_read(): %fV", calib_data);

	// Remove 1.25V of voltage reference
	*ph_volt = *ph_volt - 1.25;

	// Subtract error
	*ph_volt = *ph_volt - calib_data;

	return ret;
}

static int hydrokit_read_ec(float *ec_gain)
{
	int ret = 0;
	float ec_raw;

	ret = mcp342x_average_oneshot_conversion_volt(&adc_ec_dev, &ec_raw, ADC_READINGS);
	if (ret)
	{
		LOG_ERR("mcp342x_average_oneshot_conversion_volt() for EC failed with exit code: %d\n", ret);
		return ret;
	}

	LOG_INF("EC raw_value: %fV", ec_raw);
	// Read calibration data stored in the eeprom
	float ec_disconnected;
	ret = hydrokit_ec_calib_data_read(&ec_disconnected);
	if (ret)
	{
		LOG_ERR("hydrokit_ec_calib_data_read() failed with exit code: %d\n", ret);
		return ret;
	}
	LOG_INF("EC disconnected val: %f", ec_disconnected);

	*ec_gain = ec_raw / ec_disconnected;
	LOG_INF("EC gain val: %f", *ec_gain);

	return ret;
}

static int hydrokit_read_orp(float *orp_volt)
{
	int ret = 0;
	ret = mcp342x_average_oneshot_conversion_volt(&adc_orp_dev, orp_volt, ADC_READINGS);
	if (ret)
	{
		LOG_ERR("mcp342x_average_oneshot_conversion_volt() for ORP failed with exit code: %d\n", ret);
		return ret;
	}

	// Read calibration data stored in the eeprom
	float calib_data = 0;
	ret = hydrokit_orp_calib_data_read(&calib_data);
	if (ret)
	{
		LOG_ERR("hydrokit_orp_calib_data_read() failed with exit code: %d\n", ret);
		return ret;
	}
	LOG_INF("hydrokit_orp_calib_data_read(): %fV", calib_data);

	// Remove 1.25V of voltage reference
	*orp_volt = *orp_volt - 1.25;

	// Subtract error
	*orp_volt = *orp_volt - calib_data;

	return ret;
}

// TODO: rework all the RTD calculations, we are not doing it right
static int hydrokit_read_temp(float *temp_c_deg)
{
	int ret = 0;
	float rtd_volt;
	ret = mcp342x_average_oneshot_conversion_volt(&adc_temp_dev, &rtd_volt, ADC_READINGS);
	if (ret)
	{
		LOG_ERR("mcp342x_average_oneshot_conversion_volt() for ORP failed with exit code: %d\n", ret);
		return ret;
	}

	LOG_INF("RTD RAW: %fV", rtd_volt);

	// // Read calibration data stored in the eeprom
	// float calib_data = 0;
	// ret = hydrokit_rtd_calib_data_read(&calib_data);
	// if (ret)
	// {
	// 	LOG_ERR("hydrokit_rtd_calib_data_read() failed with exit code: %d\n", ret);
	// 	return ret;
	// }
	// LOG_INF("hydrokit_rtd_calib_data_read(): %fV", calib_data);
	// // Subtract error
	// rtd_volt = rtd_volt - calib_data;

	// Vout = Vin*(Rptc/(Rptc+R1))  =>  Rptc = -(R1*Vout/(Vout-Vin))
	// R1 = 1000
	// Vin = 1.25
	uint16_t R1 = 1000;
	float Vin = 1.25;
	float Rrtd = -(R1 * rtd_volt / (rtd_volt - Vin));
	LOG_INF("RTD Vout: %fV", Rrtd);

	// Rrtd to Temp (°C) from:
	// https://github.com/resol-de/RESOLino_Pt1000/blob/21dc220e7176b5ef59a1056476c4b8ede3f6fd41/RESOLino_Pt1000/RESOLino_Pt1000.cpp#L107
	*temp_c_deg = (3383.81f - 1315.9f * sqrt(7.61247f - Rrtd / 1000));
	LOG_INF("RTD Temp: %f °C", *temp_c_deg);

	return ret;
}

static hyper_result_t hyper_device_hydrokit_get_data(uint8_t *data, uint8_t *data_len)
{
	float ph_volt, ec_gain, orp_volt, temp_c_deg;
	int ret = 0;

	ret = hydrokit_read_ph(&ph_volt);
	if (ret)
	{
		LOG_ERR("hydrokit_read_ph() for pH failed with exit code: %d\n", ret);
	}
	else
	{
		hyper_device_13_set_raw_ph_adc_value(&hyper_device_13, ph_volt);
	}

	ret = hydrokit_read_ec(&ec_gain);
	if (ret)
	{
		LOG_ERR("hydrokit_read_ec() for EC failed with exit code: %d\n", ret);
	}
	else
	{
		hyper_device_13_set_raw_ec_adc_value(&hyper_device_13, ec_gain);
	}

	ret = hydrokit_read_orp(&orp_volt);
	if (ret)
	{
		LOG_ERR("hydrokit_read_orp() for ORP failed with exit code: %d\n", ret);
	}
	else
	{
		hyper_device_13_set_raw_orp_adc_value(&hyper_device_13, orp_volt);
	}

	ret = hydrokit_read_temp(&temp_c_deg);
	if (ret)
	{
		LOG_ERR("hydrokit_read_temp() for Temperature failed with exit code: %d\n", ret);
	}
	else
	{
		hyper_device_13_set_water_temperature(&hyper_device_13, temp_c_deg);
	}

	// Pretty-print device.
	hyper_device_13_print(&hyper_device_13);

	// [SEND] Encode device.
	hyper_result_t res;

	res = hyper_device_13_encode(&hyper_device_13, data, data_len);
	if (ret != HYPER_OK)
	{
		LOG_ERR("hyper_device_11_encode failed with error %d", ret);
		return ret;
	}
	LOG_INF("Encoding device data (size=%d)", *data_len);

	return res;
}

bool hyper_device_hydrokit_is_hydrokit(uint32_t class_id)
{
	int i;
	for (i = 0; i < sizeof(hydrokit_class_ids); i++)
	{
		if (hydrokit_class_ids[i] == class_id)
		{
			return true;
		}
	}
	return true;
}

int hyper_device_hydrokit_init(hyper_device_reg_t *reg)
{
	int ret = 0;
	uint8_t extension_uid[6] = {0};

	hyper_extension_eui48_read(extension_uid);
	if (ret < 0)
	{
		LOG_ERR("hyper_extension_eui48_read() failed with exit code: %d\n", ret);
		return ret;
	}

	hyper_device_13_init(&hyper_device_13, extension_uid);

	// init ADCs
	adc_init(&adc_ph_dev, MCP342X_ADDR_MIN);
	adc_init(&adc_ec_dev, MCP342X_ADDR_MIN + 1);
	adc_init(&adc_orp_dev, MCP342X_ADDR_MIN + 2);
	adc_init(&adc_temp_dev, MCP342X_ADDR_MIN + 3);

	hyper_extensions_registry_insert(reg, extension_uid, hyper_device_hydrokit_get_data, NULL);

	return 0;
};
