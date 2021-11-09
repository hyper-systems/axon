// --- HYPER GENERATED CODE %VERSION% 1636634129 ---
//
// WARNING
//
// This file was automatically generated by the Hyper Platform.
// Manual changes MUST NOT be made, consult documentation at hyper.ag for more
// information.

#ifndef __HYPER_DEVICE_CLASS_13_H__
#define __HYPER_DEVICE_CLASS_13_H__


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "cmp.h"

#define HYPER_ATTRIBUTES_COUNT 4



// --- DEVICE TYPE ---

typedef struct {
  uint8_t version;
  uint32_t device_class_id;
  uint8_t device_id[6];
  

  uint8_t raw_ph_adc_value_set : 1;
  float raw_ph_adc_value;
  uint8_t raw_orp_adc_value_set : 1;
  float raw_orp_adc_value;
  uint8_t raw_ec_adc_value_set : 1;
  float raw_ec_adc_value;
  uint8_t water_temperature_set : 1;
  float water_temperature;
  
} hyper_device_13_t;

// --- MSGPACK HELPERS ---


typedef struct {
  uint8_t *buf;
  size_t offset;
} msgpack_t;

static bool msgpack_reader(cmp_ctx_t *ctx, void *data, size_t count) {
  msgpack_t *msgpack = (msgpack_t *)ctx->buf;
  memcpy(data, msgpack->buf + msgpack->offset, count);
  msgpack->offset += count;
  return true;
}

static size_t msgpack_writer(cmp_ctx_t *ctx, const void *data, size_t count) {
  msgpack_t *msgpack = (msgpack_t *)ctx->buf;
  memcpy(msgpack->buf + msgpack->offset, data, count);
  msgpack->offset += count;

  return count;
}

static void error_and_exit(const char *msg) {
    fprintf(stderr, "%s\n\n", msg);
    exit(EXIT_FAILURE);
}


void hyper_device_13_init(hyper_device_13_t * device, uint8_t device_id[6]) {
  device->version = 2;
  device->device_class_id = 13;
  memcpy(device->device_id, device_id, 6);


  device->raw_ph_adc_value_set = 0;
  device->raw_ph_adc_value = 0.0;
  device->raw_orp_adc_value_set = 0;
  device->raw_orp_adc_value = 0.0;
  device->raw_ec_adc_value_set = 0;
  device->raw_ec_adc_value = 0.0;
  device->water_temperature_set = 0;
  device->water_temperature = 0.0;

}



// --- PRINT DEVICE ---

void hyper_device_13_pp(hyper_device_13_t * device) {

      printf("(hyper_device_13_t\n"
              "  (version %d)\n"
              "  (device_class_id %lu)\n"
              "  (device_id (%02x %02x %02x %02x %02x %02x))\n",
              device->version, (long unsigned)device->device_class_id, device->device_id[0],
              device->device_id[1], device->device_id[2], device->device_id[3],
              device->device_id[4], device->device_id[5]);
      

      if (device->raw_ph_adc_value_set) {
        printf("  (raw_ph_adc_value (f32 %f))\n", device->raw_ph_adc_value);
      } else {
        printf("  (raw_ph_adc_value (f32 none))\n");
      }

      if (device->raw_orp_adc_value_set) {
        printf("  (raw_orp_adc_value (f32 %f))\n", device->raw_orp_adc_value);
      } else {
        printf("  (raw_orp_adc_value (f32 none))\n");
      }

      if (device->raw_ec_adc_value_set) {
        printf("  (raw_ec_adc_value (f32 %f))\n", device->raw_ec_adc_value);
      } else {
        printf("  (raw_ec_adc_value (f32 none))\n");
      }

      if (device->water_temperature_set) {
        printf("  (water_temperature (f32 %f))\n", device->water_temperature);
      } else {
        printf("  (water_temperature (f32 none))\n");
      }

      printf("\b)\n");
    
}



// --- BIND ATTRIBUTES ---



// --- SET ATTRIBUTES ---

void hyper_device_13_set_raw_ph_adc_value(hyper_device_13_t * device, float value) {
  device->raw_ph_adc_value_set = 1;
  device->raw_ph_adc_value = value;
}


void hyper_device_13_set_raw_orp_adc_value(hyper_device_13_t * device, float value) {
  device->raw_orp_adc_value_set = 1;
  device->raw_orp_adc_value = value;
}


void hyper_device_13_set_raw_ec_adc_value(hyper_device_13_t * device, float value) {
  device->raw_ec_adc_value_set = 1;
  device->raw_ec_adc_value = value;
}


void hyper_device_13_set_water_temperature(hyper_device_13_t * device, float value) {
  device->water_temperature_set = 1;
  device->water_temperature = value;
}



// --- ENCODE DEVICE ---

