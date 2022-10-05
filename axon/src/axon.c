#include <zephyr.h>
#include <kernel.h>
#include <logging/log.h>
#include <sys/printk.h>
#include <sys/reboot.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <drivers/i2c.h>
#include <drivers/gpio.h>
#include <drivers/led.h>
#include <dk_buttons_and_leds.h>

#include "driver_utils.h"
#include "misc_utils.h"
#include "axon.h"

LOG_MODULE_REGISTER(axon_device, CONFIG_MAIN_LOG_LEVEL);

#define BUTTON1 0
#define BUTTON1_MSK BIT(BUTTON1)

#define buttons_init(...) dk_buttons_init(__VA_ARGS__)

#if DT_NODE_HAS_STATUS(DT_INST(0, pwm_leds), okay)
#define LED_PWM_NODE_ID DT_INST(0, pwm_leds)
#define LED_PWM_DEV_NAME DEVICE_DT_NAME(LED_PWM_NODE_ID)
#else
#error "No LED PWM device found"
#endif

#define LED_PWM_LABEL(led_node_id) DT_PROP_OR(led_node_id, label, NULL),

#define PWM_LED_0 0
#define PWM_LED_1 1

const struct device *led_pwm;

struct device *gpio_dev0;
struct device *gpio_dev1;

#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
#define RESET_MUX_GPIO_GROUP gpio_dev0
#define RESET_MUX_GPIO_PIN 8
#define ON_VCC_EX_GPIO_GROUP gpio_dev1
#define ON_VCC_EX_GPIO_PIN 13
#endif
#define CON0_GPIO_GROUP gpio_dev1
#define CON0_GPIO_PIN 11
#define CON1_GPIO_GROUP gpio_dev1
#define CON1_GPIO_PIN 10
#define CON2_GPIO_GROUP gpio_dev0
#define CON2_GPIO_PIN 3
#define CON3_GPIO_GROUP gpio_dev0
#define CON3_GPIO_PIN 28

#define BME280 DT_INST(0, bosch_bme280)
#define SHT3XD DT_INST(0, sensirion_sht3xd)

#if DT_NODE_HAS_STATUS(BME280, okay)
#define ENV_SENSORS_LABEL DT_LABEL(BME280)
#elif DT_NODE_HAS_STATUS(SHT3XD, okay)
#define ENV_SENSORS_LABEL DT_LABEL(SHT3XD)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280" or "sensirion,sht3xd"
#define ENV_SENSORS_LABEL "<none>"
#endif

#define OPT3001 DT_INST(0, ti_opt3001)
#define VEML7700 DT_INST(0, vishay_veml7700)

#if DT_NODE_HAS_STATUS(OPT3001, okay)
#define ALS_SENSOR_LABEL DT_LABEL(OPT3001)
#elif DT_NODE_HAS_STATUS(VEML7700, okay)
#include "veml7700.h"
#define ALS_SENSOR_LABEL DT_LABEL(VEML7700)
#else
#error Your devicetree has no enabled nodes with compatible "ti,opt3001" or "vishay,veml7700"
#define ALS_SENSOR_LABEL "<none>"
#endif

typedef struct __attribute((packed))
{
	float temp;
#ifdef ENVIRONMENT_SENSOR_HAS_PRESSURE
	float press;
#endif
	float humid;
} environment_vals_t;

static struct device *env_sensors;
static struct device *als_sensor;
static environment_vals_t environment_sensor_vals = {0};
static float als_val = 0;

struct device *ext_i2c_bus_dev;

static uint16_t axon_publish_interval;

static void (*axon_publish_interval_cb)(void) = NULL;

float axon_temp_get(void)
{
	return environment_sensor_vals.temp;
}

#ifdef ENVIRONMENT_SENSOR_HAS_PRESSURE
float axon_press_get(void)
{
	return environment_sensor_vals.press;
}
#endif

float axon_humid_get(void)
{
	return environment_sensor_vals.humid;
}

