/**
 * @file ns2_lib_protocol.c
 * @brief Switch 2 command processing, pairing, and input report packing (@ref ns2_lib_protocol.h).
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
#include "ns2_lib_protocol.h"
#include "ns2_lib_memory.h"
#include "ns2_lib_aes.h"

/* --- Command IDs --- */
#define NS2_CMD_NFC 0x01u
#define NS2_CMD_MEMORY 0x02u
#define NS2_CMD_INIT 0x03u
#define NS2_CMD_UNKNOWN_07 0x07u
#define NS2_CMD_PLAYER_LED 0x09u
#define NS2_CMD_VIBRATION 0x0Au
#define NS2_CMD_FEATURE 0x0Cu
#define NS2_CMD_FW_INFO 0x10u
#define NS2_CMD_UNKNOWN_11 0x11u
#define NS2_CMD_PAIRING 0x15u
#define NS2_CMD_UNKNOWN_16 0x16u
#define NS2_CMD_UNKNOWN_18 0x18u

#define NS2_CMD_DIR_REQUEST 0x91u
#define NS2_CMD_DIR_RESPONSE 0x01u

/* Queue of pending command responses waiting for the transport to notify them. */
#define NS2_NOTIFY_QUEUE_LEN 4u

typedef struct
{
    uint16_t handle;
    uint16_t len;
    uint8_t data[NS2_NOTIFY_MAX_SIZE];
} ns2_notify_entry_s;

typedef struct
{
    /* Connection */
    uint8_t peer_mac[6];
    bool peer_known;
    uint8_t ccc_input;       /* CCC for NS2_HANDLE_PRO2_INPUT_VALUE */
    uint8_t ccc_cmd_resp;    /* CCC for NS2_HANDLE_CMD_RESPONSE_VALUE */
    uint8_t ccc_vib_cmd_resp;/* CCC for NS2_HANDLE_VIB_CMD_RESPONSE_VALUE */

    /* Feature select */
    uint8_t feature_mask;
    uint8_t features_enabled;
    bool reporting;

    /* Pairing */
    uint8_t ltk_wire[16];    /* A1 ^ B1, on-air byte order. */
    bool ltk_valid;
    bool paired;

    /* Report */
    uint8_t counter;

    /* Output queue */
    ns2_notify_entry_s queue[NS2_NOTIFY_QUEUE_LEN];
    uint8_t q_head;
    uint8_t q_count;
} ns2_protocol_state_s;

static ns2_protocol_state_s s_ns2;

/* Controller half of the pairing key exchange. Retail controllers always send this value. */
static const uint8_t s_ns2_pairing_b1[16] = {
    0x5C, 0xF6, 0xEE, 0x79, 0x2C, 0xDF, 0x05, 0xE1,
    0xBA, 0x2B, 0x63, 0x25, 0xC4, 0x1A, 0x5F, 0x10,
};

/* Readable value of attribute 0x0003 (unknown meaning, captured from retail hardware). */
static const uint8_t s_ns2_config_0003[] = {0x04, 0x00, 0x05, 0x00, 0x01, 0x01, 0x00};

/* Readable value of attribute 0x0007 (unknown meaning, captured from retail hardware). */
static const uint8_t s_ns2_config_0007[] = {0x36, 0x80, 0x74, 0xEE, 0xBB, 0x3D, 0x8E, 0x13};

/* Command 0x11 subcommand 0x03 response (unknown meaning, captured from retail hardware). */
static const uint8_t s_ns2_cmd_11_03[] = {
    0x01, 0x20, 0x03, 0x00, 0x00, 0x0A, 0xE8, 0x1C,
    0x3B, 0x79, 0x7D, 0x8B, 0x3A, 0x0A, 0xE8, 0x9C,
    0x42, 0x58, 0xA0, 0x0B, 0x42, 0x0A, 0xE8, 0x9C,
    0x41, 0x58, 0xA0, 0x0B, 0x41,
};

