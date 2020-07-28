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
#include "mtk_keymaster1.h"
#else
#include <hardware/keymaster1.h>
#endif
#include <assert.h>

#include "test_km_aes.h"
#include "test_km_util.h"

#undef  LOG_ANDROID
#undef  LOG_TAG
#define LOG_TAG "TlcTeeKeyMasterTest"
#include "log.h"

static keymaster_error_t test_km_aes_roundtrip(
    keymaster1_device_t *device,
    const keymaster_key_blob_t *key_blob,
    keymaster_block_mode_t mode,
    size_t msg_part1_len,
    size_t msg_part2_len)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[2];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[16];
    uint8_t input[64];
    keymaster_blob_t input_blob = {0, 0};
    keymaster_blob_t enc_output1 = {0, 0};
    keymaster_blob_t enc_output2 = {0, 0};
    keymaster_blob_t dec_output1 = {0, 0};
    keymaster_blob_t dec_output2 = {0, 0};
    size_t input_consumed = 0;

    assert(msg_part1_len + msg_part2_len <= 64);

    /* populate input message */
    memset(input, 7, sizeof(input));

    /* populate nonce */
    memset(nonce_bytes, 3, 16);
    nonce.data = &nonce_bytes[0];
    nonce.data_length = (mode == KM_MODE_GCM) ? 12 : 16;

    key_param[0].tag = KM_TAG_BLOCK_MODE;
    key_param[0].enumerated = mode;
    key_param[1].tag = KM_TAG_NONCE; // ignored for ECB
    key_param[1].blob = nonce;
    paramset.length = 2;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT,
        key_blob,
        &paramset,
        NULL, // no out_params
        &handle));

    input_blob.data = &input[0];
    input_blob.data_length = msg_part1_len;
    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &input_blob, // first half of message
        &input_consumed,
        NULL, // no out_params
        &enc_output1));
    CHECK_TRUE(input_consumed == msg_part1_len);
    CHECK_TRUE(enc_output1.data_length == msg_part1_len);

    input_blob.data = input + msg_part1_len;
    input_blob.data_length = msg_part2_len;
    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &input_blob, // second half of message
        &input_consumed,
        NULL, // no out_params
        &enc_output2));
    CHECK_TRUE(input_consumed == msg_part2_len);
    CHECK_TRUE(enc_output2.data_length == msg_part2_len);

    CHECK_RESULT_OK(device->finish(device,
        handle,
        NULL, // no params
        NULL, // no signature
        NULL, // no out_params
        NULL));

    // key_params as for encrypt above
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_DECRYPT,
        key_blob,
        &paramset,
        NULL, // no out_params
        &handle));

    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &enc_output1, // first half of ciphertext
        &input_consumed,
        NULL, // no out_params
        &dec_output1));
    CHECK_TRUE(input_consumed == msg_part1_len);
    CHECK_TRUE(dec_output1.data_length == msg_part1_len);

    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &enc_output2, // second half of ciphertext
        &input_consumed,
        NULL, // no out_params
        &dec_output2));
    CHECK_TRUE(input_consumed == msg_part2_len);
    CHECK_TRUE(dec_output2.data_length == msg_part2_len);

    CHECK_RESULT_OK(device->finish(device,
        handle,
        NULL, // no params
        NULL, // no signature
        NULL, // no out_params
        NULL));

    CHECK_TRUE((memcmp(&input[0], dec_output1.data, msg_part1_len) == 0) &&
               (memcmp(&input[msg_part1_len], dec_output2.data, msg_part2_len) == 0));

end:
    km_free_blob(&enc_output1);
    km_free_blob(&enc_output2);
    km_free_blob(&dec_output1);
    km_free_blob(&dec_output2);

    return res;
}

