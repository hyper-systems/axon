// --- HYPER GENERATED CODE %VERSION% 1647449985 ---
//
// WARNING
//
// This file was automatically generated by the Hyper Platform.
// Manual changes MUST NOT be made, consult documentation at hyper.ag for more
// information.

#ifndef __HYPER_DEVICE_CLASS_18__
#define __HYPER_DEVICE_CLASS_18__

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


  uint8_t voltage_set : 1;
  float voltage;
  uint8_t current_set : 1;
  float current;
  uint8_t power_set : 1;
  float power;
  uint8_t apparent_power_set : 1;
  float apparent_power;
  uint8_t reactive_power_set : 1;
  float reactive_power;
  uint8_t power_factor_set : 1;
  float power_factor;
  uint8_t frequency_set : 1;
  float frequency;
  uint8_t import_active_energy_set : 1;
  float import_active_energy;
  uint8_t export_active_energy_set : 1;
  float export_active_energy;
  uint8_t import_reactive_energy_set : 1;
  float import_reactive_energy;
  uint8_t export_reactive_energy_set : 1;
  float export_reactive_energy;
  uint8_t total_active_energy_set : 1;
  float total_active_energy;
  uint8_t total_reactive_energy_set : 1;
  float total_reactive_energy;

} hyper_device_18_t;

void hyper_device_18_reset(hyper_device_18_t * device) {
  device->voltage_set = 0;
  device->voltage = 0.0;
  device->current_set = 0;
  device->current = 0.0;
  device->power_set = 0;
  device->power = 0.0;
  device->apparent_power_set = 0;
  device->apparent_power = 0.0;
  device->reactive_power_set = 0;
  device->reactive_power = 0.0;
  device->power_factor_set = 0;
  device->power_factor = 0.0;
  device->frequency_set = 0;
  device->frequency = 0.0;
  device->import_active_energy_set = 0;
  device->import_active_energy = 0.0;
  device->export_active_energy_set = 0;
  device->export_active_energy = 0.0;
  device->import_reactive_energy_set = 0;
  device->import_reactive_energy = 0.0;
  device->export_reactive_energy_set = 0;
  device->export_reactive_energy = 0.0;
  device->total_active_energy_set = 0;
  device->total_active_energy = 0.0;
  device->total_reactive_energy_set = 0;
  device->total_reactive_energy = 0.0;
}


void hyper_device_18_init(hyper_device_18_t * device, uint8_t device_id[6]) {
  device->version = 3;
  device->device_class_id = 18;
  memcpy(device->device_id, device_id, 6);
  hyper_device_18_reset(device);
}


// --- PRINT DEVICE ---
void hyper_device_18_print(hyper_device_18_t * device) {
  HYPER_PRINTF(
    "(hyper_device_18_t\n"
    "  (version %d)\n"
    "  (device_class_id %lu)\n"
    "  (device_id (%02x %02x %02x %02x %02x %02x))\n",
    device->version, (long unsigned)device->device_class_id, device->device_id[0],
    device->device_id[1], device->device_id[2], device->device_id[3],
    device->device_id[4], device->device_id[5]);


  if (device->voltage_set) {
    HYPER_PRINTF("  (voltage (f32 %f))\n", device->voltage);
  } else {
    HYPER_PRINTF("  (voltage (f32 none))\n");
  }

  if (device->current_set) {
    HYPER_PRINTF("  (current (f32 %f))\n", device->current);
  } else {
    HYPER_PRINTF("  (current (f32 none))\n");
  }

  if (device->power_set) {
    HYPER_PRINTF("  (power (f32 %f))\n", device->power);
  } else {
    HYPER_PRINTF("  (power (f32 none))\n");
  }

  if (device->apparent_power_set) {
    HYPER_PRINTF("  (apparent_power (f32 %f))\n", device->apparent_power);
  } else {
    HYPER_PRINTF("  (apparent_power (f32 none))\n");
  }

  if (device->reactive_power_set) {
    HYPER_PRINTF("  (reactive_power (f32 %f))\n", device->reactive_power);
  } else {
    HYPER_PRINTF("  (reactive_power (f32 none))\n");
  }

  if (device->power_factor_set) {
    HYPER_PRINTF("  (power_factor (f32 %f))\n", device->power_factor);
  } else {
    HYPER_PRINTF("  (power_factor (f32 none))\n");
  }

  if (device->frequency_set) {
    HYPER_PRINTF("  (frequency (f32 %f))\n", device->frequency);
  } else {
    HYPER_PRINTF("  (frequency (f32 none))\n");
  }

  if (device->import_active_energy_set) {
    HYPER_PRINTF("  (import_active_energy (f32 %f))\n", device->import_active_energy);
  } else {
    HYPER_PRINTF("  (import_active_energy (f32 none))\n");
  }

  if (device->export_active_energy_set) {
    HYPER_PRINTF("  (export_active_energy (f32 %f))\n", device->export_active_energy);
  } else {
    HYPER_PRINTF("  (export_active_energy (f32 none))\n");
  }

  if (device->import_reactive_energy_set) {
    HYPER_PRINTF("  (import_reactive_energy (f32 %f))\n", device->import_reactive_energy);
  } else {
    HYPER_PRINTF("  (import_reactive_energy (f32 none))\n");
  }

  if (device->export_reactive_energy_set) {
    HYPER_PRINTF("  (export_reactive_energy (f32 %f))\n", device->export_reactive_energy);
  } else {
    HYPER_PRINTF("  (export_reactive_energy (f32 none))\n");
  }

  if (device->total_active_energy_set) {
    HYPER_PRINTF("  (total_active_energy (f32 %f))\n", device->total_active_energy);
  } else {
    HYPER_PRINTF("  (total_active_energy (f32 none))\n");
  }

  if (device->total_reactive_energy_set) {
    HYPER_PRINTF("  (total_reactive_energy (f32 %f))\n", device->total_reactive_energy);
  } else {
    HYPER_PRINTF("  (total_reactive_energy (f32 none))\n");
  }
  HYPER_PRINTF(")\n");
}


