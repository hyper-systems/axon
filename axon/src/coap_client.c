/*
 * Copyright (c) 2021 Hyper Collective LTD
 */
#include <zephyr.h>
#include <net/coap_utils.h>
#include <logging/log.h>
#include <net/openthread.h>
#include <net/socket.h>
#include <openthread/thread.h>
#include "coap_client.h"
#ifdef CONFIG_NET_L2_OPENTHREAD
#include "ot_utils.h"
#endif
#include "misc_utils.h"

LOG_MODULE_REGISTER(coap_client, CONFIG_COAP_CLIENT_LOG_LEVEL);

#define CONSOLE_LABEL DT_LABEL(DT_CHOSEN(zephyr_console))

/* Options supported by the server */
static char from_addr_str[INET6_ADDRSTRLEN];
static const char *const coap_uri_path_options[] = {"incoming", "v3", NULL};

void (*on_response_data_handler)(uint8_t *data, uint8_t data_len) = NULL;

/* Variable for storing server address acquiring in provisioning handshake */
static struct sockaddr_in6 unique_local_addr = {
	.sin6_family = AF_INET6,
	.sin6_port = htons(COAP_SERVER_PORT),
	.sin6_addr.s6_addr = {
		0,
	},
	.sin6_scope_id = 0U};

static int coap_post_reply(const struct coap_packet *response,
						   struct coap_reply *reply,
						   const struct sockaddr *from)
{
	int ret = 0;
	const uint8_t *data;
	uint16_t data_len = 0u;
	uint8_t response_code;
	ARG_UNUSED(reply);

	data = coap_packet_get_payload(response, &data_len);

	inet_ntop(AF_INET6, &net_sin6(from)->sin6_addr, from_addr_str,
			  INET6_ADDRSTRLEN);

	response_code = coap_header_get_code(response);
	LOG_INF("Received CoAP response from address %s with code %u.%02d",
			from_addr_str,
			(0b11110000 & response_code) >> 5,
			0b00001111 & response_code);

	switch (response_code)
	{
	case COAP_RESPONSE_CODE_CONTENT:
		if (data_len > 0)
		{
			on_response_data_handler((uint8_t *)data, data_len);
		}
		else
		{
			on_response_data_handler(NULL, 0);
			LOG_INF("No data received!");
		}
		break;

	default:
		if (data_len > 0)
		{
			LOG_INF("CoAP Payload:  datalen: %u data(string):%.*s\n", data_len, (int)data_len, data);
		}
		else
		{
			LOG_INF("No data received!");
		}
	}

	return ret;
}

static int coap_client_post(uint8_t *data, uint8_t len)
{
	if (!inet_pton(AF_INET6, COAP_SERVER, &unique_local_addr.sin6_addr))
	{
		LOG_WRN("error converting address");
		return -1;
	}
	if (unique_local_addr.sin6_addr.s6_addr16[0] == 0)
	{
		LOG_WRN("Peer address not set.");
		return -1;
	}
	LOG_INF("===== COAP sending hex =====");
	hyper_hexdump(data, len);
	LOG_INF("Send 'hyper_data' request to: %s",
			log_strdup(COAP_SERVER));

	int ret = coap_send_request(COAP_METHOD_POST,
								(const struct sockaddr *)&unique_local_addr,
								coap_uri_path_options, data, len, coap_post_reply);

	LOG_INF("coap_send_request return: %d", ret);
	return ret;
}

void transport_coap_client_init(void (*on_response_data_handler_cb)(uint8_t *data, uint8_t data_len))
{
	on_response_data_handler = on_response_data_handler_cb;
	coap_init(AF_INET6, NULL);
}

int transport_coap_client_post(uint8_t *data, uint8_t len)
{
	return coap_client_post(data, len);
}