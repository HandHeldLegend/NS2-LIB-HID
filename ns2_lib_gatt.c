/**
 * @file ns2_lib_gatt.c
 * @brief Switch 2 controller GATT attribute table (@ref ns2_lib_gatt.h).
 *
 * Layout follows a retail Pro Controller 2 on firmware 2.x, as documented by
 * ndeadly/switch2_controller_research (bluetooth_interface.md).
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

#include <stddef.h>

#include "ns2_lib_gatt.h"

/* 128-bit UUIDs, little-endian (on-air) byte order. */

/* 00c5af5d-1964-4e30-8f51-1956f96bd280 */
static const uint8_t NS2_UUID_SVC_CONFIG[16] = {0x80, 0xd2, 0x6b, 0xf9, 0x56, 0x19, 0x51, 0x8f, 0x30, 0x4e, 0x64, 0x19, 0x5d, 0xaf, 0xc5, 0x00};
/* 00c5af5d-1964-4e30-8f51-1956f96bd281 */
static const uint8_t NS2_UUID_CHR_CONFIG_0003[16] = {0x81, 0xd2, 0x6b, 0xf9, 0x56, 0x19, 0x51, 0x8f, 0x30, 0x4e, 0x64, 0x19, 0x5d, 0xaf, 0xc5, 0x00};
/* 00c5af5d-1964-4e30-8f51-1956f96bd282 */
static const uint8_t NS2_UUID_CHR_CONFIG_0005[16] = {0x82, 0xd2, 0x6b, 0xf9, 0x56, 0x19, 0x51, 0x8f, 0x30, 0x4e, 0x64, 0x19, 0x5d, 0xaf, 0xc5, 0x00};
/* 00c5af5d-1964-4e30-8f51-1956f96bd283 */
static const uint8_t NS2_UUID_CHR_CONFIG_0007[16] = {0x83, 0xd2, 0x6b, 0xf9, 0x56, 0x19, 0x51, 0x8f, 0x30, 0x4e, 0x64, 0x19, 0x5d, 0xaf, 0xc5, 0x00};
/* ab7de9be-89fe-49ad-828f-118f09df7fd0 */
static const uint8_t NS2_UUID_SVC_HID[16] = {0xd0, 0x7f, 0xdf, 0x09, 0x8f, 0x11, 0x8f, 0x82, 0xad, 0x49, 0xfe, 0x89, 0xbe, 0xe9, 0x7d, 0xab};
/* ab7de9be-89fe-49ad-828f-118f09df7fd2 */
static const uint8_t NS2_UUID_CHR_COMMON_INPUT[16] = {0xd2, 0x7f, 0xdf, 0x09, 0x8f, 0x11, 0x8f, 0x82, 0xad, 0x49, 0xfe, 0x89, 0xbe, 0xe9, 0x7d, 0xab};
/* 679d5510-5a24-4dee-9557-95df80486ecb */
static const uint8_t NS2_UUID_DSC_REPORT_RATE[16] = {0xcb, 0x6e, 0x48, 0x80, 0xdf, 0x95, 0x57, 0x95, 0xee, 0x4d, 0x24, 0x5a, 0x10, 0x55, 0x9d, 0x67};
/* 7492866c-ec3e-4619-8258-32755ffcc0f8 */
static const uint8_t NS2_UUID_CHR_PRO2_INPUT[16] = {0xf8, 0xc0, 0xfc, 0x5f, 0x75, 0x32, 0x58, 0x82, 0x19, 0x46, 0x3e, 0xec, 0x6c, 0x86, 0x92, 0x74};
/* cc483f51-9258-427d-a939-630c31f72b05 */
static const uint8_t NS2_UUID_CHR_PRO2_VIBRATION[16] = {0x05, 0x2b, 0xf7, 0x31, 0x0c, 0x63, 0x39, 0xa9, 0x7d, 0x42, 0x58, 0x92, 0x51, 0x3f, 0x48, 0xcc};
/* 649d4ac9-8eb7-4e6c-af44-1ea54fe5f005 */
static const uint8_t NS2_UUID_CHR_COMMAND[16] = {0x05, 0xf0, 0xe5, 0x4f, 0xa5, 0x1e, 0x44, 0xaf, 0x6c, 0x4e, 0xb7, 0x8e, 0xc9, 0x4a, 0x9d, 0x64};
/* 3dacbc7e-6955-40b5-8eaf-6f9809e8b379 */
static const uint8_t NS2_UUID_CHR_PRO2_VIB_CMD[16] = {0x79, 0xb3, 0xe8, 0x09, 0x98, 0x6f, 0xaf, 0x8e, 0xb5, 0x40, 0x55, 0x69, 0x7e, 0xbc, 0xac, 0x3d};
/* 4147423d-fdae-4df7-a4f7-d23e5df59f8d */
static const uint8_t NS2_UUID_CHR_FW_UPDATE[16] = {0x8d, 0x9f, 0xf5, 0x5d, 0x3e, 0xd2, 0xf7, 0xa4, 0xf7, 0x4d, 0xae, 0xfd, 0x3d, 0x42, 0x47, 0x41};
/* c765a961-d9d8-4d36-a20a-5315b111836a */
static const uint8_t NS2_UUID_CHR_CMD_RESPONSE[16] = {0x6a, 0x83, 0x11, 0xb1, 0x15, 0x53, 0x0a, 0xa2, 0x36, 0x4d, 0xd8, 0xd9, 0x61, 0xa9, 0x65, 0xc7};
/* b746df8c-f358-495b-9cd2-e3bbeda4f979 */
static const uint8_t NS2_UUID_DSC_UNKNOWN[16] = {0x79, 0xf9, 0xa4, 0xed, 0xbb, 0xe3, 0xd2, 0x9c, 0x5b, 0x49, 0x58, 0xf3, 0x8c, 0xdf, 0x46, 0xb7};
/* 506d9f7d-4278-4e95-a549-326ba77657e0 */
static const uint8_t NS2_UUID_CHR_PRO2_CMD_RESPONSE[16] = {0xe0, 0x57, 0x76, 0xa7, 0x6b, 0x32, 0x49, 0xa5, 0x95, 0x4e, 0x78, 0x42, 0x7d, 0x9f, 0x6d, 0x50};
/* d3bd69d2-841c-4241-ab15-f86f406d2a80 */
static const uint8_t NS2_UUID_CHR_UNKNOWN_0022[16] = {0x80, 0x2a, 0x6d, 0x40, 0x6f, 0xf8, 0x15, 0xab, 0x41, 0x42, 0x1c, 0x84, 0xd2, 0x69, 0xbd, 0xd3};
/* ab7de9be-89fe-49ad-828f-118f09df7fde */
static const uint8_t NS2_UUID_CHR_UNKNOWN_0026[16] = {0xde, 0x7f, 0xdf, 0x09, 0x8f, 0x11, 0x8f, 0x82, 0xad, 0x49, 0xfe, 0x89, 0xbe, 0xe9, 0x7d, 0xab};
/* ab7de9be-89fe-49ad-828f-118f09df7fdf */
static const uint8_t NS2_UUID_CHR_UNKNOWN_002A[16] = {0xdf, 0x7f, 0xdf, 0x09, 0x8f, 0x11, 0x8f, 0x82, 0xad, 0x49, 0xfe, 0x89, 0xbe, 0xe9, 0x7d, 0xab};
/* cc483f51-9258-427d-a939-630c31f72b06 */
static const uint8_t NS2_UUID_CHR_PRO2_HEADSET_OUT[16] = {0x06, 0x2b, 0xf7, 0x31, 0x0c, 0x63, 0x39, 0xa9, 0x7d, 0x42, 0x58, 0x92, 0x51, 0x3f, 0x48, 0xcc};
/* 7492866c-ec3e-4619-8258-32755ffcc0f9 */
static const uint8_t NS2_UUID_CHR_PRO2_HEADSET_IN[16] = {0xf9, 0xc0, 0xfc, 0x5f, 0x75, 0x32, 0x58, 0x82, 0x19, 0x46, 0x3e, 0xec, 0x6c, 0x86, 0x92, 0x74};
/* 3dacbc7e-6955-40b5-8eaf-6f9809e8b380 */
static const uint8_t NS2_UUID_CHR_PRO2_UNKNOWN_0032[16] = {0x80, 0xb3, 0xe8, 0x09, 0x98, 0x6f, 0xaf, 0x8e, 0xb5, 0x40, 0x55, 0x69, 0x7e, 0xbc, 0xac, 0x3d};

