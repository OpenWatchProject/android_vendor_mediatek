/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
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

#include "keymaster_ta_defs.h"
#include "km_util.h"
#include "km_shared_util.h"
#include "authlist.h"

#define CHK(a) CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT, a)

/* Sequence of explicit tags in an AuthorizationList, all of them optional.
 * See documentation of attestKey in IKeymasterDevice.hal.
 */
static const keymaster_tag_t km_tags[] = {
    KM_TAG_PURPOSE,                        //   1
    KM_TAG_ALGORITHM,                      //   2
    KM_TAG_KEY_SIZE,                       //   3
    KM_TAG_BLOCK_MODE,                     //   4
    KM_TAG_DIGEST,                         //   5
    KM_TAG_PADDING,                        //   6
    KM_TAG_CALLER_NONCE,                   //   7
    KM_TAG_MIN_MAC_LENGTH,                 //   8
    KM_TAG_EC_CURVE,                       //  10
    KM_TAG_RSA_PUBLIC_EXPONENT,            // 200
    KM_TAG_ROLLBACK_RESISTANCE,            // 303
    KM_TAG_ACTIVE_DATETIME,                // 400
    KM_TAG_ORIGINATION_EXPIRE_DATETIME,    // 401
    KM_TAG_USAGE_EXPIRE_DATETIME,          // 402
    KM_TAG_USER_SECURE_ID,                 // 502
    KM_TAG_NO_AUTH_REQUIRED,               // 503
    KM_TAG_USER_AUTH_TYPE,                 // 504
    KM_TAG_AUTH_TIMEOUT,                   // 505
    KM_TAG_ALLOW_WHILE_ON_BODY,            // 506
    KM_TAG_TRUSTED_USER_PRESENCE_REQUIRED, // 507
    KM_TAG_TRUSTED_CONFIRMATION_REQUIRED,  // 508
    KM_TAG_UNLOCKED_DEVICE_REQUIRED,       // 509
    KM_TAG_CREATION_DATETIME,              // 701
    KM_TAG_ORIGIN,                         // 702
    KM_TAG_ROOT_OF_TRUST,                  // 704
    KM_TAG_OS_VERSION,                     // 705
    KM_TAG_OS_PATCHLEVEL,                  // 706
    KM_TAG_ATTESTATION_APPLICATION_ID,     // 709
    KM_TAG_ATTESTATION_ID_BRAND,           // 710
    KM_TAG_ATTESTATION_ID_DEVICE,          // 711
    KM_TAG_ATTESTATION_ID_PRODUCT,         // 712
    KM_TAG_ATTESTATION_ID_SERIAL,          // 713
    KM_TAG_ATTESTATION_ID_IMEI,            // 714
    KM_TAG_ATTESTATION_ID_MEID,            // 715
    KM_TAG_ATTESTATION_ID_MANUFACTURER,    // 716
    KM_TAG_ATTESTATION_ID_MODEL,           // 717
    KM_TAG_VENDOR_PATCHLEVEL,              // 718
    KM_TAG_BOOT_PATCHLEVEL,                // 719
};
#define N_TAGS (sizeof(km_tags) / sizeof(keymaster_tag_t))
#define TAGID(x) ((x) & 0x0FFFFFFF)

