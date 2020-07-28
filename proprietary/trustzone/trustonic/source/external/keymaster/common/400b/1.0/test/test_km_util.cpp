/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
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

#ifdef SOTER
#include "mtk_keymaster_defs.h"
#include "mtk_keymaster1.h"
#else
#include <hardware/keymaster_defs.h>
#include <hardware/keymaster1.h>
#endif

#include "test_km_util.h"

#undef  LOG_ANDROID
#undef  LOG_TAG
#define LOG_TAG "TlcTeeKeyMasterTest"
#include "log.h"

// static void print_blob(
//     const keymaster_blob_t *blob)
// {
//     print_buffer(blob->data, blob->data_length);
// }

static void print_uint64(
    uint64_t x)
{
    printf("0x%08x%08x\n", (uint32_t)(x >> 32), (uint32_t)(x & 0xFFFFFFFF));
}

static void print_bool(
    bool x)
{
    if (x) {
        printf("true\n");
    } else {
        printf("false\n");
    }
}

static void print_param_set(
    const keymaster_key_param_set_t *param_set)
{
    for (size_t i = 0; i < param_set->length; i++) {
        const keymaster_key_param_t param = param_set->params[i];
        switch (param.tag) {
#define PRINT_ENUM printf("0x%08x\n", param.enumerated)
#define PRINT_UINT printf("%u\n", param.integer)
#define PRINT_ULONG print_uint64(param.long_integer)
#define PRINT_BOOL print_bool(param.boolean)
#define PRINT_DATETIME print_uint64(param.date_time)
#define PARAM_CASE(tag, printval) \
    case tag: \
        printf("%s: ", #tag); \
        printval; \
        break;
#define PARAM_CASE_ENUM(tag) PARAM_CASE(tag, PRINT_ENUM)
#define PARAM_CASE_UINT(tag) PARAM_CASE(tag, PRINT_UINT)
#define PARAM_CASE_ULONG(tag) PARAM_CASE(tag, PRINT_ULONG)
#define PARAM_CASE_BOOL(tag) PARAM_CASE(tag, PRINT_BOOL)
#define PARAM_CASE_DATETIME(tag) PARAM_CASE(tag, PRINT_DATETIME)
        PARAM_CASE_DATETIME(KM_TAG_ACTIVE_DATETIME)
        PARAM_CASE_ENUM(KM_TAG_ALGORITHM)
        PARAM_CASE_UINT(KM_TAG_AUTH_TIMEOUT)
        PARAM_CASE_ENUM(KM_TAG_BLOB_USAGE_REQUIREMENTS)
        PARAM_CASE_ENUM(KM_TAG_BLOCK_MODE)
        PARAM_CASE_BOOL(KM_TAG_BOOTLOADER_ONLY)
        PARAM_CASE_BOOL(KM_TAG_CALLER_NONCE)
        PARAM_CASE_DATETIME(KM_TAG_CREATION_DATETIME)
        PARAM_CASE_ENUM(KM_TAG_DIGEST)
        PARAM_CASE_UINT(KM_TAG_KEY_SIZE)
        PARAM_CASE_UINT(KM_TAG_MAX_USES_PER_BOOT)
        PARAM_CASE_UINT(KM_TAG_MIN_MAC_LENGTH)
        PARAM_CASE_UINT(KM_TAG_MIN_SECONDS_BETWEEN_OPS)
        PARAM_CASE_BOOL(KM_TAG_NO_AUTH_REQUIRED)
        PARAM_CASE_ENUM(KM_TAG_ORIGIN)
        PARAM_CASE_DATETIME(KM_TAG_ORIGINATION_EXPIRE_DATETIME)
        PARAM_CASE_ENUM(KM_TAG_PADDING)
        PARAM_CASE_ENUM(KM_TAG_PURPOSE)
        PARAM_CASE_BOOL(KM_TAG_ROLLBACK_RESISTANT)
        PARAM_CASE_ULONG(KM_TAG_RSA_PUBLIC_EXPONENT)
        PARAM_CASE_DATETIME(KM_TAG_USAGE_EXPIRE_DATETIME)
        PARAM_CASE_ENUM(KM_TAG_USER_AUTH_TYPE)
        PARAM_CASE_ULONG(KM_TAG_USER_SECURE_ID)
#undef PARAM_CASE_ENUM
#undef PARAM_CASE_UINT
#undef PARAM_CASE_ULONG
#undef PARAM_CASE_BOOL
#undef PARAM_CASE_DATETIME
#undef PARAM_CASE
#undef PRINT_ENUM
#undef PRINT_UINT
#undef PRINT_ULONG
#undef PRINT_BOOL
#undef PRINT_DATETIME
            default:
                printf("(unknown tag 0x%08x)\n", param.tag);
        }
    }
}

void print_buffer(
    const uint8_t *buf,
    size_t buflen)
{
    printf("[");
    for (size_t i = 0; i < buflen; i++) {
        printf("0x%02x, ", buf[i]);
    }
    printf("]\n");
}

void print_characteristics(
    const keymaster_key_characteristics_t *characteristics)
{
    printf("\nHardware-enforced characteristics:\n");
    print_param_set(&characteristics->hw_enforced);
    printf("\nSoftware-enforced characteristics:\n");
    print_param_set(&characteristics->sw_enforced);
}

keymaster_error_t print_raw_key(
    keymaster1_device_t *device,
    const keymaster_key_blob_t *key_blob)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_blob_t key_data = {0, 0};

    CHECK_RESULT_OK( device->export_key(device,
        KM_KEY_FORMAT_RAW, key_blob, NULL, NULL, &key_data) );

    print_buffer(key_data.data, key_data.data_length);

end:
    km_free_blob(&key_data);
    return res;
}

uint32_t block_length(
    keymaster_digest_t digest)
{
    switch (digest) {
        case KM_DIGEST_MD5:
            return 128;
        case KM_DIGEST_SHA1:
            return 160;
        case KM_DIGEST_SHA_2_224:
            return 224;
        case KM_DIGEST_SHA_2_256:
            return 256;
        case KM_DIGEST_SHA_2_384:
            return 384;
        case KM_DIGEST_SHA_2_512:
            return 512;
        default:
            return 0;
    }
}

void km_free_blob(
    keymaster_blob_t *blob)
{
    if (blob != NULL) {
        free((void*)blob->data);
        blob->data = NULL;
        blob->data_length = 0;
    }
}

void km_free_key_blob(
    keymaster_key_blob_t *key_blob)
{
    if (key_blob != NULL) {
        free((void*)key_blob->key_material);
        key_blob->key_material = NULL;
        key_blob->key_material_size = 0;
    }
}