static keymaster_error_t test_km_aes_roundtrip_noncegen(
    keymaster1_device_t *device,
    const keymaster_key_blob_t *key_blob,
    keymaster_block_mode_t mode)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t param[2];
    keymaster_key_param_set_t paramset = {param, 0};
    keymaster_key_param_set_t out_paramset = {NULL, 0};
    keymaster_operation_handle_t handle;
    uint8_t input[16];
    keymaster_blob_t input_blob = {input, sizeof(input)};
    keymaster_blob_t enc_output = {0, 0};
    keymaster_blob_t enc_output1 = {0, 0};
    keymaster_blob_t dec_output = {0, 0};
    size_t input_consumed = 0;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[16];

    /* populate input message */
    memset(input, 7, sizeof(input));

    /* initialize nonce */
    nonce.data = nonce_bytes;
    nonce.data_length = (mode == KM_MODE_GCM) ? 12 : 16;

    param[0].tag = KM_TAG_BLOCK_MODE;
    param[0].enumerated = mode;
    paramset.length = 1;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, &out_paramset, &handle));
    CHECK_TRUE(out_paramset.length == 1);
    CHECK_TRUE(out_paramset.params[0].tag == KM_TAG_NONCE);
    CHECK_TRUE(out_paramset.params[0].blob.data_length == 16);
    memcpy(nonce_bytes, out_paramset.params[0].blob.data, 16);

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &input_blob, &input_consumed, NULL, &enc_output));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(enc_output.data_length == sizeof(input));

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, NULL));

    // Try again supplying same nonce -- should get same ciphertext.
    param[1].tag = KM_TAG_NONCE;
    param[1].blob = nonce;
    paramset.length = 2;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, &handle));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &input_blob, &input_consumed, NULL, &enc_output1));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(enc_output1.data_length == sizeof(input));
    CHECK_TRUE(memcmp(enc_output.data, enc_output1.data, sizeof(input)) == 0);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, NULL));

    // And now decrypt
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, &handle));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output,  &input_consumed, NULL, &dec_output));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(dec_output.data_length == sizeof(input));

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, NULL));

    CHECK_TRUE(memcmp(input, dec_output.data, sizeof(input)) == 0);

end:
    km_free_blob(&enc_output);
    km_free_blob(&enc_output1);
    km_free_blob(&dec_output);
    keymaster_free_param_set(&out_paramset);

    return res;
}

static keymaster_error_t test_km_aes_gcm_roundtrip_noncegen(
    keymaster1_device_t *device,
    const keymaster_key_blob_t *key_blob)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t param[3];
    keymaster_key_param_set_t paramset = {param, 0};
    keymaster_key_param_set_t out_paramset = {NULL, 0};
    keymaster_operation_handle_t handle;
    uint8_t input[16];
    keymaster_blob_t input_blob = {input, sizeof(input)};
    keymaster_blob_t enc_output = {0, 0};
    keymaster_blob_t enc_output1 = {0, 0};
    keymaster_blob_t dec_output = {0, 0};
    keymaster_blob_t dec_final_output = {0, 0};
    keymaster_blob_t tag = {0, 0};
    keymaster_blob_t tag1 = {0, 0};
    size_t input_consumed = 0;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[12];

    /* populate input message */
    memset(input, 7, sizeof(input));

    /* initialize nonce */
    nonce.data = nonce_bytes;
    nonce.data_length = sizeof(nonce_bytes);

    param[0].tag = KM_TAG_BLOCK_MODE;
    param[0].enumerated = KM_MODE_GCM;
    param[1].tag = KM_TAG_MAC_LENGTH;
    param[1].integer = 128;
    paramset.length = 2;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, &out_paramset, &handle));
    CHECK_TRUE(out_paramset.length == 1);
    CHECK_TRUE(out_paramset.params[0].tag == KM_TAG_NONCE);
    CHECK_TRUE(out_paramset.params[0].blob.data_length == 12);
    memcpy(nonce_bytes, out_paramset.params[0].blob.data, 12);

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &input_blob, &input_consumed, NULL, &enc_output));
    CHECK_TRUE(input_consumed == sizeof(input));

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, &tag));

    // Try again supplying same nonce -- should get same ciphertext.
    param[2].tag = KM_TAG_NONCE;
    param[2].blob = nonce;
    paramset.length = 3;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, &handle));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &input_blob, &input_consumed, NULL, &enc_output1));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(
        (enc_output.data_length == enc_output1.data_length) &&
        (memcmp(enc_output.data, enc_output1.data, enc_output.data_length) == 0));

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, &tag1));
    CHECK_TRUE(
        (tag.data_length == tag1.data_length) &&
        (memcmp(tag.data, tag1.data, tag.data_length) == 0));

    // And now decrypt
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, &handle));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output, &input_consumed, NULL, &dec_output));
    CHECK_TRUE(input_consumed == sizeof(input));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &tag, &input_consumed, NULL, &dec_final_output));
    CHECK_TRUE(input_consumed == tag.data_length);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, NULL));

    CHECK_TRUE(
        (dec_output.data_length + dec_final_output.data_length == sizeof(input)) &&
        (memcmp(&input[0], dec_output.data, dec_output.data_length) == 0) &&
        (memcmp(&input[dec_output.data_length], dec_final_output.data, dec_final_output.data_length) == 0));

