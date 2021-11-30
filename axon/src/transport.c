#include <logging/log.h>
#include "transport.h"
#ifdef CONFIG_NET_L2_OPENTHREAD
#include "ot_utils.h"
#endif

#include "coap_client.h"

LOG_MODULE_REGISTER(transport, CONFIG_MAIN_LOG_LEVEL);

int transport_publish(uint8_t *data, uint8_t data_len)
{
	return transport_coap_client_post(data, data_len);
}

int transport_init(void (*on_response_data_handler_cb)(uint8_t *data, uint8_t data_len))
{
	int ret = 0;
#ifdef CONFIG_NET_L2_OPENTHREAD
	ret = ot_utils_init();
	if (ret)
	{
		LOG_ERR("ot_utils_init() failed with exit code: %d\n", ret);
		return ret;
	}
#endif
	transport_coap_client_init(on_response_data_handler_cb);

	return ret;
}
