/*
 * Copyright (c) 2021 Hyper Collective LTD
 */
#include <zephyr.h>
#include <logging/log.h>

#if defined(CONFIG_MCUMGR)
#include "smp.h"
#endif
#ifdef CONFIG_BOOTLOADER_MCUBOOT
#include <dfu/mcuboot.h>
#endif
#include <ram_pwrdn.h>

#include "axon.h"
#include "controller.h"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

void main(void)
{
	int ret;

	if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY))
	{
		power_down_unused_ram();
	}

#ifdef CONFIG_BOOTLOADER_MCUBOOT
	/* Check if the image is run in the REVERT mode and eventually
	 * confirm it to prevent reverting on the next boot.
	 */
	if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT)
	{
		if (boot_write_img_confirmed())
		{
			LOG_ERR("Confirming firmware image failed, it will be reverted on the next "
					"boot.");
		}
		else
		{
			LOG_INF("New device firmware image confirmed.");
		}
	}
#endif

	// Hardware related inits
	ret = axon_init();
	if (ret)
	{
		LOG_ERR("axon_init() failed with exit code: %d\n", ret);
		k_panic();
	}

	// detect hyper devices and init them
	ret = hyper_controller_init();
	if (ret)
	{
		LOG_ERR("hyper_controller_init() failed with exit code: %d\n", ret);
	}

#if defined(CONFIG_MCUMGR)
	smp_loop();
#endif

	LOG_INF("Axon main() initialized!");
}