end:
    km_free_blob(&enc_output);
    km_free_blob(&enc_output1);
    km_free_blob(&dec_output);
    km_free_blob(&dec_final_output);
    km_free_blob(&tag);
    km_free_blob(&tag1);
    keymaster_free_param_set(&out_paramset);

    return res;
}

static keymaster_error_t test_km_aes_pkcs7(
    keymaster1_device_t *device,
    const keymaster_key_blob_t *key_blob,
    keymaster_block_mode_t mode,
    size_t message_len)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[16];
    uint8_t input[64];
    keymaster_blob_t input_blob = {0, 0};
    keymaster_blob_t enc_output = {0, 0};
    keymaster_blob_t enc_output1 = {0, 0};
    keymaster_blob_t dec_output = {0, 0};
    keymaster_blob_t dec_output1 = {0, 0};
    keymaster_blob_t dec_output2 = {0, 0};
    size_t input_consumed = 0;

    assert(message_len <= 64);

    /* populate input message */
    memset(input, 7, sizeof(input));

    /* populate nonce */
    memset(nonce_bytes, 3, sizeof(nonce_bytes));
    nonce.data = nonce_bytes;
    nonce.data_length = sizeof(nonce_bytes);

    key_param[0].tag = KM_TAG_BLOCK_MODE;
    key_param[0].enumerated = mode;
    key_param[1].tag = KM_TAG_NONCE; // ignored for ECB
    key_param[1].blob = nonce;
    key_param[2].tag = KM_TAG_PADDING;
    key_param[2].enumerated = KM_PAD_PKCS7;
    paramset.length = 3;

    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, &handle));

    input_blob.data = input;
    input_blob.data_length = message_len;
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &input_blob, &input_consumed, NULL, &enc_output));
    CHECK_TRUE(input_consumed == message_len);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, &enc_output1));
    CHECK_TRUE((enc_output.data_length + enc_output1.data_length > message_len) &&
           (enc_output.data_length + enc_output1.data_length <= message_len + 16) &&
           ((enc_output.data_length + enc_output1.data_length) % 16 == 0));

    // key_params as for encrypt above
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, &handle));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output, &input_consumed, NULL, &dec_output));
    CHECK_TRUE(input_consumed == enc_output.data_length);

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output1, &input_consumed, NULL, &dec_output1));
    CHECK_TRUE(input_consumed == enc_output1.data_length);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, &dec_output2));

    CHECK_TRUE(dec_output.data_length + dec_output1.data_length + dec_output2.data_length == message_len);
    CHECK_TRUE((memcmp(input, dec_output.data, dec_output.data_length) == 0) &&
        (memcmp(input + dec_output.data_length, dec_output1.data, dec_output1.data_length) == 0) &&
        (memcmp(input + dec_output.data_length + dec_output1.data_length, dec_output2.data, dec_output2.data_length) == 0));

end:
    km_free_blob(&enc_output);
    km_free_blob(&enc_output1);
    km_free_blob(&dec_output);
    km_free_blob(&dec_output1);
    km_free_blob(&dec_output2);

    return res;
}

