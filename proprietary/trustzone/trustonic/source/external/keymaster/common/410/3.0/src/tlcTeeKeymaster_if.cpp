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

/* Get PRIiN and friends from <inttypes.h> despite the C++ standard not
 * wanting to let us.
 */
#define __STDC_FORMAT_MACROS

#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>

#include <api_keymaster_defs.h>

#include "tlcTeeKeymasterN_if.h"
#include "buildTag.h"
#include "km_util.h"
#include "serialization.h"
#include "km_encodings.h"

#if !RPMB_SUPPORT
#include "cust_tee_keymaster_utils.h"
#define ATTEST_KEYBOX_FILE "/mnt/vendor/persist/attest_keybox.so"
#endif

#define KEY_ATTESTATION_DEBUG (0)

#if KEY_ATTESTATION_DEBUG
#define KEY_TO_ATTEST_FILE          "/data/misc/key_to_attest.der"
#define ATTESTATION_CERT0_FILE      "/data/misc/cert0.der"
#define ATTESTATION_CERT1_FILE      "/data/misc/cert1.der"
#define ATTESTATION_CERT2_FILE      "/data/misc/cert2.der"
#define ATTESTATION_CERT3_FILE      "/data/misc/cert3.der"
#endif

#define MAX_RETRY_TIMES 10

#define RSA_MAX_KEY_SIZE 4096
#define EC_MAX_KEY_SIZE 521
#define AES_MAX_KEY_SIZE 256
//KOSHI: should be VTS bug, but in order to pass the tests, enlarge the MAX size define.
#define HMAC_MAX_KEY_SIZE 2048
//#define HMAC_MAX_KEY_SIZE 1024

#ifdef NDEBUG
/* In Release mode these macros are void. */
#define PRINT_BUFFER(data, data_length)
#define PRINT_BLOB(blob)
#define PRINT_BLOB_HEX(blob, length)
#define PRINT_PARAM_SET(params)

#else
#include <sstream>

#define SBUF_SIZE 1024
static char sbuf[SBUF_SIZE]; // for formatting debug output prior to LOG_I

static int snprint_buffer(
    char *s, size_t n,
    const uint8_t *data,
    size_t data_length)
{
    if (data == NULL) {
        return snprintf(s, n, "<NULL>\n");
    } else {
        return snprintf(s, n, "<data of length %zu>\n", data_length);
    }
}

static int snprint_uint64(
    char *s, size_t n,
    uint64_t x)
{
    return snprintf(s, n, "0x%08x%08x\n", (uint32_t)(x >> 32), (uint32_t)(x & 0xFFFFFFFF));
}

static int snprint_bool(
    char *s, size_t n,
    bool x)
{
    return snprintf(s, n, x ? "true\n" : "false\n");
}

static int snprint_blob(
    char *s, size_t n,
    const keymaster_blob_t *blob)
{
    if (blob == NULL) {
        return snprintf(s, n, "<NULL>\n");
    } else {
        return snprint_buffer(s, n, blob->data, blob->data_length);
    }
}

static int snprint_param_set(
    char *s, size_t n,
    const keymaster_key_param_set_t *param_set)
{
    if (param_set == NULL) {
        return snprintf(s, n, "<NULL>\n");
    }
    else {
        int r;
        size_t i = 0, l = 0;
        while ((i < param_set->length) && (l < SBUF_SIZE)) {
            const keymaster_key_param_t param = param_set->params[i];
            switch (param.tag) {
#define PRINT_ENUM snprintf(s + l, SBUF_SIZE - l, "0x%08x\n", param.enumerated)
#define PRINT_UINT snprintf(s + l, SBUF_SIZE - l, "%u\n", param.integer)
#define PRINT_ULONG snprint_uint64(s + l, SBUF_SIZE - l, param.long_integer)
#define PRINT_DATE snprint_uint64(s + l, SBUF_SIZE - l, param.date_time)
#define PRINT_BOOL snprint_bool(s + l, SBUF_SIZE - l, param.boolean)
// #define PRINT_BIGNUM
#define PRINT_BYTES snprint_blob(s + l, SBUF_SIZE - l, &param.blob)
#define PARAM_CASE(tag, printval) \
    case tag: \
        r = snprintf(s + l, SBUF_SIZE - l, "%s: ", #tag); \
        if (r < 0) return r; \
        l += r; \
        if (l < SBUF_SIZE) { \
            r = printval; \
            if (r < 0) return r; \
            l += r; \
        } \
        break;
#define PARAM_CASE_ENUM(tag) PARAM_CASE(tag, PRINT_ENUM)
#define PARAM_CASE_UINT(tag) PARAM_CASE(tag, PRINT_UINT)
#define PARAM_CASE_ULONG(tag) PARAM_CASE(tag, PRINT_ULONG)
#define PARAM_CASE_DATE(tag) PARAM_CASE(tag, PRINT_DATE)
#define PARAM_CASE_BOOL(tag) PARAM_CASE(tag, PRINT_BOOL)
// #define PARAM_CASE_BIGNUM
#define PARAM_CASE_BYTES(tag) PARAM_CASE(tag, PRINT_BYTES)
                PARAM_CASE_DATE(KM_TAG_ACTIVE_DATETIME)
                PARAM_CASE_ENUM(KM_TAG_ALGORITHM)
                PARAM_CASE_BOOL(KM_TAG_ALL_APPLICATIONS)
                PARAM_CASE_BOOL(KM_TAG_ALL_USERS)
                PARAM_CASE_BOOL(KM_TAG_ALLOW_WHILE_ON_BODY)
                PARAM_CASE_BYTES(KM_TAG_APPLICATION_DATA)
                PARAM_CASE_BYTES(KM_TAG_APPLICATION_ID)
                PARAM_CASE_BYTES(KM_TAG_ASSOCIATED_DATA)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_CHALLENGE)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_APPLICATION_ID)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_BRAND)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_DEVICE)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_PRODUCT)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_SERIAL)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_IMEI)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_MEID)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_MANUFACTURER)
                PARAM_CASE_BYTES(KM_TAG_ATTESTATION_ID_MODEL)
                PARAM_CASE_UINT(KM_TAG_AUTH_TIMEOUT)
                PARAM_CASE_BYTES(KM_TAG_AUTH_TOKEN)
                PARAM_CASE_ENUM(KM_TAG_BLOB_USAGE_REQUIREMENTS)
                PARAM_CASE_ENUM(KM_TAG_BLOCK_MODE)
                PARAM_CASE_BOOL(KM_TAG_BOOTLOADER_ONLY)
                PARAM_CASE_BOOL(KM_TAG_CALLER_NONCE)
                PARAM_CASE_DATE(KM_TAG_CREATION_DATETIME)
                PARAM_CASE_ENUM(KM_TAG_DIGEST)
                PARAM_CASE_ENUM(KM_TAG_EC_CURVE)
                PARAM_CASE_BOOL(KM_TAG_ECIES_SINGLE_HASH_MODE)
                PARAM_CASE_BOOL(KM_TAG_EXPORTABLE)
                PARAM_CASE_BOOL(KM_TAG_INCLUDE_UNIQUE_ID)
                PARAM_CASE_ENUM(KM_TAG_KDF)
                PARAM_CASE_UINT(KM_TAG_KEY_SIZE)
                PARAM_CASE_UINT(KM_TAG_MAC_LENGTH)
                PARAM_CASE_UINT(KM_TAG_MAX_USES_PER_BOOT)
                PARAM_CASE_UINT(KM_TAG_MIN_MAC_LENGTH)
                PARAM_CASE_UINT(KM_TAG_MIN_SECONDS_BETWEEN_OPS)
                PARAM_CASE_BOOL(KM_TAG_NO_AUTH_REQUIRED)
                PARAM_CASE_BYTES(KM_TAG_NONCE)
                PARAM_CASE_ENUM(KM_TAG_ORIGIN)
                PARAM_CASE_DATE(KM_TAG_ORIGINATION_EXPIRE_DATETIME)
                PARAM_CASE_UINT(KM_TAG_OS_PATCHLEVEL)
                PARAM_CASE_UINT(KM_TAG_OS_VERSION)
                PARAM_CASE_ENUM(KM_TAG_PADDING)
                PARAM_CASE_ENUM(KM_TAG_PURPOSE)
                PARAM_CASE_BOOL(KM_TAG_RESET_SINCE_ID_ROTATION)
                PARAM_CASE_BOOL(KM_TAG_ROLLBACK_RESISTANT)
                PARAM_CASE_BYTES(KM_TAG_ROOT_OF_TRUST)
                PARAM_CASE_ULONG(KM_TAG_RSA_PUBLIC_EXPONENT)
                PARAM_CASE_BYTES(KM_TAG_UNIQUE_ID)
                PARAM_CASE_BOOL(KM_TAG_UNLOCKED_DEVICE_REQUIRED)
                PARAM_CASE_DATE(KM_TAG_USAGE_EXPIRE_DATETIME)
                PARAM_CASE_ENUM(KM_TAG_USER_AUTH_TYPE)
                PARAM_CASE_UINT(KM_TAG_USER_ID)
                PARAM_CASE_ULONG(KM_TAG_USER_SECURE_ID)

                default:
                    r = snprintf(s + l, SBUF_SIZE - l, "<unknown tag 0x%08x>\n", param.tag);
                    if (r < 0) return r;
                    l += r;
            }
            i++;
        }
        return l;
    }
}

