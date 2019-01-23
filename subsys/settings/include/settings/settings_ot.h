/*
 * Copyright (c) 2019, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __STTNGS_OT_LOG_H
#define __STTNGS_OT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Logging enable */
#if CONFIG_SETTINGS_OT_DEBUG

#define STTNGS_OT_DBG(fmt, ...) LOG_DBG(fmt, ##__VA_ARGS__)
#define STTNGS_OT_ERR(fmt, ...) LOG_ERR(fmt, ##__VA_ARGS__)
#define STTNGS_OT_WARN(fmt, ...) LOG_WRN(fmt, ##__VA_ARGS__)
#define STTNGS_OT_INF(fmt, ...) LOG_INF(fmt, ##__VA_ARGS__)

#else
#define STTNGS_OT_DBG(fmt, ...)
#define STTNGS_OT_ERR(fmt, ...)
#define STTNGS_OT_WRN(fmt, ...)
#define STTNGS_OT_INF(fmt, ...)

#endif

/* Setting types identifiers */
enum settings_ot_type {
	SETTINGS_OT_PANID,
	SETTINGS_OT_CHANNEL,
	SETTINGS_OT_NET_NAME,
	SETTINGS_OT_XPANID,
	SETTINGS_OT_MASTERKEY,
	SETTINGS_OT_NONE
};

/* Initialize settings subsystem and load stored values */
int settings_ot_init(void);

/* Return number of written bytes or negative on failure */
int settings_ot_write(enum settings_ot_type type, void *src);

/* Return number of written bytes or negative on failure */
int settings_ot_read(enum settings_ot_type type, void *dest);

#ifdef __cplusplus
}
#endif

#endif