static keymaster_error_t test_km_aes_gcm_roundtrip(
    keymaster1_device_t *device,
    const keymaster_key_blob_t *key_blob,
    uint32_t tagsize,
    size_t msg_part1_len,
    size_t msg_part2_len)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[4];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[12];
    keymaster_blob_t aad;
    uint8_t aad_bytes[23];
    uint8_t input[64];
    keymaster_blob_t input_blob = {0, 0};
    keymaster_blob_t enc_output1 = {0, 0};
    keymaster_blob_t enc_output2 = {0, 0};
    keymaster_blob_t enc_output3 = {0, 0};
    keymaster_blob_t dec_output1 = {0, 0};
    keymaster_blob_t dec_output2 = {0, 0};
    keymaster_blob_t dec_output3 = {0, 0};
    size_t input_consumed = 0;

    LOG_I("Using %u-bit tag ...", tagsize);

    assert(msg_part1_len + msg_part2_len <= 64);

    /* populate input message */
    memset(input, 7, sizeof(input));

    /* populate nonce */
    memset(nonce_bytes, 3, sizeof(nonce_bytes));
    nonce.data = &nonce_bytes[0];
    nonce.data_length = sizeof(nonce_bytes);

    /* populate AAD */
    memset(aad_bytes, 11, sizeof(aad_bytes));
    aad.data = &aad_bytes[0];
    aad.data_length = sizeof(aad_bytes);

    key_param[0].tag = KM_TAG_BLOCK_MODE;
    key_param[0].enumerated = KM_MODE_GCM;
    key_param[1].tag = KM_TAG_NONCE;
    key_param[1].blob = nonce;
    key_param[2].tag = KM_TAG_MAC_LENGTH;
    key_param[2].integer = tagsize;
    paramset.length = 3;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT,
        key_blob,
        &paramset,
        NULL, // no out_params
        &handle));

    input_blob.data = &input[0];
    input_blob.data_length = msg_part1_len;
    key_param[0].tag = KM_TAG_ASSOCIATED_DATA;
    key_param[0].blob = aad;
    paramset.length = 1;
    CHECK_RESULT_OK(device->update(device,
        handle,
        &paramset, // AAD
        &input_blob, // first half of message
        &input_consumed,
        NULL, // no out_params
        &enc_output1));
    CHECK_TRUE(input_consumed == msg_part1_len);

    input_blob.data = &input[msg_part1_len];
    input_blob.data_length = msg_part2_len;
    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &input_blob, // second half of message
        &input_consumed,
        NULL, // no params
        &enc_output2));
    CHECK_TRUE(input_consumed == msg_part2_len);

    CHECK_RESULT_OK(device->finish(device,
        handle,
        NULL, // no params
        NULL, // no signature
        NULL, // no out_params
        &enc_output3));
    CHECK_TRUE(enc_output1.data_length + enc_output2.data_length + enc_output3.data_length == msg_part1_len + msg_part2_len + tagsize/8);

    key_param[0].tag = KM_TAG_BLOCK_MODE;
    key_param[0].enumerated = KM_MODE_GCM;
    paramset.length = 3;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_DECRYPT,
        key_blob,
        &paramset, // as for 'begin encrypt' above
        NULL, // no out_params
        &handle));

    key_param[0].tag = KM_TAG_ASSOCIATED_DATA;
    key_param[0].blob = aad;
    paramset.length = 1;
    CHECK_RESULT_OK(device->update(device,
        handle,
        &paramset, // as for 'update encrypt (1)' above
        &enc_output1, // first half of ciphertext
        &input_consumed,
        NULL, // no out_params
        &dec_output1));
    CHECK_TRUE(input_consumed == enc_output1.data_length);

    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &enc_output2, // second half of ciphertext
        &input_consumed,
        NULL, // no out_params
        &dec_output2));
    CHECK_TRUE(input_consumed == enc_output2.data_length);

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output3, &input_consumed, NULL, &dec_output3));
    CHECK_TRUE(input_consumed == enc_output3.data_length);

    CHECK_RESULT_OK(device->finish(device,
        handle,
        NULL, // no params
        NULL, // no signature
        NULL, // no out_params
        NULL));

    CHECK_TRUE((dec_output1.data_length + dec_output2.data_length + dec_output3.data_length == msg_part1_len + msg_part2_len) &&
               (memcmp(&input[0], dec_output1.data, dec_output1.data_length) == 0) &&
               (memcmp(&input[dec_output1.data_length], dec_output2.data, dec_output2.data_length) == 0) &&
               (memcmp(&input[dec_output1.data_length + dec_output2.data_length], dec_output3.data, dec_output3.data_length) == 0));

