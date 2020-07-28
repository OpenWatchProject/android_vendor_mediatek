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

#include <stdlib.h>
#include <assert.h>
#include <memory>
#include <time.h>
#include "keymaster_ta_defs.h"
#include "serialization.h"
#include "km_util.h"

#define MAX_DYNAMIC_PARAM_BUFFERS 20

keymaster_error_t km_serialize_params(
    scoped_buf_ptr_t& serialized_data,
    const keymaster_key_param_set_t *params,
    bool add_time,
    uint32_t key_size,
    uint64_t rsa_pubexp)
{
    keymaster_error_t ret = KM_ERROR_OK;
    uint8_t* pos = NULL;
    uint32_t size;
    uint32_t n_params;
    bool key_size_present = false;
    bool rsa_pubexp_present = false;
    bool add_key_size = false;
    bool add_rsa_pubexp = false;
    uint32_t n_serialized_params = 0;
    keymaster_tag_t tag;

    n_params = (params != NULL) ? params->length : 0;
    size = 4; // uint32_t (number of parameters)

   /* First calculate the required buffer size */
    for (size_t i = 0; i < n_params; i++) {
        tag = params->params[i].tag;

        if (tag == KM_TAG_KEY_SIZE) {
            key_size_present = true;
        } else if (tag == KM_TAG_RSA_PUBLIC_EXPONENT) {
            rsa_pubexp_present = true;
        } else if (tag == KM_TAG_CREATION_DATETIME) {
            // Never serialize KM_TAG_CREATION_DATETIME tag. It's controlled by add_time flag
            continue;
        }

        /* length of tag */
        size += 4; // uint32_t <= keymaster_tag_t

        /* length of value */
        switch (keymaster_tag_get_type(tag)) {
            case KM_ENUM:
            case KM_ENUM_REP:
            case KM_UINT:
            case KM_UINT_REP:
            case KM_BOOL:
                size += 4; // uint32_t
                break;
            case KM_ULONG:
            case KM_DATE:
            case KM_ULONG_REP:
                size += 8; // uint64_t
                break;
            case KM_BIGNUM:
            case KM_BYTES:
                size += 4; // uint32_t
                size += params->params[i].blob.data_length;
                break;
            default: // bad tag
                ret = KM_ERROR_INVALID_TAG;
                goto end;
        }
        n_serialized_params++;
    }

    add_key_size = (key_size != 0) && !key_size_present;
    add_rsa_pubexp = (rsa_pubexp != 0) && !rsa_pubexp_present;

    if (add_time) {
        size += 4 + 8;
    }
    if (add_key_size) {
        size += 4 + 4;
    }
    if (add_rsa_pubexp) {
        size += 4 + 8;
    }

    /* Allocate memory for the buffer */
    serialized_data.size = size;
    serialized_data.buf.reset(new (std::nothrow) uint8_t[size]);
    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED, serialized_data.buf);

    /* Position to the beginning of the buffer */
    pos = serialized_data.buf.get();

    /* Copy parameter count */
    set_u32_increment_pos(&pos, n_serialized_params + (add_time       ? 1 : 0)
                                                    + (add_key_size   ? 1 : 0)
                                                    + (add_rsa_pubexp ? 1 : 0));

    /* Copy data */
    for (size_t i = 0; i < n_params; i++) {
        const keymaster_tag_t tag = params->params[i].tag;

        if( KM_TAG_CREATION_DATETIME == tag ) {
            // Never serialize KM_TAG_CREATION_DATETIME tag. It's controlled by add_time flag
            continue;
        }

        set_u32_increment_pos(&pos, tag);

        switch (keymaster_tag_get_type(tag)) {
            case KM_ENUM:
            case KM_ENUM_REP:
                set_u32_increment_pos(&pos, params->params[i].enumerated);
                break;
            case KM_UINT:
            case KM_UINT_REP:
                set_u32_increment_pos(&pos, params->params[i].integer);
                break;
            case KM_BOOL:
                set_u32_increment_pos(&pos, params->params[i].boolean ? 1 : 0);
                break;
            case KM_ULONG:
            case KM_ULONG_REP:
                set_u64_increment_pos(&pos, params->params[i].long_integer);
                break;
            case KM_DATE:
                set_u64_increment_pos(&pos, params->params[i].date_time);
                break;
            case KM_BIGNUM:
            case KM_BYTES:
                set_u32_increment_pos(&pos, params->params[i].blob.data_length);
                set_data_increment_pos(&pos, params->params[i].blob.data, params->params[i].blob.data_length);
                break;
            default: // bad tag
                ret = KM_ERROR_INVALID_TAG;
                goto end;
        }
    }
    if (add_time) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        set_u32_increment_pos(&pos, KM_TAG_CREATION_DATETIME);
        /* datetime should be in unit of ms */
        set_u64_increment_pos(&pos, ((uint64_t)tv.tv_usec + 1000000ull*(uint64_t)tv.tv_sec) / 1000);
    }
    if (add_key_size) {
        set_u32_increment_pos(&pos, KM_TAG_KEY_SIZE);
        set_u32_increment_pos(&pos, key_size);
    }
    if (add_rsa_pubexp) {
        set_u32_increment_pos(&pos, KM_TAG_RSA_PUBLIC_EXPONENT);
        set_u64_increment_pos(&pos, rsa_pubexp);
    }

end:
    return ret;
}

