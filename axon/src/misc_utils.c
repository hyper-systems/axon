#include <logging/log.h>
#include "misc_utils.h"

LOG_MODULE_REGISTER(misc_utils, CONFIG_MAIN_LOG_LEVEL);

void hyper_hexdump(uint8_t *data, uint8_t len)
{
	LOG_HEXDUMP_DBG(data, len, "");
}