end:
    km_free_blob(&enc_output1);
    km_free_blob(&enc_output2);
    km_free_blob(&enc_output3);
    km_free_blob(&dec_output1);
    km_free_blob(&dec_output2);
    km_free_blob(&dec_output3);

    return res;
}

/**
 * Update AAD twice, then update cipher twice, each time with a single byte.
 */
static keymaster_error_t test_km_aes_gcm_roundtrip_incr(
    keymaster1_device_t *device,
    const keymaster_key_blob_t *key_blob)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t param[4];
    keymaster_key_param_set_t paramset = {param, 0};
    keymaster_operation_handle_t handle;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[12];
    keymaster_blob_t aad;
    uint8_t aad_byte = 11;
    uint8_t input_byte = 7;
    keymaster_blob_t input_blob = {0, 0};
    keymaster_blob_t enc_output1 = {0, 0};
    keymaster_blob_t enc_output2 = {0, 0};
    keymaster_blob_t enc_output3 = {0, 0};
    keymaster_blob_t dec_output1 = {0, 0};
    keymaster_blob_t dec_output2 = {0, 0};
    keymaster_blob_t dec_output3 = {0, 0};
    size_t input_consumed = 0;

    /* populate input blob */
    input_blob.data = &input_byte;
    input_blob.data_length = 1;

    /* populate nonce */
    memset(nonce_bytes, 3, sizeof(nonce_bytes));
    nonce.data = &nonce_bytes[0];
    nonce.data_length = sizeof(nonce_bytes);

    /* populate AAD */
    aad.data = &aad_byte;
    aad.data_length = 1;

    /* begin encrypt */
    param[0].tag = KM_TAG_BLOCK_MODE;
    param[0].enumerated = KM_MODE_GCM;
    param[1].tag = KM_TAG_NONCE;
    param[1].blob = nonce;
    param[2].tag = KM_TAG_MAC_LENGTH;
    param[2].integer = 128;
    paramset.length = 3;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, &handle));

    /* update AAD (twice) */
    param[0].tag = KM_TAG_ASSOCIATED_DATA;
    param[0].blob = aad;
    paramset.length = 1;
    for (int i = 0; i < 2; i++) {
        CHECK_RESULT_OK(device->update(device,
            handle, &paramset, NULL, &input_consumed, NULL, NULL));
        CHECK_TRUE(input_consumed == 0);
    }

    /* update cipher (twice) */
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &input_blob, &input_consumed, NULL, &enc_output1));
    CHECK_TRUE(input_consumed == 1);
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &input_blob, &input_consumed, NULL, &enc_output2));
    CHECK_TRUE(input_consumed == 1);

    /* finish */
    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, &enc_output3));
    CHECK_TRUE(enc_output1.data_length + enc_output2.data_length + enc_output3.data_length == 2 + 16);

    /* begin decrypt */
    param[0].tag = KM_TAG_BLOCK_MODE;
    param[0].enumerated = KM_MODE_GCM;
    paramset.length = 3;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, &handle));

    /* update AAD (twice) */
    param[0].tag = KM_TAG_ASSOCIATED_DATA;
    param[0].blob = aad;
    paramset.length = 1;
    for (int i = 0; i < 2; i++) {
        CHECK_RESULT_OK(device->update(device,
            handle, &paramset, NULL, &input_consumed, NULL, NULL));
        CHECK_TRUE(input_consumed == 0);
    }

    /* update cipher (three times) */
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output1, &input_consumed, NULL, &dec_output1));
    CHECK_TRUE(input_consumed == enc_output1.data_length);
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output2, &input_consumed, NULL, &dec_output2));
    CHECK_TRUE(input_consumed == enc_output2.data_length);
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &enc_output3, &input_consumed, NULL, &dec_output3));
    CHECK_TRUE(input_consumed == enc_output3.data_length);

    /* finish */
    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, NULL));

