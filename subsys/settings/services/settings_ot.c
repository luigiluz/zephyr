/** @file
 *  @brief OpenThread Settings handler
 */

/*
 * Copyright (c) 2019, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <settings/settings.h>
#include <settings/settings_ot.h>

#include <logging/log.h>

#if CONFIG_SETTINGS_OT_DEBUG
LOG_MODULE_REGISTER(settings_ot, CONFIG_SETTINGS_OT_DEBUG_LEVEL);
#endif

/* Storage path identifiers */
#define OT_SETTINGS_NAME	"ot"
#define PATH_SEPARATOR		"/"

#define PANID_KEY		"panid"
#define CHANNEL_KEY		"channel"
#define NET_NAME_KEY		"net_name"
#define XPANID_KEY		"xpanid"
#define MASTERKEY_KEY		"masterkey"

#define LOAD_PANID_KEY		PANID_KEY
#define LOAD_CHANNEL_KEY	CHANNEL_KEY
#define LOAD_NET_NAME_KEY	NET_NAME_KEY
#define LOAD_XPANID_KEY		XPANID_KEY
#define LOAD_MASTERKEY_KEY	MASTERKEY_KEY

#define SAVE_PANID_KEY		OT_SETTINGS_NAME PATH_SEPARATOR PANID_KEY
#define SAVE_CHANNEL_KEY	OT_SETTINGS_NAME PATH_SEPARATOR CHANNEL_KEY
#define SAVE_NET_NAME_KEY	OT_SETTINGS_NAME PATH_SEPARATOR NET_NAME_KEY
#define SAVE_XPANID_KEY		OT_SETTINGS_NAME PATH_SEPARATOR XPANID_KEY
#define SAVE_MASTERKEY_KEY	OT_SETTINGS_NAME PATH_SEPARATOR MASTERKEY_KEY

/* Parameters sizes */
#define NET_NAME_LEN	17
#define XPANID_LEN	24
#define MASTERKEY_LEN	48

/* Buffers */
static u16_t panid;			// PAN Id
static u8_t channel;			// Channel
static char net_name[NET_NAME_LEN];	// Network name
static char xpanid[XPANID_LEN];		// Expanded PAN Id
static char masterkey[MASTERKEY_LEN];	// Master key

/* Loaded values flags */
static bool panid_loaded;
static bool channel_loaded;
static bool net_name_loaded;
static bool xpanid_loaded;
static bool masterkey_loaded;

static int set(int argc, char **argv, void *value_ctx)
{
	int len;
	int rc;
	void *target;
	bool *loaded_flag;

	if (argc != 1)
		return -ENOENT;

	/* Get values from storage */
	if (!strcmp(argv[0], LOAD_PANID_KEY)) {
		len = sizeof(panid);
		target = &panid;
		loaded_flag = &panid_loaded;
	} else if (!strcmp(argv[0], LOAD_CHANNEL_KEY)) {
		len = sizeof(channel);
		target = &channel;
		loaded_flag = &channel_loaded;
	} else if (!strcmp(argv[0], LOAD_NET_NAME_KEY)) {
		len = sizeof(net_name);
		target = net_name;
		loaded_flag = &net_name_loaded;
	} else if (!strcmp(argv[0], LOAD_XPANID_KEY)) {
		len = sizeof(xpanid);
		target = xpanid;
		loaded_flag = &xpanid_loaded;
	} else if (!strcmp(argv[0], LOAD_MASTERKEY_KEY)) {
		len = sizeof(masterkey);
		target = masterkey;
		loaded_flag = &masterkey_loaded;
	} else {
		/* Unknown parameter */
		return -ENOENT;
	}

	rc = settings_val_read_cb(value_ctx, target, len);
	if (rc < 0){
		STTNGS_OT_ERR("Failed to load value from storage");
		return rc;
	}

	*loaded_flag = true;
	return 0;
}

static struct settings_handler ot_settings = {
	.name = OT_SETTINGS_NAME,
	.h_set = set,
};

