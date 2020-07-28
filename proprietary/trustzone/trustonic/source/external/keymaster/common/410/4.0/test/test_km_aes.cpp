/*
 * Copyright (c) 2015-2018 TRUSTONIC LIMITED
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

#include "test_km_aes.h"
#include "test_km_util.h"

#include "log.h"

static keymaster_error_t test_km_aes_roundtrip(
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
    uint8_t nonce_bytes[16];
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
    memset(nonce_bytes, 3, 16);
    nonce.data = &nonce_bytes[0];
    nonce.data_length = (mode == KM_MODE_GCM) ? 12 : 16;
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

static keymaster_error_t test_km_aes_roundtrip_noncegen(
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
    uint8_t nonce_bytes[16];

    /* populate input message */
    memset(input, 7, sizeof(input));

    /* initialize nonce */
    nonce.data = nonce_bytes;
    nonce.data_length = (mode == KM_MODE_GCM) ? 12 : 16;

    param[0].tag = KM_TAG_BLOCK_MODE;
    param[0].enumerated = mode;
    paramset.length = 1;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, &out_paramset, &handle));
    CHECK_TRUE(out_paramset.length == 1);
    CHECK_TRUE(out_paramset.params[0].tag == KM_TAG_NONCE);
    CHECK_TRUE(out_paramset.params[0].blob.data_length == 16);
    memcpy(nonce_bytes, out_paramset.params[0].blob.data, 16);

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

static keymaster_error_t test_km_aes_gcm_roundtrip_noncegen(
    TrustonicKeymaster4DeviceImpl *impl,
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
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, &out_paramset, &handle));
    CHECK_TRUE(out_paramset.length == 1);
    CHECK_TRUE(out_paramset.params[0].tag == KM_TAG_NONCE);
    CHECK_TRUE(out_paramset.params[0].blob.data_length == 12);
    memcpy(nonce_bytes, out_paramset.params[0].blob.data, 12);

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &input_blob, NULL, NULL, &input_consumed, NULL, &enc_output));
    CHECK_TRUE(input_consumed == sizeof(input));

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &tag));

    // Try again supplying same nonce -- should get same ciphertext.
    param[2].tag = KM_TAG_NONCE;
    param[2].blob = nonce;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &input_blob, NULL, NULL, &input_consumed, NULL, &enc_output1));
    CHECK_TRUE(input_consumed == sizeof(input));
    CHECK_TRUE(
        (enc_output.data_length == enc_output1.data_length) &&
        (memcmp(enc_output.data, enc_output1.data, enc_output.data_length) == 0));

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &tag1));
    CHECK_TRUE(
        (tag.data_length == tag1.data_length) &&
        (memcmp(tag.data, tag1.data, tag.data_length) == 0));

    // And now decrypt
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output, NULL, NULL, &input_consumed, NULL, &dec_output));
    CHECK_TRUE(input_consumed == sizeof(input));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &tag, NULL, NULL, &input_consumed, NULL, &dec_final_output));
    CHECK_TRUE(input_consumed == tag.data_length);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, NULL));

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
           (enc_output.data_length + enc_output1.data_length <= message_len + 16) &&
           ((enc_output.data_length + enc_output1.data_length) % 16 == 0));

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

static keymaster_error_t test_km_aes_gcm_roundtrip(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t tagsize,
    size_t msg_part1_len,
    size_t msg_part2_len,
    size_t finish_len)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[4];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t enchandle = 0, dechandle = 0;
    keymaster_blob_t nonce;
    uint8_t nonce_bytes[12];
    keymaster_blob_t aad;
    uint8_t aad_bytes[23];
    uint8_t input[64];
    keymaster_blob_t input_blob = {0, 0}, output_blob = {0, 0};
    DECL_SGLIST(sgin, 1);
    DECL_SGLIST(sgout, 3);
    size_t input_consumed = 0, off = 0, ctsz = 0, i = 0;

    LOG_I("Using %u-bit tag ...", tagsize);

    assert(msg_part1_len + msg_part2_len + finish_len <= sizeof(input));

    /* populate input message */
    memset(input, 7, sizeof(input));
    ADD_TO_SGLIST(sgin, input, msg_part1_len + msg_part2_len + finish_len);

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
        &paramset, // as for 'begin encrypt' above
        NULL,
        NULL, // no out_params
        &dechandle));

    input_blob.data = input + off;
    input_blob.data_length = msg_part1_len;
    off += msg_part1_len;
    key_param[0].tag = KM_TAG_ASSOCIATED_DATA;
    key_param[0].blob = aad;
    paramset.length = 1;
    CHECK_RESULT_OK(impl->update(
        enchandle,
        &paramset, // AAD
        &input_blob, // first half of message
        NULL, NULL,
        &input_consumed,
        NULL, // no out_params
        &output_blob));
    CHECK_TRUE(input_consumed == msg_part1_len);
    ctsz += output_blob.data_length;

    input_blob = output_blob;
    CHECK_RESULT_OK(impl->update(
        dechandle,
        &paramset, // as for 'update encrypt (1)' above
        &input_blob, // first half of ciphertext
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
        NULL, // no params
        &output_blob));
    CHECK_TRUE(input_consumed == msg_part2_len);
    ctsz += output_blob.data_length;

    input_blob = output_blob;
    CHECK_RESULT_OK(impl->update(
        dechandle,
        NULL, // no params
        &input_blob, // second half of ciphertext
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
        &input_blob, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &output_blob));
    ctsz += output_blob.data_length;
    CHECK_TRUE(ctsz == off + tagsize/8);
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