float axon_luminosity_get(void)
{
	return als_val;
}

static int axon_env_sensor_sample_fetch(void)
{
	struct sensor_value temp, humidity;

	if (env_sensors != NULL)
	{
		sensor_sample_fetch(env_sensors);
		sensor_channel_get(env_sensors, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(env_sensors, SENSOR_CHAN_HUMIDITY, &humidity);
#ifdef ENVIRONMENT_SENSOR_HAS_PRESSURE
		struct sensor_value press;
		sensor_channel_get(env_sensors, SENSOR_CHAN_PRESS, &press);

		LOG_INF("temp: %d.%06d; press: %d.%06d; humidity: %d.%06d;\n",
			temp.val1, temp.val2, press.val1, press.val2,
			humidity.val1, humidity.val2);

		environment_sensor_vals.temp = temp.val1 + ((float)temp.val2 / 1000000);
		environment_sensor_vals.press = press.val1 + ((float)press.val2 / 1000000);
		environment_sensor_vals.humid = humidity.val1 + ((float)humidity.val2 / 1000000);

		printk("temp: %f; press: %f; humidity: %f;\n",
		       environment_sensor_vals.temp, environment_sensor_vals.press, environment_sensor_vals.humid);
#else
		LOG_INF("temp: %d.%06d; humidity: %d.%06d;\n",
			temp.val1, temp.val2, humidity.val1, humidity.val2);

		environment_sensor_vals.temp = temp.val1 + ((float)temp.val2 / 1000000);
		environment_sensor_vals.humid = humidity.val1 + ((float)humidity.val2 / 1000000);

		printk("temp: %f; humidity: %f;\n",
		       environment_sensor_vals.temp, environment_sensor_vals.humid);
#endif

		return 0;
	}
	else
	{
		return 1;
	}
}

static int axon_als_sample_fetch(void)
{
	struct sensor_value luminosity;
	if (als_sensor != NULL)
	{
		sensor_sample_fetch(als_sensor);
		sensor_channel_get(als_sensor, SENSOR_CHAN_LIGHT, &luminosity);
		LOG_INF("luminosity: %d.%06d;\n", luminosity.val1, luminosity.val2);
		als_val = luminosity.val1 + ((float)luminosity.val2 / 1000000);

		printk("luminosity: %f;\n", als_val);

		return 0;
	}
	else
	{
		return 1;
	}
}

int axon_sensors_sample_fetch(void)
{
	int ret;
	ret = axon_env_sensor_sample_fetch();
	if (ret)
	{
		return ret;
	}
	ret = axon_als_sample_fetch();
	if (ret)
	{
		return ret;
	}

	return ret;
}

void axon_macaddr_get(uint8_t *macaddr)
{
	hyper_macaddr_get(macaddr);
}

int64_t axon_uptime_get(void)
{
	return k_uptime_get();
}

int hyper_extension_bus_i2c_write(const uint8_t *buf, uint32_t num_bytes, uint16_t addr)
{
	int ret = i2c_write(ext_i2c_bus_dev, buf, num_bytes, addr);
	if (ret == -EBUSY)
	{
		LOG_ERR("i2c returned -EBUSY (-16), panicing...");
		k_panic();
	}
	return ret;
}

int hyper_extension_bus_i2c_read(uint8_t *buf, uint32_t num_bytes, uint16_t addr)
{
	int ret = i2c_read(ext_i2c_bus_dev, buf, num_bytes, addr);
	if (ret == -EBUSY)
	{
		LOG_ERR("i2c returned -EBUSY (-16), panicing...");
		k_panic();
	}
	return ret;
}

int hyper_extension_bus_i2c_write_read(uint16_t addr, const void *write_buf, size_t num_write,
				       void *read_buf, size_t num_read)
{
	int ret = i2c_write_read(ext_i2c_bus_dev, addr, write_buf, num_write, read_buf, num_read);
	if (ret == -EBUSY)
	{
		LOG_ERR("i2c returned -EBUSY (-16), panicing...");
		k_panic();
	}
	return ret;
}

int hyper_extension_bus_i2c_burst_read(uint16_t dev_addr, uint8_t start_addr, uint8_t *buf,
				       uint32_t num_bytes)
{
	int ret = i2c_burst_read(ext_i2c_bus_dev, dev_addr, start_addr, buf, num_bytes);
	if (ret == -EBUSY)
	{
		LOG_ERR("i2c returned -EBUSY (-16), panicing...");
		k_panic();
	}
	return ret;
}

int hyper_extension_bus_i2c_reg_read_byte(uint16_t dev_addr, uint8_t reg_addr, uint8_t *value)
{
	int ret = i2c_reg_read_byte(ext_i2c_bus_dev, dev_addr, reg_addr, value);
	if (ret == -EBUSY)
	{
		LOG_ERR("i2c returned -EBUSY (-16), panicing...");
		k_panic();
	}
	return ret;
}

int hyper_extension_bus_i2c_reg_write_byte(uint16_t dev_addr, uint8_t reg_addr, uint8_t value)
{
	int ret = i2c_reg_write_byte(ext_i2c_bus_dev, dev_addr, reg_addr, value);
	if (ret == -EBUSY)
	{
		LOG_ERR("i2c returned -EBUSY (-16), panicing...");
		k_panic();
	}
	return ret;
}

int hyper_extension_eeprom_read(uint8_t *buff, uint8_t len, uint8_t pointer, uint16_t i2c_addr)
{
	int ret = i2c_write(ext_i2c_bus_dev, &pointer, 1, i2c_addr);
	if (ret != 0)
	{
		return ret;
	}
	// i2c_addr + 1 for read address
	return i2c_read(ext_i2c_bus_dev, buff, len, i2c_addr + 1);
}

__unused static int hyper_extension_eeprom_write(const uint8_t *buff, uint8_t len, uint8_t pointer, uint16_t i2c_addr)
{
	// add pointer to buffer
	uint8_t buff_pointer[len + 1];
	buff_pointer[0] = pointer;
	memcpy(&buff_pointer[1], buff, len);
	return i2c_write(ext_i2c_bus_dev, buff_pointer, sizeof(buff_pointer), i2c_addr + 1);
}

// Read EUI-48 from extension
int hyper_extension_eui48_read(uint8_t *data)
{
	return hyper_extension_eeprom_read(data, 6, 0xFA, EEPROM_24AA02E48_I2C_ADDR);
}

int hyper_extension_class_id_read(uint32_t *data)
{
	return hyper_extension_eeprom_read((uint8_t *)data, sizeof(uint32_t), 0x00,
					   EEPROM_24AA02E48_I2C_ADDR);
}

int hyper_extension_data_read(uint8_t *data, uint8_t len, uint8_t offset)
{
	return hyper_extension_eeprom_read(data, len,
					   HYPER_EXTENSION_DATA_EEPROM_OFFSET + offset,
					   EEPROM_24AA02E48_I2C_ADDR);
}

// checks if extension is connected by probing its EEPROM
hyper_extension_type_t hyper_extension_get_type()
{
	if (hyper_i2c_device_is_present(ext_i2c_bus_dev, EEPROM_24AA02E48_I2C_ADDR))
	{
		return HYPER_EXTENSION_EEPROM;
	}

	if (hyper_i2c_device_is_present(ext_i2c_bus_dev, HYPER_EXTENSION_MCU_I2C_ADDR))
	{
		return HYPER_EXTENSION_MCU;
	}

	return HYPER_EXTENSION_NOT_CONNECTED;
}

static void on_button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (buttons & BUTTON1_MSK)
	{
		axon_publish_interval_cb();
		axon_publish_interval_timer_reset();
	}
}

