#ifndef __MCP342X_H__
#define __MCP342X_H__
#include <stdbool.h>
#include <stdint.h>
#define MCP342X_ADDR_MIN 0x68
#define MCP342X_ADDR_MAX 0x6f

/**
 * Device operation mode
 */
typedef enum
{
	MCP342X_ONESHOT = 0, //!< One-shot conversion mode
	MCP342X_CONTINUOUS   //!< Continuous conversions mode, default
} mcp342x_mode_t;

/**
 * Input channel
 */
typedef enum
{
	MCP342X_CHANNEL1 = 0, //!< Channel 1, default
	MCP342X_CHANNEL2,     //!< Channel 2
	MCP342X_CHANNEL3,     //!< Channel 3 (MCP3428 only, treated as channel 1 by the MCP3426/MCP3427)
	MCP342X_CHANNEL4      //!< Channel 4 (MCP3428 only, treated as channel 2 by the MCP3426/MCP3427)
} mcp342x_channel_t;

/**
 * Resolution
 */
typedef enum
{
	MCP342X_RES_12 = 0, //!< 12 bits, 240 samples per second
	MCP342X_RES_14,	    //!< 14 bits, 60 samples per second
	MCP342X_RES_16,	    //!< 16 bits, 15 samples per second
	MCP342X_RES_18	    //!< 18 bits, 3.75 samples per second
} mcp342x_resolution_t;

/**
 * PGA gain
 */
typedef enum
{
	MCP342X_GAIN1 = 0, //!< x1, default
	MCP342X_GAIN2,	   //!< x2
	MCP342X_GAIN4,	   //!< x4
	MCP342X_GAIN8	   //!< x8
} mcp342x_gain_t;

/**
 * Device descriptor
 */
typedef struct
{
	const struct device *i2c_dev;
	uint8_t i2c_addr;
	mcp342x_mode_t mode;		 //!< Operational mode
	mcp342x_channel_t channel;	 //!< Input channel
	mcp342x_resolution_t resolution; //!< Resolution
	mcp342x_gain_t gain;		 //!< PGA gain
} mcp342x_t;

/**
 * @brief Initialize device descriptor
 *
 * @param dev Device descriptor
 * @param port I2C port
 * @param addr Device address
 * @param sda_gpio SDA GPIO pin
 * @param scl_gpio SCL GPIO pin
 * @return `0` on success
 */
int mcp342x_init_desc(mcp342x_t *dev, const char *label, uint8_t addr);

/**
 * @brief Free device descriptor
 *
 * @param dev Device descriptor
 * @return `0` on success
 */
int mcp342x_free_desc(mcp342x_t *dev);

/**
 * @brief Configure device
 *
 * @param dev Device descriptor
 * @return `0` on success
 */
int mcp342x_set_config(mcp342x_t *dev);

/**
 * @brief Read device configuration
 *
 * @param dev Device descriptor
 * @return `0` on success
 */
int mcp342x_get_config(mcp342x_t *dev);

/**
 * @brief Get conversion time in microseconds
 *
 * @param dev Device descriptor
 * @param[out] us Conversion time, us
 * @return `0` on success
 */
int mcp342x_get_sample_time_us(mcp342x_t *dev, uint32_t *us);

/**
 * @brief Start conversion
 *
 * @param dev Device descriptor
 * @return `0` on success
 */
int mcp342x_start_conversion(mcp342x_t *dev);

/**
 * @brief Get raw ADC value
 *
 * @param dev Device descriptor
 * @param[out] data ADC value
 * @param[out] ready Data validity flag
 * @return `0` on success
 */
int mcp342x_get_data(mcp342x_t *dev, int32_t *data, bool *ready);

/**
 * @brief Get ADC voltage
 *
 * @param dev Device descriptor
 * @param[out] volts ADC voltage, volts
 * @param[out] ready Data validity flag
 * @return `0` on success
 */
int mcp342x_get_voltage(mcp342x_t *dev, float *volts, bool *ready);

/**
 * @brief Do a single conversion
 *
 * - start conversion
 * - wait conversion time
 * - read conversion result
 *
 * @param dev Device descriptor
 * @param[out] data ADC value
 * @return `0` on success
 */
int mcp342x_oneshot_conversion(mcp342x_t *dev, int32_t *data);

float mcp342x_raw_to_voltage(mcp342x_t *dev, int32_t raw);

int mcp342x_average_oneshot_conversion(mcp342x_t *dev, int32_t *data, int32_t num_readings);

int mcp342x_average_oneshot_conversion_volt(mcp342x_t *dev, float *adc_volt, int32_t num_readings);
#endif /* __MCP342X_H__ */
