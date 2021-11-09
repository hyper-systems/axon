/**
 * @file
 * @defgroup ot_utils API
 * @{
 */

/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2021 Hyper Collective LTD
 * 
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef __OT_UTILS_H__
#define __OT_UTILS_H__

#include <stdbool.h>

/** @brief Initialize OpenThread.
 */
int ot_utils_init(void);

/** @brief Toggle SED to MED and MED to SED modes.
 *
 * @note Active when the device is working as Minimal Thread Device.
 */
void ot_utils_toggle_minimal_sleepy_end_device(void);

bool ot_is_connected(void);

#endif

/**
 * @}
 */
