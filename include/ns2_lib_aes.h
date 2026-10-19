/**
 * @file ns2_lib_aes.h
 * @brief Minimal AES-128 (ECB, encrypt only) used by the pairing key confirmation.
 *
 * Copyright (c) 2026 Mitchell Cairns, Hand Held Legend, LLC.
 *
 * Licensed under the Creative Commons Attribution-NonCommercial 4.0 International License
 * (CC BY-NC 4.0). Non-commercial use with attribution; commercial licensing from Hand Held Legend.
 * Full terms: https://creativecommons.org/licenses/by-nc/4.0/legalcode — see LICENSING.md in this folder
 *
 * SPDX-License-Identifier: CC-BY-NC-4.0
 */

#ifndef NS2_LIB_AES_H
#define NS2_LIB_AES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Encrypt one 16-byte block with AES-128 (FIPS-197).
 *
 * @param key 16-byte key.
 * @param in 16-byte plaintext.
 * @param out 16-byte ciphertext (may alias @p in).
 */
void ns2_aes128_encrypt(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);

#ifdef __cplusplus
}
#endif

#endif /* NS2_LIB_AES_H */
