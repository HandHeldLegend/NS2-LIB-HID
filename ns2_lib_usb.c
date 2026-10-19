/**
 * @file ns2_lib_usb.c
 * @brief USB transport stubs (@ref ns2_lib_usb.h).
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

#include "ns2_lib_usb.h"

bool ns2_usb_get_descriptor_params(const uint8_t **device_descriptor, const uint8_t **config_descriptor,
                                   const uint8_t **hid_report_descriptor, uint16_t *hid_report_descriptor_len)
{
    /* TODO: USB device/config/HID descriptors for Switch 2 controllers. */
    if (device_descriptor)
    {
        *device_descriptor = NULL;
    }
    if (config_descriptor)
    {
        *config_descriptor = NULL;
    }
    if (hid_report_descriptor)
    {
        *hid_report_descriptor = NULL;
    }
    if (hid_report_descriptor_len)
    {
        *hid_report_descriptor_len = 0;
    }
    return false;
}

uint16_t ns2_usb_process_command(const uint8_t *data, uint16_t len, uint8_t *out, uint16_t out_size)
{
    /* TODO: route through ns2_protocol_process_command once the USB framing is implemented. */
    (void)data;
    (void)len;
    (void)out;
    (void)out_size;
    return 0;
}

bool ns2_usb_generate_inputreport(uint8_t *data, uint16_t *len)
{
    /* TODO: USB input report (report ID 0x09 for Pro Controller 2). */
    (void)data;
    if (len)
    {
        *len = 0;
    }
    return false;
}