static void _ns2_reverse(const uint8_t *in, uint8_t *out, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        out[i] = in[len - 1 - i];
    }
}

/* --- Notification queue --- */

static void _ns2_queue_push(uint16_t handle, const uint8_t *data, uint16_t len)
{
    if (len > NS2_NOTIFY_MAX_SIZE)
    {
        len = NS2_NOTIFY_MAX_SIZE;
    }
    /* Drop the oldest entry rather than the newest; the console waits on the latest reply. */
    if (s_ns2.q_count == NS2_NOTIFY_QUEUE_LEN)
    {
        s_ns2.q_head = (uint8_t)((s_ns2.q_head + 1u) % NS2_NOTIFY_QUEUE_LEN);
        s_ns2.q_count--;
    }
    ns2_notify_entry_s *e = &s_ns2.queue[(s_ns2.q_head + s_ns2.q_count) % NS2_NOTIFY_QUEUE_LEN];
    e->handle = handle;
    e->len = len;
    memcpy(e->data, data, len);
    s_ns2.q_count++;
}

bool ns2_protocol_pop_notification(uint16_t *handle, uint8_t *data, uint16_t *len)
{
    if (s_ns2.q_count == 0u || handle == NULL || data == NULL || len == NULL)
    {
        return false;
    }
    ns2_notify_entry_s *e = &s_ns2.queue[s_ns2.q_head];
    *handle = e->handle;
    *len = e->len;
    memcpy(data, e->data, e->len);
    s_ns2.q_head = (uint8_t)((s_ns2.q_head + 1u) % NS2_NOTIFY_QUEUE_LEN);
    s_ns2.q_count--;
    return true;
}

bool ns2_protocol_notification_pending(void)
{
    return s_ns2.q_count > 0u;
}

/* --- State --- */

void ns2_protocol_reset(void)
{
    uint8_t paired = s_ns2.paired;
    memset(&s_ns2, 0, sizeof(s_ns2));
    s_ns2.paired = paired || ns2_config_has_host();
}

void ns2_protocol_set_peer(const uint8_t host_mac[6])
{
    memcpy(s_ns2.peer_mac, host_mac, 6);
    s_ns2.peer_known = true;
}

/* --- Command handlers. Each returns the response data length written to `out`. --- */

static uint16_t _ns2_cmd_memory(uint8_t sub, const uint8_t *data, uint16_t data_len, uint8_t *out)
{
    /* Memory read: len(1) 7E 00 00 addr(4, LE) */
    if (sub != 0x04u || data_len < 8u)
    {
        return 0;
    }
    uint16_t read_len = data[0];
    uint32_t addr = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) |
                    ((uint32_t)data[7] << 24);

    if (read_len == 0u || read_len > NS2_MEMORY_READ_MAX)
    {
        return 0;
    }

    /* The response echoes the request arguments (with byte 1 cleared) before the data. */
    memcpy(out, data, 8);
    out[1] = 0x00;
    if (!ns2_memory_read(addr, read_len, &out[8]))
    {
        return 0;
    }
    return (uint16_t)(8u + read_len);
}

static uint16_t _ns2_cmd_player_led(uint8_t sub, const uint8_t *data, uint16_t data_len)
{
    static uint8_t led_mask;
    static bool flashing;

    switch (sub)
    {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
        led_mask = (uint8_t)(1u << (sub - 1u));
        break;
    case 0x05:
        led_mask = 0x0F;
        break;
    case 0x06:
        led_mask = 0x00;
        break;
    case 0x07:
        if (data_len >= 1u)
        {
            led_mask = (uint8_t)(data[0] & 0x0Fu);
        }
        break;
    case 0x08:
        if (data_len >= 1u)
        {
            flashing = data[0] != 0u;
        }
        break;
    default:
        return 0;
    }
    ns2_api_hook_set_led(led_mask, flashing);
    return 0;
}

