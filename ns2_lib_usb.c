/**
 * @file ns2_lib_usb.c
 * @brief USB transport (@ref ns2_lib_usb.h). Descriptors and replies as a retail GameCube controller
 * gives them on a Switch 2.
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
#include <string.h>

#include "ns2_lib.h"
#include "ns2_lib_usb.h"
#include "ns2_lib_protocol.h"
#include "ns2_lib_memory.h"

#define NS2_CMD_PAIRING 0x15u

static const uint8_t s_gc_device_descriptor[18] = {
    0x12, 0x01, 0x00, 0x02, /* USB 2.0 */
    0xEF, 0x02, 0x01, 0x40, /* Miscellaneous / IAD, EP0 64 bytes */
    0x7E, 0x05, 0x73, 0x20, /* 057E:2073 */
    0x01, 0x01,             /* bcdDevice 1.01 */
    0x01, 0x02, 0x03,       /* Manufacturer, product, serial strings */
    0x01,                   /* One configuration */
};

static const uint8_t s_gc_config_descriptor[80] = {
    0x09, 0x02, 0x50, 0x00, 0x02, 0x01, 0x04, 0xC0, 0xFA, /* 2 interfaces, self powered, 500 mA */

    /* Input: HID, interrupt IN 0x81 / OUT 0x01, 64 bytes, every 4 ms */
    0x08, 0x0B, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
    0x09, 0x04, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x05,
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, 0x61, 0x00,
    0x07, 0x05, 0x81, 0x03, 0x40, 0x00, 0x04,
    0x07, 0x05, 0x01, 0x03, 0x40, 0x00, 0x04,

    /* Commands: vendor, bulk OUT 0x02 / IN 0x82, 64 bytes */
    0x08, 0x0B, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x00,
    0x09, 0x04, 0x01, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x06,
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
    0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00,
};

/* Report 0x05 (63 vendor bytes in), report 0x0A (buttons, sticks, triggers), report 0x03 (63 out). */
static const uint8_t s_gc_hid_report_descriptor[97] = {
    0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x85, 0x05, 0x05, 0xFF, 0x09, 0x01, 0x15, 0x00, 0x26, 0xFF,
    0x00, 0x95, 0x3F, 0x75, 0x08, 0x81, 0x02, 0x85, 0x0A, 0x09, 0x01, 0x95, 0x02, 0x81, 0x02, 0x05,
    0x09, 0x19, 0x01, 0x29, 0x15, 0x25, 0x01, 0x95, 0x15, 0x75, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75,
    0x03, 0x81, 0x03, 0x05, 0x01, 0x09, 0x01, 0xA1, 0x00, 0x09, 0x30, 0x09, 0x31, 0x09, 0x33, 0x09,
    0x35, 0x26, 0xFF, 0x0F, 0x95, 0x04, 0x75, 0x0C, 0x81, 0x02, 0xC0, 0x05, 0xFF, 0x09, 0x02, 0x26,
    0xFF, 0x00, 0x95, 0x34, 0x75, 0x08, 0x81, 0x02, 0x85, 0x03, 0x09, 0x01, 0x95, 0x3F, 0x91, 0x02,
    0xC0,
};

/* Vendor request 0x02: firmware 1.1.2, BT patch 12, then the Bluetooth address (reversed). */
static const uint8_t s_gc_request_02_prefix[10] = {0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00};

static bool _ns2_usb_supported(void)
{
    return ns2_config_get_ready() && ns2_config_get_type() == NS2_DEVTYPE_GC &&
           ns2_config_get_transport() == NS2_TRANSPORT_USB;
}

bool ns2_usb_get_descriptor_params(const uint8_t **device_descriptor, const uint8_t **config_descriptor,
                                   uint16_t *config_descriptor_len, const uint8_t **hid_report_descriptor,
                                   uint16_t *hid_report_descriptor_len)
{
    bool ok = _ns2_usb_supported();
    if (device_descriptor)
    {
        *device_descriptor = ok ? s_gc_device_descriptor : NULL;
    }
    if (config_descriptor)
    {
        *config_descriptor = ok ? s_gc_config_descriptor : NULL;
    }
    if (config_descriptor_len)
    {
        *config_descriptor_len = ok ? (uint16_t)sizeof(s_gc_config_descriptor) : 0u;
    }
    if (hid_report_descriptor)
    {
        *hid_report_descriptor = ok ? s_gc_hid_report_descriptor : NULL;
    }
    if (hid_report_descriptor_len)
    {
        *hid_report_descriptor_len = ok ? (uint16_t)sizeof(s_gc_hid_report_descriptor) : 0u;
    }
    return ok;
}

