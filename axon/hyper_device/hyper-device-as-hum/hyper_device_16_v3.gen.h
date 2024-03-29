// --- HYPER GENERATED CODE %VERSION% 1644418594 ---
//
// WARNING
//
// This file was automatically generated by the Hyper Platform.
// Manual changes MUST NOT be made, consult documentation at hyper.ag for more
// information.

#ifndef __HYPER_DEVICE_CLASS_16__
#define __HYPER_DEVICE_CLASS_16__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cmp.h"
#include "hyper_device_core_v3.h"

// --- DEVICE TYPE ---
typedef struct {
  uint8_t version;
  uint32_t device_class_id;
  uint8_t device_id[6];
  

  uint8_t environment_temperature_set : 1;
  float environment_temperature;
  uint8_t humidity_set : 1;
  float humidity;
  uint8_t dew_point_temperature_set : 1;
  float dew_point_temperature;
  
} hyper_device_16_t;

void hyper_device_16_reset(hyper_device_16_t * device) {
  device->environment_temperature_set = 0;
  device->environment_temperature = 0.0;
  device->humidity_set = 0;
  device->humidity = 0.0;
  device->dew_point_temperature_set = 0;
  device->dew_point_temperature = 0.0;
}


void hyper_device_16_init(hyper_device_16_t * device, uint8_t device_id[6]) {
  device->version = 3;
  device->device_class_id = 16;
  memcpy(device->device_id, device_id, 6);
  hyper_device_16_reset(device);
}


// --- PRINT DEVICE ---
void hyper_device_16_print(hyper_device_16_t * device) {
  HYPER_PRINTF(
    "(hyper_device_16_t\n"
    "  (version %d)\n"
    "  (device_class_id %lu)\n"
    "  (device_id (%02x %02x %02x %02x %02x %02x))\n",
    device->version, (long unsigned)device->device_class_id, device->device_id[0],
    device->device_id[1], device->device_id[2], device->device_id[3],
    device->device_id[4], device->device_id[5]);
      

  if (device->environment_temperature_set) {
    HYPER_PRINTF("  (environment_temperature (f32 %f))\n", device->environment_temperature);
  } else {
    HYPER_PRINTF("  (environment_temperature (f32 none))\n");
  }

  if (device->humidity_set) {
    HYPER_PRINTF("  (humidity (f32 %f))\n", device->humidity);
  } else {
    HYPER_PRINTF("  (humidity (f32 none))\n");
  }

  if (device->dew_point_temperature_set) {
    HYPER_PRINTF("  (dew_point_temperature (f32 %f))\n", device->dew_point_temperature);
  } else {
    HYPER_PRINTF("  (dew_point_temperature (f32 none))\n");
  }
  HYPER_PRINTF(")\n");
}


// --- BIND ATTRIBUTES ---

// --- SET ATTRIBUTES ---
void hyper_device_16_set_environment_temperature(hyper_device_16_t * device, float value) {
  device->environment_temperature_set = 1;
  device->environment_temperature = value;
}


void hyper_device_16_set_humidity(hyper_device_16_t * device, float value) {
  device->humidity_set = 1;
  device->humidity = value;
}


void hyper_device_16_set_dew_point_temperature(hyper_device_16_t * device, float value) {
  device->dew_point_temperature_set = 1;
  device->dew_point_temperature = value;
}


