/**
 * @file ns2_lib_memory.c
 * @brief Simulated controller flash memory (@ref ns2_lib_memory.h).
 *
 * The console reads several factory blocks during initialisation (device info, stick
 * calibration, ...). The fixed block contents below were captured from a Pro Controller 2 and
 * published by zhantss/ESP32-BLE5-NSController-Emulator (MIT). See LICENSING.md.
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

#include "ns2_lib_memory.h"
#include "ns2_lib_config.h"

typedef struct
{
    uint32_t start_addr;
    uint16_t len;
    const uint8_t *data;
} ns2_memory_block_s;

/* 0x13000: device info. Rebuilt from the configuration by ns2_memory_init(). */
static uint8_t s_mem_013000[37];

/* 0x13040: unknown. */
static const uint8_t s_mem_013040[] = {
    0x3B, 0xE0, 0xD3, 0x41, 0xC6, 0x60, 0x6A, 0xBC,
    0x4D, 0xD7, 0xA2, 0xBB, 0x71, 0x1E, 0xDD, 0x37,
};

/* 0x13080: primary stick factory data (unknown layout, required for pairing). */
static const uint8_t s_mem_013080[] = {
    0x01, 0xAD, 0xD9, 0x9A, 0x55, 0x56, 0x65, 0xA0, 0x00, 0x0A,
    0xA0, 0x00, 0x0A, 0xE2, 0x20, 0x0E, 0xE2, 0x20, 0x0E, 0x9A,
    0xAD, 0xD9, 0x9A, 0xAD, 0xD9, 0x0A, 0xA5, 0x50, 0x0A, 0xA5,
    0x50, 0x2F, 0xF6, 0x62, 0x2F, 0xF6, 0x62, 0x0A, 0xFF, 0xFF,
};

/* 0x130A8: primary (left) stick calibration. */
static const uint8_t s_mem_0130A8[] = {
    0xB3, 0x67, 0x83, 0x2E, 0x66, 0x5E, 0x3A, 0x06, 0x5F,
};

/* 0x130C0: secondary stick factory data (same layout as 0x13080). */
static const uint8_t s_mem_0130C0[] = {
    0x01, 0xAD, 0xD9, 0x9A, 0x55, 0x56, 0x65, 0xA0, 0x00, 0x0A,
    0xA0, 0x00, 0x0A, 0xE2, 0x20, 0x0E, 0xE2, 0x20, 0x0E, 0x9A,
    0xAD, 0xD9, 0x9A, 0xAD, 0xD9, 0x0A, 0xA5, 0x50, 0x0A, 0xA5,
    0x50, 0x2F, 0xF6, 0x62, 0x2F, 0xF6, 0x62, 0x0A, 0xFF, 0xFF,
};

/* 0x130E8: secondary (right) stick calibration. */
static const uint8_t s_mem_0130E8[] = {
    0x2C, 0x08, 0x84, 0xD1, 0x65, 0x63, 0x2A, 0x26, 0x62,
};

/* 0x13100: unknown. */
static const uint8_t s_mem_013100[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xA6, 0xF2, 0x62, 0xBD,
    0xA8, 0x00, 0x08, 0x3D, 0x2F, 0xED, 0x20, 0x41,
};

/* --- GameCube controller (NSO), read over USB from a retail controller --- */

/* 0x13040: unknown. */
static const uint8_t s_mem_gc_013040[] = {
    0x4C, 0xFE, 0xC3, 0x41, 0x7C, 0x11, 0xDF, 0x3A, 0x4B, 0xCE,
    0x41, 0xBC, 0x93, 0x35, 0x4E, 0xBC,
};

/* 0x13080: main stick factory data (unknown layout), up to its calibration. */
static const uint8_t s_mem_gc_013080[] = {
    0xFF, 0x47, 0x79, 0x94, 0xB8, 0x86, 0x6B, 0xA0, 0x00, 0x0A,
    0xA0, 0x00, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBE, 0xE3, 0x3B, 0xBE, 0xE3,
    0x3B, 0x06, 0x65, 0x50, 0x06, 0x65, 0x50, 0x0A, 0xFF, 0xFF,
};

/* 0x130C0: C-stick factory data (unknown layout), up to its calibration. */
static const uint8_t s_mem_gc_0130C0[] = {
    0xFF, 0x47, 0x79, 0x94, 0xB8, 0x86, 0x6B, 0xA0, 0x00, 0x0A,
    0xA0, 0x00, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x18, 0x83, 0x31, 0x18, 0x83,
    0x31, 0x5F, 0xF4, 0x45, 0x5F, 0xF4, 0x45, 0x0A, 0xFF, 0xFF,
};

/*
 * 0x130A8 / 0x130E8: stick calibration, three packed 12-bit X/Y pairs: center, then the distance
 * to the top/right and to the bottom/left. The full range (2048 +/- 2047): the firmware applies its
 * own calibration and reports the whole span. (A retail controller's own: about 2030 +1274/-1202.)
 */
static const uint8_t s_mem_gc_stick_cal[] = {
    0x00, 0x08, 0x80, 0xFF, 0xF7, 0x7F, 0xFF, 0xF7, 0x7F,
};