keymaster_error_t deserialize_param_set(
    keymaster_key_param_set_t *param_set,
    uint8_t **pos,
    uint32_t *remain)
{
    keymaster_error_t ret = KM_ERROR_OK;
    uint32_t n;
    uint32_t data_length;

    CHECK_NOT_NULL(param_set);
    CHECK_NOT_NULL(pos);
    CHECK_NOT_NULL(remain);

    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
        *remain >= 4);
    n = get_u32(*pos);
    *pos += 4; *remain -= 4;
    param_set->length = n;

    if (n == 0) {
        param_set->params = NULL;
    } else {
        param_set->params = (keymaster_key_param_t*)calloc(sizeof(keymaster_key_param_t), n);
        CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
            param_set->params != NULL);
        memset(param_set->params, 0, n * sizeof(keymaster_key_param_t));

        for (uint32_t i = 0; i < n; i++) {
            keymaster_key_param_t *param = param_set->params + i;

            // read tag
            CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                *remain >= 4);
            param->tag = (keymaster_tag_t)get_u32(*pos);
            *pos += 4; *remain -= 4;

            // read value
            switch (keymaster_tag_get_type(param->tag)) {
                case KM_ENUM:
                case KM_ENUM_REP:
                    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                        *remain >= 4);
                    param->enumerated = get_u32(*pos);
                    *pos += 4; *remain -= 4;
                    break;
                case KM_UINT:
                case KM_UINT_REP:
                    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                        *remain >= 4);
                    param->integer = get_u32(*pos);
                    *pos += 4; *remain -= 4;
                    break;
                case KM_BOOL:
                    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                        *remain >= 4);
                    param->boolean = (get_u32(*pos) != 0);
                    *pos += 4; *remain -= 4;
                    break;
                case KM_ULONG:
                case KM_ULONG_REP:
                    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                        *remain >= 8);
                    param->long_integer = get_u64(*pos);
                    *pos += 8; *remain -= 8;
                    break;
                case KM_DATE:
                    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                        *remain >= 8);
                    param->date_time = get_u64(*pos);
                    *pos += 8; *remain -= 8;
                    break;
                case KM_BIGNUM:
                case KM_BYTES:
                    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                        *remain >= 4);
                    data_length = get_u32(*pos);
                    *pos += 4; *remain -= 4;
                    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                        *remain >= data_length);
                    param->blob.data = (const uint8_t*)malloc(data_length);
                    CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
                        param->blob.data != NULL);
                    memcpy((void*)param->blob.data, *pos, data_length);
                    param->blob.data_length = data_length;
                    *pos += data_length; *remain -= data_length;
                    break;
                default:
                    ret = KM_ERROR_INVALID_TAG;
                    goto end;
            }
        }
    }

end:
    return ret;
}

keymaster_error_t km_deserialize_characteristics(
    keymaster_key_characteristics_t *characteristics,
    const uint8_t *buffer,
    uint32_t buffer_length)
{
    keymaster_error_t ret = KM_ERROR_OK;
    uint8_t *pos = (uint8_t*)buffer;
    uint32_t remain = buffer_length;

    CHECK_NOT_NULL(characteristics);
    CHECK_NOT_NULL(buffer);

    memset(characteristics, 0, sizeof(keymaster_key_characteristics_t));

    CHECK_RESULT_OK(deserialize_param_set(&characteristics->hw_enforced, &pos, &remain));
    CHECK_RESULT_OK(deserialize_param_set(&characteristics->sw_enforced, &pos, &remain));

end:
    if (ret != KM_ERROR_OK) {
        keymaster_free_characteristics(characteristics);
    }
    return ret;
}

keymaster_error_t km_deserialize_attestation(
    keymaster_cert_chain_t *cert_chain,
    const uint8_t *buffer,
    uint32_t buffer_length)
{
    keymaster_error_t ret = KM_ERROR_OK;
    uint8_t *pos = (uint8_t*)buffer;
    uint32_t remain = buffer_length;
    uint32_t n = 0;

    CHECK_NOT_NULL(cert_chain);
    CHECK_NOT_NULL(buffer);

    memset(cert_chain, 0, sizeof(keymaster_cert_chain_t));

    CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
        remain >= 4);
    n = get_u32(pos);
    pos += 4; remain -= 4;
    cert_chain->entry_count = n;
    if (n == 0) {
        cert_chain->entries = NULL;
    }
    else {
        cert_chain->entries = (keymaster_blob_t*)calloc(sizeof(keymaster_blob_t), n);
        CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
            cert_chain->entries != NULL);
        memset(cert_chain->entries, 0, n * sizeof(keymaster_blob_t));
        for (uint32_t i = 0; i < n; i++) {
            keymaster_blob_t *blob = cert_chain->entries + i;

            CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                remain >= 4);
            blob->data_length = get_u32(pos);
            pos += 4; remain -= 4;

            CHECK_TRUE(KM_ERROR_UNKNOWN_ERROR,
                blob->data_length != 0);
            blob->data = (const uint8_t*)malloc(blob->data_length);
            CHECK_TRUE(KM_ERROR_MEMORY_ALLOCATION_FAILED,
                blob->data != NULL);
            CHECK_TRUE(KM_ERROR_INSUFFICIENT_BUFFER_SPACE,
                remain >= blob->data_length);
            memcpy((void*)blob->data, pos, blob->data_length);
            pos += blob->data_length; remain -= blob->data_length;
        }
    }

end:
    if (ret != KM_ERROR_OK) {
        keymaster_free_cert_chain(cert_chain);
    }
    return ret;
}
