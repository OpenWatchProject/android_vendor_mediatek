/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#ifdef TRUSTLET
#include "keymaster_ta_defs.h"
#include "tlStd.h"
#include "TlApi/TlApi.h"
#define KM_LOG(fmt, ...) tlDbgPrintf("%s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)
#define KM_ALLOC(l) tlApiMalloc(l, 0)
#define KM_FREE(a) tlApiFree(a)
#else
#include "keymaster_ta_defs.h"
#define LOG_TAG "TlcTeeKeyMaster"
#ifdef TT_BUILD
#include <log.h>
#else
#include <log/log.h>
#define LOG_E ALOGE
#define LOG_I ALOGI
#define LOG_D ALOGD
#endif
#define KM_LOG(fmt, ...) LOG_E(fmt, __VA_ARGS__)
#define KM_ALLOC(l) malloc(l)
#define KM_FREE(a) free(a)
#endif

#define CHECK_RESULT_OK(expr) \
    do { \
        ret = (expr); \
        if (ret != KM_ERROR_OK) { \
            KM_LOG("%s == %d", #expr, ret); \
            goto end; \
        } \
    } while (0)

#define CHECK_TRUE(errcode, expr) \
    do { \
        if (!(expr)) { \
            KM_LOG("'%s' is false", #expr); \
            ret = (errcode); \
            goto end; \
        } \
    } while (false)

#define CHECK_NOT_NULL(expr) \
    CHECK_TRUE(KM_ERROR_UNEXPECTED_NULL_POINTER, (expr) != NULL)

#define CHECK_TLAPI_OK(expr) \
    do { \
        tlApiResult_t tret = (expr); \
        if (tret != TLAPI_OK) { \
            KM_LOG("%s == 0x%08x", #expr, tret); \
            ret = tl_to_km_err(tret); \
            goto end; \
        } \
    } while (0)

#ifndef BITS_TO_BYTES
#define BITS_TO_BYTES(n) (((n)+7)/8)
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

typedef struct {
    uint32_t os_version;
    uint32_t os_patchlevel;
    uint32_t vendor_patchlevel;
    uint32_t boot_patchlevel;
} bootimage_info_t;

/**
 * Allocate a (zeroed) buffer.
 *
 * Buffer must later be freed with km_free().
 *
 * @param a pointer to allocation address
 * @param l desired length of buffer
 *
 * @return KM_ERROR_OK or error
 */
keymaster_error_t km_alloc(
    uint8_t **a,
    uint32_t l);

void km_free(
    uint8_t *a);

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

/***** ASN.1 and DER utilities *****/

/* Read a tag and length from position *p, with *len remaining, return the
 * length in *l and update *p and *len. Return true on success, false on failure
 * or if length exceeds 65535 or if length exceeds length of data remaining
 * afterwards.
 *
 * der_read_integer_length strips any leading zero and adjusts the length
 * accordingly (thus it is only intended for unsigned integers).
 */
bool der_read_integer_length(
    size_t *l,
    const uint8_t **p,
    size_t *len);
bool der_read_bit_string_length(
    size_t *l,
    const uint8_t **p,
    size_t *len);
bool der_read_octet_string_length(
    size_t *l,
    const uint8_t **p,
    size_t *len);
bool der_read_oid_length(
    size_t *l,
    const uint8_t **p,
    size_t *len);
bool der_read_sequence_length(
    size_t *l,
    const uint8_t **p,
    size_t *len);

bool der_read_explicit_tag_and_length(
    int *tag,
    size_t *l,
    const uint8_t **p,
    size_t *len);

/**
 * Decode key data and reserialize it into our standard format.
 *
 * This function allocates \p key_data. The caller should free it.
 *
 * @param[out] key_data reserialized key data
 * @param[out] key_data_len length of \p key_data
 * @param[in] key_format format of input data
 * @param[in] key_type type of encoded key
 * @param[in] data input data
 * @param[in] data_len length of input data
 *
 * @return KM_ERROR_OK or error
 */
keymaster_error_t decode_key_data(
    uint8_t **key_data,
    size_t *key_data_len,
    keymaster_key_format_t key_format,
    keymaster_algorithm_t key_type,
    const uint8_t *data,
    size_t data_len);

/***********************************/

/**
 * Find the KM_TAG_KEY_SIZE corresponding to a KM_TAG_EC_CURVE
 *
 * @return key size, or 0 if curve not recognized
 */
uint32_t ec_bitlen(
    keymaster_ec_curve_t curve);

#define KM_MAX_N_USER_SECURE_ID 8 // arbitrary

/**
 * Maximum amount of memory needed for serialized certificate chain.
 */
#define KM_CERT_CHAIN_SIZE (16*1024)

/* Maximum size of array storing parameters added on key import or generation.
 *
 * 32-bit tags (max 8):
 *
 * KM_TAG_BLOB_USAGE_REQUIREMENTS
 * KM_TAG_EC_CURVE
 * KM_TAG_ORIGIN
 * KM_TAG_ROLLBACK_RESISTANT
 * KM_TAG_OS_VERSION
 * KM_TAG_OS_PATCHLEVEL
 * KM_TAG_VENDOR_PATCHLEVEL
 * KM_TAG_BOOT_PATCHLEVEL
 *
 * 64-bit tags (max 2):
 *
 * KM_TAG_USER_SECURE_ID (up to 2)
 */
#define OWN_PARAMS_MAX_SIZE (8*(4+4) + 2*(4+8)) // tag + value

/**
 * Size of out_params buffer when required for begin() operation.
 *
 * This is enough to hold a 16-byte IV field, serialized
 * (param_count | tag | blob_length | blob_data).
 */
#define TEE_BEGIN_OUT_PARAMS_SIZE (4 + 4 + 4 + 16)

#endif /* __KM_SHARED_UTIL_H__ */