uint8_t hyper_device_13_encode(hyper_device_13_t * device, uint8_t * out) {
  cmp_ctx_t cmp_ctx = {0};
  msgpack_t msgpack = {out, 0};

  cmp_init(&cmp_ctx, (void *)&msgpack, NULL, NULL, msgpack_writer);

  // Encode message as array of elements.
  if (!cmp_write_array(&cmp_ctx, 3)) {
    error_and_exit(cmp_strerror(&cmp_ctx));
  }

  // Device info.
  if (!cmp_write_u32(&cmp_ctx, device->device_class_id)) {
    error_and_exit(cmp_strerror(&cmp_ctx));
  }

  if (!cmp_write_bin(&cmp_ctx, device->device_id, 6)) {
    error_and_exit(cmp_strerror(&cmp_ctx));
  }

  // Attributes
  if (!cmp_write_array(&cmp_ctx, HYPER_ATTRIBUTES_COUNT)) {
    error_and_exit(cmp_strerror(&cmp_ctx));
  }


  if (device->raw_ph_adc_value_set) {
    if (!cmp_write_float(&cmp_ctx, device->raw_ph_adc_value)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  } else {
    if (!cmp_write_nil(&cmp_ctx)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  }

  if (device->raw_orp_adc_value_set) {
    if (!cmp_write_float(&cmp_ctx, device->raw_orp_adc_value)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  } else {
    if (!cmp_write_nil(&cmp_ctx)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  }

  if (device->raw_ec_adc_value_set) {
    if (!cmp_write_float(&cmp_ctx, device->raw_ec_adc_value)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  } else {
    if (!cmp_write_nil(&cmp_ctx)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  }

  if (device->water_temperature_set) {
    if (!cmp_write_float(&cmp_ctx, device->water_temperature)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  } else {
    if (!cmp_write_nil(&cmp_ctx)) {
      error_and_exit(cmp_strerror(&cmp_ctx));
    }
  }
  return msgpack.offset;
}



// --- DECODE DEVICE ---

uint8_t hyper_device_13_decode(hyper_device_13_t * device, uint8_t * in, uint8_t in_size) {
  cmp_ctx_t cmp_ctx = {0};
  msgpack_t msgpack = {in, 0};

  cmp_init(&cmp_ctx, (void *)&msgpack, msgpack_reader, NULL, NULL);
      
  uint32_t msg_array_size = 0;

  // Main msg array
  if (!cmp_read_array(&cmp_ctx, &msg_array_size) && msg_array_size != 3) {
    error_and_exit(cmp_strerror(&cmp_ctx));
  }
  printf("hyper_device_decode: msg_array_size=%lu\n", (long unsigned)msg_array_size);

  cmp_object_t obj;
  // Device class id
  if (!cmp_read_object(&cmp_ctx, &obj)) {
    error_and_exit("could not read object");
  }
  if (!cmp_object_as_uint(&obj, &device->device_class_id)) {
    error_and_exit("could not read device_class_id\n");
  }

  // Device id
  uint32_t device_id_size = 0;
  if (!cmp_read_bin_size(&cmp_ctx, &device_id_size)) {
    error_and_exit(cmp_strerror(&cmp_ctx));
  }

  if (device_id_size != 6) {
    error_and_exit("hyper_device_decode: wrong device_id_size\n");
  }

  memcpy(device->device_id, in + msgpack.offset, 6);
  msgpack.offset += 6;

  // Attributes
  uint32_t attributes_array_size = 0;
  if (!cmp_read_array(&cmp_ctx, &attributes_array_size) && attributes_array_size != HYPER_ATTRIBUTES_COUNT) {
    error_and_exit(cmp_strerror(&cmp_ctx));
  }

    

  if (!cmp_read_object(&cmp_ctx, &obj)) {
    error_and_exit("could not read object");
  }
  if (!cmp_object_is_nil(&obj)) {
    if (cmp_object_as_float(&obj, &device->raw_ph_adc_value)) {
      device->raw_ph_adc_value_set = 1; 
    } else {
      error_and_exit("could not read value\n");
    }
  }

  if (!cmp_read_object(&cmp_ctx, &obj)) {
    error_and_exit("could not read object");
  }
  if (!cmp_object_is_nil(&obj)) {
    if (cmp_object_as_float(&obj, &device->raw_orp_adc_value)) {
      device->raw_orp_adc_value_set = 1; 
    } else {
      error_and_exit("could not read value\n");
    }
  }

  if (!cmp_read_object(&cmp_ctx, &obj)) {
    error_and_exit("could not read object");
  }
  if (!cmp_object_is_nil(&obj)) {
    if (cmp_object_as_float(&obj, &device->raw_ec_adc_value)) {
      device->raw_ec_adc_value_set = 1; 
    } else {
      error_and_exit("could not read value\n");
    }
  }

  if (!cmp_read_object(&cmp_ctx, &obj)) {
    error_and_exit("could not read object");
  }
  if (!cmp_object_is_nil(&obj)) {
    if (cmp_object_as_float(&obj, &device->water_temperature)) {
      device->water_temperature_set = 1; 
    } else {
      error_and_exit("could not read value\n");
    }
  }
  return msgpack.offset;
}



// --- DISPATCH DEVICE ---

void hyper_device_13_dispatch(hyper_device_13_t * device) {

}



#endif