/* 0x13100: motion sensor calibration (the last float is 9.9: gravity). */
static const uint8_t s_mem_gc_013100[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE3, 0x75, 0xAE, 0xBC, 0x5B, 0xB6, 0xDA, 0xBE,
    0x1F, 0x9F, 0x1E, 0x41,
};

/*
 * 0x13140: trigger rest values, it seems (a retail controller: 0x24, 0x22, with its triggers resting
 * near 33). Zero: the firmware reports its triggers over the full range, 0 at rest.
 */
static const uint8_t s_mem_gc_013140[] = {
    0x00, 0x00,
};

/*
 * Regions that exist but read back fully erased. 0x1FC040 holds user stick calibration and is
 * uninitialised unless the user calibrated the controller.
 */
static const ns2_memory_block_s s_blocks_pro2[] = {
    {0x013000u, sizeof(s_mem_013000), s_mem_013000},
    {0x013040u, sizeof(s_mem_013040), s_mem_013040},
    {0x013060u, 0u, NULL},
    {0x013080u, sizeof(s_mem_013080), s_mem_013080},
    {0x0130A8u, sizeof(s_mem_0130A8), s_mem_0130A8},
    {0x0130C0u, sizeof(s_mem_0130C0), s_mem_0130C0},
    {0x0130E8u, sizeof(s_mem_0130E8), s_mem_0130E8},
    {0x013100u, sizeof(s_mem_013100), s_mem_013100},
    {0x1FC040u, 0u, NULL},
};

/* 0x13060 and the user calibration at 0x1FC040 read back erased. */
static const ns2_memory_block_s s_blocks_gc[] = {
    {0x013000u, sizeof(s_mem_013000), s_mem_013000},
    {0x013040u, sizeof(s_mem_gc_013040), s_mem_gc_013040},
    {0x013060u, 0u, NULL},
    {0x013080u, sizeof(s_mem_gc_013080), s_mem_gc_013080},
    {0x0130A8u, sizeof(s_mem_gc_stick_cal), s_mem_gc_stick_cal},
    {0x0130C0u, sizeof(s_mem_gc_0130C0), s_mem_gc_0130C0},
    {0x0130E8u, sizeof(s_mem_gc_stick_cal), s_mem_gc_stick_cal},
    {0x013100u, sizeof(s_mem_gc_013100), s_mem_gc_013100},
    {0x013140u, sizeof(s_mem_gc_013140), s_mem_gc_013140},
    {0x1FC040u, 0u, NULL},
};

void ns2_memory_init(void)
{
    ns2_device_config_s cfg;
    ns2_config_get(&cfg);

    uint8_t *m = s_mem_013000;
    memset(m, 0, sizeof(s_mem_013000));

    /* Block header. */
    m[0] = 0x01;
    m[1] = 0x00;
    /* ASCII serial number, 16 bytes zero padded. */
    memcpy(&m[2], cfg.device_serial, 16);
    /* Vendor / product ID, little-endian. */
    m[18] = (uint8_t)(NS2_VENDOR_ID & 0xFFu);
    m[19] = (uint8_t)(NS2_VENDOR_ID >> 8);
    uint16_t pid = ns2_config_get_product_id();
    m[20] = (uint8_t)(pid & 0xFFu);
    m[21] = (uint8_t)(pid >> 8);
    /* Unknown (hardware revision?), copied from a retail Pro Controller 2 / GameCube controller. */
    bool gc = cfg.type == NS2_DEVTYPE_GC;
    m[22] = 0x01;
    m[23] = gc ? 0x04 : 0x06;
    m[24] = 0x01;
    /* Colors: body, buttons, highlight, grip. */
    const ns2_rgb_s *colors[4] = {&cfg.colors.body, &cfg.colors.buttons, &cfg.colors.highlight, &cfg.colors.grip};
    for (int i = 0; i < 4; i++)
    {
        m[25 + i * 3 + 0] = colors[i]->r;
        m[25 + i * 3 + 1] = colors[i]->g;
        m[25 + i * 3 + 2] = colors[i]->b;
    }
}

bool ns2_memory_read(uint32_t addr, uint16_t len, uint8_t *out)
{
    if (out == NULL || len == 0u || len > NS2_MEMORY_READ_MAX)
    {
        return false;
    }

    memset(out, 0xFF, len);

    uint32_t addr_end = addr + len;
    bool found = false;

    bool gc = ns2_config_get_type() == NS2_DEVTYPE_GC;
    const ns2_memory_block_s *blocks = gc ? s_blocks_gc : s_blocks_pro2;
    size_t count = gc ? sizeof(s_blocks_gc) / sizeof(s_blocks_gc[0]) : sizeof(s_blocks_pro2) / sizeof(s_blocks_pro2[0]);

    for (size_t i = 0; i < count; i++)
    {
        const ns2_memory_block_s *b = &blocks[i];
        uint32_t block_end = b->start_addr + b->len;

        /* Empty regions only mark the address as known; their contents stay erased. */
        if (b->len == 0u)
        {
            if (addr == b->start_addr)
            {
                found = true;
            }
            continue;
        }

        if (block_end <= addr || b->start_addr >= addr_end)
        {
            continue;
        }

        uint32_t start = (addr > b->start_addr) ? addr : b->start_addr;
        uint32_t end = (addr_end < block_end) ? addr_end : block_end;
        memcpy(&out[start - addr], &b->data[start - b->start_addr], end - start);
        found = true;
    }

    return found;
}
