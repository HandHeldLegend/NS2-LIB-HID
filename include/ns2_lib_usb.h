/**
 * @file ns2_lib_usb.h
 * @brief USB transport entry points (stubs — not implemented yet).
 *
 * Switch 2 controllers also talk to the console over USB, using the same command set on a
 * vendor bulk interface alongside a HID interface. These functions reserve the API shape so
 * firmware can wire a USB stack now; they currently report "unsupported".
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

#ifndef NS2_LIB_USB_H
#define NS2_LIB_USB_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get USB descriptors for the configured device (stub).
 *
 * @return false — USB descriptors are not implemented yet; all outputs are set to NULL / 0.
 */
bool ns2_usb_get_descriptor_params(const uint8_t **device_descriptor, const uint8_t **config_descriptor,
                                   const uint8_t **hid_report_descriptor, uint16_t *hid_report_descriptor_len);

/**
 * @brief Forward a command received on the USB bulk OUT endpoint (stub).
 *
 * @param out Response buffer.
 * @return Response length; always 0 until USB is implemented.
 */
uint16_t ns2_usb_process_command(const uint8_t *data, uint16_t len, uint8_t *out, uint16_t out_size);

/**
 * @brief Build one USB HID input report (stub).
 *
 * @return false — not implemented yet.
 */
bool ns2_usb_generate_inputreport(uint8_t *data, uint16_t *len);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_USB_H */