#define PRINT_BUFFER(data, data_length) \
    do { \
        if (snprint_buffer(sbuf, SBUF_SIZE, data, data_length) >= 0) { \
            LOG_D("%s = %s", #data, sbuf); \
        } \
    } while (0)
#define PRINT_BLOB(blob) \
    do { \
        if (snprint_blob(sbuf, SBUF_SIZE, blob) >= 0) { \
            LOG_D("%s = %s", #blob, sbuf); \
        } \
    } while (0)
#define PRINT_PARAM_SET(params) \
    do { \
        if (snprint_param_set(sbuf, SBUF_SIZE, params) >= 0) { \
            LOG_D("%s =\n%s", #params, sbuf); \
        } \
    } while (0)

#define PRINT_BLOB_HEX(blob, len)                           \
    do {                                                    \
        std::ostringstream buf;                             \
        for(size_t i=0; i<len; ++i) {                       \
            buf << std::hex << (unsigned) blob[i] << " ";   \
        }                                                   \
        LOG_D("%s = %s\n", #blob, buf.str().c_str());       \
    }                                                       \
    while (0)

#endif

/* Global definitions */
static const __attribute__((used)) char* buildtag = MOBICORE_COMPONENT_BUILD_TAG;
static const uint32_t gDeviceId = MC_DEVICE_ID_DEFAULT;
static const mcUuid_t gUuid = TEE_KEYMASTER_N_TA_UUID;

extern inline void keymaster_free_param_values(keymaster_key_param_t* param, size_t param_count);
extern inline void keymaster_free_param_set(keymaster_key_param_set_t* set);
extern inline void keymaster_free_characteristics(keymaster_key_characteristics_t* characteristics);

/* Acceptable params for all commands */
#define AUTH_TAGS \
    KM_TAG_ALL_USERS, \
    KM_TAG_USER_ID, \
    KM_TAG_USER_SECURE_ID, \
    KM_TAG_NO_AUTH_REQUIRED, \
    KM_TAG_USER_AUTH_TYPE, \
    KM_TAG_AUTH_TIMEOUT, \
    KM_TAG_ALL_APPLICATIONS, \
    KM_TAG_APPLICATION_ID, \
    KM_TAG_APPLICATION_DATA

/* Acceptable params for generate_key() and import_key() */
#define KEY_CREATION_ALLOWED_TAGS \
    AUTH_TAGS, \
    KM_TAG_PURPOSE, \
    KM_TAG_ALGORITHM, \
    KM_TAG_KEY_SIZE, \
    KM_TAG_BLOCK_MODE, \
    KM_TAG_DIGEST, \
    KM_TAG_PADDING, \
    KM_TAG_CALLER_NONCE, \
    KM_TAG_MIN_MAC_LENGTH, \
    KM_TAG_EC_CURVE, \
    KM_TAG_RSA_PUBLIC_EXPONENT, \
    KM_TAG_ACTIVE_DATETIME, \
    KM_TAG_ORIGINATION_EXPIRE_DATETIME, \
    KM_TAG_USAGE_EXPIRE_DATETIME, \
    KM_TAG_MIN_SECONDS_BETWEEN_OPS, \
    KM_TAG_MAX_USES_PER_BOOT, \
    KM_TAG_INCLUDE_UNIQUE_ID, \
    KM_TAG_BOOTLOADER_ONLY, \
    KM_TAG_BLOB_USAGE_REQUIREMENTS, \
    KM_TAG_ALLOW_WHILE_ON_BODY, \
    KM_TAG_UNLOCKED_DEVICE_REQUIRED

static const keymaster_tag_t key_creation_allowed_params [] = {
    KEY_CREATION_ALLOWED_TAGS
};

/* Acceptable params for import_key() */
static const keymaster_tag_t key_import_allowed_params [] = {
    KEY_CREATION_ALLOWED_TAGS,
    KM_TAG_CREATION_DATETIME /* Allow this to work around Keystore bug. */
};

/* Acceptable params for begin() */
static const keymaster_tag_t op_begin_allowed_params [] = {
    AUTH_TAGS,
    KM_TAG_ALGORITHM,
    KM_TAG_BLOCK_MODE,
    KM_TAG_DIGEST,
    KM_TAG_PADDING,
    KM_TAG_ASSOCIATED_DATA,
    KM_TAG_NONCE,
    KM_TAG_AUTH_TOKEN,
    KM_TAG_MAC_LENGTH,
    KM_TAG_PURPOSE
};

/* Acceptable params for update() and finish() */
static const keymaster_tag_t op_allowed_params [] = {
    AUTH_TAGS,
    KM_TAG_ASSOCIATED_DATA,
    KM_TAG_AUTH_TOKEN,
    KM_TAG_DIGEST,
    KM_TAG_PADDING,
    KM_TAG_ALGORITHM
};

/* Acceptable params for attest_key() */
static const keymaster_tag_t attest_allowed_params [] = {
    KM_TAG_ATTESTATION_CHALLENGE,
    KM_TAG_RESET_SINCE_ID_ROTATION,
    KM_TAG_APPLICATION_ID,
    KM_TAG_APPLICATION_DATA,
    KM_TAG_ATTESTATION_APPLICATION_ID,
    KM_TAG_ATTESTATION_ID_BRAND,
    KM_TAG_ATTESTATION_ID_DEVICE,
    KM_TAG_ATTESTATION_ID_PRODUCT,
    KM_TAG_ATTESTATION_ID_SERIAL,
    KM_TAG_ATTESTATION_ID_IMEI,
    KM_TAG_ATTESTATION_ID_MEID,
    KM_TAG_ATTESTATION_ID_MANUFACTURER,
    KM_TAG_ATTESTATION_ID_MODEL
};

/**
 * Check that all tags in \p params occur in the given list.
 */
static keymaster_error_t check_params(
    const keymaster_key_param_set_t *params,
    const keymaster_tag_t *tags,
    size_t tag_count)
{
    if ((tags == NULL) && (tag_count != 0)) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    if ((params != NULL) && (params->length != 0)) {
        if (params->params == NULL) {
            return KM_ERROR_UNEXPECTED_NULL_POINTER;
        }
        for (uint32_t i = 0; i < params->length; i++) {
            keymaster_tag_t tag = params->params[i].tag;
            bool ok = false;
            size_t j = 0;
            while (!ok && (j < tag_count)) {
                ok = (tags[j] == tag);
                j++;
            }
            if (!ok && (tag != KM_TAG_ATTESTATION_APPLICATION_ID)) { /* HACK */
                LOG_E("%s: Invalid tag 0x%08x", __func__, tag);
                return KM_ERROR_INVALID_TAG;
            }
        }
    }

    return KM_ERROR_OK;
}

/**
 * Maximum supported key size in bits.
 *
 * @param algorithm key type
 * @return maximum supported size in bits
 */
static uint32_t km_max_key_size(
    keymaster_algorithm_t algorithm)
{
    switch (algorithm) {
        case KM_ALGORITHM_RSA:
            return RSA_MAX_KEY_SIZE;
        case KM_ALGORITHM_EC:
            return EC_MAX_KEY_SIZE;
        case KM_ALGORITHM_AES:
            return AES_MAX_KEY_SIZE;
        case KM_ALGORITHM_HMAC:
            return HMAC_MAX_KEY_SIZE;
        default:
            return 0;
    }
}

/**
 * Upper bound km_key_data in plain form.
 *
 * @param algorithm key type
 * @param key_size_in_bits key size in bits if known; if zero, assume worst case
 * @return upper bound for km_key_data size
 */
static uint32_t km_key_data_max_size(
    keymaster_algorithm_t algorithm,
    uint32_t  key_size_in_bits)
{
    uint32_t keylen = BITS_TO_BYTES(
        (key_size_in_bits > 0) ? key_size_in_bits : km_max_key_size(algorithm));
    uint32_t prelim_size = 4 + 4; // key type and key size, both uint32_t
    switch(algorithm)
    {
        case KM_ALGORITHM_RSA:
            /* n + e + d + p + q + dp + dq + qinv */
            return prelim_size + KM_RSA_METADATA_SIZE + 8 * keylen;
        case KM_ALGORITHM_EC:
            /* k + x + y */
            return prelim_size + KM_EC_METADATA_SIZE + 3 * keylen;
        case KM_ALGORITHM_AES:
        case KM_ALGORITHM_HMAC:
            /* no metadata */
            return prelim_size + keylen;
        default:
            /* Unsupported algorithm */
            return 0;
    }
}

/**
 * Calculate upper bound on length of exported key data.
 *
 * @return upper bound on length of exported data, or 0 on error
 */
static uint32_t export_data_length(
    keymaster_algorithm_t key_type,
    uint32_t key_size) // bits
{
    switch (key_type) {
        case KM_ALGORITHM_AES:
        case KM_ALGORITHM_HMAC:
            // export not supported for symmetric keys
            return 0;
        case KM_ALGORITHM_RSA:
            // type | size | size | n_len | e_len | n | e
            return 5*4 + 2*BITS_TO_BYTES(key_size);
        case KM_ALGORITHM_EC:
            // type | size | curve | x_len | y_len | x | y
            return 5*4 + 2*BITS_TO_BYTES(key_size);;
        default:
            return 0;
    }
}

/**
 * Calculate upper bound for size in bytes of material in encrypted key blob.
 *
 * \param key_size_in_bits Key size in bits if known. If zero assume worst case.
 */
static uint32_t key_blob_max_size(
    keymaster_algorithm_t algorithm,
    uint32_t key_size_in_bits,
    uint32_t params_size_in_bytes)
{
    /* params_len (uint32_t) + caller-supplied params */
    uint32_t plain_material_size = 4 + params_size_in_bytes;

    /* characteristics set by us */
    plain_material_size += OWN_PARAMS_MAX_SIZE;

    /* raw key data */
    plain_material_size += km_key_data_max_size(algorithm, key_size_in_bits);

    /*
     * Blob consists of:
     * - 16-byte nonce, ciphertext, and 16-byte tag in version 0
     * - 4-byte header, 16-byte nonce, ciphertext, and 12-byte tag in version 1
     */
    return plain_material_size + 32;
}

/**
 * Map a buffer.
 */
keymaster_error_t map_buffer(
    mcSessionHandle_t* session_handle,
    const uint8_t *buf, uint32_t buflen,
    mcBulkMap_t *bufinfo)
{
    if ((buf != NULL) && (buflen != 0)) {
        mcResult_t mcRet = mcMap(session_handle, (void*)buf, buflen, bufinfo);
        if (mcRet != MC_DRV_OK) {
            LOG_E("%s: mcMap() returned 0x%08x", __func__, mcRet);
            return KM_ERROR_SECURE_HW_COMMUNICATION_FAILED;
        }
    }
    return KM_ERROR_OK;
}

/**
 * Unmap a buffer.
 */
void unmap_buffer(
    mcSessionHandle_t* session_handle,
    const uint8_t *buf,
    mcBulkMap_t *bufinfo)
{
    if (bufinfo->sVirtualAddr != 0) {
        mcResult_t mcRet = mcUnmap(session_handle, (void*)buf, bufinfo);
        if (mcRet != MC_DRV_OK) {
            LOG_E("%s: mcUnmap() returned 0x%08x", __func__, mcRet);
        }
    }
}

/**
 * Allocate memory and zero it.
 *
 * @param a pointer to address to allocate
 * @param l number of bytes
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t km_alloc(
    uint8_t **a,
    uint32_t l)
{
    *a = (uint8_t*)malloc(l);

    if ( *a == NULL) {
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }
    memset(*a, 0, l);
    return KM_ERROR_OK;
}

/**
 * Notify the trusted application and wait for response.
 */
keymaster_error_t transact(
    mcSessionHandle_t* session_handle,
    tciMessage_ptr tci)
{
    keymaster_error_t ret = KM_ERROR_OK;
    mcResult_t mcRet;

    mcRet = mcNotify(session_handle);
    if (mcRet != MC_DRV_OK) {
        LOG_E("%s: mcNotify() returned 0x%08x", __func__, mcRet);
        ret = KM_ERROR_SECURE_HW_COMMUNICATION_FAILED;
        goto end;
    }

    mcRet = mcWaitNotification(session_handle, MC_INFINITE_TIMEOUT);
    if (mcRet != MC_DRV_OK) {
        LOG_E("%s: mcWaitNotification() returned 0x%08x", __func__, mcRet);
        ret = KM_ERROR_SECURE_HW_COMMUNICATION_FAILED;
        goto end;
    }

    CHECK_RESULT_OK( (keymaster_error_t)tci->response.header.returnCode );

end:
    return ret;
}

/**
 * Find the operation record given the handle.
 */
static keymaster_error_t lookup_operation(TEE_Session *session,
    keymaster_operation_handle_t handle,
    struct operation **op)
{
    size_t i;
    keymaster_error_t ret = KM_ERROR_INVALID_OPERATION_HANDLE;

    for (i = 0; i < MAX_SESSION_NUM; i++) {
        if (session->op[i].live &&
            session->op[i].handle == handle) {
            *op = &session->op[i];
            ret = KM_ERROR_OK;
            break;
        }
    }
    if (ret) {
        LOG_E("%s: op %#016" PRIx64 " not found", __func__, handle);
    }
    return ret;
}

/**
 * Find a spare operation slot. This can't fail; or, to put it another way,
 * if there isn't a spare slot, we've done something wrong.
 */
static struct operation *find_spare_operation_slot(TEE_Session *session,
    keymaster_operation_handle_t handle)
{
    size_t i;
    struct operation *op = NULL;

    for (i = 0; i < MAX_SESSION_NUM; i++) {
        if (!session->op[i].live) {
            op = &session->op[i];
            break;
        }
    }
    assert(op);
    op->live = true;
    op->handle = handle;
    session->live_ops++;
    LOG_D("%s: allocate op %#016" PRIx64 " to slot #%zu (%u ops live)",
        __func__, handle, i, session->live_ops);
    return op;
}

/**
 * Release an operation slot.  Do nothing if OP is null on entry.
 */
static void release_operation_slot(TEE_Session *session,
    struct operation *op)
{
    if (!op) return;
    assert(op->live);
    op->live = 0;
    session->live_ops--;
    LOG_D("%s: release op slot #%td (handle %#016" PRIx64 "; %u ops live)",
        __func__, op - session->op, op->handle, session->live_ops);
}

int TEE_Open(TEE_SessionHandle *pSessionHandle)
{
    struct TEE_Session *session;
    mcResult_t     mcRet;
    int rc = 0;
    size_t i;

    /* Validate session handle */
    if (pSessionHandle == NULL) {
        LOG_E("%s: Invalid session handle", __func__);
        return -EINVAL;
    }

    session = (struct TEE_Session *)malloc(sizeof(*session));
    if (session == NULL) {
        return -ENOMEM;
    }

    /* Initialize session handle data */
    memset(session, 0, sizeof(*session));

    /* Open MobiCore device */
    mcRet = mcOpenDevice(gDeviceId);
    if ((MC_DRV_OK != mcRet) &&
        (MC_DRV_ERR_DEVICE_ALREADY_OPEN != mcRet))
    {
        LOG_E("%s: mcOpenDevice() returned %d", __func__, mcRet);
        rc = -EBUSY;
        goto end_session;
    }

    /* Allocating WSM for TCI */
    mcRet = mcMallocWsm(gDeviceId, 0, sizeof(*session->pTci), (uint8_t **) &session->pTci, 0);
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcMallocWsm() returned %d", __func__, mcRet);
        rc = -ENOMEM;
        goto end_device;
    }

    /* Open session the TEE Keymaster trusted application */
    session->sessionHandle.deviceId = gDeviceId;
    for (i = 0; i < MAX_RETRY_TIMES; ++i) {
        mcRet = mcOpenSession(&session->sessionHandle,
                              &gUuid,
                              (uint8_t *) session->pTci,
                              (uint32_t) sizeof(tciMessage_t));
        if (mcRet == MC_DRV_OK)
            break;
        else
            LOG_E("%s: mcOpenSession() returned %d, retried %zu times", __func__, mcRet, i);
    }
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcOpenSession() returned %d", __func__, mcRet);
        mcFreeWsm(gDeviceId, (uint8_t*)session->pTci);
        rc = -EBUSY;
        goto end_device;
    }
    *pSessionHandle = (TEE_SessionHandle)session;

    /* No operations in progress yet. */
    for (i = 0; i < MAX_SESSION_NUM; i++)
        session->op[i].live = false;
    session->live_ops = 0;
    goto end;

end_device:
    mcRet = mcCloseDevice(gDeviceId);
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcCloseDevice() returned: %d", __func__, mcRet);
    }
end_session:
    free(session);
end:
    return rc;
}

void TEE_Close(TEE_SessionHandle sessionHandle)
{
    mcResult_t    mcRet;

    /* Validate session handle */
    if (sessionHandle == NULL) {
        LOG_E("%s: Invalid session handle", __func__);
        return;
    }
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;

    /* Close session */
    mcRet = mcCloseSession(&session->sessionHandle);
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcCloseSession() returned %d", __func__, mcRet);
    }

    mcFreeWsm(gDeviceId, (uint8_t*)session->pTci);
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcFreeWsm() returned %d", __func__, mcRet);
    }

    /* Close MobiCore device */
    mcRet = mcCloseDevice(gDeviceId);
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcCloseDevice() returned %d", __func__, mcRet);
    }
    free(session);
}