end:
    km_free_blob(&enc_output1);
    km_free_blob(&enc_output2);
    km_free_blob(&enc_output3);
    km_free_blob(&dec_output1);
    km_free_blob(&dec_output2);
    km_free_blob(&dec_output3);

    return res;
}

keymaster_error_t test_km_aes_import(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[7];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_operation_handle_t handle;
    keymaster_blob_t enc_output1 = {0, 0};
    size_t input_consumed = 0;
    keymaster_blob_t client_id;
    uint8_t client_id_bytes[8];
    keymaster_blob_t app_data;
    uint8_t app_data_bytes[8];

    memset(client_id_bytes, 0x11, sizeof(client_id_bytes));
    client_id.data = client_id_bytes;
    client_id.data_length = sizeof(client_id_bytes);
    memset(app_data_bytes, 0x22, sizeof(app_data_bytes));
    app_data.data = app_data_bytes;
    app_data.data_length = sizeof(app_data_bytes);

    /* First NIST test vector from ECBVarKey128.rsp
       http://csrc.nist.gov/groups/STM/cavp/documents/aes/KAT_AES.zip
       KEY = 80000000000000000000000000000000
       PLAINTEXT = 00000000000000000000000000000000
       CIPHERTEXT = 0edd33d3c621e546455bd8ba1418bec8
     */

    const uint8_t key[16] = {
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t plaintext[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t ciphertext[16] = {
        0x0e, 0xdd, 0x33, 0xd3, 0xc6, 0x21, 0xe5, 0x46,
        0x45, 0x5b, 0xd8, 0xba, 0x14, 0x18, 0xbe, 0xc8};

    const keymaster_blob_t key_data = {key, sizeof(key)};
    const keymaster_blob_t plainblob = {plaintext, sizeof(plaintext)};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_AES;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 128;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_ENCRYPT;
    key_param[4].tag = KM_TAG_BLOCK_MODE;
    key_param[4].enumerated = KM_MODE_ECB;
    key_param[5].tag = KM_TAG_APPLICATION_ID;
    key_param[5].blob = client_id;
    key_param[6].tag = KM_TAG_APPLICATION_DATA;
    key_param[6].blob = app_data;
    paramset.length = 7;
    CHECK_RESULT_OK(device->import_key(device,
        &paramset,
        KM_KEY_FORMAT_RAW,
        &key_data,
        &key_blob,
        NULL));

    key_param[0].tag = KM_TAG_BLOCK_MODE;
    key_param[0].enumerated = KM_MODE_ECB;
    key_param[1].tag = KM_TAG_APPLICATION_ID;
    key_param[1].blob = client_id;
    key_param[2].tag = KM_TAG_APPLICATION_DATA;
    key_param[2].blob = app_data;
    paramset.length = 3;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_ENCRYPT,
        &key_blob,
        &paramset,
        NULL, // no out_params
        &handle));

    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &plainblob,
        &input_consumed,
        NULL, // no out_params
        &enc_output1));
    CHECK_TRUE(input_consumed == 16);
    CHECK_TRUE(enc_output1.data_length == 16);

    CHECK_RESULT_OK(device->finish(device,
        handle,
        NULL, // no params
        NULL, // no signature
        NULL, // no out_params
        NULL));

    CHECK_TRUE(memcmp(enc_output1.data, ciphertext, 16) == 0);

end:
    km_free_key_blob(&key_blob);
    km_free_blob(&enc_output1);

    return res;
}

keymaster_error_t test_km_aes_import_bad_length(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[5];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};

    const uint8_t key[16] = {
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    keymaster_blob_t key_data = {key, sizeof(key)};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_AES;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 128;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_ENCRYPT;
    key_param[4].tag = KM_TAG_BLOCK_MODE;
    key_param[4].enumerated = KM_MODE_ECB;
    paramset.length = 5;

    CHECK_RESULT_OK(device->import_key(device,
        &paramset, KM_KEY_FORMAT_RAW, &key_data, &key_blob, NULL));

    km_free_key_blob(&key_blob);
    key_data.data_length = 1;
    key_param[1].integer = 8;
    CHECK_RESULT(KM_ERROR_INVALID_ARGUMENT, device->import_key(device,
        &paramset, KM_KEY_FORMAT_RAW, &key_data, &key_blob, NULL));

    km_free_key_blob(&key_blob);
    key_data.data_length = 0;
    key_param[1].integer = 0;
    CHECK_RESULT(KM_ERROR_INVALID_ARGUMENT, device->import_key(device,
        &paramset, KM_KEY_FORMAT_RAW, &key_data, &key_blob, NULL));

end:
    km_free_key_blob(&key_blob);

    return res;
}

