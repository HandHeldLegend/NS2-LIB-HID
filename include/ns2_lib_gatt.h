/**
 * @file ns2_lib_gatt.h
 * @brief Stack-agnostic description of the Switch 2 controller GATT database.
 *
 * The console addresses attributes by fixed handle, so the BLE stack must place every
 * attribute at exactly the handle listed here (starting at 0x0001, with no stack-provided
 * services in front). @ref ns2_gatt_get_table returns the attribute list in handle order so a
 * transport can serialize it into its own database format.
 *
 * Copyright (c) 2026 Mitchell Cairns, Hand Held Legend, LLC.
 *
 * Licensed under the Creative Commons Attribution-NonCommercial 4.0 International License
 * (CC BY-NC 4.0). Non-commercial use with attribution; commercial licensing from Hand Held Legend.
 * Full terms: https://creativecommons.org/licenses/by-nc/4.0/legalcode — see LICENSING.md in this folder
 *
 * SPDX-License-Identifier: CC-BY-NC-4.0
 *
 * TRADEMARK AND AFFILIATION DISCLAIMER:
 * This library is not affiliated, associated, authorized, endorsed by, or in any way officially
 * connected with Nintendo Co., Ltd., or any of its subsidiaries or its affiliates. Nintendo and
 * related marks are trademarks of their respective owners.
 */

#ifndef NS2_LIB_GATT_H
#define NS2_LIB_GATT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GATT characteristic property bits (Bluetooth Core, Vol 3 Part G 3.3.1.1). */
#define NS2_GATT_PROP_READ 0x02u
#define NS2_GATT_PROP_WRITE_WITHOUT_RESPONSE 0x04u
#define NS2_GATT_PROP_WRITE 0x08u
#define NS2_GATT_PROP_NOTIFY 0x10u

/* --- Attribute handles (Pro Controller 2, firmware 2.x layout) --- */

/* Configuration service 00c5af5d-...-d280 */
#define NS2_HANDLE_CONFIG_SERVICE 0x0001u
#define NS2_HANDLE_CONFIG_0003_VALUE 0x0003u /**< READ, unknown. */
#define NS2_HANDLE_CONFIG_0005_VALUE 0x0005u /**< WRITE, console writes `01 00`. */
#define NS2_HANDLE_CONFIG_0007_VALUE 0x0007u /**< READ, unknown. */

/* HID service ab7de9be-...-7fd0 */
#define NS2_HANDLE_HID_SERVICE 0x0008u
#define NS2_HANDLE_COMMON_INPUT_VALUE 0x000Au /**< READ|NOTIFY, input report 0x05. */
#define NS2_HANDLE_COMMON_INPUT_CCC 0x000Bu
#define NS2_HANDLE_COMMON_INPUT_RATE 0x000Cu
#define NS2_HANDLE_PRO2_INPUT_VALUE 0x000Eu /**< READ|NOTIFY, input report 0x09. */
#define NS2_HANDLE_PRO2_INPUT_CCC 0x000Fu
#define NS2_HANDLE_PRO2_INPUT_RATE 0x0010u
#define NS2_HANDLE_VIBRATION_VALUE 0x0012u /**< WRITE_NO_RSP, HD rumble output. */
#define NS2_HANDLE_COMMAND_VALUE 0x0014u /**< WRITE_NO_RSP, command (responses on 0x001A). */
#define NS2_HANDLE_VIB_COMMAND_VALUE 0x0016u /**< WRITE_NO_RSP, rumble + command (responses on 0x001E). */
#define NS2_HANDLE_FW_UPDATE_VALUE 0x0018u /**< WRITE_NO_RSP, chunked large commands. */
#define NS2_HANDLE_CMD_RESPONSE_VALUE 0x001Au /**< NOTIFY, command response #1. */
#define NS2_HANDLE_CMD_RESPONSE_CCC 0x001Bu
#define NS2_HANDLE_CMD_RESPONSE_DESC 0x001Cu
#define NS2_HANDLE_VIB_CMD_RESPONSE_VALUE 0x001Eu /**< NOTIFY, command response #2. */
#define NS2_HANDLE_VIB_CMD_RESPONSE_CCC 0x001Fu
#define NS2_HANDLE_VIB_CMD_RESPONSE_DESC 0x0020u
#define NS2_HANDLE_UNKNOWN_0022_VALUE 0x0022u
#define NS2_HANDLE_UNKNOWN_0022_CCC 0x0023u
#define NS2_HANDLE_UNKNOWN_0022_DESC 0x0024u
#define NS2_HANDLE_UNKNOWN_0026_VALUE 0x0026u
#define NS2_HANDLE_UNKNOWN_0026_CCC 0x0027u
#define NS2_HANDLE_UNKNOWN_0026_RATE 0x0028u
#define NS2_HANDLE_UNKNOWN_002A_VALUE 0x002Au
#define NS2_HANDLE_HEADSET_OUT_VALUE 0x002Cu
#define NS2_HANDLE_HEADSET_IN_VALUE 0x002Eu
#define NS2_HANDLE_HEADSET_IN_CCC 0x002Fu
#define NS2_HANDLE_HEADSET_IN_RATE 0x0030u
#define NS2_HANDLE_UNKNOWN_0032_VALUE 0x0032u

/* Standard services, placed after the vendor services like on retail controllers. */
#define NS2_HANDLE_GAP_SERVICE 0x0033u
#define NS2_HANDLE_GAP_DEVICE_NAME_VALUE 0x0035u
#define NS2_HANDLE_GAP_APPEARANCE_VALUE 0x0037u
#define NS2_HANDLE_GATT_SERVICE 0x0038u

typedef enum
{
    /** Primary service declaration at `handle`. */
    NS2_ATT_PRIMARY_SERVICE,
    /** Characteristic declaration at `handle`, value attribute at `handle + 1`. */
    NS2_ATT_CHARACTERISTIC,
    /** Client Characteristic Configuration descriptor (0x2902). */
    NS2_ATT_CCC,
    /** Vendor descriptor with a 128-bit UUID (readable/writable, dynamic). */
    NS2_ATT_DESCRIPTOR,
} ns2_att_kind_t;

/**
 * @brief One entry of the GATT table.
 *
 * Exactly one of `uuid16` (non-zero) or `uuid128` (non-NULL) identifies the service,
 * characteristic value, or descriptor. 128-bit UUIDs are stored little-endian (on-air order).
 */
typedef struct
{
    uint16_t handle;
    uint8_t kind;           /**< @ref ns2_att_kind_t */
    uint8_t properties;     /**< NS2_GATT_PROP_* bits (characteristics only). */
    uint16_t uuid16;
    const uint8_t *uuid128;
    /** Static value for characteristics; NULL means the value is dynamic (served by the library). */
    const uint8_t *value;
    uint16_t value_len;
} ns2_gatt_attribute_s;

/**
 * @brief Get the GATT attribute table in ascending handle order.
 *
 * @param[out] count Number of entries.
 * @return Pointer to the static table.
 */
const ns2_gatt_attribute_s *ns2_gatt_get_table(uint16_t *count);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_GATT_H */