#define TEE_DR_ANDROID_UUID { { 7, 0x17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
static const mcUuid_t driver_Uuid = TEE_DR_ANDROID_UUID;

keymaster_error_t TEE_Configure(
    TEE_SessionHandle               sessionHandle,
    const keymaster_key_param_set_t *params)
{
    LOG_D("TEE_Configure");
    PRINT_PARAM_SET(params);

    struct TEE_Session *driver_session = NULL;
    mcResult_t mcRet = MC_DRV_OK;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    keymaster_error_t ret = KM_ERROR_OK;
    mcSessionHandle_t *session_handle = &session->sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcBulkMap_t param_map;
    scoped_buf_ptr_t serialized_params;

    /* Initialize session handle data */
    CHECK_RESULT_OK(km_alloc((uint8_t**)&driver_session, sizeof(*driver_session)));

    /* Allocating WSM for TCI */
    mcRet = mcMallocWsm(gDeviceId, 0, sizeof(*driver_session->pTci), (uint8_t **) &driver_session->pTci, 0);
    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE, mcRet == MC_DRV_OK);

    /* Open session the TEE Android secure driver */
    driver_session->sessionHandle.deviceId = gDeviceId;
    mcRet = mcOpenSession(&driver_session->sessionHandle,
                          &driver_Uuid,
                          (uint8_t *) driver_session->pTci,
                          (uint32_t) sizeof(tciMessage_t));
    CHECK_TRUE(KM_ERROR_SECURE_HW_COMMUNICATION_FAILED, mcRet == MC_DRV_OK);

    CHECK_RESULT_OK(km_serialize_params(serialized_params,
        params, false, 0, 0));
    CHECK_RESULT_OK(map_buffer(session_handle,
        serialized_params.buf.get(), serialized_params.size, &param_map));

    tci->command.header.commandId = CMD_ID_TEE_CONFIGURE;
    tci->configure.params.data = (uint32_t)param_map.sVirtualAddr;
    tci->configure.params.data_length = param_map.sVirtualLen;

    CHECK_RESULT_OK(transact(session_handle, tci));

end:
    /* Close session */
    if (driver_session) {
        mcRet = mcCloseSession(&driver_session->sessionHandle);
        if (mcRet != MC_DRV_OK)
            LOG_E("%s: mcCloseSession() returned %d", __func__, mcRet);

        mcRet = mcFreeWsm(gDeviceId, (uint8_t*)driver_session->pTci);
        if (mcRet != MC_DRV_OK)
            LOG_E("%s: mcFreeWsm() returned %d", __func__, mcRet);

        free(driver_session);
    }

    unmap_buffer(session_handle, serialized_params.buf.get(), &param_map);

    LOG_D("TEE_Configure exiting with %d", ret);
    return ret;
}

keymaster_error_t TEE_AddRngEntropy(
    TEE_SessionHandle sessionHandle,
    const uint8_t *data,
    uint32_t dataLength)
{
    LOG_D("TEE_AddRngEntropy");
    PRINT_BUFFER(data, dataLength);

    keymaster_error_t ret = KM_ERROR_OK;
    mcBulkMap_t dataInfo = {0, 0};
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcSessionHandle_t* session_handle = &session->sessionHandle;
    scoped_buf_ptr_t data1;

    CHECK_NOT_NULL(data);
    CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT,
        dataLength != 0);

    /* Hack to ensure that non-writable memory can be mapped. */
    CHECK_RESULT_OK(copy_to_scoped_buf(data, dataLength, data1));

    /* Map data */
    CHECK_RESULT_OK( map_buffer(session_handle, data1.buf.get(), data1.size, &dataInfo) );

    /* Update TCI buffer */
    tci->command.header.commandId = CMD_ID_TEE_ADD_RNG_ENTROPY;
    tci->add_rng_entropy.rng_data.data = (uint32_t)dataInfo.sVirtualAddr;
    tci->add_rng_entropy.rng_data.data_length = dataInfo.sVirtualLen;

    CHECK_RESULT_OK( transact(session_handle, tci) );

end:
    unmap_buffer(session_handle, data1.buf.get(), &dataInfo);
    if (data1.buf) {
        memset(data1.buf.get(), 0, data1.size);
    }

    LOG_D("TEE_AddRngEntropy exiting with %d", ret);
    return ret;
}


keymaster_error_t TEE_GenerateKey(
    TEE_SessionHandle                   sessionHandle,
    const keymaster_key_param_set_t*    params,
    keymaster_key_blob_t*               key_blob,
    keymaster_key_characteristics_t*    characteristics)
{
    LOG_D("TEE_GenerateKey");
    PRINT_PARAM_SET(params);

    keymaster_error_t ret = KM_ERROR_OK;
    mcBulkMap_t paramsInfo = {0, 0};
    mcBulkMap_t keyBlobInfo = {0, 0};
    mcBulkMap_t characteristicsInfo = {0, 0};
    uint32_t keySizeInBits = 0;
    uint64_t rsa_pubexp = 0;
    keymaster_algorithm_t algorithm;
    scoped_buf_ptr_t serializedData;
    uint8_t *key_chars = NULL;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcSessionHandle_t* session_handle = &session->sessionHandle;

    if (characteristics) {
        characteristics->hw_enforced = { NULL, 0 };
        characteristics->sw_enforced = { NULL, 0 };
    }
    CHECK_NOT_NULL(params);
    CHECK_NOT_NULL(key_blob);

    key_blob->key_material = NULL;

    /* Check parameters are valid for generate_key */
    CHECK_RESULT_OK(check_params(params, key_creation_allowed_params,
        sizeof(key_creation_allowed_params) / sizeof(keymaster_tag_t)) );

    /* Find algorithm, key size and RSA public exponent */
    CHECK_RESULT_OK( get_enumerated_tag(params,
        KM_TAG_ALGORITHM, reinterpret_cast<uint32_t*>(&algorithm)));
    /* check the param KM_TAG_KEY_SIZE value except the case KM_ALGORITHM_EC */
    if (algorithm != KM_ALGORITHM_EC) {
        CHECK_TRUE(KM_ERROR_UNSUPPORTED_KEY_SIZE,
            KM_ERROR_OK == get_integer_tag(params,
            KM_TAG_KEY_SIZE, &keySizeInBits));
    }
    if (algorithm == KM_ALGORITHM_RSA) {
        if (KM_ERROR_OK == get_long_integer_tag(params,
            KM_TAG_RSA_PUBLIC_EXPONENT, &rsa_pubexp))
        {
            CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT,
                (rsa_pubexp % 2 == 1) && (rsa_pubexp != 1));
        } else {
            rsa_pubexp = 65537; // default
        }
    }

    /* Serialize key parameters */
    CHECK_RESULT_OK(km_serialize_params(
        serializedData, params, true, 0, rsa_pubexp));

    /* Map key generation parameters */
    CHECK_RESULT_OK( map_buffer(session_handle, serializedData.buf.get(), serializedData.size, &paramsInfo) );

    /* Allocate memory for key material */
    key_blob->key_material_size = key_blob_max_size(
        algorithm, keySizeInBits, serializedData.size);
    CHECK_RESULT_OK(km_alloc((uint8_t**)&key_blob->key_material, key_blob->key_material_size));

    /* Map key blob buffer */
    CHECK_RESULT_OK( map_buffer(session_handle,
        key_blob->key_material, key_blob->key_material_size, &keyBlobInfo) );

    if (characteristics != NULL) {
        /* Allocate memory for the key characteristics. */
        CHECK_RESULT_OK(km_alloc(&key_chars, KM_CHARACTERISTICS_SIZE));
        /* Map buffer for key characteristics. */
        CHECK_RESULT_OK( map_buffer(session_handle,
            key_chars, KM_CHARACTERISTICS_SIZE, &characteristicsInfo) );
    }

    /* Update TCI buffer */
    tci->command.header.commandId = CMD_ID_TEE_GENERATE_KEY;
    tci->generate_key.params.data = (uint32_t)paramsInfo.sVirtualAddr;
    tci->generate_key.params.data_length = serializedData.size;
    tci->generate_key.key_blob.data = (uint32_t)keyBlobInfo.sVirtualAddr;
    tci->generate_key.key_blob.data_length = key_blob->key_material_size;
    tci->generate_key.characteristics.data = (uint32_t)characteristicsInfo.sVirtualAddr;
    tci->generate_key.characteristics.data_length =
        (characteristics != NULL) ? KM_CHARACTERISTICS_SIZE : 0;

    CHECK_RESULT_OK( transact(session_handle, tci) );

    /* Update key blob length */
    key_blob->key_material_size = tci->generate_key.key_blob.data_length;

    if (characteristics != NULL) { // Give characteristics to caller.
        CHECK_RESULT_OK(km_deserialize_characteristics(
            characteristics, key_chars, KM_CHARACTERISTICS_SIZE));
    }
