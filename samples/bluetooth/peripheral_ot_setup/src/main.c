/* main.c - Application main entry point */

/*
 * Copyright (c) 2019, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include <settings/settings.h>
#include <settings/settings_ot.h>

/* Buffers sizes */
#define NET_NAME_LEN	17
#define XPANID_LEN	24
#define MASTERKEY_LEN	48

static u16_t panid;			// PAN Id
static u8_t channel;			// Channel
static char net_name[NET_NAME_LEN];	// Network name
static char xpanid[XPANID_LEN];		// Expanded PAN Id
static char masterkey[MASTERKEY_LEN];	// Master key

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR))
};

static void connected(struct bt_conn *conn, u8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

/* Get stored values and put into buffers */
static int load_values(void)
{
	int rc = 0;

	rc = settings_ot_read(SETTINGS_OT_PANID, &panid);
	if (rc < 0) {
		printk("Failed to read panid\n");
		return rc;
	}
	rc = settings_ot_read(SETTINGS_OT_CHANNEL, &channel);
	if (rc < 0) {
		printk("Failed to read channel\n");
		return rc;
	}
	rc = settings_ot_read(SETTINGS_OT_XPANID, xpanid);
	if (rc < 0) {
		printk("Failed to read xpanid\n");
		return rc;
	}
	rc = settings_ot_read(SETTINGS_OT_NET_NAME, net_name);
	if (rc < 0) {
		printk("Failed to read net_name\n");
		return rc;
	}
	rc = settings_ot_read(SETTINGS_OT_MASTERKEY, masterkey);
	if (rc < 0) {
		printk("Failed to read masterkey\n");
		return rc;
	}

	return 0; // Not descriptive return value
}

/* Print buffered values */
static void print_buffers(void)
{
	printk("BUFFERED VALUES:\n");
	printk("\tpanid: %u\n", panid);
	printk("\tchannel: %u\n", channel);
	printk("\txpanid: %s\n", xpanid);
	printk("\tnet_name: %s\n", net_name);
	printk("\tmasterkey: %s\n", masterkey);
}

void main(void)
{
	int err;

	err = settings_ot_init();
	if (err) {
		printk("Settings OT init failed (err %d)\n", err);
		return;
	}

	printk("Loading stored values\n");
	err = settings_load();
	if (err)
		printk("Settings load failed (err %d)\n", err);

	err = load_values();
	if (err)
		printk("OT settings not loaded\n");
	else
		print_buffers();

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	bt_conn_cb_register(&conn_callbacks);

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");

	/* Reset OT settings if invalid PAN ID detected on storage */
	if (panid == 0xFFFF) {
		printk("Reseting OT settings\n");
		err = settings_ot_reset();
	}
}