static int axon_buttons_init()
{
	return buttons_init(on_button_changed);
}

static int axon_sensors_init(void)
{
	int retval = 0;
	if (hyper_dev_init(&env_sensors, ENV_SENSORS_LABEL))
#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
		retval = 1;
#else
		retval = 0;
#endif

	if (hyper_dev_init(&als_sensor, ALS_SENSOR_LABEL))
#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
		retval = 1;
#else
		retval = 0;
#endif

	return retval;
}

void axon_publish_interval_callback_set(void (*publish_interval_cb)(void))
{
	axon_publish_interval_cb = publish_interval_cb;
}

static void axon_publish_interval_work_handler(struct k_work *work)
{
	axon_publish_interval_cb();
}

K_WORK_DEFINE(axon_publish_interval_work, axon_publish_interval_work_handler);

static void axon_publish_interval_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&axon_publish_interval_work);
}

K_TIMER_DEFINE(axon_publish_interval_timer, axon_publish_interval_timer_handler, NULL);

void axon_publish_interval_set(uint16_t interval_sec)
{
	axon_publish_interval = interval_sec;
	k_timer_stop(&axon_publish_interval_timer);
	k_timer_start(&axon_publish_interval_timer, K_SECONDS(interval_sec), K_SECONDS(interval_sec));
}