/* Increments n_params and req_len */
static keymaster_error_t check_AuthorizationItem(
    keymaster_tag_t km_tag,
    uint32_t *n_params,
    size_t *req_len,
    const uint8_t *data, size_t len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    size_t l;
    size_t len0 = len;

    switch (km_tag) {
        case KM_TAG_PURPOSE:
        case KM_TAG_BLOCK_MODE:
        case KM_TAG_DIGEST:
        case KM_TAG_PADDING:
            /* SET OF INTEGER */
            CHK(len >= 2 && data[0] == 0x31 && data[1] == len - 2);
            /* Each integer is < 128 so takes 3 bytes (0x02, 1, val) */
            CHK(len % 3 == 2);
            *n_params += (len - 2)/3;
            *req_len += (4 + 4)*((len - 2)/3); // tag + value
            break;
        case KM_TAG_ALGORITHM:
        case KM_TAG_KEY_SIZE:
        case KM_TAG_EC_CURVE:
        case KM_TAG_USER_AUTH_TYPE:
        case KM_TAG_AUTH_TIMEOUT:
        case KM_TAG_ORIGIN:
        case KM_TAG_OS_VERSION:
        case KM_TAG_OS_PATCHLEVEL:
        case KM_TAG_VENDOR_PATCHLEVEL:
        case KM_TAG_BOOT_PATCHLEVEL:
            /* INTEGER (4 bytes) */
            CHK(len >= 2 && data[0] == 0x02 && data[1] == len - 2);
            (*n_params)++; *req_len += 4 + 4; // tag + value
            break;
        case KM_TAG_RSA_PUBLIC_EXPONENT:
        case KM_TAG_ACTIVE_DATETIME:
        case KM_TAG_ORIGINATION_EXPIRE_DATETIME:
        case KM_TAG_USAGE_EXPIRE_DATETIME:
        case KM_TAG_USER_SECURE_ID:
        case KM_TAG_CREATION_DATETIME:
            /* INTEGER (8 bytes) */
            CHK(len >= 2 && data[0] == 0x02 && data[1] == len - 2);
            (*n_params)++; *req_len += 4 + 8; // tag + value
            break;
        case KM_TAG_ROLLBACK_RESISTANCE:
        case KM_TAG_NO_AUTH_REQUIRED:
        case KM_TAG_ALLOW_WHILE_ON_BODY:
        case KM_TAG_TRUSTED_USER_PRESENCE_REQUIRED:
        case KM_TAG_TRUSTED_CONFIRMATION_REQUIRED:
        case KM_TAG_UNLOCKED_DEVICE_REQUIRED:
        case KM_TAG_ALL_APPLICATIONS:
            /* NULL */
            CHK(len == 2 && data[0] == 0x05 && data[1] == 0);
            (*n_params)++; *req_len += 4 + 4; // tag + value
            break;
        case KM_TAG_ROOT_OF_TRUST:
            /* RootOfTrust */
            /* Assume verifiedBootKey has length 32 (TODO generalize this?) */
            CHK(len == 42 && data[0] == 0x30 && data[1] == 40
                          && data[2] == 0x04 && data[3] == 32
                          && data[36] == 0x01 && data[37] == 1
                          && data[39] == 0x0A && data[40] == 1);
            /* We don't add this to the key params. */
            break;
        case KM_TAG_ATTESTATION_CHALLENGE:
        case KM_TAG_ATTESTATION_APPLICATION_ID:
        case KM_TAG_ATTESTATION_ID_BRAND:
        case KM_TAG_ATTESTATION_ID_DEVICE:
        case KM_TAG_ATTESTATION_ID_PRODUCT:
        case KM_TAG_ATTESTATION_ID_SERIAL:
        case KM_TAG_ATTESTATION_ID_IMEI:
        case KM_TAG_ATTESTATION_ID_MEID:
        case KM_TAG_ATTESTATION_ID_MANUFACTURER:
        case KM_TAG_ATTESTATION_ID_MODEL:
            /* OCTET_STRING */
            CHK(der_read_octet_string_length(&l, &data, &len0));
            CHK(l == len && len0 == 0);
            (*n_params)++; *req_len += 4 + 4 + l; // tag + length + value
            break;
        default:
            CHK(false);
    }

end:
    return ret;
}

static uint32_t read_u32(const uint8_t *a, size_t l) {
    uint32_t v = 0;
    for (size_t i = 0; i < l; i++) {
        v *= 256; v += a[i];
    }
    return v;
}

static uint64_t read_u64(const uint8_t *a, size_t l) {
    uint64_t v = 0;
    for (size_t i = 0; i < l; i++) {
        v *= 256; v += a[i];
    }
    return v;
}

