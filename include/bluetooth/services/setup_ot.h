/** @file
 *  @brief GATT OpenThread Setup Service
 */

/*
 * Copyright (c) 2019, CESAR. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Callback function to be called everytime after a successful value change */
typedef void (*setup_ot_updated_cb)(void);

void setup_ot_updated_cb_register(setup_ot_updated_cb cb);

#ifdef __cplusplus
}
#endif
