#include "hyper_device_core_v3.h"
#include "cmp.h"
#include <string.h>

bool hyper_device_msgpack_reader(cmp_ctx_t *ctx, void *data, size_t count) {
    buffer_t *msgpack_buf = ctx->buf;
    memcpy(data, msgpack_buf->data + msgpack_buf->offset, count);
    msgpack_buf->offset += count;
    return true;
}

bool hyper_device_msgpack_skipper(cmp_ctx_t *ctx, size_t count) {
    buffer_t *msgpack_buf = ctx->buf;
    msgpack_buf->offset += count;
    return true;
}

size_t hyper_device_msgpack_writer(cmp_ctx_t *ctx, const void *data, size_t count) {
    buffer_t *msgpack_buf = ctx->buf;
    memcpy(msgpack_buf->data + msgpack_buf->offset, data, count);
    msgpack_buf->offset += count;

    return count;
}

hyper_result_t hyper_msgpack_decode_device_id(uint8_t *device_id, const uint8_t *in,
                                              uint8_t in_len) {
    cmp_ctx_t cmp_ctx = {0};
    buffer_t msgpack_buf = {(uint8_t *)in, 0};

    cmp_init(&cmp_ctx, (void *)&msgpack_buf, hyper_device_msgpack_reader,
             hyper_device_msgpack_skipper, NULL);

    // Main msg array
    uint32_t msg_array_size = 0;
    if (!cmp_read_array(&cmp_ctx, &msg_array_size)) {
        HYPER_PRINTF("%s", cmp_strerror(&cmp_ctx));
        return -1;
    }
    if (msg_array_size != 3) {
        HYPER_PRINTF("msgpack message must be an array with size 3");
        return -1;
    }

    cmp_object_t obj;

    // Skip device class id
    if (!cmp_skip_object(&cmp_ctx, &obj)) {
        HYPER_PRINTF("could not skip device class id");
        return -1;
    }

    // Read device id
    uint32_t device_id_size = 0;
    if (!cmp_read_bin_size(&cmp_ctx, &device_id_size)) {
        HYPER_PRINTF("%s", cmp_strerror(&cmp_ctx));
        return -1;
    }
    if (device_id_size != 6) {
        HYPER_PRINTF("device id must have size 6\n");
        return -1;
    }

    memcpy(device_id, in + msgpack_buf.offset, 6);

    return 0;
}

hyper_result_t hyper_msgpack_process_multimessage(void (*on_message)(uint32_t device_class_id,
                                                                     uint8_t device_id[6],
                                                                     uint8_t *data, uint8_t size),
                                                  uint8_t *input, uint8_t input_len) {

    cmp_ctx_t cmp_ctx = {0};
    buffer_t msgpack_buffer = {input, 0};
    cmp_init(&cmp_ctx, (void *)&msgpack_buffer, hyper_device_msgpack_reader,
             hyper_device_msgpack_skipper, NULL);

    cmp_object_t obj;
    uint32_t device_id_size = 0;
    uint8_t *device_id = NULL;
    uint32_t device_class_id = 0;
    uint32_t msg_array_size;
    size_t start_offset = 0;
    size_t end_offset = 0;

    do {
        msg_array_size = 0;
        device_id_size = 0;
        start_offset = msgpack_buffer.offset;

        if (!cmp_read_array(&cmp_ctx, &msg_array_size) && msg_array_size != 3) {
            if (msg_array_size == 0) {
                return HYPER_OK;
            }
            HYPER_PRINTF("Could not read array: %s(size is %d)\n", cmp_strerror(&cmp_ctx),
                         msg_array_size);
            return HYPER_ERR_DECODE;
        }

        if (!cmp_read_object(&cmp_ctx, &obj)) {
            HYPER_PRINTF("Could not read object: %s\n", cmp_strerror(&cmp_ctx));
            return HYPER_ERR_DECODE;
        }
        if (!cmp_object_as_uint(&obj, &device_class_id)) {
            HYPER_PRINTF("Could not cast device class: %s\n", cmp_strerror(&cmp_ctx));
            return HYPER_ERR_DECODE;
        }

        if (!cmp_read_bin_size(&cmp_ctx, &device_id_size)) {
            HYPER_PRINTF("Could not read device id size: %s\n", cmp_strerror(&cmp_ctx));
            return HYPER_ERR_DECODE;
        }
        
        if (device_id_size != HYPER_DEVICE_ID_LENGTH) {
            HYPER_PRINTF("%s\n", "Wrong device_id_size\n");
            return HYPER_ERR_DECODE;
        }

        device_id = input + msgpack_buffer.offset;

        msgpack_buffer.offset += device_id_size;
        
        if (!cmp_skip_object_flat(&cmp_ctx, &obj)) { // skip map needs skip flat
            HYPER_PRINTF("Could not skip: %s\n", cmp_strerror(&cmp_ctx));
            return HYPER_OK;
        }
        
        end_offset = msgpack_buffer.offset;

        on_message(device_class_id, device_id, input + start_offset, end_offset - start_offset);
    } while (msg_array_size > 0 && end_offset - start_offset < input_len);

    return HYPER_OK;
}
