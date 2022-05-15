#ifndef __HYPER_DEVICE_CORE__
#define __HYPER_DEVICE_CORE__

// Allow overriding printf.
#ifndef HYPER_PRINTF
#include <stdio.h>
#define HYPER_PRINTF(...) printf(__VA_ARGS__)
#endif

#include "cmp.h"
#include <string.h>

typedef enum {
    HYPER_OK = 0,
    HYPER_ERR_ATTRIBUTE_SET,
    HYPER_ERR_ENCODE,
    HYPER_ERR_DECODE,
} hyper_result_t;

typedef struct {
    uint8_t *data;
    size_t offset;
} hyper_msgpack_buffer_t;

bool hyper_device_msgpack_reader(cmp_ctx_t *ctx, void *data, size_t count);

bool hyper_device_msgpack_skipper(cmp_ctx_t *ctx, size_t count);

size_t hyper_device_msgpack_writer(cmp_ctx_t *ctx, const void *data,
                                   size_t count);

hyper_result_t hyper_msgpack_decode_device_id(uint8_t *device_id,
                                              const uint8_t *in,
                                              uint8_t in_len);

#define HYPER_DEVICE_ID_LENGTH 6
/**
 * Processes a buffer containing multiple msgpack messages concatenated
 *toghether Arguments should not be changed concurrently.
 **/
hyper_result_t hyper_msgpack_process_multimessage(
    void (*on_message)(uint32_t device_class_id, uint8_t device_id[6],
                       uint8_t *data, uint8_t size),
    uint8_t *input, uint8_t _input_len);
#endif // __HYPER_DEVICE_CORE__