keymaster_error_t test_km_aes(
    keymaster1_device_t *device,
    uint32_t keysize)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[12];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t *pkey_characteristics = NULL;

    LOG_I("Generate %d-bit key ...", keysize);
    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_AES;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = keysize;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_ENCRYPT;
    key_param[4].tag = KM_TAG_PURPOSE;
    key_param[4].enumerated = KM_PURPOSE_DECRYPT;
    key_param[5].tag = KM_TAG_BLOCK_MODE;
    key_param[5].enumerated = KM_MODE_ECB;
    key_param[6].tag = KM_TAG_BLOCK_MODE;
    key_param[6].enumerated = KM_MODE_CBC;
    key_param[7].tag = KM_TAG_BLOCK_MODE;
    key_param[7].enumerated = KM_MODE_CTR;
    key_param[8].tag = KM_TAG_BLOCK_MODE;
    key_param[8].enumerated = KM_MODE_GCM;
    key_param[9].tag = KM_TAG_CALLER_NONCE;
    key_param[9].boolean = true;
    key_param[10].tag = KM_TAG_PADDING;
    key_param[10].enumerated = KM_PAD_PKCS7;
    key_param[11].tag = KM_TAG_MIN_MAC_LENGTH;
    key_param[11].integer = 96;
    paramset.length = 12;
    CHECK_RESULT_OK(device->generate_key(device,
        &paramset,
        &key_blob,
        NULL));

    LOG_I("Test key characteristics ...");
    CHECK_RESULT_OK(device->get_key_characteristics(device,
        &key_blob,
        NULL, // client_id
        NULL, // app_data
        &pkey_characteristics));
    print_characteristics(pkey_characteristics);

    LOG_I("ECB round trip ...");
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_ECB, 32, 32));

    LOG_I("CBC round trip ...");
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CBC, 32, 32));
    CHECK_RESULT_OK(test_km_aes_roundtrip_noncegen(device, &key_blob, KM_MODE_CBC));

    LOG_I("CTR round trip ...");
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 32, 32));
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 32, 31));
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 15, 17));
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 33, 16));
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 16, 1));
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 16, 17));
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 16, 33));
    CHECK_RESULT_OK(test_km_aes_roundtrip(device, &key_blob, KM_MODE_CTR, 15, 33));
    CHECK_RESULT_OK(test_km_aes_roundtrip_noncegen(device, &key_blob, KM_MODE_CTR));

    LOG_I("GCM round trip ...");
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(device, &key_blob, 96, 32, 32));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(device, &key_blob, 128, 32, 32));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(device, &key_blob, 96, 16, 2));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(device, &key_blob, 128, 13, 15));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip_noncegen(device, &key_blob));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip_incr(device, &key_blob));

    LOG_I("PKCS7 padding (ECB)...");
    CHECK_RESULT_OK(test_km_aes_pkcs7(device, &key_blob, KM_MODE_ECB, 3));
    CHECK_RESULT_OK(test_km_aes_pkcs7(device, &key_blob, KM_MODE_ECB, 16));
    CHECK_RESULT_OK(test_km_aes_pkcs7(device, &key_blob, KM_MODE_ECB, 37));

    LOG_I("PKCS7 padding (CBC)...");
    CHECK_RESULT_OK(test_km_aes_pkcs7(device, &key_blob, KM_MODE_CBC, 3));
    CHECK_RESULT_OK(test_km_aes_pkcs7(device, &key_blob, KM_MODE_CBC, 16));
    CHECK_RESULT_OK(test_km_aes_pkcs7(device, &key_blob, KM_MODE_CBC, 37));

end:
    keymaster_free_characteristics(pkey_characteristics);
    free(pkey_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}
