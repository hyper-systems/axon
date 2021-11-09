#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__
#include <stdint.h>



int transport_init(void (*on_response_data_handler_cb)(uint8_t *data, uint8_t data_len));
int transport_publish(uint8_t *data, uint8_t data_len);

#endif