#include <logging/log.h>
#include <stdio.h>
#include "hyper_device_utils.h"

#include "cmp.h"

LOG_MODULE_REGISTER(hyper_device_data_utils, CONFIG_MAIN_LOG_LEVEL);

typedef struct
{
    uint8_t *buf;
    size_t offset;
} msgpack_t;

static bool msgpack_reader(cmp_ctx_t *ctx, void *data, size_t count)
{
    msgpack_t *msgpack = (msgpack_t *)ctx->buf;
    memcpy(data, msgpack->buf + msgpack->offset, count);
    msgpack->offset += count;
    return true;
}

int hyper_msgpack_extract_device_id(uint8_t *device_id, uint8_t *data, uint8_t data_len)
{
    cmp_ctx_t cmp_ctx = {0};
    uint8_t buffer[data_len];

    memcpy(buffer, data, data_len);
    msgpack_t msgpack = {buffer, 0};

    cmp_init(&cmp_ctx, (void *)&msgpack, msgpack_reader, NULL, NULL);

    uint32_t msg_array_size = 0;

    // Main msg array
    if (!cmp_read_array(&cmp_ctx, &msg_array_size) && msg_array_size != 3)
    {
        LOG_ERR("cmp_read_array() error '%s'!", cmp_strerror(&cmp_ctx));
        return -1;
    }
    printf("hyper_device_decode: msg_array_size=%lu\n", (long unsigned)msg_array_size);

    cmp_object_t obj;
    // Device class id
    if (!cmp_read_object(&cmp_ctx, &obj))
    {
        LOG_ERR("cmp_read_object() error 'could not read object'!");
        return -1;
    }
    uint32_t device_class_id;
    if (!cmp_object_as_uint(&obj, &device_class_id))
    {
        LOG_ERR("cmp_object_as_uint() error 'could not read device_class_id'!");
        return -1;
    }

    // Device id
    uint32_t device_id_size = 0;
    if (!cmp_read_bin_size(&cmp_ctx, &device_id_size))
    {
        LOG_ERR("cmp_read_array() error '%s'!", cmp_strerror(&cmp_ctx));
        return -1;
    }

    if (device_id_size != 6)
    {
        LOG_ERR("cmp_object_as_uint() error 'wrong device_id_size'!");
        return -1;
    }

    memcpy(device_id, data + msgpack.offset, 6);

    return 0;
}

void hyper_extensions_registry_insert(hyper_device_reg_t *reg, uint8_t *device_id,
                                      uint8_t (*get_data)(uint8_t *data),
                                      void (*set_data)(uint8_t *data, uint8_t data_len))
{
    uint8_t zeros[6] = {0};
    for (int i = 0; i < HYPER_EXTENSIONS_REGISTRY_MAX; i++)
    {
        // Add only if empty
        if (!memcmp(reg[i].device_id, zeros, sizeof(reg[i].device_id)))
        {
            // Add only if it doesnt exist yet
            if (memcmp(reg[i].device_id, device_id, sizeof(reg[i].device_id)))
            {
                memcpy(reg[i].device_id, device_id, sizeof(reg[i].device_id));
                reg[i].get_data = get_data;
                reg[i].set_data = set_data;
                LOG_HEXDUMP_DBG(device_id, 6, "device added to hyper_extensions_registry:");
                LOG_DBG("reg[i].get_data: %p\n", reg[i].get_data);
                LOG_DBG("reg[i].set_data: %p\n", reg[i].set_data);
                break;
            }
            else
            {
                LOG_ERR("extension already exists in hyper_extensions_registry, pos: %d\n", i);
                LOG_HEXDUMP_DBG(reg[i].device_id, 6, "reg[i].device_id");
                LOG_HEXDUMP_DBG(device_id, 6, "device_id:");
            }
        }
    }
}