end:
    unmap_buffer(session_handle, serializedData.buf.get(), &paramsInfo);
    if (key_blob != NULL) {
        unmap_buffer(session_handle, key_blob->key_material, &keyBlobInfo);
    }
    if (characteristics != NULL) {
        unmap_buffer(session_handle, key_chars, &characteristicsInfo);
    }

    free(key_chars);

    if (ret != KM_ERROR_OK) {
        if (key_blob != NULL) {
            free((void*)key_blob->key_material);
            key_blob->key_material = NULL;
            key_blob->key_material_size = 0;
        }
        if (characteristics != NULL) {
            keymaster_free_characteristics(characteristics);
        }
    }

    LOG_D("TEE_GenerateKey exiting with %d", ret);
    return ret;
}

keymaster_error_t TEE_GetKeyCharacteristics(
    TEE_SessionHandle                 sessionHandle,
    const keymaster_key_blob_t*       key_blob,
    const keymaster_blob_t*           client_id,
    const keymaster_blob_t*           app_data,
    keymaster_key_characteristics_t*  characteristics)
{
    LOG_D("TEE_GetKeyCharacteristics");

    keymaster_error_t ret = KM_ERROR_OK;
    uint8_t *key_chars = NULL;
    mcBulkMap_t keyBlobInfo = {0, 0};
    mcBulkMap_t clientIdInfo = {0, 0};
    mcBulkMap_t appDataInfo = {0, 0};
    mcBulkMap_t characteristicsInfo = {0, 0};
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcSessionHandle_t* session_handle = &session->sessionHandle;
    scoped_buf_ptr_t key_blob1;
    scoped_buf_ptr_t client_id1;
    scoped_buf_ptr_t app_data1;

    CHECK_NOT_NULL(tci);
    CHECK_NOT_NULL(key_blob);
    CHECK_NOT_NULL(characteristics);

    /* Map input buffers */
    if ( key_blob != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(key_blob->key_material, key_blob->key_material_size, key_blob1) );
        CHECK_RESULT_OK( map_buffer(session_handle,
        key_blob1.buf.get(), key_blob->key_material_size, &keyBlobInfo) );
    }

    if ( client_id != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(client_id->data, client_id->data_length, client_id1));
        CHECK_RESULT_OK( map_buffer(session_handle,
            client_id1.buf.get(), client_id1.size, &clientIdInfo) );
    }
    if ( app_data != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(app_data->data, app_data->data_length, app_data1));
        CHECK_RESULT_OK( map_buffer(session_handle,
            app_data1.buf.get(), app_data1.size, &appDataInfo) );
    }

    /* Allocate memory for characteristics */
    CHECK_RESULT_OK(km_alloc(&key_chars, KM_CHARACTERISTICS_SIZE));

    /* Map buffer for serialized key characteristics */
    CHECK_RESULT_OK( map_buffer(session_handle,
        key_chars, KM_CHARACTERISTICS_SIZE, &characteristicsInfo) );

    /* Now the get_key_characteristics command */
    tci->command.header.commandId = CMD_ID_TEE_GET_KEY_CHARACTERISTICS;
    tci->get_key_characteristics.key_blob.data = (uint32_t)keyBlobInfo.sVirtualAddr;
    tci->get_key_characteristics.key_blob.data_length = key_blob->key_material_size;
    tci->get_key_characteristics.client_id.data = (uint32_t)clientIdInfo.sVirtualAddr;
    tci->get_key_characteristics.client_id.data_length = clientIdInfo.sVirtualLen;
    tci->get_key_characteristics.app_data.data = (uint32_t)appDataInfo.sVirtualAddr;
    tci->get_key_characteristics.app_data.data_length = appDataInfo.sVirtualLen;
    tci->get_key_characteristics.characteristics.data = (uint32_t)characteristicsInfo.sVirtualAddr;
    tci->get_key_characteristics.characteristics.data_length = KM_CHARACTERISTICS_SIZE;

    CHECK_RESULT_OK( transact(session_handle, tci) );

    /* Deserialize */
    CHECK_RESULT_OK(km_deserialize_characteristics(
        characteristics, key_chars, KM_CHARACTERISTICS_SIZE));

end:
    if (key_blob1.buf) {
        unmap_buffer(session_handle, key_blob1.buf.get(), &keyBlobInfo);
    }
    if (client_id1.buf) {
        unmap_buffer(session_handle, client_id1.buf.get(), &clientIdInfo);
    }
    if (app_data1.buf) {
        unmap_buffer(session_handle, app_data1.buf.get(), &appDataInfo);
    }

    if (key_chars != NULL) {
        unmap_buffer(session_handle, key_chars, &characteristicsInfo);
    }

    free(key_chars);

    if (ret != KM_ERROR_OK) {
        if (characteristics != NULL) {
            keymaster_free_characteristics(characteristics);
        }
    }

    LOG_D("TEE_GetKeyCharacteristics exiting with %d", ret);
    return ret;
}

/**
 * Check if a format is supported for key import
 * @param format data format
 * @param algorithm key type
 * @return whether import is supported
 */
static bool import_format_supported(
    keymaster_key_format_t format,
    keymaster_algorithm_t algorithm)
{
    switch (format) {
        case KM_KEY_FORMAT_PKCS8:
            return ((algorithm == KM_ALGORITHM_RSA) ||
                    (algorithm == KM_ALGORITHM_EC));
        case KM_KEY_FORMAT_RAW:
            return ((algorithm == KM_ALGORITHM_AES) ||
                    (algorithm == KM_ALGORITHM_HMAC));
        default:
            return false;
    }
}

/**
 * Check if we can import a key of a given size
 * @param algorithm key type
 * @param keysize key size in bits
 * @return whether key size is supported
 */
static bool key_size_supported(
    keymaster_algorithm_t algorithm,
    uint32_t keysize)
{
    switch (algorithm) {
        case KM_ALGORITHM_RSA:
            return ((keysize >= 256) &&
                    (keysize <= RSA_MAX_KEY_SIZE) &&
                    (keysize % 64 == 0));
        case KM_ALGORITHM_EC:
            return ((keysize == 192) ||
                    (keysize == 224) ||
                    (keysize == 256) ||
                    (keysize == 384) ||
                    (keysize == EC_MAX_KEY_SIZE));
        case KM_ALGORITHM_AES:
            return ((keysize == 128) ||
                    (keysize == 192) ||
                    (keysize == AES_MAX_KEY_SIZE));
        case KM_ALGORITHM_HMAC:
            return ((keysize >= 64) &&
                    (keysize <= HMAC_MAX_KEY_SIZE) &&
                    (keysize % 8 == 0));
        default:
            return false;
    }
}

/**
 * Allocate and populate a buffer with km_key_data for passing to the TA.
 *
 * @param format format of \p key_encoding
 * @param algorithm key type
 * @param[in,out] keySizeInBits key size if known, otherwise 0 on entry, actual on exit
 * @param[in,out] rsa_pubexp RSA public exponent if knowm, otherwise 0 on entry, actual on exit
 * @param key_encoding data for decoding/encoding
 * @param[out] key_data pointer to allocated buffer, or NULL on error
 * @param[out] key_data_len length of allocated buffer
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t decode_key(
    keymaster_key_format_t format,
    keymaster_algorithm_t algorithm,
    uint32_t *keySizeInBits,
    uint64_t *rsa_pubexp,
    const keymaster_blob_t *key_encoding,
    uint8_t **key_data,
    uint32_t *key_data_len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    uint8_t *pos;
    uint32_t core_key_data_len;

    assert(keySizeInBits != NULL);
    assert((rsa_pubexp != NULL) || (algorithm != KM_ALGORITHM_RSA));
    assert(key_data != NULL);
    assert(key_data_len != NULL);

    *key_data_len = km_key_data_max_size(algorithm, *keySizeInBits); // upper bound
    CHECK_RESULT_OK(km_alloc(key_data, *key_data_len));

    /* First the key type and size */
    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
        *key_data_len >= 8);
    core_key_data_len = *key_data_len - 8;

    pos = *key_data;
    set_u32_increment_pos(&pos, algorithm);
    pos += 4; // leave space at *key_data + 4 to put the key size when we know it

    // now pos points to buffer for core key data
    switch (format) {
        case KM_KEY_FORMAT_PKCS8:
            CHECK_RESULT_OK(decode_pkcs8(
                pos, core_key_data_len, keySizeInBits, rsa_pubexp, key_encoding));
            break;
        case KM_KEY_FORMAT_RAW:
            /* Just copy the bytes. */
            CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT,
                core_key_data_len >= key_encoding->data_length);
            memcpy(pos, key_encoding->data, key_encoding->data_length);
            if (*keySizeInBits > 0) {
                CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT,
                    key_encoding->data_length == BITS_TO_BYTES(*keySizeInBits));
            } else {
                *keySizeInBits = 8 * key_encoding->data_length;
            }
            break;
        default:
            ret = KM_ERROR_UNSUPPORTED_KEY_FORMAT;
            goto end;
    }

    CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT,
        key_size_supported(algorithm, *keySizeInBits));

    // Now we can compute the real key_data_len
    *key_data_len = km_key_data_max_size(algorithm, *keySizeInBits);
    *key_data = (uint8_t*)realloc(*key_data, *key_data_len);
    CHECK_NOT_NULL(*key_data);

    /* Now set the actual key size. */
    set_u32(*key_data + 4, *keySizeInBits);

end:
    if (ret != KM_ERROR_OK) {
        free(*key_data);
        *key_data = NULL;
        *key_data_len = 0;
    }

    return ret;
}