// --- BIND ATTRIBUTES ---

// --- SET ATTRIBUTES ---
void hyper_device_18_set_voltage(hyper_device_18_t * device, float value) {
  device->voltage_set = 1;
  device->voltage = value;
}


void hyper_device_18_set_current(hyper_device_18_t * device, float value) {
  device->current_set = 1;
  device->current = value;
}


void hyper_device_18_set_power(hyper_device_18_t * device, float value) {
  device->power_set = 1;
  device->power = value;
}


void hyper_device_18_set_apparent_power(hyper_device_18_t * device, float value) {
  device->apparent_power_set = 1;
  device->apparent_power = value;
}


void hyper_device_18_set_reactive_power(hyper_device_18_t * device, float value) {
  device->reactive_power_set = 1;
  device->reactive_power = value;
}


void hyper_device_18_set_power_factor(hyper_device_18_t * device, float value) {
  device->power_factor_set = 1;
  device->power_factor = value;
}


void hyper_device_18_set_frequency(hyper_device_18_t * device, float value) {
  device->frequency_set = 1;
  device->frequency = value;
}


void hyper_device_18_set_import_active_energy(hyper_device_18_t * device, float value) {
  device->import_active_energy_set = 1;
  device->import_active_energy = value;
}


void hyper_device_18_set_export_active_energy(hyper_device_18_t * device, float value) {
  device->export_active_energy_set = 1;
  device->export_active_energy = value;
}


void hyper_device_18_set_import_reactive_energy(hyper_device_18_t * device, float value) {
  device->import_reactive_energy_set = 1;
  device->import_reactive_energy = value;
}


void hyper_device_18_set_export_reactive_energy(hyper_device_18_t * device, float value) {
  device->export_reactive_energy_set = 1;
  device->export_reactive_energy = value;
}


void hyper_device_18_set_total_active_energy(hyper_device_18_t * device, float value) {
  device->total_active_energy_set = 1;
  device->total_active_energy = value;
}


void hyper_device_18_set_total_reactive_energy(hyper_device_18_t * device, float value) {
  device->total_reactive_energy_set = 1;
  device->total_reactive_energy = value;
}