// --- ENCODE DEVICE ---
hyper_result_t hyper_device_16_encode(hyper_device_16_t * device, uint8_t * out, uint8_t * out_len) {
  cmp_ctx_t cmp_ctx = {0};
  buffer_t msgpack_buf = {out, 0};

  cmp_init(&cmp_ctx, (void *)&msgpack_buf, NULL, NULL, hyper_device_msgpack_writer);

  // Encode message as array of elements.
  if (!cmp_write_array(&cmp_ctx, 3)) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_ENCODE;
  }
         
  // Device info.
  if (!cmp_write_uinteger(&cmp_ctx, device->device_class_id)) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_ENCODE;
  }

  if (!cmp_write_bin(&cmp_ctx, device->device_id, 6)) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_ENCODE;
  }

  uint8_t defined_attributes_count = 0;
  if (device->environment_temperature_set) ++defined_attributes_count;
  if (device->humidity_set) ++defined_attributes_count;
  if (device->dew_point_temperature_set) ++defined_attributes_count;

  // Attributes
  if (!cmp_write_map(&cmp_ctx, defined_attributes_count)) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_ENCODE;
  }

  // 0: environment_temperature
  if (device->environment_temperature_set) {
    if (!cmp_write_u8(&cmp_ctx, 0)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->environment_temperature)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 1: humidity
  if (device->humidity_set) {
    if (!cmp_write_u8(&cmp_ctx, 1)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->humidity)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 2: dew_point_temperature
  if (device->dew_point_temperature_set) {
    if (!cmp_write_u8(&cmp_ctx, 2)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->dew_point_temperature)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  *out_len = msgpack_buf.offset;
  return HYPER_OK;
}


// --- DECODE DEVICE ---
hyper_result_t hyper_device_16_decode(hyper_device_16_t * device, uint8_t * in, uint8_t in_size) {
  cmp_ctx_t cmp_ctx = {0};
  buffer_t msgpack_buf = {in, 0};

  cmp_init(&cmp_ctx, (void *)&msgpack_buf, hyper_device_msgpack_reader, NULL, NULL);
      
  uint32_t msg_array_size = 0;

  // Main msg array
  if (!cmp_read_array(&cmp_ctx, &msg_array_size) && msg_array_size != 3) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_DECODE;
  }

  cmp_object_t obj;

  // Device class id
  if (!cmp_read_object(&cmp_ctx, &obj)) {
    HYPER_PRINTF("%s\n", "could not read object");
    return HYPER_ERR_DECODE;
  }
  if (!cmp_object_as_uint(&obj, &device->device_class_id)) {
    HYPER_PRINTF("%s\n", "could not read device_class_id\n");
    return HYPER_ERR_DECODE;
  }

  // Device id
  uint32_t device_id_size = 0;
  if (!cmp_read_bin_size(&cmp_ctx, &device_id_size)) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_DECODE;
  }

  if (device_id_size != 6) {
    HYPER_PRINTF("%s\n", "hyper_device_decode: wrong device_id_size\n");
    return HYPER_ERR_DECODE;
  }

  memcpy(device->device_id, in + msgpack_buf.offset, 6);
  msgpack_buf.offset += 6;

  // Attributes
  uint32_t attributes_map_len = 0;
  if (!cmp_read_map(&cmp_ctx, &attributes_map_len)) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_DECODE;
  }

  uint8_t key = 0xFF;
  for (uint16_t i = 0; i < attributes_map_len; ++i) {
    // Key
    if (!cmp_read_object(&cmp_ctx, &obj)) {
      HYPER_PRINTF("%s\n", "could not read map key");
      return HYPER_ERR_DECODE;
    }
    if (cmp_object_as_uchar(&obj, &key)) {
      // Value
      if (!cmp_read_object(&cmp_ctx, &obj)) {
        HYPER_PRINTF("%s\n", "could not read attribute value");
        return HYPER_ERR_DECODE;
      }
      switch (key) {
      case 0: // environment_temperature
        if (cmp_object_as_float(&obj, &device->environment_temperature)) {
          device->environment_temperature_set = 1; 
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 1: // humidity
        if (cmp_object_as_float(&obj, &device->humidity)) {
          device->humidity_set = 1; 
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 2: // dew_point_temperature
        if (cmp_object_as_float(&obj, &device->dew_point_temperature)) {
          device->dew_point_temperature_set = 1; 
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      default:
        HYPER_PRINTF("%s\n", "attribute index out of bounds\n");
        return HYPER_ERR_DECODE;
      }
    } else {
      HYPER_PRINTF("%s\n", "could not read value\n");
      return HYPER_ERR_DECODE;
    }
  }
  return HYPER_OK;
}


// --- DISPATCH DEVICE ---
uint8_t hyper_device_16_dispatch(hyper_device_16_t * device) {
  uint8_t count = 0;

  return count;
}


#endif // __HYPER_DEVICE_CLASS_16__
