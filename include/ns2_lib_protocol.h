/**
 * @file ns2_lib_protocol.h
 * @brief Switch 2 command processing, pairing, and input report packing (internal).
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

#ifndef NS2_LIB_PROTOCOL_H
#define NS2_LIB_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#include "ns2_lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Command header length (request and response). */
#define NS2_CMD_HEADER_LEN 8u

/** Zero-filled rumble prefix on writes to the Pro Controller 2 vibration + command attribute. */
#define NS2_PRO2_VIB_CMD_PREFIX_LEN 0x21u

/** Zero-filled prefix on command responses sent on the extended response attribute. */
#define NS2_PRO2_CMD_RESPONSE_PREFIX_LEN 0x0Eu

/** Clear all per-connection state. */
void ns2_protocol_reset(void);

/** Remember the connected peer (display order); used as the pairing host address. */
void ns2_protocol_set_peer(const uint8_t host_mac[6]);

void ns2_protocol_ble_write(uint16_t handle, const uint8_t *data, uint16_t len);

uint16_t ns2_protocol_ble_read(uint16_t handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

bool ns2_protocol_pop_notification(uint16_t *handle, uint8_t *data, uint16_t *len);

bool ns2_protocol_notification_pending(void);

bool ns2_protocol_generate_inputreport(uint8_t data[NS2_INPUT_REPORT_SIZE]);

/**
 * @brief Process one command (header + data) and build its response (header + data).
 *
 * Exposed for transports that carry commands without BLE framing (e.g. USB) and for tests.
 *
 * @param cmd Command bytes, starting with the 8-byte header.
 * @param len Command length.
 * @param out Response buffer, at least NS2_CMD_HEADER_LEN + NS2_MEMORY_READ_MAX + 8 bytes.
 * @return Response length, or 0 when @p cmd is malformed.
 */
uint16_t ns2_protocol_process_command(const uint8_t *cmd, uint16_t len, uint8_t *out);

/** @brief true once the console started input reports (feature enable over BLE, USB initialise). */
bool ns2_protocol_reporting(void);

/** GameCube controller USB input report: ID, then 63 bytes. */
#define NS2_GC_USB_REPORT_ID 0x0Au
#define NS2_GC_USB_REPORT_SIZE 64u

/** @brief The GameCube controller's USB input report (report ID first); false until reporting. */
bool ns2_protocol_generate_gc_report(uint8_t data[NS2_GC_USB_REPORT_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_PROTOCOL_H */