/**
 * Allocate and populate a buffer with encoded key data.
 *
 * @param format desired export format
 * @param key_type key type
 * @param key_size key size in bits
 * @param core_pub_data public key data
 * @param core_pub_data_len length of \p core_pub_data
 * @param[out] export_data encoded key data
 *
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t encode_key(
    keymaster_key_format_t format,
    keymaster_algorithm_t key_type,
    uint32_t key_size,
    const uint8_t *core_pub_data,
    uint32_t core_pub_data_len,
    keymaster_blob_t *export_data)
{
    keymaster_error_t ret = KM_ERROR_OK;

    assert(export_data != NULL);

    export_data->data = NULL;
    export_data->data_length = 0;

    CHECK_TRUE(KM_ERROR_UNSUPPORTED_KEY_FORMAT,
        format == KM_KEY_FORMAT_X509);

    CHECK_RESULT_OK(encode_x509(export_data, key_type, key_size,
        core_pub_data, core_pub_data_len));
    // no need to free export_data on error

end:
    return ret;
}

keymaster_error_t TEE_ImportKey(
    TEE_SessionHandle                   sessionHandle,
    const keymaster_key_param_set_t*    params,
    keymaster_key_format_t              key_format,
    const keymaster_blob_t*             key_data, // encoded
    keymaster_key_blob_t*               key_blob,
    keymaster_key_characteristics_t*    characteristics)
{
    LOG_D("TEE_ImportKey");
    PRINT_PARAM_SET(params);
    LOG_D("key_format = 0x%08x", key_format);
    PRINT_BLOB(key_data);

    keymaster_error_t ret = KM_ERROR_OK;
    scoped_buf_ptr_t serializedData;
    mcBulkMap_t paramsInfo = {0, 0};
    mcBulkMap_t keyDataInfo = {0, 0};
    mcBulkMap_t keyBlobInfo = {0, 0};
    mcBulkMap_t characteristicsInfo = {0, 0};
    uint32_t keySizeInBits = 0;
    uint64_t rsa_pubexp = 0;
    keymaster_algorithm_t algorithm;
    keymaster_error_t ret1;
    uint8_t *key_chars = NULL;
    uint8_t *km_key_data = NULL; // to hold km_key_data passed to TA
    uint32_t km_key_data_len = 0;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcSessionHandle_t *session_handle = &session->sessionHandle;

    CHECK_NOT_NULL(tci);
    CHECK_NOT_NULL(key_blob);

    key_blob->key_material = NULL;

    /* Check parameters are valid for import_key */
    CHECK_RESULT_OK(check_params(params, key_import_allowed_params,
        sizeof(key_import_allowed_params) / sizeof(keymaster_tag_t)) );

    /* Extract algorithm and (if present) key size and RSA public exponent from
     * the key parameters */
    CHECK_RESULT_OK( get_enumerated_tag(params, KM_TAG_ALGORITHM,
        reinterpret_cast<uint32_t*>(&algorithm)) );
    ret1 = get_integer_tag(params, KM_TAG_KEY_SIZE, &keySizeInBits);
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        (ret1 == KM_ERROR_OK) || (ret1 == KM_ERROR_INVALID_TAG));
    ret1 = get_long_integer_tag(params,
        KM_TAG_RSA_PUBLIC_EXPONENT, &rsa_pubexp);
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        (ret1 == KM_ERROR_OK) || (ret1 == KM_ERROR_INVALID_TAG));
    /* If we got KM_ERROR_INVALID_TAG, the tag was not present; then
     * the value will still be 0 at this point. */

    /* Check consistency of format and algorithm */
    CHECK_TRUE(KM_ERROR_INCOMPATIBLE_KEY_FORMAT,
        import_format_supported(key_format, algorithm));

    /* Allocate and fill buffer for decoded key data for passing to TA */
    CHECK_RESULT_OK(decode_key(key_format, algorithm,
        &keySizeInBits, &rsa_pubexp, key_data, &km_key_data, &km_key_data_len));

    /* Serialize key parameters */
    CHECK_RESULT_OK(km_serialize_params(
        serializedData, params, true, keySizeInBits, rsa_pubexp));

    /* Map key parameters */
    CHECK_RESULT_OK( map_buffer(session_handle,
        serializedData.buf.get(), serializedData.size, &paramsInfo) );

    /* Allocate memory for key blob */
    key_blob->key_material_size = key_blob_max_size(
        algorithm, keySizeInBits, serializedData.size);
    CHECK_RESULT_OK(km_alloc((uint8_t**)&key_blob->key_material, key_blob->key_material_size));

    /* Map key data */
    CHECK_RESULT_OK( map_buffer(session_handle,
        km_key_data, km_key_data_len, &keyDataInfo) );

    /* Map key blob buffer */
    CHECK_RESULT_OK( map_buffer(session_handle,
        key_blob->key_material, key_blob->key_material_size, &keyBlobInfo) );

    if (characteristics != NULL) {
        /* Allocate memory for the serialized key characteristics.*/
        CHECK_RESULT_OK(km_alloc(&key_chars, KM_CHARACTERISTICS_SIZE));
         /* Map buffer for key characteristics. */
        CHECK_RESULT_OK( map_buffer(session_handle,
            key_chars, KM_CHARACTERISTICS_SIZE, &characteristicsInfo) );
    }

    /* Update TCI buffer */
    tci->command.header.commandId = CMD_ID_TEE_IMPORT_KEY;
    tci->import_key.params.data = (uint32_t)paramsInfo.sVirtualAddr;
    tci->import_key.params.data_length = serializedData.size;
    tci->import_key.key_data.data = (uint32_t)keyDataInfo.sVirtualAddr;
    tci->import_key.key_data.data_length = km_key_data_len;
    tci->import_key.key_blob.data = (uint32_t)keyBlobInfo.sVirtualAddr;
    tci->import_key.key_blob.data_length = key_blob->key_material_size;
    tci->import_key.characteristics.data = (uint32_t)characteristicsInfo.sVirtualAddr;
    tci->import_key.characteristics.data_length =
        (characteristics != NULL) ? KM_CHARACTERISTICS_SIZE : 0;

    CHECK_RESULT_OK( transact(session_handle, tci) );

    /* Update key blob length */
    key_blob->key_material_size = tci->import_key.key_blob.data_length;

    if (characteristics != NULL) { // Give characteristics to caller.
        CHECK_RESULT_OK(km_deserialize_characteristics(
            characteristics, key_chars, KM_CHARACTERISTICS_SIZE));
    }

end:
    unmap_buffer(session_handle, serializedData.buf.get(), &paramsInfo);
    unmap_buffer(session_handle, km_key_data, &keyDataInfo);
    if (key_blob != NULL) {
        unmap_buffer(session_handle, (uint8_t*)key_blob->key_material, &keyBlobInfo);
    }
    if (key_chars != NULL) {
        unmap_buffer(session_handle, key_chars, &characteristicsInfo);
    }

    free(km_key_data);
    free(key_chars);

    if (ret != KM_ERROR_OK) {
        if (key_blob != NULL) {
            free((void*)key_blob->key_material);
            key_blob->key_material = NULL;
            key_blob->key_material_size = 0;
        }
        if (characteristics != NULL) {
            keymaster_free_characteristics(characteristics);
        }
    }

    LOG_D("TEE_ImportKey exiting with %d", ret);
    return ret;
}

keymaster_error_t TEE_ExportKey(
    TEE_SessionHandle                   sessionHandle,
    keymaster_key_format_t              export_format,
    const keymaster_key_blob_t*         key_to_export,
    const keymaster_blob_t*             client_id,
    const keymaster_blob_t*             app_data,
    keymaster_blob_t*                   export_data)
{
    LOG_D("TEE_ExportKey");
    LOG_D("export_format = 0x%08x", export_format);

    keymaster_error_t ret = KM_ERROR_OK;
    mcBulkMap_t keyBlobInfo = {0, 0};
    mcBulkMap_t clientIdInfo = {0, 0};
    mcBulkMap_t appDataInfo = {0, 0};
    mcBulkMap_t keyDataInfo = {0, 0};
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    tciMessage_ptr     tci = session->pTci;
    mcSessionHandle_t* session_handle = &session->sessionHandle;
    uint8_t *core_pub_data = NULL;
    uint32_t core_pub_data_len = 0;
    keymaster_algorithm_t key_type;
    uint32_t key_size;
    scoped_buf_ptr_t key_to_export1;
    scoped_buf_ptr_t client_id1;
    scoped_buf_ptr_t app_data1;

    CHECK_NOT_NULL(tci);
    CHECK_NOT_NULL(key_to_export);
    CHECK_NOT_NULL(export_data);

    export_data->data = NULL;

    /* Map input buffers */
    if ( key_to_export != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(key_to_export->key_material, key_to_export->key_material_size, key_to_export1) );
        CHECK_RESULT_OK( map_buffer(session_handle,
        key_to_export1.buf.get(), key_to_export->key_material_size, &keyBlobInfo) );
    }

    if ( client_id != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(client_id->data, client_id->data_length, client_id1));
        CHECK_RESULT_OK( map_buffer(session_handle,
            client_id1.buf.get(), client_id1.size, &clientIdInfo) );
    }
    if ( app_data != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(app_data->data, app_data->data_length, app_data1));
        CHECK_RESULT_OK( map_buffer(session_handle,
            app_data1.buf.get(), app_data1.size, &appDataInfo) );
    }

    /* First need to determine required length of key data. */
    tci->command.header.commandId = CMD_ID_TEE_GET_KEY_INFO;
    tci->get_key_info.key_blob.data = (uint32_t)keyBlobInfo.sVirtualAddr;
    tci->get_key_info.key_blob.data_length = key_to_export->key_material_size;
    tci->get_key_info.client_id.data = (uint32_t)clientIdInfo.sVirtualAddr;
    tci->get_key_info.client_id.data_length = clientIdInfo.sVirtualLen;
    tci->get_key_info.app_data.data = (uint32_t)appDataInfo.sVirtualAddr;
    tci->get_key_info.app_data.data_length = appDataInfo.sVirtualLen;
    CHECK_RESULT_OK( transact(session_handle, tci) );
    key_type = (keymaster_algorithm_t)tci->get_key_info.key_type;
    key_size = tci->get_key_info.key_size;

    core_pub_data_len = export_data_length(key_type, key_size);
    CHECK_TRUE(KM_ERROR_UNSUPPORTED_KEY_FORMAT,
        core_pub_data_len > 0);

    /* Allocate memory for exported public key data */
    CHECK_RESULT_OK(km_alloc(&core_pub_data, core_pub_data_len));

    /* Map buffer */
    CHECK_RESULT_OK( map_buffer(session_handle, core_pub_data, core_pub_data_len, &keyDataInfo) );

    /* Now the export_key command */
    tci->command.header.commandId = CMD_ID_TEE_EXPORT_KEY;

    tci->export_key.key_blob.data = (uint32_t)keyBlobInfo.sVirtualAddr;
    tci->export_key.key_blob.data_length = key_to_export->key_material_size;
    tci->export_key.client_id.data = (uint32_t)clientIdInfo.sVirtualAddr;
    tci->export_key.client_id.data_length = clientIdInfo.sVirtualLen;
    tci->export_key.app_data.data = (uint32_t)appDataInfo.sVirtualAddr;
    tci->export_key.app_data.data_length = appDataInfo.sVirtualLen;
    tci->export_key.key_data.data = (uint32_t)keyDataInfo.sVirtualAddr;
    tci->export_key.key_data.data_length = core_pub_data_len;
    CHECK_RESULT_OK( transact(session_handle, tci) );

    /* Allocate and fill buffer for encoded key data for passing to caller */
    CHECK_RESULT_OK(encode_key(export_format, key_type, key_size,
        core_pub_data, core_pub_data_len, export_data));

end:
    if (key_to_export1.buf)
    {
        unmap_buffer(session_handle, key_to_export1.buf.get(), &keyBlobInfo);
    }
    if (client_id1.buf) {
        unmap_buffer(session_handle, client_id1.buf.get(), &clientIdInfo);
    }
    if (app_data1.buf) {
        unmap_buffer(session_handle, app_data1.buf.get(), &appDataInfo);
    }
    unmap_buffer(session_handle, (uint8_t*)core_pub_data, &keyDataInfo);

    if (ret != KM_ERROR_OK) {
        if (export_data != NULL) {
            free((void*)export_data->data);
            export_data->data = NULL;
            export_data->data_length = 0;
        }
    }

    free(core_pub_data);

    LOG_D("TEE_ExportKey exiting with %d", ret);
    return ret;
}

