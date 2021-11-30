/*
 * Copyright (c) 2021 Hyper Collective LTD
 */
#include <zephyr.h>

#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
#include <device.h>
#include <fs/fs.h>
#include "fs_mgmt/fs_mgmt.h"
#include <fs/littlefs.h>
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
#include <stats/stats.h>
#include "stat_mgmt/stat_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
#include "shell_mgmt/shell_mgmt.h"
#endif

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(smp_sample);

#include "smp.h"

#ifdef CONFIG_MCUMGR_SMP_UDP
#include <mgmt/mcumgr/smp_udp.h>
#include <net/net_mgmt.h>
#include <net/net_event.h>
#include <net/net_conn_mgr.h>

#define EVENT_MASK (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

static struct net_mgmt_event_callback mgmt_cb;

static void event_handler(struct net_mgmt_event_callback *cb,
			  uint32_t mgmt_event, struct net_if *iface)
{
	if ((mgmt_event & EVENT_MASK) != mgmt_event)
	{
		return;
	}

	if (mgmt_event == NET_EVENT_L4_CONNECTED)
	{
		LOG_INF("Network connected");

		if (smp_udp_open() < 0)
		{
			LOG_ERR("could not open smp udp");
		}

		return;
	}

	if (mgmt_event == NET_EVENT_L4_DISCONNECTED)
	{
		LOG_INF("Network disconnected");
		smp_udp_close();
		return;
	}
}

void start_smp_udp(void)
{
	LOG_INF("STARTING SMP UDP");
	net_mgmt_init_event_callback(&mgmt_cb, event_handler, EVENT_MASK);
	net_mgmt_add_event_callback(&mgmt_cb);
	net_conn_mgr_resend_status();
}
#endif

#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
/* Define an example stats group; approximates seconds since boot. */
STATS_SECT_START(smp_svr_stats)
STATS_SECT_ENTRY(ticks)
STATS_SECT_END;

/* Assign a name to the `ticks` stat. */
STATS_NAME_START(smp_svr_stats)
STATS_NAME(smp_svr_stats, ticks)
STATS_NAME_END(smp_svr_stats);

/* Define an instance of the stats group. */
STATS_SECT_DECL(smp_svr_stats)
smp_svr_stats;
#endif

#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(cstorage);
static struct fs_mount_t littlefs_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &cstorage,
    .storage_dev = (void *)FLASH_AREA_ID(storage),
    .mnt_point = "/lfs1"};
#endif

#if defined(CONFIG_MCUMGR_CMD_IMG_MGMT)
static int software_update_confirmation_handler(uint32_t offset, uint32_t size,
						void *arg)
{
	/* For now just print update progress and confirm data chunk without any additional
	 * checks.
	 */
	LOG_INF("Device firmware upgrade progress %d B / %d B", offset, size);

	return 0;
}
#endif

void smp_loop(void)
{
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
	int rc = STATS_INIT_AND_REG(smp_svr_stats, STATS_SIZE_32,
				    "smp_svr_stats");

	if (rc < 0)
	{
		LOG_ERR("Error initializing stats system [%d]", rc);
	}
#endif

	/* Register the built-in mcumgr command handlers. */
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
	rc = fs_mount(&littlefs_mnt);
	if (rc < 0)
	{
		LOG_ERR("Error mounting littlefs [%d]", rc);
	}

	fs_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
	os_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
	img_mgmt_register_group();
	img_mgmt_set_upload_cb(software_update_confirmation_handler, NULL);
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
	stat_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
	shell_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_SMP_UDP
	start_smp_udp();
#endif
	/* using __TIME__ ensure that a new binary will be built on every
	 * compile which is convient when testing firmware upgrade.
	 */
	LOG_INF("SMP: build time: " __DATE__ " " __TIME__);

	/* The system work queue handles all incoming mcumgr requests.  Let the
	 * main thread idle while the mcumgr server runs.
	 */
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
	while (1)
	{
		k_sleep(K_MSEC(1000));
		STATS_INC(smp_svr_stats, ticks);
	}
#endif
}
