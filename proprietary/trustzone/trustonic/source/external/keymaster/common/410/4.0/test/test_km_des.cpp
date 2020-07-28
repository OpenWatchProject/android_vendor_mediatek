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

#include "TrustonicKeymaster4DeviceImpl.h"
#include <assert.h>

#include "test_km_des.h"
#include "test_km_util.h"

#include "log.h"

static keymaster_error_t test_km_des_roundtrip(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    keymaster_block_mode_t mode,
    size_t msg_part1_len,
    size_t msg_part2_len,
    size_t finish_len)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[2];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t enchandle = 0, dechandle = 0;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[8];
    uint8_t input[64];
    keymaster_blob_t input_blob = {0, 0}, output_blob = {0, 0};
    DECL_SGLIST(sgin, 1);
    DECL_SGLIST(sgout, 3);
    size_t input_consumed = 0;
    size_t off = 0;
    size_t i;

    assert(msg_part1_len + msg_part2_len + finish_len <= sizeof(input));

    /* populate input message */
    memset(input, 7, sizeof(input));
    ADD_TO_SGLIST(sgin, input, msg_part1_len + msg_part2_len + finish_len);

    /* populate nonce */
    memset(nonce_bytes, 3, 8);
    nonce.data = &nonce_bytes[0];
    nonce.data_length = 8;
    key_param[0].tag = KM_TAG_BLOCK_MODE;
    key_param[0].enumerated = mode;
    key_param[1].tag = KM_TAG_NONCE; // ignored for ECB
    key_param[1].blob = nonce;
    paramset.length = 2;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT,
        key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &enchandle));
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT,
        key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &dechandle));

    input_blob.data = input + off;
    input_blob.data_length = msg_part1_len;
    off += msg_part1_len;
    CHECK_RESULT_OK(impl->update(
        enchandle,
        NULL, // no params
        &input_blob, // first half of message
        NULL, NULL,
        &input_consumed,
        NULL, // no out_params
        &output_blob));
    CHECK_TRUE(input_consumed == input_blob.data_length);

    input_blob = output_blob;
    CHECK_RESULT_OK(impl->update(
        dechandle,
        NULL, // no params
        &input_blob, // first piece of ciphertext
        NULL, NULL,
        &input_consumed,
        NULL, // no out_params
        &output_blob));
    CHECK_TRUE(input_consumed == input_blob.data_length);
    ADD_BLOB_TO_SGLIST(sgout, output_blob); /* 1 */
    km_free_blob(&input_blob);

    input_blob.data = input + off;
    input_blob.data_length = msg_part2_len;
    off += msg_part2_len;
    CHECK_RESULT_OK(impl->update(
        enchandle,
        NULL, // no params
        &input_blob, // second half of message
        NULL, NULL,
        &input_consumed,
        NULL, // no out_params
        &output_blob));
    CHECK_TRUE(input_consumed == input_blob.data_length);

    input_blob = output_blob;
    CHECK_RESULT_OK(impl->update(
        dechandle,
        NULL, // no params
        &input_blob, // second piece of ciphertext
        NULL, NULL,
        &input_consumed,
        NULL, // no out_params
        &output_blob));
    CHECK_TRUE(input_consumed == input_blob.data_length);
    ADD_BLOB_TO_SGLIST(sgout, output_blob); /* 2 */
    km_free_blob(&input_blob);

    input_blob.data = input + off;
    input_blob.data_length = finish_len;
    off += finish_len;
    CHECK_RESULT_OK(impl->finish(
        enchandle,
        NULL, // no params
        &input_blob, // final input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &output_blob));
    enchandle = 0;

    input_blob = output_blob;
    CHECK_RESULT_OK(impl->finish(
        dechandle,
        NULL, // no params
        &input_blob, // final piece of cipertext
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &output_blob));
    ADD_BLOB_TO_SGLIST(sgout, output_blob); /* 3 */
    dechandle = 0;

    CHECK_TRUE(equal_sglists(sgin, n_sgin, sgout, n_sgout));

end:
    for (i = 0; i < n_sgout; i++) free(const_cast<void *>(sgout[i].p));
    if (enchandle) impl->abort( enchandle);
    if (dechandle) impl->abort( dechandle);
    return res;
}

static keymaster_error_t test_km_des_roundtrip_noncegen(
    TrustonicKeymaster4DeviceImpl *impl,
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
    uint8_t nonce_bytes[8];

    /* populate input message */
    memset(input, 7, sizeof(input));

    /* initialize nonce */
    nonce.data = nonce_bytes;
    nonce.data_length = 8;

    param[0].tag = KM_TAG_BLOCK_MODE;
    param[0].enumerated = mode;
    paramset.length = 1;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, &out_paramset, &handle));
    CHECK_TRUE(out_paramset.length == 1);
    CHECK_TRUE(out_paramset.params[0].tag == KM_TAG_NONCE);
    CHECK_TRUE(out_paramset.params[0].blob.data_length == 8);
    memcpy(nonce_bytes, out_paramset.params[0].blob.data, 8);

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &input_blob, NULL, NULL, &input_consumed, NULL, &enc_output));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(enc_output.data_length == sizeof(input));

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, NULL));

    // Try again supplying same nonce -- should get same ciphertext.
    param[1].tag = KM_TAG_NONCE;
    param[1].blob = nonce;
    paramset.length = 2;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &input_blob, NULL, NULL, &input_consumed, NULL, &enc_output1));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(enc_output1.data_length == sizeof(input));
    CHECK_TRUE(memcmp(enc_output.data, enc_output1.data, sizeof(input)) == 0);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, NULL));

    // And now decrypt
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output, NULL, NULL, &input_consumed, NULL, &dec_output));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(dec_output.data_length == sizeof(input));

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, NULL));

    CHECK_TRUE(memcmp(input, dec_output.data, sizeof(input)) == 0);

