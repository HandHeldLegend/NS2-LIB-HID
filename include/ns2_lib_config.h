/**
 * @file ns2_lib_config.h
 * @brief Global device configuration storage for NS2-LIB-HID (`ns2_lib_config.c`).
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

#ifndef NS2_LIB_CONFIG_H
#define NS2_LIB_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#include "ns2_lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Nintendo USB vendor ID, also reported over BLE. */
#define NS2_VENDOR_ID 0x057Eu

/**
 * @brief Check whether @p cfg has required fields for active use.
 */
ns2_config_status_t ns2_config_validate(const ns2_device_config_s *cfg);

/** @brief Replace the library's stored configuration (copy). */
ns2_config_status_t ns2_config_set(const ns2_device_config_s *cfg);

/** @brief Copy the current stored configuration into @p out. */
void ns2_config_get(ns2_device_config_s *out);

/** @brief true once a valid configuration has been applied. */
bool ns2_config_get_ready(void);

/** @brief Product ID for the configured device type (e.g. 0x2069 for Pro Controller 2). */
uint16_t ns2_config_get_product_id(void);

/** @brief Firmware type byte reported by command 0x10 (0x02 = Pro Controller). */
uint8_t ns2_config_get_fw_type(void);

void ns2_config_get_device_mac(uint8_t out[6]);

void ns2_config_get_host_mac(uint8_t out[6]);

void ns2_config_set_host_mac(const uint8_t mac[6]);

/** @brief true when a non-zero host address is configured. */
bool ns2_config_has_host(void);

ns2_transport_t ns2_config_get_transport(void);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_CONFIG_H */