uint16_t axon_publish_interval_get(void)
{
	return axon_publish_interval;
}

void axon_publish_interval_timer_reset(void)
{
	k_timer_stop(&axon_publish_interval_timer);
	k_timer_start(&axon_publish_interval_timer, K_SECONDS(axon_publish_interval),
		      K_SECONDS(axon_publish_interval));
}

static int axon_led_init()
{
	int ret = 0;

	led_pwm = device_get_binding(LED_PWM_DEV_NAME);
	if (led_pwm)
	{
		LOG_INF("Found device %s", LED_PWM_DEV_NAME);
	}
	else
	{
		LOG_ERR("Device %s not found", LED_PWM_DEV_NAME);
		return -1;
	}

	return ret;
}

int axon_led_0_set_blink(uint32_t delay_on, uint32_t delay_off)
{
	int ret = 0;
	ret = led_blink(led_pwm, PWM_LED_0, delay_on, delay_off);
	if (ret < 0)
	{
		LOG_ERR("led_blink() failed with exit code: %d\n", ret);
		return ret;
	}

	return ret;
}

int axon_led_0_set_on(uint8_t brightness)
{
	int ret = 0;

	ret = led_on(led_pwm, PWM_LED_0);
	if (ret < 0)
	{
		LOG_ERR("led_on() failed with exit code: %d\n", ret);
		return ret;
	}

	ret = led_set_brightness(led_pwm, PWM_LED_0, brightness);
	if (ret < 0)
	{
		LOG_ERR("led_on() failed with exit code: %d\n", ret);
		return ret;
	}

	return ret;
}

int axon_led_0_set_off()
{
	int ret = 0;

	ret = led_off(led_pwm, PWM_LED_0);
	if (ret < 0)
	{
		LOG_ERR("led_off() failed with exit code: %d\n", ret);
		return ret;
	}

	return ret;
}

#if defined(CONFIG_BOARD_AXON_V2_0_NRF52840)
int axon_led_1_set_blink(uint32_t delay_on, uint32_t delay_off)
{
	int ret = 0;
	ret = led_blink(led_pwm, PWM_LED_1, delay_on, delay_off);
	if (ret < 0)
	{
		LOG_ERR("led_blink() failed with exit code: %d\n", ret);
		return ret;
	}

	return ret;
}

int axon_led_1_set_on(uint8_t brightness)
{
	int ret = 0;

	ret = led_on(led_pwm, PWM_LED_1);
	if (ret < 0)
	{
		LOG_ERR("led_on() failed with exit code: %d\n", ret);
		return ret;
	}

	ret = led_set_brightness(led_pwm, PWM_LED_1, brightness);
	if (ret < 0)
	{
		LOG_ERR("led_on() failed with exit code: %d\n", ret);
		return ret;
	}

	return ret;
}