end:
    km_free_blob(&enc_output);
    km_free_blob(&enc_output1);
    km_free_blob(&dec_output);
    keymaster_free_param_set(&out_paramset);

    return res;
}

static keymaster_error_t test_km_des_pkcs7(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    keymaster_block_mode_t mode,
    size_t message_len)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[8];
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

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, NULL, &handle));

    input_blob.data = input;
    input_blob.data_length = message_len;
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &input_blob, NULL, NULL, &input_consumed, NULL, &enc_output));
    CHECK_TRUE(input_consumed == message_len);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &enc_output1));
    CHECK_TRUE((enc_output.data_length + enc_output1.data_length > message_len) &&
           (enc_output.data_length + enc_output1.data_length <= message_len + 8) &&
           ((enc_output.data_length + enc_output1.data_length) % 8 == 0));

    // key_params as for encrypt above
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output, NULL, NULL, &input_consumed, NULL, &dec_output));
    CHECK_TRUE(input_consumed == enc_output.data_length);

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output1, NULL, NULL, &input_consumed, NULL, &dec_output1));
    CHECK_TRUE(input_consumed == enc_output1.data_length);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &dec_output2));

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

keymaster_error_t test_km_des_import(
    TrustonicKeymaster4DeviceImpl *impl)
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

    /* "TECBMMT3 Encrypt 0" from NIST CAVP
       KEY = a2b5bc67da13dc92cd9d344aa238544a0e1fa79ef76810cd
       PLAINTEXT = 329d86bdf1bc5af4
       CIPHERTEXT = d946c2756d78633f
     */
    const uint8_t key[24] = {
        0xa2, 0xb5, 0xbc, 0x67, 0xda, 0x13, 0xdc, 0x92,
        0xcd, 0x9d, 0x34, 0x4a, 0xa2, 0x38, 0x54, 0x4a,
        0x0e, 0x1f, 0xa7, 0x9e, 0xf7, 0x68, 0x10, 0xcd};
    const uint8_t plaintext[8] = {
        0x32, 0x9d, 0x86, 0xbd, 0xf1, 0xbc, 0x5a, 0xf4};
    const uint8_t ciphertext[8] = {
        0xd9, 0x46, 0xc2, 0x75, 0x6d, 0x78, 0x63, 0x3f};

    const keymaster_blob_t key_data = {key, sizeof(key)};
    const keymaster_blob_t plainblob = {plaintext, sizeof(plaintext)};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_TRIPLE_DES;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 168;
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
    CHECK_RESULT_OK(impl->import_key(
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
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT,
        &key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &handle));

    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &plainblob,
        NULL, NULL,
        &input_consumed,
        NULL, // no out_params
        &enc_output1));
    CHECK_TRUE(input_consumed == 8);
    CHECK_TRUE(enc_output1.data_length == 8);

    CHECK_RESULT_OK(impl->finish(
        handle,
        NULL, // no params
        NULL, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        NULL));

    CHECK_TRUE(memcmp(enc_output1.data, ciphertext, 8) == 0);

