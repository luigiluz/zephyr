/** @file
 *  @brief GATT OpenThread Setup Service
 */

/*
 * Copyright (c) 2019, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <zephyr/types.h>
#include <misc/byteorder.h>
#include <init.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/setup_ot.h>

#include <settings/settings_ot.h>

/* String sizes */
#define NET_NAME_LEN	17
#define XPANID_LEN	24
#define MASTERKEY_LEN	48

static u16_t panid;			// PAN Id
static u8_t channel;			// Channel
static char net_name[NET_NAME_LEN];	// Network name
static char xpanid[XPANID_LEN];		// Expanded PAN Id
static char masterkey[MASTERKEY_LEN];	// Master key

setup_ot_updated_cb updated_cb = NULL;

#define READ_STR_CALLBACK_DECLARE(KEY, VALUE)				       \
	static ssize_t VALUE ## _read_cb(struct bt_conn *conn,		       \
					 const struct bt_gatt_attr *attr,      \
					 void *buf, u16_t len, u16_t offset)   \
{									       \
	return str_read_cb(conn, attr, buf, len, offset, KEY);		       \
}

#define WRITE_STR_CALLBACK_DECLARE(KEY, VALUE)				       \
	static ssize_t VALUE ## _write_cb(struct bt_conn *conn,		       \
					 const struct bt_gatt_attr *attr,      \
					 const void *buf, u16_t len,	       \
					 u16_t offset, u8_t flags)	       \
{									       \
	return str_write_cb(conn, attr, buf, len, offset, flags,	       \
			  KEY, sizeof(VALUE) -1);			       \
}

#define READ_CALLBACK_USE(VALUE) VALUE ## _read_cb

#define WRITE_CALLBACK_USE(VALUE) VALUE ## _write_cb

/* Custom Service Variables */
static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
	0x30, 0x0d, 0x90, 0xb4, 0x7b, 0x81, 0xec, 0x9b,
	0x41, 0xd4, 0x9a, 0xaa, 0x9c, 0xe4, 0xa9, 0xa8);

const static struct bt_uuid_128 channel_uuid = BT_UUID_INIT_128(
	0x31, 0x0d, 0x90, 0xb4, 0x7b, 0x81, 0xec, 0x9b,
	0x41, 0xd4, 0x9a, 0xaa, 0x9c, 0xe4, 0xa9, 0xa8);

const static struct bt_uuid_128 net_name_uuid = BT_UUID_INIT_128(
	0x32, 0x0d, 0x90, 0xb4, 0x7b, 0x81, 0xec, 0x9b,
	0x41, 0xd4, 0x9a, 0xaa, 0x9c, 0xe4, 0xa9, 0xa8);

const static struct bt_uuid_128 panid_uuid = BT_UUID_INIT_128(
	0x33, 0x0d, 0x90, 0xb4, 0x7b, 0x81, 0xec, 0x9b,
	0x41, 0xd4, 0x9a, 0xaa, 0x9c, 0xe4, 0xa9, 0xa8);

const static struct bt_uuid_128 xpanid_uuid = BT_UUID_INIT_128(
	0x34, 0x0d, 0x90, 0xb4, 0x7b, 0x81, 0xec, 0x9b,
	0x41, 0xd4, 0x9a, 0xaa, 0x9c, 0xe4, 0xa9, 0xa8);

const static struct bt_uuid_128 masterkey_uuid = BT_UUID_INIT_128(
	0x35, 0x0d, 0x90, 0xb4, 0x7b, 0x81, 0xec, 0x9b,
	0x41, 0xd4, 0x9a, 0xaa, 0x9c, 0xe4, 0xa9, 0xa8);

/* Read string characteristic function */
static ssize_t str_read_cb(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     void *buf, u16_t len, u16_t offset,
			     const enum settings_ot_type type)
{
	int rc;
	u16_t max_len;
	void *value = attr->user_data;

	/* Get updated value*/
	rc = settings_ot_read(type, value);
	if (rc < 0)
		return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);

	max_len = strlen(value);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, max_len);
}

/* Read uint8 characteristic generic function */
static ssize_t channel_read_cb(struct bt_conn *conn,
			    const struct bt_gatt_attr *attr, void *buf,
			    u16_t len, u16_t offset)
{
	int rc;
	u16_t max_len = sizeof(channel);
	u8_t *value = attr->user_data;

	/* Get updated value*/
	rc = settings_ot_read(SETTINGS_OT_CHANNEL, value);
	if (rc != max_len)
		return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, max_len);
}