int axon_led_1_set_off()
{
	int ret = 0;

	ret = led_off(led_pwm, PWM_LED_1);
	if (ret < 0)
	{
		LOG_ERR("led_off() failed with exit code: %d\n", ret);
		return ret;
	}

	return ret;
}
#endif

#ifndef CONFIG_BOARD_AXON_V0_2_NRF52840
__unused
#endif
    static int
    axon_gpio_write(const struct device *gpio_dev, gpio_pin_t gpio_pin, uint8_t value)
{
	int err = gpio_pin_set_raw(gpio_dev, gpio_pin, value);
	if (err)
	{
		LOG_ERR("Cannot write %s, pin: %u, err: %d", gpio_dev->name, gpio_pin, err);
		return 1;
	}

	return 0;
}

__unused static int axon_gpio_read(const struct device *gpio_dev, gpio_pin_t gpio_pin)
{
	int val = gpio_pin_get_raw(gpio_dev, gpio_pin);
	if (val < 0)
	{
		LOG_ERR("Cannot read gpio %s, pin: %u, err: %d", gpio_dev->name, gpio_pin, val);
		return 1;
	}
	LOG_ERR("read gpio %s, pin: %u, val: %d", gpio_dev->name, gpio_pin, val);

	return val;
}

#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
static int axon_vcc_ex_set_on()
{
	if (axon_gpio_write(ON_VCC_EX_GPIO_GROUP, ON_VCC_EX_GPIO_PIN, 0) != 0)
	{
		LOG_ERR("Failed turn on VCC_EX!");
		return 1;
	}

	return 0;
}

static int axon_vcc_ex_set_off()
{
	if (axon_gpio_write(ON_VCC_EX_GPIO_GROUP, ON_VCC_EX_GPIO_PIN, 1) != 0)
	{
		LOG_ERR("Failed turn off VCC_EX!");
		return 1;
	}
	return 0;
}

static int axon_vcc_mux_set_on()
{
	if (axon_gpio_write(RESET_MUX_GPIO_GROUP, RESET_MUX_GPIO_PIN, 1) != 0)
	{
		LOG_ERR("Failed turn set RESET_MUX to High!");
		return 1;
	}
	return 0;
}

static int axon_vcc_mux_set_off()
{
	if (axon_gpio_write(RESET_MUX_GPIO_GROUP, RESET_MUX_GPIO_PIN, 0) != 0)
	{
		LOG_ERR("Failed turn set RESET_MUX to Low!");
		return 1;
	}
	return 0;
}
#endif

void axon_trigger_cold_reboot(void)
{
	LOG_WRN("Triggering: FORCED COLD REBOOT\n");
#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
	axon_vcc_ex_set_off();
	axon_vcc_mux_set_off();
#endif
	sys_reboot(SYS_REBOOT_COLD);
}

void hyper_extension_bus_mux_reset(void)
{
	uint8_t mux_write = 0b00000000;
	if (hyper_extension_bus_i2c_write(&mux_write, 1, 0x70))
	{
		LOG_ERR("Failed to reset mux ports! Panicing...");
		k_panic();
	};
}

void hyper_extension_bus_mux_port_select(uint8_t port)
{
	uint8_t mux_write = 0;
	mux_write = 0 | 1 << (port - 1);
	if (hyper_extension_bus_i2c_write(&mux_write, 1, 0x70))
	{
		LOG_ERR("Failed to select mux port %d! Panicing...", port);
		k_panic();
	};
}

hyper_extension_bus_mux_state_t hyper_extension_bus_mux_state_get(void)
{
	uint8_t mux_read = 0;
	hyper_extension_bus_i2c_read(&mux_read, 1, 0x70);
	return mux_read;
}

void hyper_extension_bus_mux_state_print(hyper_extension_bus_mux_state_t mux_read)
{
	LOG_DBG("===================================================");
	LOG_DBG("mux_read: " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(mux_read));
	LOG_DBG("===================================================");
}