int settings_ot_init(void)
{
	int err;

	/* Clearing flags */
	panid_loaded = false;
	channel_loaded = false;
	net_name_loaded = false;
	xpanid_loaded = false;
	masterkey_loaded = false;

	STTNGS_OT_DBG("Initializing OT Settings");
	err = settings_subsys_init();
	if (err) {
		STTNGS_OT_ERR("settings_subsys_init failed (err %d)", err);
		return err;
	}

	STTNGS_OT_DBG("Register settings handler");
	err = settings_register(&ot_settings);
	if (err) {
		STTNGS_OT_ERR("Settings register failed (err %d)", err);
		return err;
	}

	STTNGS_OT_DBG("Loading settings");
	err = settings_load();
	if (err) {
		STTNGS_OT_ERR("Settings load failed (err %d)", err);
		return err;
	}

	return 0;
}

int settings_ot_write(enum settings_ot_type type, void *src)
{
	int len;
	int err;
	bool *loaded_flag;
	void *dest;

	switch (type) {
	case SETTINGS_OT_PANID:
		/* PANID */
		STTNGS_OT_INF("Saving panid");
		len = sizeof(panid);
		dest = &panid;
		loaded_flag = &panid_loaded;

		err = settings_save_one(SAVE_PANID_KEY, src, len);
		break;
	case SETTINGS_OT_CHANNEL:
		/* CHANNEL */
		STTNGS_OT_INF("Saving channel");
		len = sizeof(channel);
		dest = &channel;
		loaded_flag = &channel_loaded;

		err = settings_save_one(SAVE_CHANNEL_KEY, src, len);
		break;
	case SETTINGS_OT_NET_NAME:
		/* NET_NAME */
		STTNGS_OT_INF("Saving net_name");
		len = sizeof(net_name);
		dest = net_name;
		loaded_flag = &net_name_loaded;

		err = settings_save_one(SAVE_NET_NAME_KEY, src, len);
		break;
	case SETTINGS_OT_XPANID:
		/* XPANID */
		STTNGS_OT_INF("Saving xpanid");
		len = sizeof(xpanid);
		dest = xpanid;
		loaded_flag = &xpanid_loaded;

		err = settings_save_one(SAVE_XPANID_KEY, src, len);
		break;
	case SETTINGS_OT_MASTERKEY:
		/* MASTERKEY */
		STTNGS_OT_INF("Saving masterkey");
		len = sizeof(masterkey);
		dest = masterkey;
		loaded_flag = &masterkey_loaded;

		err = settings_save_one(SAVE_MASTERKEY_KEY, src, len);
		break;
	default:
		STTNGS_OT_ERR("Failed to save setting. Item not supported");
		return -ENOENT;
	}

	/* Return error code if any */
	if (err) {
		STTNGS_OT_ERR("Setting save failed (err %d)", err);
		return err;
	}


	/* Save value on buffer if no error found */
	memcpy(dest, src, len);
	*loaded_flag = true;
	return len;
}

int settings_ot_read(enum settings_ot_type type, void *dest)
{
	int len;
	void *src;

	switch (type) {
	case SETTINGS_OT_PANID:
		if (!panid_loaded)
			return -ESRCH; // Item not stored
		STTNGS_OT_INF("Loading panid");
		len = sizeof(panid);
		src = &panid;
		break;
	case SETTINGS_OT_CHANNEL:
		if (!channel_loaded)
			return -ESRCH; // Item not stored
		STTNGS_OT_INF("Loading channel");
		len = sizeof(channel);
		src = &channel;
		break;
	case SETTINGS_OT_NET_NAME:
		if (!net_name_loaded)
			return -ESRCH; // Item not stored
		STTNGS_OT_INF("Loading net_name");
		len = sizeof(net_name);
		src = net_name;
		break;
	case SETTINGS_OT_XPANID:
		if (!xpanid_loaded)
			return -ESRCH; // Item not stored
		STTNGS_OT_INF("Loading xpanid");
		len = sizeof(xpanid);
		src = xpanid;
		break;
	case SETTINGS_OT_MASTERKEY:
		if (!masterkey_loaded)
			return -ESRCH; // Item not stored
		STTNGS_OT_INF("Loading masterkey");
		len = sizeof(masterkey);
		src = masterkey;
		break;
	default:
		STTNGS_OT_ERR("Failed to load setting. Item not supported");
		return -ENOENT;
	}
	/* Write value on destination */
	memcpy(dest, src, len);

	return len;
}
