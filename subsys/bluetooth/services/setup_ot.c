/** @file
 *  @brief GATT OpenThread Setup Service
 */

/*
 * Copyright (c) 2019, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <misc/byteorder.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr.h>
#include <init.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

/* String sizes */
#define NET_NAME_LEN 16
#define XPANID_LEN 23
#define MASTERKEY_LEN 47

static u16_t panid;			// PAN Id
static u8_t channel;			// Channel
static char net_name[NET_NAME_LEN];	// Network name
static char xpanid[XPANID_LEN];		// Expanded PAN Id
static char masterkey[MASTERKEY_LEN];	// Master key

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

/* Return true if address points to an uint16 */
static bool is_uint16(const void *addr)
{
	/* Only multibyte integer is panid */
	return (addr == &panid);
}

/* Return variable length based on its address */
static u16_t get_buffer_len(const void *addr)
{
	if (addr == &panid)
		return sizeof(panid);
	else if (addr == &channel)
		return sizeof(channel);
	else if (addr == &net_name)
		return sizeof(net_name);
	else if (addr == &xpanid)
		return sizeof(xpanid);
	else if (addr == &masterkey)
		return sizeof(masterkey);
	else
		return 0;
}

/* Read characteristic callback function */
static ssize_t read_chrc(struct bt_conn *conn,
			   const struct bt_gatt_attr *attr, void *buf,
			   u16_t len, u16_t offset)
{
	const char *value = attr->user_data;

	/* Using little-endian for uint16 values */
	if (is_uint16(value))
		(*(u16_t *) value) = sys_cpu_to_le16(* (u16_t *) value);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 get_buffer_len(value));
}

/* Write characteristic callback function */
static ssize_t write_chrc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, u16_t len, u16_t offset,
			 u8_t flags)
{
	u16_t max_len;
	u8_t *value = attr->user_data;

	max_len = get_buffer_len(value);

	if (offset + len > max_len)
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

	if (offset == 0)
		memset(value, 0, max_len);

	memcpy(value + offset, buf, len);

	/* Check for prepare write flag */
	if (flags & BT_GATT_WRITE_FLAG_PREPARE)
		return 0;

	/* Using little-endian for uint16 values */
	if (is_uint16(value))
		(*(u16_t *) value) = sys_le16_to_cpu(* (u16_t *) value);

	return len;
}

/* GATT Service Declaration */
static struct bt_gatt_attr attrs[] = {
	BT_GATT_PRIMARY_SERVICE(&service_uuid),
	BT_GATT_CHARACTERISTIC(&channel_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			read_chrc, write_chrc,
			&channel),
	BT_GATT_CHARACTERISTIC(&net_name_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			read_chrc, write_chrc,
			net_name),
	BT_GATT_CHARACTERISTIC(&panid_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			read_chrc, write_chrc,
			&panid),
	BT_GATT_CHARACTERISTIC(&xpanid_uuid.uuid,
			BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			read_chrc, write_chrc,
			xpanid),
	BT_GATT_CHARACTERISTIC(&masterkey_uuid.uuid,
			BT_GATT_CHRC_WRITE,
			BT_GATT_PERM_WRITE |
			BT_GATT_PERM_PREPARE_WRITE,
			NULL, write_chrc,
			masterkey),
};

static struct bt_gatt_service gatt_ot_svc = BT_GATT_SERVICE(attrs);

/* Prepare buffers to receive data */
static void clear_buffers(void)
{
	memset(&panid, 0, sizeof(panid));
	memset(&channel, 0, sizeof(channel));
	memset(&net_name, 0, sizeof(net_name));
	memset(&xpanid, 0, sizeof(xpanid));
	memset(&masterkey, 0, sizeof(masterkey));
}

static int gatt_ot_init(struct device *dev)
{
	ARG_UNUSED(dev);

	clear_buffers();

	return bt_gatt_service_register(&gatt_ot_svc);
}

SYS_INIT(gatt_ot_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