static void _ns2_encode_feature_info(uint8_t flags, uint8_t out[8])
{
    memset(out, 0, 8);
    if (flags & NS2_FEATURE_BUTTONS)
    {
        out[0] = 0x07;
    }
    if (flags & NS2_FEATURE_STICKS)
    {
        out[1] = 0x07;
    }
    if (flags & NS2_FEATURE_IMU)
    {
        out[2] = 0x01;
    }
    if (flags & NS2_FEATURE_MAGNETOMETER)
    {
        out[3] = 0x01;
    }
    if (flags & NS2_FEATURE_MOUSE)
    {
        out[4] = 0x01;
    }
    if (flags & NS2_FEATURE_RUMBLE)
    {
        out[5] = 0x03;
    }
}

static uint16_t _ns2_cmd_feature(uint8_t sub, const uint8_t *data, uint16_t data_len, uint8_t *out)
{
    uint8_t flags = (data_len >= 1u) ? data[0] : 0u;

    memset(out, 0, 4);
    switch (sub)
    {
    case 0x01: /* Get feature info */
        _ns2_encode_feature_info(flags, &out[4]);
        return 12;
    case 0x02: /* Set feature mask */
        s_ns2.feature_mask = flags;
        return 4;
    case 0x03: /* Clear feature mask */
        s_ns2.feature_mask = 0;
        s_ns2.features_enabled = 0;
        return 4;
    case 0x04: /* Enable features */
        s_ns2.features_enabled |= (uint8_t)(flags & s_ns2.feature_mask);
        s_ns2.reporting = true;
        return 4;
    case 0x05: /* Disable features */
        s_ns2.features_enabled &= (uint8_t)~flags;
        return 4;
    default:
        return 4;
    }
}