static const uint8_t NS2_GAP_DEVICE_NAME[] = {'P', 'r', 'o', ' ', 'C', 'o', 'n', 't', 'r', 'o', 'l', 'l', 'e', 'r'};
/* Appearance 0x03C4 (Gamepad), little-endian. */
static const uint8_t NS2_GAP_APPEARANCE[] = {0xC4, 0x03};

#define NS2_R NS2_GATT_PROP_READ
#define NS2_W NS2_GATT_PROP_WRITE
#define NS2_WNR NS2_GATT_PROP_WRITE_WITHOUT_RESPONSE
#define NS2_N NS2_GATT_PROP_NOTIFY

#define NS2_SVC128(h, u) {(h), NS2_ATT_PRIMARY_SERVICE, 0, 0, (u), NULL, 0}
#define NS2_SVC16(h, u) {(h), NS2_ATT_PRIMARY_SERVICE, 0, (u), NULL, NULL, 0}
#define NS2_CHR128(h, p, u) {(h), NS2_ATT_CHARACTERISTIC, (p), 0, (u), NULL, 0}
#define NS2_CHR16_STATIC(h, p, u, v) {(h), NS2_ATT_CHARACTERISTIC, (p), (u), NULL, (v), sizeof(v)}
#define NS2_CCC(h) {(h), NS2_ATT_CCC, 0, 0x2902u, NULL, NULL, 0}
#define NS2_DSC128(h, u) {(h), NS2_ATT_DESCRIPTOR, 0, 0, (u), NULL, 0}