end:
    km_free_key_blob(&key_blob);
    km_free_blob(&enc_output1);

    return res;
}

keymaster_error_t test_km_des_import_bad_length(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[5];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};

    const uint8_t key[24] = {
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    keymaster_blob_t key_data = {key, sizeof(key)};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_TRIPLE_DES;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 168;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_ENCRYPT;
    key_param[4].tag = KM_TAG_BLOCK_MODE;
    key_param[4].enumerated = KM_MODE_ECB;
    paramset.length = 5;

    CHECK_RESULT_OK(impl->import_key(
        &paramset, KM_KEY_FORMAT_RAW, &key_data, &key_blob, NULL));

    km_free_key_blob(&key_blob);
    key_data.data_length = 1;
    key_param[1].integer = 8;
    CHECK_RESULT(KM_ERROR_UNSUPPORTED_KEY_SIZE, impl->import_key(
        &paramset, KM_KEY_FORMAT_RAW, &key_data, &key_blob, NULL));

    km_free_key_blob(&key_blob);
    key_data.data_length = 0;
    key_param[1].integer = 0;
    CHECK_RESULT(KM_ERROR_UNSUPPORTED_KEY_SIZE, impl->import_key(
        &paramset, KM_KEY_FORMAT_RAW, &key_data, &key_blob, NULL));

end:
    km_free_key_blob(&key_blob);

    return res;
}

keymaster_error_t test_km_des(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[9];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t key_characteristics;

    memset(&key_characteristics, 0, sizeof(keymaster_key_characteristics_t));

    LOG_I("Generate key ...");
    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_TRIPLE_DES;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 168;
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
    key_param[7].tag = KM_TAG_CALLER_NONCE;
    key_param[7].boolean = true;
    key_param[8].tag = KM_TAG_PADDING;
    key_param[8].enumerated = KM_PAD_PKCS7;
    paramset.length = 9;
    CHECK_RESULT_OK(impl->generate_key(
        &paramset,
        &key_blob,
        NULL));

    LOG_I("Test key characteristics ...");
    CHECK_RESULT_OK(impl->get_key_characteristics(
        &key_blob,
        NULL, // client_id
        NULL, // app_data
        &key_characteristics));

    LOG_I("ECB round trip ...");
    CHECK_RESULT_OK(test_km_des_roundtrip(impl, &key_blob, KM_MODE_ECB, 32, 32, 0));
    CHECK_RESULT_OK(test_km_des_roundtrip(impl, &key_blob, KM_MODE_ECB, 21, 21, 22));

    LOG_I("CBC round trip ...");
    CHECK_RESULT_OK(test_km_des_roundtrip(impl, &key_blob, KM_MODE_CBC, 32, 32, 0));
    CHECK_RESULT_OK(test_km_des_roundtrip(impl, &key_blob, KM_MODE_CBC, 27, 37, 0));
    CHECK_RESULT_OK(test_km_des_roundtrip(impl, &key_blob, KM_MODE_CBC, 21, 21, 22));
    CHECK_RESULT_OK(test_km_des_roundtrip_noncegen(impl, &key_blob, KM_MODE_CBC));

    LOG_I("PKCS7 padding (ECB)...");
    CHECK_RESULT_OK(test_km_des_pkcs7(impl, &key_blob, KM_MODE_ECB, 3));
    CHECK_RESULT_OK(test_km_des_pkcs7(impl, &key_blob, KM_MODE_ECB, 16));
    CHECK_RESULT_OK(test_km_des_pkcs7(impl, &key_blob, KM_MODE_ECB, 37));

    LOG_I("PKCS7 padding (CBC)...");
    CHECK_RESULT_OK(test_km_des_pkcs7(impl, &key_blob, KM_MODE_CBC, 3));
    CHECK_RESULT_OK(test_km_des_pkcs7(impl, &key_blob, KM_MODE_CBC, 16));
    CHECK_RESULT_OK(test_km_des_pkcs7(impl, &key_blob, KM_MODE_CBC, 37));

end:
    keymaster_free_characteristics(&key_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}