static keymaster_error_t serialize_AuthorizationItem(
    keymaster_tag_t km_tag,
    uint8_t **key_params,
    uint32_t *key_type,
    uint64_t *sid,
    const uint8_t *data, size_t len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    int n;
    uint32_t v32;
    uint64_t v64;

    switch (km_tag) {
        case KM_TAG_PURPOSE:
        case KM_TAG_BLOCK_MODE:
        case KM_TAG_DIGEST:
        case KM_TAG_PADDING:
            /* SET OF INTEGER */
            CHK(len >= 2 && data[0] == 0x31 && data[1] == len - 2);
            n = data[1];
            CHK(n % 3 == 0);
            n /= 3;
            for (int i = 0; i < n; i++) {
                CHK(data[2 + 3*i] == 0x02);
                CHK(data[2 + 3*i + 1] == 1);
                v32 = data[2 + 3*i + 2];
                set_u32_increment_pos(key_params, km_tag);
                set_u32_increment_pos(key_params, v32);
            }
            break;
        case KM_TAG_ALGORITHM:
        case KM_TAG_KEY_SIZE:
        case KM_TAG_EC_CURVE:
        case KM_TAG_USER_AUTH_TYPE:
        case KM_TAG_AUTH_TIMEOUT:
        case KM_TAG_ORIGIN:
        case KM_TAG_OS_VERSION:
        case KM_TAG_OS_PATCHLEVEL:
        case KM_TAG_VENDOR_PATCHLEVEL:
        case KM_TAG_BOOT_PATCHLEVEL:
            /* INTEGER (4 bytes) */
            CHK(len >= 2 && data[0] == 0x02 && data[1] == len - 2);
            data += 2; len -= 2;
            CHK(len <= 4);
            v32 = read_u32(data, len);
            set_u32_increment_pos(key_params, km_tag);
            set_u32_increment_pos(key_params, v32);
            if (km_tag == KM_TAG_ALGORITHM) {
                *key_type = v32;
            }
            break;
        case KM_TAG_RSA_PUBLIC_EXPONENT:
        case KM_TAG_ACTIVE_DATETIME:
        case KM_TAG_ORIGINATION_EXPIRE_DATETIME:
        case KM_TAG_USAGE_EXPIRE_DATETIME:
        case KM_TAG_USER_SECURE_ID:
        case KM_TAG_CREATION_DATETIME:
            /* INTEGER (8 bytes) */
            CHK(len >= 2 && data[0] == 0x02 && data[1] == len - 2);
            data += 2; len -= 2;
            CHK(len <= 8);
            v64 = read_u64(data, len);
            set_u32_increment_pos(key_params, km_tag);
            set_u64_increment_pos(key_params, v64);
            if (km_tag == KM_TAG_USER_SECURE_ID) {
                *sid = v64;
            }
            break;
        case KM_TAG_ROLLBACK_RESISTANCE:
        case KM_TAG_NO_AUTH_REQUIRED:
        case KM_TAG_ALLOW_WHILE_ON_BODY:
        case KM_TAG_TRUSTED_USER_PRESENCE_REQUIRED:
        case KM_TAG_TRUSTED_CONFIRMATION_REQUIRED:
        case KM_TAG_UNLOCKED_DEVICE_REQUIRED:
        case KM_TAG_ALL_APPLICATIONS:
            /* NULL */
            CHK(len == 2 && data[0] == 0x05 && data[1] == 0);
            set_u32_increment_pos(key_params, km_tag);
            set_u32_increment_pos(key_params, 1);
            break;
        case KM_TAG_ROOT_OF_TRUST:
            /* RootOfTrust */
            break;
        case KM_TAG_ATTESTATION_CHALLENGE:
        case KM_TAG_ATTESTATION_APPLICATION_ID:
        case KM_TAG_ATTESTATION_ID_BRAND:
        case KM_TAG_ATTESTATION_ID_DEVICE:
        case KM_TAG_ATTESTATION_ID_PRODUCT:
        case KM_TAG_ATTESTATION_ID_SERIAL:
        case KM_TAG_ATTESTATION_ID_IMEI:
        case KM_TAG_ATTESTATION_ID_MEID:
        case KM_TAG_ATTESTATION_ID_MANUFACTURER:
        case KM_TAG_ATTESTATION_ID_MODEL:
            /* OCTET_STRING */
            CHK(len >= 2 && data[0] == 0x04 && data[1] == len - 2);
            set_u32_increment_pos(key_params, km_tag);
            set_u32_increment_pos(key_params, len - 2);
            memcpy(*key_params, data + 2, len - 2);
            *key_params += (len - 2);
            break;
        default:
            CHK(false);
    }

end:
    return ret;
}

/* Work out how many parameters there are, and how much memory we need to
 * allocate to serialize them in the required format.
 */
static keymaster_error_t check_AuthorizationList(
    uint32_t *n_params,
    size_t *req_len,
    const uint8_t *data, size_t len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    int tag;
    size_t item_len;
    const uint8_t *p = data;
    unsigned i = 0;

    *n_params = 0;
    *req_len = 4; // 4 bytes for n_params
    while (len) {
        CHK(der_read_explicit_tag_and_length(&tag, &item_len, &p, &len));

        /* Look for the tag in AuthorizationList and check length required for value */
        while (i < N_TAGS && TAGID(km_tags[i]) != tag) i++;
        CHK(i < N_TAGS);
        keymaster_tag_t km_tag = km_tags[i];
        CHECK_RESULT_OK(check_AuthorizationItem(
            km_tag, n_params, req_len, p, item_len));
        p += item_len; len -= item_len;
    }

end:
    return ret;
}

static keymaster_error_t serialize_AuthorizationList(
    uint8_t *key_params,
    uint32_t *key_type,
    uint64_t *sid,
    const uint8_t *data, size_t len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    int tag;
    size_t item_len;
    const uint8_t *p = data;
    unsigned i = 0;

    while (len) {
        CHK(der_read_explicit_tag_and_length(&tag, &item_len, &p, &len));

        /* Look for the tag in AuthorizationList and write value */
        while (i < N_TAGS && TAGID(km_tags[i]) != tag) i++;
        CHK(i < N_TAGS);
        keymaster_tag_t km_tag = km_tags[i];
        CHECK_RESULT_OK(serialize_AuthorizationItem(
            km_tag, &key_params, key_type, sid, p, item_len));
        p += item_len; len -= item_len;
    }

end:
    return ret;
}

keymaster_error_t parse_AuthorizationList(
    uint8_t **key_params, size_t *key_params_len,
    uint32_t *key_type,
    uint64_t *sid,
    const uint8_t *data, size_t len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    size_t seq_len;
    uint32_t n_params;
    const uint8_t *p = data;

    CHK(der_read_sequence_length(&seq_len, &p, &len) && seq_len == len);

    /* Find out number of parameters and required length */
    CHECK_RESULT_OK(check_AuthorizationList(&n_params, key_params_len, p, len));

    /* Allocate buffer */
    CHECK_RESULT_OK(km_alloc(key_params, *key_params_len));

    /* Write the number of parameters */
    set_u32(*key_params, n_params);

    /* Write the serialized key parameters */
    CHECK_RESULT_OK(serialize_AuthorizationList(*key_params + 4, key_type, sid, p, len));

end:
    return ret;
}