keymaster_error_t TEE_AttestKey(
    TEE_SessionHandle sessionHandle,
    const keymaster_key_blob_t* key_to_attest,
    const keymaster_key_param_set_t* attest_params,
    keymaster_cert_chain_t* cert_chain)
{
    LOG_D("TEE_AttestKey");
    PRINT_PARAM_SET(attest_params);

    keymaster_error_t ret = KM_ERROR_OK;

    struct TEE_Session *session = (struct TEE_Session*)sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcSessionHandle_t* session_handle = &session->sessionHandle;
    mcBulkMap_t keyBlobInfo = {0, 0};
    mcBulkMap_t paramsInfo = {0, 0};
    mcBulkMap_t certsInfo = {0, 0};
    scoped_buf_ptr_t serializedParams;
    scoped_buf_ptr_t key_to_attest1;
    uint8_t *certs = NULL;
#if KEY_ATTESTATION_DEBUG
#if 0 /* need to add selinux policy before I can do this */
	size_t dbg_file_len = 0;
#endif
#endif
#if !RPMB_SUPPORT
    static bool attestation_data_load = false;
    mcBulkMap_t attestKeyInfo = {0, 0};
    uint8_t *so_p = NULL;
    uint32_t so_len = 0;

    if (attestation_data_load)
        goto attest;

    so_len = getFileContent(ATTEST_KEYBOX_FILE, &so_p);
    if (so_len == 0) {
        LOG_E("Open attest keybox failed!\n");
        goto attest;
    } else
        LOG_D("%s:%d: getFileContent = %u\n", __func__, __LINE__, so_len);

    ret = map_buffer(session_handle, so_p, so_len, &attestKeyInfo);
    if (ret != KM_ERROR_OK) {
        LOG_E("Map buffer failed: %d!\n", ret);
        free(so_p);
        goto attest;
    }

    tci->command.header.commandId = CMD_ID_TEE_LOAD_ATTESTATION_DATA;
    tci->load_attestation_data.data.data = (uint32_t)attestKeyInfo.sVirtualAddr;
    tci->load_attestation_data.data.data_length = (uint32_t)attestKeyInfo.sVirtualLen;

    ret = transact(session_handle, tci);
    if (ret == KM_ERROR_OK)
        attestation_data_load = true;
    else
        LOG_E("Load attest keybox failed: %d!\n", ret);

    unmap_buffer(session_handle, so_p, &attestKeyInfo);
    free(so_p);
attest:
#endif

    CHECK_NOT_NULL(key_to_attest);
    CHECK_NOT_NULL(cert_chain);

    /* Check parameters are valid for attest_key */
    CHECK_RESULT_OK(check_params(attest_params, attest_allowed_params,
        sizeof(attest_allowed_params) / sizeof(keymaster_tag_t)));

    /* Map input const buffer key_to_attest */
    if ( key_to_attest != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(key_to_attest->key_material, key_to_attest->key_material_size, key_to_attest1) );
        CHECK_RESULT_OK( map_buffer(session_handle,
        key_to_attest1.buf.get(), key_to_attest->key_material_size, &keyBlobInfo) );
    }

    /* Serialize attestation parameters */
    CHECK_RESULT_OK(km_serialize_params(
        serializedParams, attest_params, false, 0, 0));

    /* Map attestation parameters */
    CHECK_RESULT_OK(map_buffer(session_handle,
        serializedParams.buf.get(), serializedParams.size, &paramsInfo));

    /* Allocate memory for the serialized cert chain */
    CHECK_RESULT_OK(km_alloc(&certs, KM_CERT_CHAIN_SIZE));

    /* Map buffer for serialized cert chain */
    CHECK_RESULT_OK(map_buffer(session_handle,
        certs, KM_CERT_CHAIN_SIZE, &certsInfo));

    /* Update TCI buffer */
    tci->command.header.commandId = CMD_ID_TEE_ATTEST_KEY;
    tci->attest_key.key_to_attest.data = (uint32_t)keyBlobInfo.sVirtualAddr;
    tci->attest_key.key_to_attest.data_length = keyBlobInfo.sVirtualLen;
    tci->attest_key.attest_params.data = (uint32_t)paramsInfo.sVirtualAddr;
    tci->attest_key.attest_params.data_length = paramsInfo.sVirtualLen;
    tci->attest_key.cert_chain.data = (uint32_t)certsInfo.sVirtualAddr;
    tci->attest_key.cert_chain.data_length = certsInfo.sVirtualLen;

    /* Talk to TA */
    CHECK_RESULT_OK(transact(session_handle, tci));

    /* Deserialize certs -> cert_chain, allocating memory for the latter. */
    CHECK_RESULT_OK(km_deserialize_attestation(cert_chain, certs, KM_CERT_CHAIN_SIZE));

#if KEY_ATTESTATION_DEBUG
	#if 0 /* need to add selinux policy before I can do this */
	dbg_file_len = writeFile(KEY_TO_ATTEST_FILE, key_to_attest->key_material, key_to_attest->key_material_size);
	if (dbg_file_len == 0)
        LOG_E("write file failed! (%s)\n", KEY_TO_ATTEST_FILE);
	else
		LOG_E("write file successfully! (%s)\n", KEY_TO_ATTEST_FILE);
	#endif
	{
		size_t i;

		LOG_D("[KEY_ATTESTATION_DEBUG] %s:%d: KM3 cert count = %zu\n", __func__, __LINE__, cert_chain->entry_count);

		for (i = 0; i < cert_chain->entry_count; i++) {
			size_t data_len = cert_chain->entries[i].data_length;

			LOG_D("[KEY_ATTESTATION_DEBUG] %s:%d: i = %zu, data_len = %zu\n", __func__, __LINE__, i, cert_chain->entries[i].data_length);
			LOG_D("[KEY_ATTESTATION_DEBUG] %s:%d: i = %zu, data = 0x%x, 0x%x, 0x%x, 0x%x\n", __func__, __LINE__, i, cert_chain->entries[i].data[0], cert_chain->entries[i].data[1], cert_chain->entries[i].data[2], cert_chain->entries[i].data[3]);
			if (data_len >= 4)
				LOG_D("[KEY_ATTESTATION_DEBUG] %s:%d: i = %zu, data(END) = 0x%x, 0x%x, 0x%x, 0x%x\n", __func__, __LINE__, i, cert_chain->entries[i].data[data_len - 4], cert_chain->entries[i].data[data_len - 3], cert_chain->entries[i].data[data_len - 2], cert_chain->entries[i].data[data_len - 1]);
			/* it looks stupid, but my intention is to make it less error-prone so
			  * time spent in debugging could be reduced.
			  */
			#if 0 /* need to add selinux policy before I can do this */
			if (i == 0) {
				dbg_file_len = writeFile(ATTESTATION_CERT0_FILE, cert_chain->entries[i].data, cert_chain->entries[i].data_length);
				if (dbg_file_len == 0)
					LOG_E("write file failed! (%s)\n", ATTESTATION_CERT0_FILE);
				else
					LOG_D("write file successfully! (%s)\n", ATTESTATION_CERT0_FILE);
			}
			if (i == 1) {
				dbg_file_len = writeFile(ATTESTATION_CERT1_FILE, cert_chain->entries[i].data, cert_chain->entries[i].data_length);
				if (dbg_file_len == 0)
					LOG_E("write file failed! (%s)\n", ATTESTATION_CERT1_FILE);
				else
					LOG_D("write file successfully! (%s)\n", ATTESTATION_CERT1_FILE);
			}
			if (i == 2) {
				dbg_file_len = writeFile(ATTESTATION_CERT2_FILE, cert_chain->entries[i].data, cert_chain->entries[i].data_length);
				if (dbg_file_len == 0)
					LOG_E("write file failed! (%s)\n", ATTESTATION_CERT2_FILE);
				else
					LOG_D("write file successfully! (%s)\n", ATTESTATION_CERT2_FILE);
			}
			if (i == 3) {
				dbg_file_len = writeFile(ATTESTATION_CERT3_FILE, cert_chain->entries[i].data, cert_chain->entries[i].data_length);
				if (dbg_file_len == 0)
					LOG_E("write file failed! (%s)\n", ATTESTATION_CERT3_FILE);
				else
					LOG_D("write file successfully! (%s)\n", ATTESTATION_CERT3_FILE);
			}
			#endif
		}
	}
#endif

end:
    if (key_to_attest1.buf) {
        unmap_buffer(session_handle,
            key_to_attest1.buf.get(), &keyBlobInfo);
    }
    if (serializedParams.buf) {
        unmap_buffer(session_handle, serializedParams.buf.get(), &paramsInfo);
    }
    if (certs != NULL) {
        unmap_buffer(session_handle, certs, &certsInfo);
    }
    free(certs);

    if (ret != KM_ERROR_OK) {
        keymaster_free_cert_chain(cert_chain);
    }

    LOG_D("TEE_AttestKey exiting with %d", ret);
    return ret;
}

#define UPGRADE_OVERHEAD (2*(4 + 4))
/* Space for the additional parameters which `upgrade_key' might insert into the
 * blob.  Two parameters, each with 32-bit tag and value.
 */

keymaster_error_t TEE_UpgradeKey(
    TEE_SessionHandle sessionHandle,
    const keymaster_key_blob_t* key_to_upgrade,
    const keymaster_key_param_set_t* upgrade_params,
    keymaster_key_blob_t* upgraded_key)
{
    keymaster_error_t ret = KM_ERROR_OK;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    mcSessionHandle_t* session_handle = &session->sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcBulkMap_t keyin_map = { 0, 0 }, keyout_map = { 0, 0 };
    mcBulkMap_t param_map = { 0, 0 };
    scoped_buf_ptr_t serialized_params;
    scoped_buf_ptr_t key_to_upgrade1;
    uint8_t *kbuf = NULL;
    size_t kbufsz;

    /* Picky checking on input pointers. */
    CHECK_NOT_NULL(key_to_upgrade);
    CHECK_NOT_NULL(upgrade_params);
    CHECK_NOT_NULL(upgraded_key);

    /* Map input const buffer key_to_upgrade */
    if ( key_to_upgrade != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(key_to_upgrade->key_material, key_to_upgrade->key_material_size, key_to_upgrade1) );
        CHECK_RESULT_OK( map_buffer(session_handle,
        key_to_upgrade1.buf.get(), key_to_upgrade->key_material_size, &keyin_map) );
    }

    CHECK_RESULT_OK(km_serialize_params(serialized_params,
        upgrade_params, false, 0, 0));
    CHECK_RESULT_OK(map_buffer(session_handle,
        serialized_params.buf.get(), serialized_params.size, &param_map));

    /* Create and map output buffer -- which means we have to guess how big to
     * make it.  If this is a really old key, it won't have the version
     * parameters at all, so it'll come out UPGRADE_OVERHEAD bytes bigger than
     * the one we fed in.  Otherwise, it should be the same length.  Of course,
     * we can't tell which from here...
     */
    kbufsz = key_to_upgrade->key_material_size + UPGRADE_OVERHEAD;
    CHECK_RESULT_OK(km_alloc(&kbuf, kbufsz));
    CHECK_RESULT_OK(map_buffer(session_handle, kbuf, kbufsz, &keyout_map));

    /* Build the command. */
    tci->command.header.commandId = CMD_ID_TEE_UPGRADE_KEY;
    tci->upgrade_key.key_to_upgrade.data = (uint32_t)keyin_map.sVirtualAddr;
    tci->upgrade_key.key_to_upgrade.data_length =
        (uint32_t)keyin_map.sVirtualLen;
    tci->upgrade_key.upgrade_params.data = (uint32_t)param_map.sVirtualAddr;
    tci->upgrade_key.upgrade_params.data_length =
        (uint32_t)param_map.sVirtualLen;
    tci->upgrade_key.upgraded_key.data = (uint32_t)keyout_map.sVirtualAddr;
    tci->upgrade_key.upgraded_key.data_length =
        (uint32_t)keyout_map.sVirtualLen;

    /* See if it worked. */
    CHECK_RESULT_OK(transact(session_handle, tci));

    /* Return the answer. */
    upgraded_key->key_material = kbuf;
    upgraded_key->key_material_size = tci->upgrade_key.upgraded_key.data_length;

end:
    if (key_to_upgrade1.buf) {
        unmap_buffer(session_handle, key_to_upgrade1.buf.get(), &keyin_map);
    }

    unmap_buffer(session_handle, serialized_params.buf.get(), &param_map);
    unmap_buffer(session_handle, kbuf, &keyout_map);
    if (ret != KM_ERROR_OK)
        free(kbuf);

    LOG_D("TEE_UpgradeKey exiting with %d", ret);
    return ret;
}

keymaster_error_t TEE_DeleteKey(
    TEE_SessionHandle session_handle,
    const keymaster_key_blob_t* key)
{
    (void) session_handle;
    (void) key;

    LOG_D("TEE_DeleteKey exiting with %d", KM_ERROR_UNIMPLEMENTED);
    return KM_ERROR_UNIMPLEMENTED;
}

keymaster_error_t TEE_DeleteAllKeys(
    TEE_SessionHandle session_handle)
{
    (void) session_handle;

    LOG_D("TEE_DeleteAllKeys exiting with %d", KM_ERROR_UNIMPLEMENTED);
    return KM_ERROR_UNIMPLEMENTED;
}

