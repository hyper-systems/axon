
#ifndef __AXON_DEVICE_H__
#define __AXON_DEVICE_H__
#include <stdint.h>
#include <stdbool.h>

#define EEPROM_24AA02E48_I2C_ADDR 0x50
#define HYPER_EXTENSION_MCU_I2C_ADDR 0x05

#define HYPER_EXTENSION_DATA_EEPROM_OFFSET 0x14

#define AXON_PUBLISH_INTERVAL_DEFAULT_SEC 30

typedef enum
{
	HYPER_EXTENSION_NOT_CONNECTED,
	HYPER_EXTENSION_EEPROM,
	HYPER_EXTENSION_MCU
} hyper_extension_type_t;

#if defined(CONFIG_BOARD_AXON_V1_0_NRF52840) || defined(CONFIG_BOARD_AXON_V0_2_NRF52840)
#define ENVIRONMENT_SENSOR_HAS_PRESSURE
#endif

int hyper_extension_bus_init(void);
int hyper_extension_eeprom_read(uint8_t *buff, uint8_t len, uint8_t pointer, uint16_t i2c_addr);
int hyper_extension_eui48_read(uint8_t *data);
int hyper_extension_class_id_read(uint32_t *data);
int hyper_extension_data_read(uint8_t *data, uint8_t len, uint8_t offset);
hyper_extension_type_t hyper_extension_get_type(void);

int hyper_extension_bus_i2c_write(const uint8_t *buf, uint32_t num_bytes, uint16_t addr);
int hyper_extension_bus_i2c_read(uint8_t *buf, uint32_t num_bytes, uint16_t addr);
int hyper_extension_bus_i2c_write_read(uint16_t addr, const void *write_buf, size_t num_write,
				       void *read_buf, size_t num_read);
int hyper_extension_bus_i2c_burst_read(uint16_t dev_addr, uint8_t start_addr, uint8_t *buf,
				       uint32_t num_bytes);
int hyper_extension_bus_i2c_reg_read_byte(uint16_t dev_addr, uint8_t reg_addr, uint8_t *value);
int hyper_extension_bus_i2c_reg_write_byte(uint16_t dev_addr, uint8_t reg_addr, uint8_t value);

float axon_temp_get(void);
#ifdef ENVIRONMENT_SENSOR_HAS_PRESSURE
float axon_press_get(void);
#endif
float axon_humid_get(void);
float axon_luminosity_get(void);
int axon_sensors_sample_fetch(void);
void axon_macaddr_get(uint8_t *macaddr);
void axon_trigger_cold_reboot(void);
int64_t axon_uptime_get(void);
uint16_t axon_publish_interval_get(void);
void axon_publish_interval_set(uint16_t publish_interval);
void axon_publish_interval_timer_reset(void);
void axon_publish_interval_callback_set(void (*publish_interval_cb)(void));
int axon_led_set_blink(uint32_t delay_on, uint32_t delay_off);
int axon_led_set_on(void);
int axon_led_set_off(void);

typedef uint8_t hyper_extension_bus_mux_state_t;
void hyper_extension_bus_mux_reset(void);
void hyper_extension_bus_mux_port_select(uint8_t port);
hyper_extension_bus_mux_state_t hyper_extension_bus_mux_state_get(void);
void hyper_extension_bus_mux_state_print(hyper_extension_bus_mux_state_t mux_read);
int axon_init(void);

#endif
