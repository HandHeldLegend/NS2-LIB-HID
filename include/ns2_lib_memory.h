/**
 * @file ns2_lib_memory.h
 * @brief Simulated controller flash memory served to command 0x02 (flash memory reads).
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

#ifndef NS2_LIB_MEMORY_H
#define NS2_LIB_MEMORY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Largest read the console issues in one command (and the most the response can carry). */
#define NS2_MEMORY_READ_MAX 0x78u

/** @brief Rebuild the device info block (0x13000) from the active configuration. */
void ns2_memory_init(void);

/**
 * @brief Read @p len bytes of simulated memory starting at @p addr.
 *
 * Unpopulated bytes read back as 0xFF (erased flash).
 *
 * @return false when @p addr is outside any known region or @p len is invalid.
 */
bool ns2_memory_read(uint32_t addr, uint16_t len, uint8_t *out);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_MEMORY_H */