static keymaster_error_t test_km_aes_gcm_roundtrip_nodata(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t op_param[3];
    keymaster_key_param_set_t paramset = {op_param, 0};
    keymaster_key_param_set_t begin_out_params = {0, 0};
    keymaster_operation_handle_t enchandle = 0, dechandle = 0;
    keymaster_blob_t aad;
    uint8_t aad_bytes[16] = {'1','2','3','4','5','6','7','8',
                             '9','0','1','2','3','4','5','6'};
    keymaster_blob_t input_blob = {0, 0};
    keymaster_blob_t output_blob = {0, 0};

    /* populate AAD */
    aad.data = &aad_bytes[0];
    aad.data_length = sizeof(aad_bytes);

    /* begin encryption operation */
    op_param[0].tag = KM_TAG_BLOCK_MODE;
    op_param[0].enumerated = KM_MODE_GCM;
    op_param[1].tag = KM_TAG_MAC_LENGTH;
    op_param[1].integer = 128;
    paramset.length = 2;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT,
        key_blob,
        &paramset,
        NULL,
        &begin_out_params,
        &enchandle));

    /* finish encryption operation, supplying only AAD */
    op_param[0].tag = KM_TAG_ASSOCIATED_DATA;
    op_param[0].blob = aad;
    paramset.length = 1;
    CHECK_RESULT_OK(impl->finish(
        enchandle,
        &paramset, // just AAD
        NULL, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &output_blob));

    /* Find nonce is in begin_out_params */
    CHECK_TRUE(begin_out_params.length == 1);
    CHECK_TRUE(begin_out_params.params[0].tag == KM_TAG_NONCE);

    /* begin decryption operation */
    op_param[0].tag = KM_TAG_BLOCK_MODE;
    op_param[0].enumerated = KM_MODE_GCM;
    op_param[1].tag = KM_TAG_MAC_LENGTH;
    op_param[1].integer = 128;
    op_param[2].tag = KM_TAG_NONCE;
    op_param[2].blob = begin_out_params.params[0].blob;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT,
        key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &dechandle));

    /* finish decryption operation */
    op_param[0].tag = KM_TAG_ASSOCIATED_DATA;
    op_param[0].blob = aad;
    paramset.length = 1;
    input_blob = output_blob;
    CHECK_RESULT_OK(impl->finish(
        dechandle,
        &paramset, // just AAD
        &input_blob,
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &output_blob));

end:
    if (enchandle) impl->abort( enchandle);
    if (dechandle) impl->abort( dechandle);
    keymaster_free_param_set(&begin_out_params);
    km_free_blob(&output_blob);

    return res;
}

/**
 * Update AAD twice, then update cipher twice, each time with a single byte.
 */