/* Read uint16 characteristic callback function */
static ssize_t panid_read_cb(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr, void *buf,
			     u16_t len, u16_t offset)
{
	int rc;
	u16_t max_len = sizeof(panid);
	u16_t *value = attr->user_data;
	u16_t value_le;

	/* Get updated value*/
	rc = settings_ot_read(SETTINGS_OT_PANID, value);
	if (rc != max_len)
		return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);

	/* Using little-endian for uint16 values */
	value_le = sys_le16_to_cpu(*value);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &value_le, max_len);
}

/* Run value updated callback if set */
static void run_updated_cb(void)
{
	if (updated_cb != NULL)
		updated_cb();
}

/* Write string characteristic callback function */
static ssize_t str_write_cb(struct bt_conn *conn,
			    const struct bt_gatt_attr *attr, const void *buf,
			    u16_t len, u16_t offset, u8_t flags,
			    const enum settings_ot_type type, u16_t max_len)
{
	u8_t *value = attr->user_data;

	if (offset + len > max_len)
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

	if (offset == 0)
		memset(value, 0, max_len);

	memcpy(value + offset, buf, len);

	/* Check for prepare write flag */
	if (flags & BT_GATT_WRITE_FLAG_PREPARE)
		return 0;

	/* Store value at Settings */
	settings_ot_write(type, value);

	run_updated_cb();

	return len;
}

/* Write PANID characteristic callback function */
static ssize_t panid_write_cb(struct bt_conn *conn,
			      const struct bt_gatt_attr *attr, const void *buf,
			      u16_t len, u16_t offset, u8_t flags)
{
	u16_t *value = attr->user_data;
	u16_t max_len = sizeof(panid);

	if (offset + len > max_len)
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

	if (offset == 0)
		memset(value, 0, max_len);

	memcpy(value + offset, buf, len);

	/* Using little-endian for uint16 values */
	*value = sys_le16_to_cpu(*value);

	/* Store value at Settings */
	settings_ot_write(SETTINGS_OT_PANID, value);

	run_updated_cb();

	return len;
}

/* Write Channel characteristic callback function */
static ssize_t channel_write_cb(struct bt_conn *conn,
				const struct bt_gatt_attr *attr,
				const void *buf, u16_t len,
				u16_t offset, u8_t flags)
{
	u16_t *value = attr->user_data;
	u16_t max_len = sizeof(channel);

	if (offset + len > max_len)
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

	if (offset == 0)
		memset(value, 0, max_len);

	memcpy(value + offset, buf, len);

	/* Store value at Settings */
	settings_ot_write(SETTINGS_OT_CHANNEL, value);

	run_updated_cb();

	return len;
}

READ_STR_CALLBACK_DECLARE(SETTINGS_OT_NET_NAME, net_name)
READ_STR_CALLBACK_DECLARE(SETTINGS_OT_XPANID, xpanid)

WRITE_STR_CALLBACK_DECLARE(SETTINGS_OT_NET_NAME, net_name)
WRITE_STR_CALLBACK_DECLARE(SETTINGS_OT_XPANID, xpanid)
WRITE_STR_CALLBACK_DECLARE(SETTINGS_OT_MASTERKEY, masterkey)

/* GATT Service Declaration */
static struct bt_gatt_attr attrs[] = {
	BT_GATT_PRIMARY_SERVICE(&service_uuid),
	BT_GATT_CHARACTERISTIC(&channel_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT,
			channel_read_cb, channel_write_cb,
			&channel),
	BT_GATT_CHARACTERISTIC(&net_name_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT,
			READ_CALLBACK_USE(net_name),
			WRITE_CALLBACK_USE(net_name),
			net_name),
	BT_GATT_CHARACTERISTIC(&panid_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT,
			panid_read_cb, panid_write_cb,
			&panid),
	BT_GATT_CHARACTERISTIC(&xpanid_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT |
			BT_GATT_PERM_PREPARE_WRITE,
			READ_CALLBACK_USE(xpanid),
			WRITE_CALLBACK_USE(xpanid),
			xpanid),
	BT_GATT_CHARACTERISTIC(&masterkey_uuid.uuid,
			BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_WRITE_ENCRYPT |
			BT_GATT_PERM_PREPARE_WRITE,
			NULL,
			WRITE_CALLBACK_USE(masterkey),
			masterkey),
};

static struct bt_gatt_service gatt_ot_svc = BT_GATT_SERVICE(attrs);

static int gatt_ot_init(struct device *dev)
{
	ARG_UNUSED(dev);

	return bt_gatt_service_register(&gatt_ot_svc);
}

void setup_ot_updated_cb_register(setup_ot_updated_cb cb)
{
	updated_cb = cb;
}

SYS_INIT(gatt_ot_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
