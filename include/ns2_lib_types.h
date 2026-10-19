/**
 * @file ns2_lib_types.h
 * @brief Shared data structures for NS2-LIB-HID console-oriented state.
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

#ifndef NS2_LIB_TYPES_H
#define NS2_LIB_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Size of the Pro Controller 2 input report body (report 0x09 without its report ID byte). */
#define NS2_INPUT_REPORT_SIZE 63u

/** Largest notification payload the library will ever hand to the transport. */
#define NS2_NOTIFY_MAX_SIZE 160u

/** Size of a complete BLE advertising payload (legacy advertising). */
#define NS2_ADV_DATA_MAX_SIZE 31u

typedef enum
{
    NS2_DEVTYPE_UNDEFINED,
    NS2_DEVTYPE_PRO2,       /**< Pro Controller 2 (BLE). */
    NS2_DEVTYPE_JOYCON2_L,  /**< Reserved, not implemented. */
    NS2_DEVTYPE_JOYCON2_R,  /**< Reserved, not implemented. */
    NS2_DEVTYPE_GC,         /**< GameCube controller for Nintendo Switch Online (USB). */
} ns2_devtype_t;

typedef enum
{
    NS2_TRANSPORT_UNDEFINED,
    NS2_TRANSPORT_USB, /**< GameCube controller only. */
    NS2_TRANSPORT_BLE,
} ns2_transport_t;

/** @brief 24-bit RGB color as reported in the controller's factory memory. */
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ns2_rgb_s;

/** @brief Controller body colors reported to the console. */
typedef struct
{
    ns2_rgb_s body;
    ns2_rgb_s buttons;
    ns2_rgb_s highlight;
    ns2_rgb_s grip;
} ns2_colordata_s;

typedef struct
{
    uint8_t type;      /**< @ref ns2_devtype_t */
    uint8_t transport; /**< @ref ns2_transport_t */
    /** Local controller Bluetooth address, display order (MSB first, AA:BB:CC:DD:EE:FF). */
    uint8_t device_mac[6];
    /**
     * Paired console Bluetooth address, display order. All zero when not paired. When set, the
     * library builds wake/reconnect advertisements that target this console.
     */
    uint8_t host_mac[6];
    /** ASCII serial number, zero padded (reported in memory block 0x13000). */
    uint8_t device_serial[16];
    /** Colors reported in memory block 0x13000. */
    ns2_colordata_s colors;
    /** USB only: @ref ns2_usb_pairing_t */
    uint8_t usb_pairing;
} ns2_device_config_s;

/**
 * @brief How a USB controller answers the console's pairing commands (0x15).
 *
 * Over USB the console pairs the controller for later wireless use. A wired-only controller
 * (or one that should not take a pairing slot) answers some other way; which way the console
 * accepts without storing anything is still being measured.
 */
typedef enum
{
    NS2_USB_PAIRING_NORMAL = 0,   /**< Pair like a retail controller. */
    NS2_USB_PAIRING_DECLINE,      /**< Answer every pairing step without data. */
    NS2_USB_PAIRING_SILENT,       /**< Leave every pairing step unanswered. */
    NS2_USB_PAIRING_FAIL_CONFIRM, /**< Pair, but answer the key confirmation wrongly; store nothing. */
} ns2_usb_pairing_t;

/**
 * @brief Result of configuration validation or operations that depend on a configured device.
 */
typedef enum
{
    NS2_CONFIG_OK = 0,
    /** Device identity / transport still default or incomplete. */
    NS2_CONFIG_NOT_CONFIGURED,
    /** Bad pointer or inconsistent arguments. */
    NS2_CONFIG_INVALID_ARG,
    /** Device type or transport is not implemented yet. */
    NS2_CONFIG_UNSUPPORTED,
} ns2_config_status_t;

/**
 * @brief Pairing credentials produced by the console's pairing command sequence.
 *
 * Switch 2 consoles do not use SMP. The long term key is derived from key material exchanged
 * over the command interface and the console then starts LE encryption with EDIV = 0 and
 * Rand = 0 (the LE Secure Connections convention).
 */
typedef struct
{
    /** Paired console address, display order (MSB first). */
    uint8_t host_mac[6];
    /**
     * Long term key, MSB first. This is the byte order BTstack uses for `sm_key_t`
     * (e.g. `le_device_db_encryption_set`). Reverse it for stacks that take the HCI
     * little-endian order (e.g. NimBLE `ble_store_value_sec.ltk`).
     */
    uint8_t ltk[16];
} ns2_pairing_s;

typedef enum
{
    NS2_POWERSRC_BATTERY = 0,
    NS2_POWERSRC_EXTERNAL = 1,
} ns2_power_source_t;

typedef enum
{
    NS2_CHARGING_IDLE = 0,
    NS2_CHARGING_ACTIVE = 1,
} ns2_charging_t;

/** @brief Battery / charging state encoded into input report byte 1. */
typedef struct
{
    ns2_power_source_t power_source;
    ns2_charging_t charging_status;
    uint8_t battery_level; /**< 0 (empty) .. 9 (full). Values above 9 are clamped. */
} ns2_powerstatus_s;

/** @brief Logical button + stick state supplied by the firmware. */
typedef struct
{
    bool a;
    bool b;
    bool x;
    bool y;
    bool l;
    bool r;
    bool zl;
    bool zr;
    bool plus;
    bool minus;
    bool home;
    bool capture;
    bool l_js;  /**< Left stick click. */
    bool r_js;  /**< Right stick click. */
    bool up;
    bool down;
    bool left;
    bool right;
    bool gl;    /**< Rear grip button (left). */
    bool gr;    /**< Rear grip button (right). */
    bool c;     /**< C (chat) button. */

    uint16_t ls_x; /**< Left stick X, 12-bit, 2048 center. */
    uint16_t ls_y; /**< Left stick Y, 12-bit, 2048 center. */
    uint16_t rs_x; /**< Right stick X, 12-bit, 2048 center. */
    uint16_t rs_y; /**< Right stick Y, 12-bit, 2048 center. */

    /** GameCube controller analog triggers, 0 (released) .. 255 (pressed through the click). */
    uint8_t lt;
    uint8_t rt;
} ns2_input_s;

/** Feature flags used by command 0x0C (feature select). */
typedef enum
{
    NS2_FEATURE_BUTTONS = 0x01,
    NS2_FEATURE_STICKS = 0x02,
    NS2_FEATURE_IMU = 0x04,
    NS2_FEATURE_MOUSE = 0x10,
    NS2_FEATURE_RUMBLE = 0x20,
    NS2_FEATURE_MAGNETOMETER = 0x80,
} ns2_feature_t;

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_TYPES_H */
