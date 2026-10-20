/**
 * @file ns2_lib_config.c
 * @brief Global NS2-LIB-HID device configuration storage and helpers (@ref ns2_lib_config.h).
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

#include "ns2_lib_config.h"

static ns2_device_config_s s_ns2_device_config;
static bool s_ns2_config_ready;

static bool _ns2_mac_is_zero(const uint8_t mac[6])
{
    for (int i = 0; i < 6; i++)
    {
        if (mac[i] != 0u)
        {
            return false;
        }
    }
    return true;
}

ns2_config_status_t ns2_config_validate(const ns2_device_config_s *cfg)
{
    if (cfg == NULL)
    {
        return NS2_CONFIG_INVALID_ARG;
    }
    if (cfg->type == NS2_DEVTYPE_UNDEFINED || cfg->transport == NS2_TRANSPORT_UNDEFINED)
    {
        return NS2_CONFIG_NOT_CONFIGURED;
    }
    if (_ns2_mac_is_zero(cfg->device_mac))
    {
        return NS2_CONFIG_NOT_CONFIGURED;
    }
    /* Implemented: the Pro Controller 2 over BLE, the GameCube controller over USB and BLE. */
    bool pro2_ble = cfg->type == NS2_DEVTYPE_PRO2 && cfg->transport == NS2_TRANSPORT_BLE;
    bool gc = cfg->type == NS2_DEVTYPE_GC;
    if (!pro2_ble && !gc)
    {
        return NS2_CONFIG_UNSUPPORTED;
    }
    return NS2_CONFIG_OK;
}

ns2_config_status_t ns2_config_set(const ns2_device_config_s *cfg)
{
    ns2_config_status_t st = ns2_config_validate(cfg);
    if (st != NS2_CONFIG_OK)
    {
        return st;
    }
    memcpy(&s_ns2_device_config, cfg, sizeof(s_ns2_device_config));
    s_ns2_config_ready = true;
    return NS2_CONFIG_OK;
}

void ns2_config_get(ns2_device_config_s *out)
{
    if (out == NULL)
    {
        return;
    }
    memcpy(out, &s_ns2_device_config, sizeof(*out));
}

bool ns2_config_get_ready(void)
{
    return s_ns2_config_ready;
}

uint16_t ns2_config_get_product_id(void)
{
    switch (s_ns2_device_config.type)
    {
    case NS2_DEVTYPE_JOYCON2_R:
        return 0x2066u;
    case NS2_DEVTYPE_JOYCON2_L:
        return 0x2067u;
    case NS2_DEVTYPE_GC:
        return 0x2073u;
    case NS2_DEVTYPE_PRO2:
    default:
        return 0x2069u;
    }
}

uint8_t ns2_config_get_fw_type(void)
{
    switch (s_ns2_device_config.type)
    {
    case NS2_DEVTYPE_JOYCON2_L:
        return 0x00u;
    case NS2_DEVTYPE_JOYCON2_R:
        return 0x01u;
    case NS2_DEVTYPE_GC:
        return 0x03u;
    case NS2_DEVTYPE_PRO2:
    default:
        return 0x02u;
    }
}

void ns2_config_get_device_mac(uint8_t out[6])
{
    memcpy(out, s_ns2_device_config.device_mac, 6);
}

void ns2_config_get_host_mac(uint8_t out[6])
{
    memcpy(out, s_ns2_device_config.host_mac, 6);
}

void ns2_config_set_host_mac(const uint8_t mac[6])
{
    memcpy(s_ns2_device_config.host_mac, mac, 6);
}

bool ns2_config_has_host(void)
{
    return !_ns2_mac_is_zero(s_ns2_device_config.host_mac);
}

ns2_devtype_t ns2_config_get_type(void)
{
    return (ns2_devtype_t)s_ns2_device_config.type;
}

uint8_t ns2_config_get_usb_pairing(void)
{
    return s_ns2_device_config.usb_pairing;
}

ns2_transport_t ns2_config_get_transport(void)
{
    return (ns2_transport_t)s_ns2_device_config.transport;
}
