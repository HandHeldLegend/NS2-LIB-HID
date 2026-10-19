/**
 * @file ns2_lib_usb.h
 * @brief USB transport: the GameCube controller (NSO) on a Switch 2.
 *
 * Over USB the controller is a HID interface (input report 0x0A every 4 ms, output reports) beside a
 * vendor interface whose bulk endpoints carry the same command set as BLE. Before the first command
 * the console makes three vendor control requests. The firmware owns the USB stack: it serves these
 * descriptors, passes control requests and bulk commands here, and sends what comes back.
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

/** Largest reply to a bulk command: header, memory read arguments and data. */
#define NS2_USB_COMMAND_REPLY_MAX 136u

/**
 * @brief Descriptors for the configured device. Any pointer may be NULL.
 * @return false when the configured device has no USB form.
 */
bool ns2_usb_get_descriptor_params(const uint8_t **device_descriptor, const uint8_t **config_descriptor,
                                   uint16_t *config_descriptor_len, const uint8_t **hid_report_descriptor,
                                   uint16_t *hid_report_descriptor_len);

/**
 * @brief A vendor control request to the device.
 * @return Bytes written to @p out for an IN request, 0 for an OUT request answered with a bare
 *         status, or -1 to stall (unknown request).
 */
int16_t ns2_usb_vendor_request(uint8_t request, uint16_t value, uint8_t *out, uint16_t out_size);

/** @brief The console (re)configured the device: start over. */
void ns2_usb_reset(void);

/**
 * @brief A command from the bulk OUT endpoint.
 * @param out Reply, at least @ref NS2_USB_COMMAND_REPLY_MAX bytes.
 * @return Reply length for the bulk IN endpoint; 0 sends nothing.
 */
uint16_t ns2_usb_process_command(const uint8_t *data, uint16_t len, uint8_t *out, uint16_t out_size);

/**
 * @brief The next input report (report ID first).
 * @return false until the console initialised the controller.
 */
bool ns2_usb_generate_inputreport(uint8_t *data, uint16_t *len);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_USB_H */
