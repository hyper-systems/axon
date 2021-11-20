#ifndef __HYPER_DEVICE_CORE__
#define __HYPER_DEVICE_CORE__

// Allow overriding printf.
#ifndef HYPER_PRINTF
#include <stdio.h>
#define HYPER_PRINTF(...) printf(__VA_ARGS__)
#endif

#include <string.h>
#include "cmp.h"

typedef enum {
  HYPER_OK = 0,
  HYPER_ERR_ATTRIBUTE_SET,
  HYPER_ERR_ENCODE,
  HYPER_ERR_DECODE,
} hyper_result_t;

typedef struct {
  uint8_t *data;
  size_t offset;
} buffer_t;

bool hyper_device_msgpack_reader(cmp_ctx_t *ctx, void *data, size_t count);

bool hyper_device_msgpack_skipper(cmp_ctx_t *ctx, size_t count);

size_t hyper_device_msgpack_writer(cmp_ctx_t *ctx, const void *data, size_t count);

hyper_result_t hyper_msgpack_decode_device_id(uint8_t *device_id, const uint8_t *in, uint8_t in_len);

#endif // __HYPER_DEVICE_CORE__