static keymaster_error_t test_km_aes_gcm_roundtrip_incr(
    TrustonicKeymaster4DeviceImpl *impl,
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
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, NULL, &handle));

    /* update AAD (twice) */
    param[0].tag = KM_TAG_ASSOCIATED_DATA;
    param[0].blob = aad;
    paramset.length = 1;
    for (int i = 0; i < 2; i++) {
        CHECK_RESULT_OK(impl->update(
            handle, &paramset, NULL, NULL, NULL, &input_consumed, NULL, NULL));
        CHECK_TRUE(input_consumed == 0);
    }

    /* update cipher (twice) */
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &input_blob, NULL, NULL, &input_consumed, NULL, &enc_output1));
    CHECK_TRUE(input_consumed == 1);
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &input_blob, NULL, NULL, &input_consumed, NULL, &enc_output2));
    CHECK_TRUE(input_consumed == 1);

    /* finish */
    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &enc_output3));
    CHECK_TRUE(enc_output1.data_length + enc_output2.data_length + enc_output3.data_length == 2 + 16);

    /* begin decrypt */
    param[0].tag = KM_TAG_BLOCK_MODE;
    param[0].enumerated = KM_MODE_GCM;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, NULL, &handle));

    /* update AAD (twice) */
    param[0].tag = KM_TAG_ASSOCIATED_DATA;
    param[0].blob = aad;
    paramset.length = 1;
    for (int i = 0; i < 2; i++) {
        CHECK_RESULT_OK(impl->update(
            handle, &paramset, NULL, NULL, NULL, &input_consumed, NULL, NULL));
        CHECK_TRUE(input_consumed == 0);
    }

    /* update cipher (three times) */
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output1, NULL, NULL, &input_consumed, NULL, &dec_output1));
    CHECK_TRUE(input_consumed == enc_output1.data_length);
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output2, NULL, NULL, &input_consumed, NULL, &dec_output2));
    CHECK_TRUE(input_consumed == enc_output2.data_length);
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output3, NULL, NULL, &input_consumed, NULL, &dec_output3));
    CHECK_TRUE(input_consumed == enc_output3.data_length);

    /* finish */
    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, NULL));

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
    CHECK_TRUE(input_consumed == 16);
    CHECK_TRUE(enc_output1.data_length == 16);

    CHECK_RESULT_OK(impl->finish(
        handle,
        NULL, // no params
        NULL, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        NULL));

    CHECK_TRUE(memcmp(enc_output1.data, ciphertext, 16) == 0);

end:
    km_free_key_blob(&key_blob);
    km_free_blob(&enc_output1);

    return res;
}

keymaster_error_t test_km_aes_import_bad_length(
    TrustonicKeymaster4DeviceImpl *impl)
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

keymaster_error_t test_km_aes(
    TrustonicKeymaster4DeviceImpl *impl,
    uint32_t keysize)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[12];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t key_characteristics;

    memset(&key_characteristics, 0, sizeof(keymaster_key_characteristics_t));

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
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_ECB, 32, 32, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_ECB, 21, 21, 22));

    LOG_I("CBC round trip ...");
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CBC, 32, 32, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CBC, 27, 37, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CBC, 21, 21, 22));
    CHECK_RESULT_OK(test_km_aes_roundtrip_noncegen(impl, &key_blob, KM_MODE_CBC));

    LOG_I("CTR round trip ...");
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 32, 32, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 32, 31, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 15, 17, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 15, 0, 17));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 33, 16, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 16, 1, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 16, 0, 1));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 16, 17, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 16, 33, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 16, 0, 33));
    CHECK_RESULT_OK(test_km_aes_roundtrip(impl, &key_blob, KM_MODE_CTR, 15, 33, 0));
    CHECK_RESULT_OK(test_km_aes_roundtrip_noncegen(impl, &key_blob, KM_MODE_CTR));

    LOG_I("GCM round trip ...");
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 96, 32, 32, 0));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 128, 32, 32, 0));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 96, 32, 16, 16));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 128, 32, 16, 16));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 96, 16, 2, 0));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 128, 13, 15, 0));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 96, 16, 2, 5));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip(impl, &key_blob, 128, 21, 21, 17));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip_nodata(impl, &key_blob));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip_noncegen(impl, &key_blob));
    CHECK_RESULT_OK(test_km_aes_gcm_roundtrip_incr(impl, &key_blob));

    LOG_I("PKCS7 padding (ECB)...");
    CHECK_RESULT_OK(test_km_aes_pkcs7(impl, &key_blob, KM_MODE_ECB, 3));
    CHECK_RESULT_OK(test_km_aes_pkcs7(impl, &key_blob, KM_MODE_ECB, 16));
    CHECK_RESULT_OK(test_km_aes_pkcs7(impl, &key_blob, KM_MODE_ECB, 37));

    LOG_I("PKCS7 padding (CBC)...");
    CHECK_RESULT_OK(test_km_aes_pkcs7(impl, &key_blob, KM_MODE_CBC, 3));
    CHECK_RESULT_OK(test_km_aes_pkcs7(impl, &key_blob, KM_MODE_CBC, 16));
    CHECK_RESULT_OK(test_km_aes_pkcs7(impl, &key_blob, KM_MODE_CBC, 37));

end:
    keymaster_free_characteristics(&key_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}
