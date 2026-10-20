/**
 * @file ns2_lib.h
 * @brief User-facing NS2-LIB-HID API: configuration, BLE transport entry points, and weak
 *        firmware hooks.
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

#ifndef NS2_LIB_API_H
#define NS2_LIB_API_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ns2_lib_types.h"
#include "ns2_lib_config.h"
#include "ns2_lib_gatt.h"
#include "ns2_lib_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- API Functions --- */

/**
 * @brief One-shot initialize: apply device config, then reset runtime state.
 *
 * @param cfg Device configuration to apply.
 * @return `NS2_CONFIG_OK` on success; validation error otherwise.
 */
ns2_config_status_t ns2_api_init(const ns2_device_config_s *cfg);

/**
 * @brief Build the legacy advertising payload (Flags + Nintendo manufacturer data).
 *
 * When the configured `host_mac` is non-zero the payload targets that console, and
 * @p wake selects the wake-console variant (flag `0x81`) over plain reconnection.
 * An unpaired device always produces the standard discovery advertisement.
 *
 * @param out Destination buffer of @ref NS2_ADV_DATA_MAX_SIZE bytes.
 * @param wake true to request the console wake flag (paired devices only).
 * @return Number of bytes written to @p out.
 */
uint8_t ns2_api_get_advertisement(uint8_t out[NS2_ADV_DATA_MAX_SIZE], bool wake);

/**
 * @brief Notify the library that a console connected.
 *
 * Resets per-connection protocol state (feature flags, CCC state, queued responses).
 *
 * @param host_mac Connected peer address, display order (MSB first).
 */
void ns2_api_ble_connected(const uint8_t host_mac[6]);

/** @brief Notify the library that the console disconnected. */
void ns2_api_ble_disconnected(void);

/**
 * @brief Forward a GATT write (write request or write command) from the console.
 *
 * Handles CCC updates, command writes, and vibration writes. Command responses are queued and
 * must be drained with @ref ns2_api_ble_get_notification.
 *
 * @param handle Attribute handle written by the console.
 * @param data Written bytes.
 * @param len Number of written bytes.
 */
void ns2_api_ble_write(uint16_t handle, const uint8_t *data, uint16_t len);

/**
 * @brief Service a GATT read of a dynamic attribute.
 *
 * Follows the BTstack read-callback convention: with @p buffer NULL the full attribute length
 * is returned, otherwise up to @p buffer_size bytes starting at @p offset are copied.
 *
 * @return Attribute length (buffer NULL) or number of bytes copied.
 */
uint16_t ns2_api_ble_read(uint16_t handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

/**
 * @brief Pop the next queued notification (command responses).
 *
 * @param[out] handle Attribute value handle to notify on.
 * @param[out] data Destination buffer of @ref NS2_NOTIFY_MAX_SIZE bytes.
 * @param[out] len Notification length.
 * @return true when a notification was dequeued.
 */
bool ns2_api_ble_get_notification(uint16_t *handle, uint8_t data[NS2_NOTIFY_MAX_SIZE], uint16_t *len);

/** @brief true when at least one notification is waiting in the queue. */
bool ns2_api_ble_notification_pending(void);

/**
 * @brief Build one outgoing input report (Pro Controller 2 report 0x09, no report ID).
 *
 * Send the result as a notification on @ref NS2_HANDLE_PRO2_INPUT_VALUE.
 *
 * @param data Output buffer of @ref NS2_INPUT_REPORT_SIZE bytes.
 * @return false while the console has not enabled input reporting yet.
 */
bool ns2_api_generate_inputreport(uint8_t data[NS2_INPUT_REPORT_SIZE]);

/** @brief true once pairing completed or a stored host address was configured. */
bool ns2_api_is_paired(void);

/**
 * @brief Restore the stored pairing key (@ref ns2_pairing_s `ltk`, MSB first).
 *
 * A console that already paired confirms the key (0x15/0x02) when it reconnects; without the key
 * the library can't answer and the console pairs again.
 */
void ns2_api_set_ltk(const uint8_t ltk[16]);

/* --- API Platform Hook Functions --- */

/**
 * @brief Fill logical button/stick state for report generation (weak, user-overridable).
 *
 * @param out Output input state structure (pre-cleared, sticks centered).
 */
void ns2_api_hook_get_input(ns2_input_s *out);

/**
 * @brief Fill battery / charging status for input report byte 1 (weak, user-overridable).
 */
void ns2_api_hook_get_powerstatus(ns2_powerstatus_s *out);

/**
 * @brief Player LED callback (weak, user-overridable).
 *
 * @param led_mask Bits 0-3 map to player LEDs 1-4.
 * @param flashing true when the console requested the pattern to flash.
 */
void ns2_api_hook_set_led(uint8_t led_mask, bool flashing);

/**
 * @brief Raw HD rumble data from the console (weak, user-overridable).
 *
 * The Pro Controller 2 rumble format is not decoded yet; the raw bytes are forwarded as-is.
 * For writes to the vibration + command attribute, @p data holds the left and right LRA
 * blocks (0x10 bytes each).
 */
void ns2_api_hook_set_haptic_raw(const uint8_t *data, uint16_t len);

/** @brief Console request to play a built-in vibration sample (weak, user-overridable). */
void ns2_api_hook_play_vibration_sample(uint8_t sample_id);

/**
 * @brief Pairing completed callback (weak, user-overridable).
 *
 * Called when the console finalises pairing. Firmware must persist @p pairing and install the
 * LTK into its BLE stack so the console can start encryption on this connection and on later
 * reconnects.
 */
void ns2_api_hook_set_pairing(const ns2_pairing_s *pairing);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_API_H */
