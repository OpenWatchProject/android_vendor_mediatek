/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __KM_SHARED_UTIL_H__
#define __KM_SHARED_UTIL_H__

#ifdef SOTER
#include "mtk_keymaster_defs.h"
#else
#include "hardware/keymaster_defs.h"
#endif
#ifndef BITS_TO_BYTES
#define BITS_TO_BYTES(n) (((n)+7)/8)
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/**
 * Read a serialized little-endian encoding of a uint32_t.
 *
 * @param  pos position to read from
 * @return     value
 */
uint32_t get_u32(
    const uint8_t *pos);

/**
 * Read a serialized little-endian encoding of a uint64_t.
 *
 * @param  pos position to read from
 * @return     value
 */
uint64_t get_u64(
    const uint8_t *pos);

/**
 * Write a serialized little-endian encoding of a uint32_t.
 *
 * @param pos position to write to
 * @param val value
 */
void set_u32(
    uint8_t *pos,
    uint32_t val);

/**
 * Write a serialized little-endian encoding of a uint64_t.
 *
 * @param pos position to write to
 * @param val value
 */
void set_u64(
    uint8_t *pos,
    uint64_t val);

/**
 * Write a serialized little-endian encoding of a uint32_t and increment the
 * position by 4 bytes.
 *
 * @param pos pointer to position to write to
 * @param val value
 */
void set_u32_increment_pos(
    uint8_t **pos,
    uint32_t val);

/**
 * Write a serialized little-endian encoding of a uint64_t and increment the
 * position by 8 bytes.
 *
 * @param pos pointer to position to write to
 * @param val value
 */
void set_u64_increment_pos(
    uint8_t **pos,
    uint64_t val);

/**
 * Write data and increment position by length of data.
 *
 * @param pos pointer to position to write to
 * @param src buffer to write
 * @param len length of buffer
 */
void set_data_increment_pos(
    uint8_t **pos,
    const uint8_t *src,
    uint32_t len);

/**
 * Copy data and increment source position.
 *
 * @param dest destination for data
 * @param src pointer to position to copy from
 * @param len length of data to copy and then to increment by
 */
void set_data_increment_src(
    uint8_t *dest,
    uint8_t **src,
    uint32_t len);

/**
 * Set a pointer and increment position.
 *
 * @param ptr pointer to pointer to set
 * @param src pointer to position to set it to
 * @param len length by which to increment \p *src
 */
void set_ptr_increment_src(
    uint8_t **ptr,
    uint8_t **src,
    uint32_t len);

/**
 * Check consistency of parameters.
 * @param algorithm key type
 * @param purpose operation purpose
 * @return whether \p algorithm and \p purpose are consistent
 */
bool check_algorithm_purpose(
    keymaster_algorithm_t algorithm,
    keymaster_purpose_t purpose);

/**
 * Memory needed to store a set of (HW- or SW-enforced) characteristics, n32 of
 * which are uint32_t (enum, uint, bool) and n64 of which are uint64_t (ulong,
 * date).
 */
#define KM_W_CHARACTERISTICS_SIZE(n32,n64) (4 + (4 + 4)*(n32) + (4 + 8)*(n64))

/* Hardware-enforced characteristics:
 *   KM_TAG_PURPOSE, // enum, uint32_t, up to 4 of these
 *   KM_TAG_ALGORITHM, // enum, uint32_t
 *   KM_TAG_KEY_SIZE, // uint, uint32_t
 *   KM_TAG_BLOCK_MODE, // enum, uint32_t, up to 4 of these
 *   KM_TAG_DIGEST, // enum, uint32_t, up to 7 of these
 *   KM_TAG_PADDING, // enum, uint32_t, up to 5 of these
 *   KM_TAG_RSA_PUBLIC_EXPONENT, // ulong, uint64_t
 *   KM_TAG_BLOB_USAGE_REQUIREMENTS, // enum, uint32_t
 *   KM_TAG_BOOTLOADER_ONLY, // bool, uint32_t
 *   KM_TAG_ORIGIN, // enum, uint32_t
 *   KM_TAG_ROLLBACK_RESISTANT, // bool, uint32_t
 *   KM_TAG_USER_SECURE_ID, // uint64_t, up to ? of these
 *   KM_TAG_NO_AUTH_REQUIRED, // bool, uint32_t
 *   KM_TAG_USER_AUTH_TYPE, // enum, uint32_t
 *   KM_TAG_AUTH_TIMEOUT, // uint, uint32_t
 *   KM_TAG_CALLER_NONCE, // bool, uint32_t
 *   KM_TAG_MIN_MAC_LENGTH, // uint, uint32_t
 */
#define KM_MAX_N_USER_SECURE_ID 8 // arbitrary
#define KM_N_HW_32 31
#define KM_N_HW_64 (1 + KM_MAX_N_USER_SECURE_ID)

/* Software-enforced characteristics: (set and enforced by keystore)
 *   KM_TAG_ACTIVE_DATETIME, // date, uint64_t
 *   KM_TAG_CREATION_DATETIME, // date, uint64_t
 *   KM_TAG_MAX_USES_PER_BOOT, // uint, uint32_t
 *   KM_TAG_MIN_SECONDS_BETWEEN_OPS, // uint, uint32_t
 *   KM_TAG_ORIGINATION_EXPIRE_DATETIME, // date, uint64_t
 *   KM_TAG_USAGE_EXPIRE_DATETIME, // date, uint64_t
 */
#define KM_N_SW_32 2
#define KM_N_SW_64 4

#define KM_N_HW_CHARACTERISTICS (KM_N_HW_32 + KM_N_HW_64)
#define KM_N_SW_CHARACTERISTICS (KM_N_SW_32 + KM_N_SW_64)
#define KM_HW_CHARACTERISTICS_SIZE \
    KM_W_CHARACTERISTICS_SIZE(KM_N_HW_32, KM_N_HW_64)
#define KM_SW_CHARACTERISTICS_SIZE  \
    KM_W_CHARACTERISTICS_SIZE(KM_N_SW_32, KM_N_SW_64)

/**
 * Maximum amount of memory needed for serialized key characteristics.
 */
#define KM_CHARACTERISTICS_SIZE \
    (KM_HW_CHARACTERISTICS_SIZE + KM_SW_CHARACTERISTICS_SIZE)

/* Maximum number of parameters added by default on key import or key generation */
#define OWN_PARAMS_MAX_NB 3

/* Size of an array storing default parameters */
#define OWN_PARAMS_MAX_SIZE ( OWN_PARAMS_MAX_NB * (4+4) )  // OWN_PARAMS_NB x (tag + (enum or bool))

/**
 * Size of out_params buffer when required for begin() operation.
 *
 * This is enough to hold a 16-byte IV field, serialized
 * (param_count | tag | blob_length | blob_data).
 */
#define TEE_BEGIN_OUT_PARAMS_SIZE (4 + 4 + 4 + 16)

#endif /* __KM_SHARED_UTIL_H__ */