keymaster_error_t TEE_Begin(
    TEE_SessionHandle               sessionHandle,
    keymaster_purpose_t             purpose,
    const keymaster_key_blob_t*     key,
    const keymaster_key_param_set_t* params,
    keymaster_key_param_set_t*      out_params,
    keymaster_operation_handle_t*   operation_handle)
{
    LOG_D("TEE_Begin");
    LOG_D("purpose = 0x%08x", purpose);
    PRINT_PARAM_SET(params);

    keymaster_error_t  ret = KM_ERROR_OK;
    mcBulkMap_t        paramsInfo = {0, 0};
    mcBulkMap_t        keyBlobInfo = {0, 0};
    mcBulkMap_t        outParamsInfo = {0, 0};
    scoped_buf_ptr_t   serializedData;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    tciMessage_ptr     tci = session->pTci;
    mcSessionHandle_t* session_handle = &session->sessionHandle;
    uint8_t            serialized_out_params[TEE_BEGIN_OUT_PARAMS_SIZE];
    struct operation   *op = NULL;
    /* create key_blob buffer to copy input key content */
    scoped_buf_ptr_t   key_blob;

    /* Make valgrind happy. */
    memset(serialized_out_params, 0, TEE_BEGIN_OUT_PARAMS_SIZE);

    if (out_params != NULL) {
        out_params->params = NULL;
        out_params->length = 0;
    }

    CHECK_NOT_NULL(tci);
    CHECK_NOT_NULL(key);
    CHECK_NOT_NULL(operation_handle);

    /* Check parameters are valid for begin */
    CHECK_RESULT_OK(check_params(params, op_begin_allowed_params,
        sizeof(op_begin_allowed_params) / sizeof(keymaster_tag_t)) );

    /* Serialize params */
    CHECK_RESULT_OK(km_serialize_params(
        serializedData, params, false, 0, 0));

    /* Map params */
    CHECK_RESULT_OK( map_buffer(session_handle,
        serializedData.buf.get(), serializedData.size, &paramsInfo) );

    if ( key->key_material != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK( copy_to_scoped_buf(key->key_material, key->key_material_size, key_blob) );
        CHECK_RESULT_OK( map_buffer(session_handle,
            key_blob.buf.get(), key_blob.size, &keyBlobInfo) );
    }

    /* Map serialized_out_params */
    CHECK_RESULT_OK( map_buffer(session_handle,
        serialized_out_params, TEE_BEGIN_OUT_PARAMS_SIZE, &outParamsInfo) );

    /* Update TCI buffer */
    tci->command.header.commandId = CMD_ID_TEE_BEGIN;
    tci->begin.purpose = purpose;
    tci->begin.params.data = (uint32_t)paramsInfo.sVirtualAddr;
    tci->begin.params.data_length = serializedData.size;
    tci->begin.key_blob.data = (uint32_t)keyBlobInfo.sVirtualAddr;
    tci->begin.key_blob.data_length = key->key_material_size;
    if (out_params != NULL) {
        tci->begin.out_params.data = (uint32_t)outParamsInfo.sVirtualAddr;
        tci->begin.out_params.data_length = TEE_BEGIN_OUT_PARAMS_SIZE;
    } else {
        tci->begin.out_params.data = 0;
        tci->begin.out_params.data_length = 0;
    }

    CHECK_RESULT_OK( transact(session_handle, tci) );

    /* Deserialize out_params */
    if (out_params != NULL) {
        uint8_t *pos = serialized_out_params;
        uint32_t remain = tci->begin.out_params.data_length;
        if (remain > 0) {
            CHECK_RESULT_OK(deserialize_param_set(out_params, &pos, &remain));
        } else {
            out_params->params = NULL;
            out_params->length = 0;
        }
    }

    /* Update operation handle */
    op = find_spare_operation_slot(session, tci->begin.handle);
    op->algorithm = static_cast<keymaster_algorithm_t>(tci->begin.algorithm);
    op->final_length = tci->begin.final_length;
    *operation_handle = op->handle;

end:
    if (ret != KM_ERROR_OK) {
        if (out_params != NULL) {
            keymaster_free_param_set(out_params);
            out_params->length = 0;
        }
    }
    unmap_buffer(session_handle, serializedData.buf.get(), &paramsInfo);

    /* unmap key_blob */
    if (key_blob.buf) {
        unmap_buffer(session_handle, key_blob.buf.get(), &keyBlobInfo);
    }

    unmap_buffer(session_handle, serialized_out_params, &outParamsInfo);

    LOG_D("TEE_Begin exiting with %d", ret);
    return ret;
}

/**
 * Maximum size of buffer to process internally in an update() operation.
 *
 * Longer messages are split up into chunks this size.
 */
#define INPUT_CHUNK_SIZE 4096*4

/**
 * Process a chunk of input to an operation.
 *
 * @param session session pointer
 * @param operation_handle operation handle
 * @param paramsInfo serialized parameters, mapped
 * @param data input data (not NULL)
 * @param data_length length of \p data
 * @param[out] output output if required (pre-allocated), or NULL
 * @param[in,out] input_consumed input consumed (incremented)
 *
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t update_chunk(
    struct TEE_Session *session, const struct operation *op,
    const mcBulkMap_t *param_map,
    const mcBulkMap_t *input_map, size_t *input_consumed_r,
    const mcBulkMap_t *output_map, size_t *output_used_r,
    void *)
{
    tciMessage_ptr tci = session->pTci;
    mcSessionHandle_t *session_handle = &session->sessionHandle;
    keymaster_error_t ret = KM_ERROR_OK;

    tci->command.header.commandId = CMD_ID_TEE_UPDATE;
    tci->update.handle = op->handle;
    tci->update.params.data = (uint32_t)param_map->sVirtualAddr;
    tci->update.params.data_length = param_map->sVirtualLen;
    tci->update.input.data = (uint32_t)input_map->sVirtualAddr;
    tci->update.input.data_length = input_map->sVirtualLen;
    tci->update.output.data = (uint32_t)output_map->sVirtualAddr;
    tci->update.output.data_length = output_map->sVirtualLen;

    CHECK_RESULT_OK( transact(session_handle, tci) );

    *input_consumed_r = tci->update.input_consumed;
    *output_used_r = tci->update.output.data_length;
end:
    return ret;
}

struct finish_chunk_ctx {
    mcBulkMap_t signature_map;
};

/**
 * Process a chunk of input to an operation.
 *
 * @param session session pointer
 * @param operation_handle operation handle
 * @param paramsInfo serialized parameters, mapped
 * @param data input data (not NULL)
 * @param data_length length of \p data
 * @param[out] output output if required (pre-allocated), or NULL
 * @param[in,out] input_consumed input consumed (incremented)
 * @param ctx pointer to @c finish_chunk_ctx structure
 *
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t final_chunk(
    struct TEE_Session *session, const struct operation *op,
    const mcBulkMap_t *param_map,
    const mcBulkMap_t *input_map, size_t *input_consumed_r,
    const mcBulkMap_t *output_map, size_t *output_used_r,
    void *ctx)
{
    tciMessage_ptr tci = session->pTci;
    mcSessionHandle_t *session_handle = &session->sessionHandle;
    struct finish_chunk_ctx *fin =
        static_cast<struct finish_chunk_ctx *>(ctx);
    keymaster_error_t ret = KM_ERROR_OK;

    tci->command.header.commandId = CMD_ID_TEE_FINISH;
    tci->finish.handle = op->handle;
    tci->finish.params.data = (uint32_t)param_map->sVirtualAddr;
    tci->finish.params.data_length = param_map->sVirtualLen;
    tci->finish.input.data = (uint32_t)input_map->sVirtualAddr;
    tci->finish.input.data_length = input_map->sVirtualLen;
    tci->finish.signature.data = (uint32_t)fin->signature_map.sVirtualAddr;
    tci->finish.signature.data_length = fin->signature_map.sVirtualLen;
    tci->finish.output.data = (uint32_t)output_map->sVirtualAddr;
    tci->finish.output.data_length = output_map->sVirtualLen;

    CHECK_RESULT_OK( transact(session_handle, tci) );

    *input_consumed_r = input_map->sVirtualLen;
    *output_used_r = tci->finish.output.data_length;
end:
    return ret;
}

typedef keymaster_error_t chunk_submit_func(
    struct TEE_Session *session, const struct operation *op,
    const mcBulkMap_t *param_map,
    const mcBulkMap_t *input_map, size_t *input_consumed_r,
    const mcBulkMap_t *output_map, size_t *output_used_r,
    void *ctx);

/**
 * @param session pointer to session structure
 * @param op pointer to operation in progress
 * @param at_least_one always submit at least one chunk, even if it's empty
 * @param params parameters to pass on to submission function
 * @param input pointer to input buffer/length
 * @param input_consumed_r where to put the amount of input actually consumed
 * @param output pointer to the output buffer
 * @param output_limit one-past-the-end of the available output buffer
 * @param output_used_r where to put the amount of output actually written
 * @param submit function to call to submit most chunks
 * @param submit_list function to call to submit the final chunk
 * @param ctx uninterpreted context pointer for the @p submit functions
 *
 * @return @c KM_ERROR_OK or error
 *
 * General-purpose data chunking function.
 *
 * There's a (not very generous) limit to the amount of data we can squeeze
 * through the interface to our TA, and our caller isn't aware of it.  We can
 * sort out this mess by splitting the input into smaller pieces and submitting
 * them one at a time.  This function does that.
 *
 * The @p params contain parameter tags and values to be passed through.  These
 * are important because they can carry `additional authenticated data' for AEAD
 * schemes, which can also be arbitrarily large, and require their own special
 * handling.  In particular, there's an ordering rule: no more AAD can be
 * submitted once any of the main payload has been submitted.  This function
 * handles all of that.
 *
 * It submits individual chunks other than the last to its @p submit function
 * for actual submission to the TA; the last is instead passed to
 * @p submit_last.
 *
 * Usually, if there is no @p input data, and no AAD in the @p params, then this
 * function doesn't do anything, and in particular doesn't submit any chunks.
 * If @p at_least_one is true, then at least one chunk is submitted, even if
 * it's entirely empty; obviously, in this case, it will be sent to the
 * @p submit_last function.
 *
 * This function takes responsibility for setting up the necessary shared-memory
 * mappings.
 */