int axon_gpio_init(void)
{
	int ret = 0;
	ret = hyper_dev_init(&gpio_dev0, "GPIO_0");
	if (ret < 0)
	{
		LOG_ERR("hyper_dev_init() for GPIO_0 failed with exit code: %d\n", ret);
		return ret;
	}
	ret = hyper_dev_init(&gpio_dev1, "GPIO_1");
	if (ret < 0)
	{
		LOG_ERR("hyper_dev_init() for GPIO_1 failed with exit code: %d\n", ret);
		return ret;
	}

#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
	// ON_VCC_EX PIN
	ret = gpio_pin_configure(ON_VCC_EX_GPIO_GROUP, ON_VCC_EX_GPIO_PIN, GPIO_OUTPUT);
	if (ret < 0)
	{
		LOG_ERR("gpio_pin_configure() for ON_VCC_EX_GPIO_PIN failed with exit code: %d\n", ret);
		return ret;
	}

	// RESET_MUX pin
	ret = gpio_pin_configure(RESET_MUX_GPIO_GROUP, RESET_MUX_GPIO_PIN, GPIO_OUTPUT);
	if (ret < 0)
	{
		LOG_ERR("gpio_pin_configure() for RESET_MUX_GPIO_PIN failed with exit code: %d\n", ret);
		return ret;
	}

	// CON pins
	gpio_pin_configure(CON0_GPIO_GROUP, CON0_GPIO_PIN, GPIO_INPUT);
	gpio_pin_configure(CON1_GPIO_GROUP, CON1_GPIO_PIN, GPIO_INPUT);
	gpio_pin_configure(CON2_GPIO_GROUP, CON2_GPIO_PIN, GPIO_INPUT);
	gpio_pin_configure(CON3_GPIO_GROUP, CON3_GPIO_PIN, GPIO_INPUT);
#endif

	return 0;
}

int hyper_extension_bus_init(void)
{
#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
	axon_vcc_ex_set_on();
	axon_vcc_mux_set_on();
#endif
	if (hyper_dev_init(&ext_i2c_bus_dev, "I2C_1"))
		return 1;

#ifdef CONFIG_BOARD_AXON_V0_2_NRF52840
	hyper_extension_bus_mux_reset();
	hyper_extension_bus_mux_port_select(4);
	hyper_extension_bus_mux_state_print(hyper_extension_bus_mux_state_get());
#endif

	return 0;
}

int axon_init(void)
{
	int ret = axon_buttons_init();
	if (ret)
	{
		LOG_ERR("axon_buttons_init() failed with exit code: %d\n", ret);
		return ret;
	}

	// init GPIOs before the hyper_extension_bus
	ret = axon_gpio_init();
	if (ret)
	{
		LOG_ERR("axon_gpio_init() failed with exit code: %d\n", ret);
		return ret;
	}

	// init extension bus
	ret = hyper_extension_bus_init();
	if (ret)
	{
		LOG_ERR("hyper_extension_bus_init() failed with exit code: %d\n", ret);
		return ret;
	}

	// init LED
	ret = axon_led_init();
	if (ret)
	{
		LOG_ERR("axon_led_init() failed with exit code: %d\n", ret);
		return ret;
	}

	ret = axon_sensors_init();
	if (ret)
	{
		LOG_ERR("axon_sensors_init() failed with exit code: %d\n", ret);
		return ret;
	}

	// LED Blink test patterns
	// while (true)
	// {
	// axon_led_0_set_blink(130, 130);
	// k_sleep(K_MSEC(5000));
	// axon_led_0_set_blink(30, 130);
	// k_sleep(K_MSEC(5000));
	// axon_led_0_set_blink(130, 30);
	// k_sleep(K_MSEC(5000));
	// axon_led_0_set_blink(40, 40);
	// k_sleep(K_MSEC(5000));
	// axon_led_0_set_blink(10, 130);
	// k_sleep(K_MSEC(5000));
	// axon_led_0_set_off();
	// k_sleep(K_MSEC(5000));
	// axon_led_0_set_on();
	// k_sleep(K_MSEC(5000));
	// }

	return ret;
}
