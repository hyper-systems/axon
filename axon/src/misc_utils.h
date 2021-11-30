#ifndef __MISC_UTILS_H__
#define __MISC_UTILS_H__
#include <stdint.h>

void hyper_hexdump(uint8_t *data, uint8_t len);
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)           \
	(byte & 0x80 ? '1' : '0'),     \
	    (byte & 0x40 ? '1' : '0'), \
	    (byte & 0x20 ? '1' : '0'), \
	    (byte & 0x10 ? '1' : '0'), \
	    (byte & 0x08 ? '1' : '0'), \
	    (byte & 0x04 ? '1' : '0'), \
	    (byte & 0x02 ? '1' : '0'), \
	    (byte & 0x01 ? '1' : '0')

#endif