// --- ENCODE DEVICE ---
hyper_result_t hyper_device_18_encode(hyper_device_18_t * device, uint8_t * out, uint8_t * out_len) {
  cmp_ctx_t cmp_ctx = {0};
  hyper_msgpack_buffer_t msgpack_buf = {out, 0};

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
  if (device->voltage_set) ++defined_attributes_count;
  if (device->current_set) ++defined_attributes_count;
  if (device->power_set) ++defined_attributes_count;
  if (device->apparent_power_set) ++defined_attributes_count;
  if (device->reactive_power_set) ++defined_attributes_count;
  if (device->power_factor_set) ++defined_attributes_count;
  if (device->frequency_set) ++defined_attributes_count;
  if (device->import_active_energy_set) ++defined_attributes_count;
  if (device->export_active_energy_set) ++defined_attributes_count;
  if (device->import_reactive_energy_set) ++defined_attributes_count;
  if (device->export_reactive_energy_set) ++defined_attributes_count;
  if (device->total_active_energy_set) ++defined_attributes_count;
  if (device->total_reactive_energy_set) ++defined_attributes_count;

  // Attributes
  if (!cmp_write_map(&cmp_ctx, defined_attributes_count)) {
    HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
    return HYPER_ERR_ENCODE;
  }

  // 0: voltage
  if (device->voltage_set) {
    if (!cmp_write_u8(&cmp_ctx, 0)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->voltage)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 1: current
  if (device->current_set) {
    if (!cmp_write_u8(&cmp_ctx, 1)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->current)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 2: power
  if (device->power_set) {
    if (!cmp_write_u8(&cmp_ctx, 2)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->power)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 3: apparent_power
  if (device->apparent_power_set) {
    if (!cmp_write_u8(&cmp_ctx, 3)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->apparent_power)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 4: reactive_power
  if (device->reactive_power_set) {
    if (!cmp_write_u8(&cmp_ctx, 4)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->reactive_power)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 5: power_factor
  if (device->power_factor_set) {
    if (!cmp_write_u8(&cmp_ctx, 5)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->power_factor)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 6: frequency
  if (device->frequency_set) {
    if (!cmp_write_u8(&cmp_ctx, 6)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->frequency)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 7: import_active_energy
  if (device->import_active_energy_set) {
    if (!cmp_write_u8(&cmp_ctx, 7)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->import_active_energy)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 8: export_active_energy
  if (device->export_active_energy_set) {
    if (!cmp_write_u8(&cmp_ctx, 8)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->export_active_energy)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 9: import_reactive_energy
  if (device->import_reactive_energy_set) {
    if (!cmp_write_u8(&cmp_ctx, 9)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->import_reactive_energy)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 10: export_reactive_energy
  if (device->export_reactive_energy_set) {
    if (!cmp_write_u8(&cmp_ctx, 10)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->export_reactive_energy)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 11: total_active_energy
  if (device->total_active_energy_set) {
    if (!cmp_write_u8(&cmp_ctx, 11)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->total_active_energy)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  // 12: total_reactive_energy
  if (device->total_reactive_energy_set) {
    if (!cmp_write_u8(&cmp_ctx, 12)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
    if (!cmp_write_float(&cmp_ctx, device->total_reactive_energy)) {
      HYPER_PRINTF("%s\n", cmp_strerror(&cmp_ctx));
      return HYPER_ERR_ENCODE;
    }
  }

  *out_len = msgpack_buf.offset;
  return HYPER_OK;
}


// --- DECODE DEVICE ---
hyper_result_t hyper_device_18_decode(hyper_device_18_t * device, uint8_t * in, uint8_t in_size) {
  cmp_ctx_t cmp_ctx = {0};
  hyper_msgpack_buffer_t msgpack_buf = {in, 0};

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
      case 0: // voltage
        if (cmp_object_as_float(&obj, &device->voltage)) {
          device->voltage_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 1: // current
        if (cmp_object_as_float(&obj, &device->current)) {
          device->current_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 2: // power
        if (cmp_object_as_float(&obj, &device->power)) {
          device->power_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 3: // apparent_power
        if (cmp_object_as_float(&obj, &device->apparent_power)) {
          device->apparent_power_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 4: // reactive_power
        if (cmp_object_as_float(&obj, &device->reactive_power)) {
          device->reactive_power_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 5: // power_factor
        if (cmp_object_as_float(&obj, &device->power_factor)) {
          device->power_factor_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 6: // frequency
        if (cmp_object_as_float(&obj, &device->frequency)) {
          device->frequency_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 7: // import_active_energy
        if (cmp_object_as_float(&obj, &device->import_active_energy)) {
          device->import_active_energy_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 8: // export_active_energy
        if (cmp_object_as_float(&obj, &device->export_active_energy)) {
          device->export_active_energy_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 9: // import_reactive_energy
        if (cmp_object_as_float(&obj, &device->import_reactive_energy)) {
          device->import_reactive_energy_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 10: // export_reactive_energy
        if (cmp_object_as_float(&obj, &device->export_reactive_energy)) {
          device->export_reactive_energy_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 11: // total_active_energy
        if (cmp_object_as_float(&obj, &device->total_active_energy)) {
          device->total_active_energy_set = 1;
        } else {
          HYPER_PRINTF("%s\n", "could not read value\n");
            return HYPER_ERR_DECODE;
        }
        break;

      case 12: // total_reactive_energy
        if (cmp_object_as_float(&obj, &device->total_reactive_energy)) {
          device->total_reactive_energy_set = 1;
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
uint8_t hyper_device_18_dispatch(hyper_device_18_t * device) {
  uint8_t count = 0;

  return count;
}


#endif // __HYPER_DEVICE_CLASS_18__