static keymaster_error_t split_update_chunks(
    struct TEE_Session *session, const struct operation *op,
    bool at_least_one, const keymaster_key_param_set_t *params,
    const keymaster_blob_t *input, size_t *input_consumed_r,
    uint8_t *output, const uint8_t *output_limit, size_t *output_used_r,
    chunk_submit_func *submit, chunk_submit_func *submit_last,
    void *ctx)
{
    keymaster_error_t ret = KM_ERROR_OK;
    mcSessionHandle_t *session_handle = &session->sessionHandle;
    const uint8_t *inp, *inp_limit;
    uint8_t *output_window = NULL;
    keymaster_key_param_t *param_buf = NULL, *aad_param;
    size_t nparams = params ? params->length : 0;
    keymaster_key_param_set_t my_params;
    mcBulkMap_t param_map = { 0, 0 };
    mcBulkMap_t input_map = { 0, 0 };
    mcBulkMap_t output_map = { 0, 0 };
    scoped_buf_ptr_t serialized_params;
    scoped_buf_ptr_t aad_window;
    scoped_buf_ptr_t input_window;
    bool must_reserialize_params = params;

    LOG_D("split_update_chunks");

    /* Get the input buffer, if there is one.  (There might not be, say
     * because we're just pushing additional data at an AAD scheme.)
     */
    if (!input || !input->data_length)
        inp = inp_limit = NULL;
    else {
        inp = input->data;
        inp_limit = inp + input->data_length;
    }

    /* Sort out the paramters.  This only gets complicated if we're doing
     * AAD, i.e., if `KM_TAG_ASSOCIATED_DATA' is present.
     */
    const uint8_t *aad, *aad_limit;
    aad_param = NULL;
    for (size_t i = 0; i < nparams; i++) {
        if (params->params[i].tag == KM_TAG_ASSOCIATED_DATA) {
            aad_param = &params->params[i];
            break;
        }
    }
    if (!aad_param) {
        aad = aad_limit = NULL;
        if (params)
            my_params = *params;
        else {
            my_params.params = NULL;
            my_params.length = 0;
        }
    } else {

        /* Yes, we have AAD.  Make a copy of the parameter vector, but move
         * the AAD parameter to the end so that we can strip it out easily
         * later.
         */
        param_buf =
            (keymaster_key_param_t *)malloc(nparams*sizeof(*param_buf));
        if (!param_buf) {
            ret = KM_ERROR_MEMORY_ALLOCATION_FAILED;
            goto end;
        }
        size_t aad_off = aad_param - params->params;
        memcpy(param_buf, params->params, aad_off*sizeof(*param_buf));
        memcpy(param_buf + aad_off, params->params + aad_off + 1,
            (nparams - aad_off - 1)*sizeof(*param_buf));
        param_buf[nparams - 1] = *aad_param;
        aad_param = &param_buf[nparams - 1];

        /* Now dig the data buffer pointers out. */
        if (!aad_param->blob.data_length)
            aad = aad_limit = NULL;
        else {
            aad = aad_param->blob.data;
            aad_limit = aad + aad_param->blob.data_length;
        }
        my_params.params = param_buf;
        my_params.length = nparams;
    }

    /* Now we get to do the main work. */
    while (at_least_one || aad || inp) {
        bool maybe_last_time = true;
        size_t budget = INPUT_CHUNK_SIZE;

        at_least_one = false;

        /* First, handle the parameters.  This is actually the fiddliest bit.
         * If there's AAD still to come, or we haven't done this part, then
         * we serialize the parameters.  This ensures that we submit the AAD
         * before the main input data.
         *
         * Optimization for later: serialize the parameters once, but keep
         * track of where the AAD buffer is, and just hack that part on each
         * iteration.
         */
        if (must_reserialize_params) {
            PRINT_PARAM_SET(&my_params);
            unmap_buffer(session_handle,
                serialized_params.buf.get(), &param_map);
            if (!aad_param) {
                must_reserialize_params = false;
                aad_window.buf.reset();
            } else {
                size_t n = aad_limit - aad;
                if (n > budget) {
                    n = budget;
                    maybe_last_time = false;
                }
                CHECK_RESULT_OK(copy_to_scoped_buf(aad, n, aad_window));
                aad_param->blob.data = aad_window.buf.get();
                aad_param->blob.data_length = n;
                budget -= n;
                aad += n;
            }
            CHECK_RESULT_OK(km_serialize_params(serialized_params,
                &my_params, false, 0, 0));
            CHECK_RESULT_OK(map_buffer(session_handle,
                serialized_params.buf.get(), serialized_params.size,
                &param_map));
            if (aad_param && aad >= aad_limit) {
                aad = aad_limit = NULL;
                aad_param = NULL;
                my_params.length--;
                /* We shall reserialize without the parameters next time
                 * through (if there is one), for the last time.
                 */
            }
        }

        /* Now deal with the message input, assuming we have any budget
         * available for this.  We certainly don't want any old buffer from
         * last time hanging around.  If we're using up the last of our input
         * then switch in the other submit function.
         */
        unmap_buffer(session_handle, input_window.buf.get(), &input_map);
        input_window.buf.reset();
        if (inp && budget) {
            size_t n = inp_limit - inp;
            if (n > budget) {
                n = budget;
                maybe_last_time = false;
            }
            budget -= n;
            CHECK_RESULT_OK(copy_to_scoped_buf(inp, n, input_window));
            CHECK_RESULT_OK(map_buffer(session_handle,
                input_window.buf.get(), n, &input_map));
        }

        /* Finally, arrange some output.  There are two interesting cases.
         *
         * If we're in the middle of an operation, then the long pole is AES,
         * which might have a (nearly complete) block (16 bytes) up its
         * sleeve from before this operation; nothing else (currently!) can
         * produce output at this time.
         *
         * On the other hand, if we're at the end of our input buffer, we
         * should hand over the whole of the caller's output buffer because
         * it was presumably provided for some good reason.
         */
        unmap_buffer(session_handle, output_window, &output_map);
        if (output) {
            size_t n = output_limit - output;
            if (!maybe_last_time) {
                size_t avail = input_window.size + 16;
                if (n > avail) n = avail;
            }
            output_window = output;
            CHECK_RESULT_OK(map_buffer(session_handle,
                output_window, n, &output_map));
        }

        /* Push the next chunk through the machinery.  At this point,
         * `maybe_last_time' is definitely right.
         */
        size_t input_consumed = 0;
        size_t output_used = 0;
        CHECK_RESULT_OK((maybe_last_time ? submit_last : submit)(session, op,
            &param_map,
            &input_map, &input_consumed,
            &output_map, &output_used,
            ctx));

        /* Advance the input.  (There doesn't seem to be a way for us to
         * refuse to accept some of the AAD.)
         */
        if (input_consumed_r)
            *input_consumed_r += input_consumed;
        if (inp) {
            inp += input_consumed;
            if (inp >= inp_limit)
                inp = inp_limit = NULL;
        }

        /* Advance the output. */
        if (output_used_r)
            *output_used_r += output_used;
        if (output)
            output += output_used;
    }

end:
    unmap_buffer(session_handle, serialized_params.buf.get(), &param_map);
    unmap_buffer(session_handle, input_window.buf.get(), &input_map);
    unmap_buffer(session_handle, output_window, &output_map);
    free(param_buf);
    return ret;
}

keymaster_error_t TEE_Update(
    TEE_SessionHandle               sessionHandle,
    keymaster_operation_handle_t    operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t*         input,
    size_t*                         input_consumed,
    keymaster_key_param_set_t*      out_params,
    keymaster_blob_t*               output)
{
    LOG_D("TEE_Update");
    PRINT_PARAM_SET(params);
    PRINT_BLOB(input);

    keymaster_error_t ret = KM_ERROR_OK;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    size_t output_used = 0;
    struct operation *op = NULL;

    CHECK_RESULT_OK(lookup_operation(session, operation_handle, &op));

    /* No output parameters */
    if (out_params != NULL) {
        out_params->params = NULL;
        out_params->length = 0;
    }

    CHECK_NOT_NULL(input_consumed);
    *input_consumed = 0;

    /* Check parameters are valid for update */
    CHECK_RESULT_OK(check_params(params, op_allowed_params,
        sizeof(op_allowed_params) / sizeof(keymaster_tag_t)) );

    /* Allocate output if required. */
    if (output != NULL) {
        if (op->algorithm == KM_ALGORITHM_AES) {
            /* Allocate the output buffer. */
            output->data_length = input->data_length + 16; // up to one more block
            CHECK_RESULT_OK(km_alloc((uint8_t**)&output->data, output->data_length));
        } else {
            /* No output. */
            output->data = NULL;
            output->data_length = 0;
        }
    }

    CHECK_RESULT_OK(split_update_chunks(session, op, false,
        params, input, input_consumed,
        output ? const_cast</*unconst*/ uint8_t *>(output->data) : NULL,
        output ? output->data + output->data_length : NULL,
        &output_used, update_chunk, update_chunk, NULL));
    if (output)
        output->data_length = output_used;

end:
    if (ret != KM_ERROR_OK) {
        // free the output->data if it is allocated
        if (output != NULL) {
            free((void*)output->data);
            output->data = NULL;
            output->data_length = 0;
        }
        release_operation_slot(session, op);
    }

    LOG_D("TEE_Update exiting with %d", ret);
    return ret;
}

keymaster_error_t TEE_Finish(
    TEE_SessionHandle               sessionHandle,
    keymaster_operation_handle_t    operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t*         input,
    const keymaster_blob_t*         signature,
    keymaster_key_param_set_t*      out_params,
    keymaster_blob_t*               output)
{
    LOG_D("TEE_Finish");
    PRINT_PARAM_SET(params);
    PRINT_BLOB(signature);

    keymaster_error_t ret = KM_ERROR_OK;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    mcSessionHandle_t *session_handle = &session->sessionHandle;
    scoped_buf_ptr_t signature1;
    struct finish_chunk_ctx fin = { { 0, 0 } };
    struct operation *op = NULL;
    size_t input_consumed = 0;
    size_t output_used = 0;

    CHECK_RESULT_OK(lookup_operation(session, operation_handle, &op));

    if ( signature != NULL ) {
        /* Hack to ensure that non-writable memory can be mapped. */
        CHECK_RESULT_OK(
            copy_to_scoped_buf(signature->data, signature->data_length, signature1));
    }

    if (output != NULL) {
        output->data = NULL;
        output->data_length = 0;
    }

    /* No output parameters */
    if (out_params != NULL) {
        out_params->params = NULL;
        out_params->length = 0;
    }

    /* Check parameters are valid for finish */
    CHECK_RESULT_OK(check_params(params, op_allowed_params,
        sizeof(op_allowed_params) / sizeof(keymaster_tag_t)) );

    /* Map signature buffer */
    if (signature1.buf) {
        CHECK_RESULT_OK( map_buffer(session_handle, signature1.buf.get(),
            signature1.size, &fin.signature_map) );
    }

    if (output != NULL) {
        /* The output buffer is used for RSA-encrypt, RSA-decrypt, RSA-sign,
         * AES-GCM-encrypt, AES with PKCS7 padding, HMAC-sign, and
         * ECDSA-sign.  The `begin' request already told us how much extra
         * space we'll need for the answer.
         */
        output->data_length = op->final_length;
        if (op->algorithm == KM_ALGORITHM_AES) {
            output->data_length += (input ? input->data_length : 0);
        }


        /* Allocate the output buffer. The caller must free this. */
        if (output->data_length != 0) {
            CHECK_RESULT_OK(km_alloc((uint8_t**)&output->data,
                output->data_length));
        }
    }

    CHECK_RESULT_OK(split_update_chunks(session, op, true,
        params, input, &input_consumed,
        output ? const_cast</*unconst*/ uint8_t *>(output->data) : NULL,
        output ? output->data + output->data_length : NULL,
        &output_used, update_chunk, final_chunk, &fin));
    CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
        !input || input_consumed == input->data_length);
    if (output)
        output->data_length = output_used;

end:
    if (signature1.buf) {
        unmap_buffer(session_handle, signature1.buf.get(),
            &fin.signature_map);
    }

    if (ret != KM_ERROR_OK) {
        if (output != NULL) {
            free((void*)output->data);
            output->data = NULL;
            output->data_length = 0;
        }
    }

    release_operation_slot(session, op);
    LOG_D("TEE_Finish exiting with %d", ret);
    return ret;
}

keymaster_error_t TEE_Abort(
    TEE_SessionHandle               sessionHandle,
    keymaster_operation_handle_t    operation_handle)
{
    LOG_D("TEE_Abort");

    keymaster_error_t  ret  = KM_ERROR_OK;
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    tciMessage_ptr     tci = session->pTci;
    mcSessionHandle_t* session_handle = &session->sessionHandle;
    struct operation *op = NULL;

    CHECK_RESULT_OK(lookup_operation(session, operation_handle, &op));

    /* Update TCI buffer */
    tci->command.header.commandId = CMD_ID_TEE_ABORT;
    tci->abort.handle = operation_handle;

    CHECK_RESULT_OK( transact(session_handle, tci) );

end:
    release_operation_slot(session, op);
    LOG_D("TEE_Abort exiting with %d", ret);
    return ret;
}

#ifndef NDEBUG
keymaster_error_t tee__set_debug_lies(
    TEE_SessionHandle               sessionHandle,
    uint32_t                        os_version,
    uint32_t                        os_patchlevel)
{
    LOG_D("tee__set_debug_lies");
    LOG_D("  version = %" PRIu32 "; patchlevel = %" PRIu32 "",
        os_version, os_patchlevel);

    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    keymaster_error_t ret = KM_ERROR_OK;
    mcSessionHandle_t *session_handle = &session->sessionHandle;
    tciMessage_ptr tci = session->pTci;

    tci->command.header.commandId = CMD_ID_TEE_SET_DEBUG_LIES;
    tci->set_debug_lies.os_version = os_version;
    tci->set_debug_lies.os_patchlevel = os_patchlevel;

    CHECK_RESULT_OK(transact(session_handle, tci));

end:
    return ret;
}
#endif
