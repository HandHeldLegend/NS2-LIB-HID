/**
 * @file ns2_lib.c
 * @brief NS2-LIB-HID API entry points and weak default implementations of firmware hooks.
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

#include "ns2_lib.h"
#include "ns2_lib_protocol.h"
#include "ns2_lib_memory.h"

ns2_config_status_t ns2_api_init(const ns2_device_config_s *cfg)
{
    ns2_config_status_t st = ns2_config_set(cfg);
    if (st != NS2_CONFIG_OK)
    {
        return st;
    }
    ns2_memory_init();
    ns2_protocol_reset();
    return NS2_CONFIG_OK;
}

uint8_t ns2_api_get_advertisement(uint8_t out[NS2_ADV_DATA_MAX_SIZE], bool wake)
{
    if (out == NULL)
    {
        return 0;
    }

    uint8_t host_mac[6];
    ns2_config_get_host_mac(host_mac);
    bool paired = ns2_config_has_host();
    uint16_t pid = ns2_config_get_product_id();

    uint8_t i = 0;

    /* Flags: LE General Discoverable, BR/EDR not supported. */
    out[i++] = 0x02;
    out[i++] = 0x01;
    out[i++] = 0x06;

    /* Manufacturer specific data (26 bytes + AD type). */
    out[i++] = 27;
    out[i++] = 0xFF;
    out[i++] = 0x53; /* Company ID 0x0553 (Nintendo), little-endian */
    out[i++] = 0x05;
    out[i++] = 0x01;
    out[i++] = 0x00;
    out[i++] = 0x03;
    out[i++] = (uint8_t)(NS2_VENDOR_ID & 0xFFu);
    out[i++] = (uint8_t)(NS2_VENDOR_ID >> 8);
    out[i++] = (uint8_t)(pid & 0xFFu);
    out[i++] = (uint8_t)(pid >> 8);
    out[i++] = 0x00;
    out[i++] = 0x01;
    out[i++] = (paired && wake) ? 0x81 : 0x00;
    /* Target console address, reverse byte order; all zero for discovery. */
    for (int b = 5; b >= 0; b--)
    {
        out[i++] = paired ? host_mac[b] : 0x00;
    }
    out[i++] = 0x0F;
    for (int b = 0; b < 7; b++)
    {
        out[i++] = 0x00;
    }

    return i;
}

void ns2_api_ble_connected(const uint8_t host_mac[6])
{
    ns2_protocol_reset();
    if (host_mac != NULL)
    {
        ns2_protocol_set_peer(host_mac);
    }
}

void ns2_api_ble_disconnected(void)
{
    ns2_protocol_reset();
}

void ns2_api_ble_write(uint16_t handle, const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0u)
    {
        return;
    }
    ns2_protocol_ble_write(handle, data, len);
}

uint16_t ns2_api_ble_read(uint16_t handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    return ns2_protocol_ble_read(handle, offset, buffer, buffer_size);
}

bool ns2_api_ble_get_notification(uint16_t *handle, uint8_t data[NS2_NOTIFY_MAX_SIZE], uint16_t *len)
{
    return ns2_protocol_pop_notification(handle, data, len);
}

bool ns2_api_ble_notification_pending(void)
{
    return ns2_protocol_notification_pending();
}

bool ns2_api_generate_inputreport(uint8_t data[NS2_INPUT_REPORT_SIZE])
{
    return ns2_protocol_generate_inputreport(data);
}

bool ns2_api_is_paired(void)
{
    return ns2_config_has_host();
}

void ns2_api_gc_input_pack(const ns2_input_s *in, uint8_t out[NS2_GC_INPUT_BLOCK_LEN])
{
    if (in != NULL && out != NULL)
    {
        ns2_protocol_gc_input_pack(in, out);
    }
}

void ns2_api_gc_input_unpack(const uint8_t in[NS2_GC_INPUT_BLOCK_LEN], ns2_input_s *out)
{
    if (in != NULL && out != NULL)
    {
        ns2_protocol_gc_input_unpack(in, out);
    }
}

void ns2_api_set_ltk(const uint8_t ltk[16])
{
    if (ltk != NULL)
    {
        ns2_protocol_set_ltk(ltk);
    }
}

/*
 * Platform hooks (declared in ns2_lib.h): weak definitions — firmware should supply strong replacements.
 */
__attribute__((weak)) void ns2_api_hook_get_input(ns2_input_s *out)
{
    (void)out;
}

__attribute__((weak)) void ns2_api_hook_get_powerstatus(ns2_powerstatus_s *out)
{
    if (!out)
    {
        return;
    }
    out->power_source = NS2_POWERSRC_EXTERNAL;
    out->charging_status = NS2_CHARGING_IDLE;
    out->battery_level = 9;
}

__attribute__((weak)) void ns2_api_hook_set_led(uint8_t led_mask, bool flashing)
{
    (void)led_mask;
    (void)flashing;
}

__attribute__((weak)) void ns2_api_hook_set_haptic_raw(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
}

__attribute__((weak)) void ns2_api_hook_play_vibration_sample(uint8_t sample_id)
{
    (void)sample_id;
}

__attribute__((weak)) void ns2_api_hook_set_pairing(const ns2_pairing_s *pairing)
{
    (void)pairing;
}
