#ifndef __COAP_CLIENT_H__
#define __COAP_CLIENT_H__

/** @brief Initialize CoAP client.
 */
void transport_coap_client_init(void (*on_response_data_handler_cb)(uint8_t *data, uint8_t data_len));

/** @brief Send hyper_data to the CoAP server.
 */
int transport_coap_client_post(uint8_t *data, uint8_t len);

#define BORDER_ROUTER "fdde:ad00:beef::1"
#define BORDER_ROUTER_IPV4 "64:ff9b::10.3.2.1"
#define COAP_SERVER BORDER_ROUTER
#define COAP_SERVER_PORT 5683

#endif