static const ns2_gatt_attribute_s s_ns2_gatt_table[] = {
    NS2_SVC128(0x0001, NS2_UUID_SVC_CONFIG),
    NS2_CHR128(0x0002, NS2_R, NS2_UUID_CHR_CONFIG_0003),
    NS2_CHR128(0x0004, NS2_W, NS2_UUID_CHR_CONFIG_0005),
    NS2_CHR128(0x0006, NS2_R, NS2_UUID_CHR_CONFIG_0007),

    NS2_SVC128(0x0008, NS2_UUID_SVC_HID),
    NS2_CHR128(0x0009, NS2_R | NS2_N, NS2_UUID_CHR_COMMON_INPUT),
    NS2_CCC(0x000B),
    NS2_DSC128(0x000C, NS2_UUID_DSC_REPORT_RATE),
    NS2_CHR128(0x000D, NS2_R | NS2_N, NS2_UUID_CHR_PRO2_INPUT),
    NS2_CCC(0x000F),
    NS2_DSC128(0x0010, NS2_UUID_DSC_REPORT_RATE),
    NS2_CHR128(0x0011, NS2_WNR, NS2_UUID_CHR_PRO2_VIBRATION),
    NS2_CHR128(0x0013, NS2_WNR, NS2_UUID_CHR_COMMAND),
    NS2_CHR128(0x0015, NS2_WNR, NS2_UUID_CHR_PRO2_VIB_CMD),
    NS2_CHR128(0x0017, NS2_WNR, NS2_UUID_CHR_FW_UPDATE),
    NS2_CHR128(0x0019, NS2_N, NS2_UUID_CHR_CMD_RESPONSE),
    NS2_CCC(0x001B),
    NS2_DSC128(0x001C, NS2_UUID_DSC_UNKNOWN),
    NS2_CHR128(0x001D, NS2_N, NS2_UUID_CHR_PRO2_CMD_RESPONSE),
    NS2_CCC(0x001F),
    NS2_DSC128(0x0020, NS2_UUID_DSC_UNKNOWN),
    NS2_CHR128(0x0021, NS2_N, NS2_UUID_CHR_UNKNOWN_0022),
    NS2_CCC(0x0023),
    NS2_DSC128(0x0024, NS2_UUID_DSC_UNKNOWN),
    NS2_CHR128(0x0025, NS2_R | NS2_N, NS2_UUID_CHR_UNKNOWN_0026),
    NS2_CCC(0x0027),
    NS2_DSC128(0x0028, NS2_UUID_DSC_REPORT_RATE),
    NS2_CHR128(0x0029, NS2_WNR, NS2_UUID_CHR_UNKNOWN_002A),
    NS2_CHR128(0x002B, NS2_WNR, NS2_UUID_CHR_PRO2_HEADSET_OUT),
    NS2_CHR128(0x002D, NS2_R | NS2_N, NS2_UUID_CHR_PRO2_HEADSET_IN),
    NS2_CCC(0x002F),
    NS2_DSC128(0x0030, NS2_UUID_DSC_REPORT_RATE),
    NS2_CHR128(0x0031, NS2_WNR, NS2_UUID_CHR_PRO2_UNKNOWN_0032),

    NS2_SVC16(0x0033, 0x1800u),
    NS2_CHR16_STATIC(0x0034, NS2_R, 0x2A00u, NS2_GAP_DEVICE_NAME),
    NS2_CHR16_STATIC(0x0036, NS2_R, 0x2A01u, NS2_GAP_APPEARANCE),

    /* Generic Attribute service; empty on retail controllers too. */
    NS2_SVC16(0x0038, 0x1801u),
};

const ns2_gatt_attribute_s *ns2_gatt_get_table(uint16_t *count)
{
    if (count != NULL)
    {
        *count = (uint16_t)(sizeof(s_ns2_gatt_table) / sizeof(s_ns2_gatt_table[0]));
    }
    return s_ns2_gatt_table;
}