int16_t ns2_usb_vendor_request(uint8_t request, uint16_t value, uint8_t *out, uint16_t out_size)
{
    (void)value;
    if (!_ns2_usb_supported() || out == NULL)
    {
        return -1;
    }

    switch (request)
    {
    case 0x02: /* Firmware and Bluetooth address */
    {
        if (out_size < 16u)
        {
            return -1;
        }
        uint8_t mac[6];
        ns2_config_get_device_mac(mac);
        memcpy(out, s_gc_request_02_prefix, sizeof(s_gc_request_02_prefix));
        for (int i = 0; i < 6; i++)
        {
            out[10 + i] = mac[5 - i];
        }
        return 16;
    }

    case 0x03: /* Device info: the memory block at 0x13000 */
    {
        uint16_t n = (out_size < 64u) ? out_size : 64u;
        return ns2_memory_read(0x013000u, n, out) ? (int16_t)n : -1;
    }

    case 0x04: /* Sent by the console with value 0x0276; no data */
        return 0;

    default:
        return -1;
    }
}

void ns2_usb_reset(void)
{
    ns2_protocol_reset();
}

/* Pairing as configured: a controller that should not take a pairing slot answers differently */
static bool _ns2_usb_pairing(const uint8_t *data, uint8_t *out, uint16_t *out_len)
{
    uint8_t sub = data[3];
    switch (ns2_config_get_usb_pairing())
    {
    case NS2_USB_PAIRING_SILENT:
        *out_len = 0;
        return true;

    case NS2_USB_PAIRING_DECLINE:
        memcpy(out, data, NS2_CMD_HEADER_LEN);
        *out_len = NS2_CMD_HEADER_LEN;
        return true;

    case NS2_USB_PAIRING_FAIL_CONFIRM:
        /* Addresses and keys as usual; then a confirmation that cannot match, and a finalise that
         * stores nothing */
        if (sub == 0x02u)
        {
            memcpy(out, data, NS2_CMD_HEADER_LEN);
            memset(&out[NS2_CMD_HEADER_LEN], 0, 17);
            out[NS2_CMD_HEADER_LEN] = 0x01;
            *out_len = NS2_CMD_HEADER_LEN + 17u;
            return true;
        }
        if (sub == 0x03u)
        {
            memcpy(out, data, NS2_CMD_HEADER_LEN);
            out[NS2_CMD_HEADER_LEN] = 0x01;
            *out_len = NS2_CMD_HEADER_LEN + 1u;
            return true;
        }
        return false;

    case NS2_USB_PAIRING_NORMAL:
    default:
        return false;
    }
}

uint16_t ns2_usb_process_command(const uint8_t *data, uint16_t len, uint8_t *out, uint16_t out_size)
{
    if (!_ns2_usb_supported() || data == NULL || out == NULL || len < NS2_CMD_HEADER_LEN ||
        out_size < NS2_USB_COMMAND_REPLY_MAX)
    {
        return 0;
    }

    uint16_t n = 0;

    if (data[0] != NS2_CMD_PAIRING || !_ns2_usb_pairing(data, out, &n))
    {
        n = ns2_protocol_process_command(data, len, out);
    }
    if (n == 0u)
    {
        return 0;
    }

    /* A reply over USB: direction 01, then 00 F8 (BLE answers 10 78) */
    out[1] = 0x01;
    out[2] = data[2];
    out[4] = 0x00;
    out[5] = 0xF8;

    ns2_protocol_gc_reply_header(data, out, &n);
    return n;
}

bool ns2_usb_generate_inputreport(uint8_t *data, uint16_t *len)
{
    if (!_ns2_usb_supported() || data == NULL || !ns2_protocol_generate_gc_report(data))
    {
        if (len)
        {
            *len = 0;
        }
        return false;
    }
    if (len)
    {
        *len = NS2_GC_USB_REPORT_SIZE;
    }
    return true;
}