static uint16_t _ns2_cmd_fw_info(uint8_t sub, uint8_t *out)
{
    if (sub != 0x01u)
    {
        return 0;
    }
    /* Firmware 2.1.4 | type | BT patch 12.0.0 | pad | DSP 2.3.0 | pad (retail Pro Controller 2). */
    static const uint8_t fw_info[12] = {0x02, 0x01, 0x04, 0x02, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x00};
    /* Firmware 1.1.2 | type | BT patch 12.0.0 | pad | no DSP (retail GameCube controller). */
    static const uint8_t fw_info_gc[12] = {0x01, 0x01, 0x02, 0x03, 0x0C, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(out, (ns2_config_get_type() == NS2_DEVTYPE_GC) ? fw_info_gc : fw_info, sizeof(fw_info));
    out[3] = ns2_config_get_fw_type();
    return sizeof(fw_info);
}

static uint16_t _ns2_cmd_pairing(uint8_t sub, const uint8_t *data, uint16_t data_len, uint8_t *out)
{
    switch (sub)
    {
    case 0x01: /* Exchange addresses: 00 count addr[count] (addresses reverse byte order). */
    {
        if (!s_ns2.peer_known && data_len >= 8u && data[1] > 0u)
        {
            /* No transport-provided peer: fall back to the first address in the list. */
            uint8_t mac[6];
            _ns2_reverse(&data[2], mac, 6);
            ns2_protocol_set_peer(mac);
        }
        uint8_t device_mac[6];
        ns2_config_get_device_mac(device_mac);
        out[0] = 0x01;
        out[1] = 0x04;
        out[2] = 0x01;
        _ns2_reverse(device_mac, &out[3], 6);
        return 9;
    }

    case 0x04: /* Exchange keys: 00 A1[16] -> 01 B1[16]; LTK = A1 ^ B1 */
        if (data_len < 17u)
        {
            return 0;
        }
        for (int i = 0; i < 16; i++)
        {
            s_ns2.ltk_wire[i] = (uint8_t)(data[1 + i] ^ s_ns2_pairing_b1[i]);
        }
        s_ns2.ltk_valid = true;
        out[0] = 0x01;
        memcpy(&out[1], s_ns2_pairing_b1, 16);
        return 17;

    case 0x02: /* Confirm LTK: 00 A2[16] -> 01 B2[16]; B2 = AES128(rev(LTK), rev(A2)) */
    {
        if (data_len < 17u || !s_ns2.ltk_valid)
        {
            return 0;
        }
        uint8_t key[16];
        uint8_t challenge[16];
        _ns2_reverse(s_ns2.ltk_wire, key, 16);
        _ns2_reverse(&data[1], challenge, 16);
        out[0] = 0x01;
        ns2_aes128_encrypt(key, challenge, &out[1]);
        return 17;
    }

    case 0x03: /* Finalise pairing */
    {
        if (!s_ns2.ltk_valid)
        {
            return 0;
        }
        ns2_pairing_s pairing;
        memcpy(pairing.host_mac, s_ns2.peer_mac, 6);
        _ns2_reverse(s_ns2.ltk_wire, pairing.ltk, 16);
        ns2_config_set_host_mac(pairing.host_mac);
        s_ns2.paired = true;
        ns2_api_hook_set_pairing(&pairing);
        out[0] = 0x01;
        return 1;
    }

    default:
        return 0;
    }
}

uint16_t ns2_protocol_process_command(const uint8_t *cmd, uint16_t len, uint8_t *out)
{
    if (cmd == NULL || out == NULL || len < NS2_CMD_HEADER_LEN)
    {
        return 0;
    }

    uint8_t id = cmd[0];
    uint8_t sub = cmd[3];
    const uint8_t *data = &cmd[NS2_CMD_HEADER_LEN];
    uint16_t data_len = (uint16_t)(len - NS2_CMD_HEADER_LEN);
    uint8_t *resp = &out[NS2_CMD_HEADER_LEN];
    uint16_t resp_len = 0;

    switch (id)
    {
    case NS2_CMD_NFC:
        if (sub == 0x0Cu)
        {
            static const uint8_t nfc_0c[] = {0x61, 0x12, 0x50, 0x10};
            memcpy(resp, nfc_0c, sizeof(nfc_0c));
            resp_len = sizeof(nfc_0c);
        }
        break;

    case NS2_CMD_MEMORY:
        resp_len = _ns2_cmd_memory(sub, data, data_len, resp);
        break;

    case NS2_CMD_UNKNOWN_07:
        if (sub == 0x01u)
        {
            resp[0] = 0x00;
            resp_len = 1;
        }
        break;

    case NS2_CMD_PLAYER_LED:
        resp_len = _ns2_cmd_player_led(sub, data, data_len);
        break;

    case NS2_CMD_VIBRATION:
        if (sub == 0x02u && data_len >= 1u)
        {
            ns2_api_hook_play_vibration_sample(data[0]);
        }
        break;

    case NS2_CMD_FEATURE:
        resp_len = _ns2_cmd_feature(sub, data, data_len, resp);
        break;

    case NS2_CMD_FW_INFO:
        resp_len = _ns2_cmd_fw_info(sub, resp);
        break;

    case NS2_CMD_UNKNOWN_11:
        if (sub == 0x01u)
        {
            static const uint8_t r[] = {0x01, 0x00, 0x00, 0x00};
            memcpy(resp, r, sizeof(r));
            resp_len = sizeof(r);
        }
        else if (sub == 0x03u)
        {
            memcpy(resp, s_ns2_cmd_11_03, sizeof(s_ns2_cmd_11_03));
            resp_len = sizeof(s_ns2_cmd_11_03);
        }
        break;

    case NS2_CMD_PAIRING:
        resp_len = _ns2_cmd_pairing(sub, data, data_len, resp);
        break;

    case NS2_CMD_UNKNOWN_16:
        if (sub == 0x01u)
        {
            memset(resp, 0, 0x18);
            resp_len = 0x18;
        }
        break;

    case NS2_CMD_UNKNOWN_18:
        if (sub == 0x01u)
        {
            static const uint8_t r[] = {0x00, 0x00, 0x40, 0xF0, 0x00, 0x00, 0x60, 0x00};
            memcpy(resp, r, sizeof(r));
            resp_len = sizeof(r);
        }
        break;

    case NS2_CMD_INIT:
        /* Initialise USB: input reports start (the console's address in it may be anything) */
        if (sub == 0x0Du)
        {
            static const uint8_t r[] = {0x01, 0x00, 0x00, 0x00};
            memcpy(resp, r, sizeof(r));
            resp_len = sizeof(r);
            s_ns2.reporting = true;
        }
        break;

    default:
        /* Unknown or data-less commands are acknowledged with a bare header. */
        break;
    }

    /* Response header: id, direction, transport, subcommand, 10 78 00 00. */
    out[0] = id;
    out[1] = NS2_CMD_DIR_RESPONSE;
    out[2] = cmd[2];
    out[3] = sub;
    out[4] = 0x10;
    out[5] = 0x78;
    out[6] = 0x00;
    out[7] = 0x00;

    return (uint16_t)(NS2_CMD_HEADER_LEN + resp_len);
}

/* --- BLE attribute access --- */

static void _ns2_handle_command_write(const uint8_t *cmd, uint16_t len, uint16_t resp_handle, uint16_t prefix_len)
{
    uint8_t buf[NS2_NOTIFY_MAX_SIZE];
    memset(buf, 0, prefix_len);
    uint16_t resp_len = ns2_protocol_process_command(cmd, len, &buf[prefix_len]);
    if (resp_len == 0u)
    {
        return;
    }
    _ns2_queue_push(resp_handle, buf, (uint16_t)(prefix_len + resp_len));
}

void ns2_protocol_ble_write(uint16_t handle, const uint8_t *data, uint16_t len)
{
    if (data == NULL)
    {
        return;
    }

    switch (handle)
    {
    case NS2_HANDLE_PRO2_INPUT_CCC:
        s_ns2.ccc_input = (len >= 1u) ? data[0] : 0u;
        break;
    case NS2_HANDLE_CMD_RESPONSE_CCC:
        s_ns2.ccc_cmd_resp = (len >= 1u) ? data[0] : 0u;
        break;
    case NS2_HANDLE_VIB_CMD_RESPONSE_CCC:
        s_ns2.ccc_vib_cmd_resp = (len >= 1u) ? data[0] : 0u;
        break;

    case NS2_HANDLE_VIBRATION_VALUE:
        ns2_api_hook_set_haptic_raw(data, len);
        break;

    case NS2_HANDLE_COMMAND_VALUE:
        _ns2_handle_command_write(data, len, NS2_HANDLE_CMD_RESPONSE_VALUE, 0);
        break;

    case NS2_HANDLE_VIB_COMMAND_VALUE:
        /* Report ID (1) + left LRA (16) + right LRA (16), then the command. */
        if (len < NS2_PRO2_VIB_CMD_PREFIX_LEN)
        {
            break;
        }
        ns2_api_hook_set_haptic_raw(&data[1], NS2_PRO2_VIB_CMD_PREFIX_LEN - 1u);
        if (len >= NS2_PRO2_VIB_CMD_PREFIX_LEN + NS2_CMD_HEADER_LEN)
        {
            _ns2_handle_command_write(&data[NS2_PRO2_VIB_CMD_PREFIX_LEN], (uint16_t)(len - NS2_PRO2_VIB_CMD_PREFIX_LEN),
                                      NS2_HANDLE_VIB_CMD_RESPONSE_VALUE, NS2_PRO2_CMD_RESPONSE_PREFIX_LEN);
        }
        break;

    default:
        /* Other attributes (0x0005, report-rate descriptors, headset audio, ...) are accepted and ignored. */
        break;
    }
}

uint16_t ns2_protocol_ble_read(uint16_t handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    const uint8_t *value = NULL;
    uint16_t value_len = 0;
    uint8_t ccc[2] = {0, 0};

    switch (handle)
    {
    case NS2_HANDLE_CONFIG_0003_VALUE:
        value = s_ns2_config_0003;
        value_len = sizeof(s_ns2_config_0003);
        break;
    case NS2_HANDLE_CONFIG_0007_VALUE:
        value = s_ns2_config_0007;
        value_len = sizeof(s_ns2_config_0007);
        break;
    case NS2_HANDLE_PRO2_INPUT_CCC:
        ccc[0] = s_ns2.ccc_input;
        value = ccc;
        value_len = 2;
        break;
    case NS2_HANDLE_CMD_RESPONSE_CCC:
        ccc[0] = s_ns2.ccc_cmd_resp;
        value = ccc;
        value_len = 2;
        break;
    case NS2_HANDLE_VIB_CMD_RESPONSE_CCC:
        ccc[0] = s_ns2.ccc_vib_cmd_resp;
        value = ccc;
        value_len = 2;
        break;
    case NS2_HANDLE_COMMON_INPUT_CCC:
    case NS2_HANDLE_UNKNOWN_0022_CCC:
    case NS2_HANDLE_UNKNOWN_0026_CCC:
    case NS2_HANDLE_HEADSET_IN_CCC:
        value = ccc;
        value_len = 2;
        break;
    default:
        break;
    }

    if (buffer == NULL)
    {
        return value_len;
    }
    if (value == NULL || offset >= value_len)
    {
        return 0;
    }
    uint16_t n = (uint16_t)(value_len - offset);
    if (n > buffer_size)
    {
        n = buffer_size;
    }
    memcpy(buffer, &value[offset], n);
    return n;
}

/* --- Input report --- */

bool ns2_protocol_reporting(void)
{
    return s_ns2.reporting;
}

static void _ns2_pack_stick(uint8_t out[3], uint16_t x, uint16_t y)
{
    x &= 0x0FFFu;
    y &= 0x0FFFu;
    out[0] = (uint8_t)(x & 0xFFu);
    out[1] = (uint8_t)(((y & 0x0Fu) << 4) | (x >> 8));
    out[2] = (uint8_t)(y >> 4);
}

bool ns2_protocol_generate_inputreport(uint8_t data[NS2_INPUT_REPORT_SIZE])
{
    if (data == NULL || !s_ns2.reporting || !(s_ns2.ccc_input & 0x01u))
    {
        return false;
    }

    ns2_input_s in;
    memset(&in, 0, sizeof(in));
    in.ls_x = 2048;
    in.ls_y = 2048;
    in.rs_x = 2048;
    in.rs_y = 2048;
    ns2_api_hook_get_input(&in);

    ns2_powerstatus_s pwr = {NS2_POWERSRC_EXTERNAL, NS2_CHARGING_IDLE, 9};
    ns2_api_hook_get_powerstatus(&pwr);
    uint8_t level = (pwr.battery_level > 9u) ? 9u : pwr.battery_level;

    memset(data, 0, NS2_INPUT_REPORT_SIZE);

    data[0] = s_ns2.counter++;
    data[1] = (uint8_t)((pwr.power_source == NS2_POWERSRC_EXTERNAL ? 0x01u : 0x00u) |
                        (pwr.charging_status == NS2_CHARGING_ACTIVE ? 0x02u : 0x00u) |
                        (uint8_t)(level << 2));

    data[2] = (uint8_t)((in.b ? 0x01u : 0u) | (in.a ? 0x02u : 0u) | (in.y ? 0x04u : 0u) | (in.x ? 0x08u : 0u) |
                        (in.r ? 0x10u : 0u) | (in.zr ? 0x20u : 0u) | (in.plus ? 0x40u : 0u) | (in.r_js ? 0x80u : 0u));
    data[3] = (uint8_t)((in.down ? 0x01u : 0u) | (in.right ? 0x02u : 0u) | (in.left ? 0x04u : 0u) |
                        (in.up ? 0x08u : 0u) | (in.l ? 0x10u : 0u) | (in.zl ? 0x20u : 0u) |
                        (in.minus ? 0x40u : 0u) | (in.l_js ? 0x80u : 0u));
    data[4] = (uint8_t)((in.home ? 0x01u : 0u) | (in.capture ? 0x02u : 0u) | (in.gr ? 0x04u : 0u) |
                        (in.gl ? 0x08u : 0u) | (in.c ? 0x10u : 0u));

    _ns2_pack_stick(&data[5], in.ls_x, in.ls_y);
    _ns2_pack_stick(&data[8], in.rs_x, in.rs_y);

    data[0x0B] = (s_ns2.features_enabled & NS2_FEATURE_RUMBLE) ? 0x38u : 0x30u;
    data[0x0C] = 0x00; /* NFC state: idle */
    data[0x0D] = 0x00; /* Headset audio state: none */
    /* Motion data format is not decoded yet; report the block as present but zeroed. */
    data[0x0E] = (s_ns2.features_enabled & NS2_FEATURE_IMU) ? 0x28u : 0x00u;

    return true;
}

bool ns2_protocol_generate_gc_report(uint8_t data[NS2_GC_USB_REPORT_SIZE])
{
    if (data == NULL || !s_ns2.reporting)
    {
        return false;
    }

    ns2_input_s in;
    memset(&in, 0, sizeof(in));
    in.ls_x = 2048;
    in.ls_y = 2048;
    in.rs_x = 2048;
    in.rs_y = 2048;
    ns2_api_hook_get_input(&in);

    ns2_powerstatus_s pwr = {NS2_POWERSRC_EXTERNAL, NS2_CHARGING_IDLE, 9};
    ns2_api_hook_get_powerstatus(&pwr);
    uint8_t level = (pwr.battery_level > 9u) ? 9u : pwr.battery_level;

    memset(data, 0, NS2_GC_USB_REPORT_SIZE);

    /* Report 0x0A: the Pro Controller 2 layout one byte later, then the analog triggers. */
    data[0] = NS2_GC_USB_REPORT_ID;
    data[1] = s_ns2.counter++;
    data[2] = (uint8_t)((pwr.power_source == NS2_POWERSRC_EXTERNAL ? 0x01u : 0x00u) |
                        (pwr.charging_status == NS2_CHARGING_ACTIVE ? 0x02u : 0x00u) |
                        (uint8_t)(level << 2));

    data[3] = (uint8_t)((in.b ? 0x01u : 0u) | (in.a ? 0x02u : 0u) | (in.y ? 0x04u : 0u) | (in.x ? 0x08u : 0u) |
                        (in.r ? 0x10u : 0u) | (in.zr ? 0x20u : 0u) | (in.plus ? 0x40u : 0u) | (in.r_js ? 0x80u : 0u));
    data[4] = (uint8_t)((in.down ? 0x01u : 0u) | (in.right ? 0x02u : 0u) | (in.left ? 0x04u : 0u) |
                        (in.up ? 0x08u : 0u) | (in.l ? 0x10u : 0u) | (in.zl ? 0x20u : 0u) |
                        (in.minus ? 0x40u : 0u) | (in.l_js ? 0x80u : 0u));
    data[5] = (uint8_t)((in.home ? 0x01u : 0u) | (in.capture ? 0x02u : 0u) | (in.gr ? 0x04u : 0u) |
                        (in.gl ? 0x08u : 0u) | (in.c ? 0x10u : 0u));

    _ns2_pack_stick(&data[6], in.ls_x, in.ls_y);
    _ns2_pack_stick(&data[9], in.rs_x, in.rs_y);

    data[12] = (s_ns2.features_enabled & NS2_FEATURE_RUMBLE) ? 0x38u : 0x30u;
    data[13] = in.lt;
    data[14] = in.rt;

    return true;
}
