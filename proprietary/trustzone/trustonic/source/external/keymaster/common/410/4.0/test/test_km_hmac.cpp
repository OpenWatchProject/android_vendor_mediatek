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

#include "test_km_hmac.h"
#include "test_km_util.h"

#include "log.h"

static keymaster_error_t test_km_hmac_sign_verify(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    keymaster_digest_t digest)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[4];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message[20];
    keymaster_blob_t message_blob = {0, 0};
    keymaster_blob_t tag = {0, 0};
    size_t message_consumed = 0;

    uint32_t tagsize = BYTES_PER_BITS(block_length(digest));

    /* populate message */
    memset(message, 7, sizeof(message));

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_HMAC;
    key_param[1].tag = KM_TAG_PADDING; // ignored for HMAC
    key_param[1].enumerated = KM_PAD_PKCS7;
    key_param[2].tag = KM_TAG_DIGEST;
    key_param[2].enumerated = digest;
    key_param[3].tag = KM_TAG_MAC_LENGTH;
    key_param[3].integer = block_length(digest);
    paramset.length = 4;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN,
        key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &handle));

    message_blob.data = &message[0];
    message_blob.data_length = 11;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &message_blob, // first part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 11);

    message_blob.data = &message[11];
    message_blob.data_length = 9;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &message_blob, // last part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 9);

    CHECK_RESULT_OK(impl->finish(
        handle,
        NULL, // no params
        NULL, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &tag));
    CHECK_TRUE(tag.data_length == tagsize);

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_VERIFY,
        key_blob,
        &paramset, // same as for KM_PURPOSE_SIGN
        NULL,
        NULL, // no out_params
        &handle));

    message_blob.data = &message[0];
    message_blob.data_length = 10;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &message_blob, // first part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 10);

    message_blob.data = &message[10];
    message_blob.data_length = 10;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &message_blob, // last part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 10);

    CHECK_RESULT_OK(impl->finish(
        handle,
        NULL, // no params
        NULL, // no input
        &tag,
        NULL, NULL,
        NULL, // no out_params
        NULL));

end:
    km_free_blob(&tag);

    return res;
}

keymaster_error_t test_km_hmac_import(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[5];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_operation_handle_t handle;
    keymaster_blob_t tag = {0, 0};
    size_t message_consumed = 0;

    /*  NIST test vector from HMAC.rsp
        http://csrc.nist.gov/groups/STM/cavp/documents/mac/hmactestvectors.zip
        [L = 28]
        Count = 280
        Klen = 66
        Tlen = 24
        Key = 056d8a62bdb898b39cdf10c24d7787e0a8853b647a37f7fe84d23748c6ec8d9efddb43306a6d4255d5da3223d2f6e33c43abecea1ea7926f7052eb96a203438efb73
        Msg = ea9222041a580bdb27be76069fa60aa4f93f8e6ed6344e908623c1b8ce506a6ce89bfca0ccafc188ae7d3cabc8e90e3959c2169eeef8dc57e00930041ebd0ebf2c13c5ad6c7b58d29d45252aa15ac4f5832a3252b8e52f0fa5eee4c0628dc90ebee4c65283249963fb0077abb262f6817e5d2ab3bd640e61deb9261223276301
        Mac = e9bc98284c402d5e72d48253f643a27321172f8e70a726ea
     */

    const uint8_t key[66] = {
        0x05, 0x6d, 0x8a, 0x62, 0xbd, 0xb8, 0x98, 0xb3,
        0x9c, 0xdf, 0x10, 0xc2, 0x4d, 0x77, 0x87, 0xe0,
        0xa8, 0x85, 0x3b, 0x64, 0x7a, 0x37, 0xf7, 0xfe,
        0x84, 0xd2, 0x37, 0x48, 0xc6, 0xec, 0x8d, 0x9e,
        0xfd, 0xdb, 0x43, 0x30, 0x6a, 0x6d, 0x42, 0x55,
        0xd5, 0xda, 0x32, 0x23, 0xd2, 0xf6, 0xe3, 0x3c,
        0x43, 0xab, 0xec, 0xea, 0x1e, 0xa7, 0x92, 0x6f,
        0x70, 0x52, 0xeb, 0x96, 0xa2, 0x03, 0x43, 0x8e,
        0xfb, 0x73};
    const uint8_t message[128] = {
        0xea, 0x92, 0x22, 0x04, 0x1a, 0x58, 0x0b, 0xdb,
        0x27, 0xbe, 0x76, 0x06, 0x9f, 0xa6, 0x0a, 0xa4,
        0xf9, 0x3f, 0x8e, 0x6e, 0xd6, 0x34, 0x4e, 0x90,
        0x86, 0x23, 0xc1, 0xb8, 0xce, 0x50, 0x6a, 0x6c,
        0xe8, 0x9b, 0xfc, 0xa0, 0xcc, 0xaf, 0xc1, 0x88,
        0xae, 0x7d, 0x3c, 0xab, 0xc8, 0xe9, 0x0e, 0x39,
        0x59, 0xc2, 0x16, 0x9e, 0xee, 0xf8, 0xdc, 0x57,
        0xe0, 0x09, 0x30, 0x04, 0x1e, 0xbd, 0x0e, 0xbf,
        0x2c, 0x13, 0xc5, 0xad, 0x6c, 0x7b, 0x58, 0xd2,
        0x9d, 0x45, 0x25, 0x2a, 0xa1, 0x5a, 0xc4, 0xf5,
        0x83, 0x2a, 0x32, 0x52, 0xb8, 0xe5, 0x2f, 0x0f,
        0xa5, 0xee, 0xe4, 0xc0, 0x62, 0x8d, 0xc9, 0x0e,
        0xbe, 0xe4, 0xc6, 0x52, 0x83, 0x24, 0x99, 0x63,
        0xfb, 0x00, 0x77, 0xab, 0xb2, 0x62, 0xf6, 0x81,
        0x7e, 0x5d, 0x2a, 0xb3, 0xbd, 0x64, 0x0e, 0x61,
        0xde, 0xb9, 0x26, 0x12, 0x23, 0x27, 0x63, 0x01};
    const uint8_t mac[24] = {
        0xe9, 0xbc, 0x98, 0x28, 0x4c, 0x40, 0x2d, 0x5e,
        0x72, 0xd4, 0x82, 0x53, 0xf6, 0x43, 0xa2, 0x73,
        0x21, 0x17, 0x2f, 0x8e, 0x70, 0xa7, 0x26, 0xea};

    const keymaster_blob_t key_data = {key, sizeof(key)};
    const keymaster_blob_t messageblob = {message, sizeof(message)};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_HMAC;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 66*8;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_SIGN;
    key_param[4].tag = KM_TAG_DIGEST;
    key_param[4].enumerated = KM_DIGEST_SHA_2_224;
    paramset.length = 5;
    CHECK_RESULT_OK(impl->import_key(
        &paramset,
        KM_KEY_FORMAT_RAW,
        &key_data,
        &key_blob,
        NULL));

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_HMAC;
    key_param[1].tag = KM_TAG_DIGEST;
    key_param[1].enumerated = KM_DIGEST_SHA_2_224;
    key_param[2].tag = KM_TAG_MAC_LENGTH;
    key_param[2].integer = 24*8;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN,
        &key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &handle));

    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &messageblob,
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 128);

    CHECK_RESULT_OK(impl->finish(
        handle,
        NULL, // no params
        NULL, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &tag));
    CHECK_TRUE(tag.data_length == 24);

    CHECK_TRUE(memcmp(tag.data, mac, 24) == 0);

end:
    km_free_key_blob(&key_blob);
    km_free_blob(&tag);

    return res;
}

keymaster_error_t test_km_hmac(
    TrustonicKeymaster4DeviceImpl *impl,
    keymaster_digest_t digest,
    uint32_t keysize)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[7];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t key_characteristics;

    memset(&key_characteristics, 0, sizeof(keymaster_key_characteristics_t));

    LOG_I("Generate %u-bit key ...", keysize);
    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_HMAC;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = keysize;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_SIGN;
    key_param[4].tag = KM_TAG_PURPOSE;
    key_param[4].enumerated = KM_PURPOSE_VERIFY;
    key_param[5].tag = KM_TAG_DIGEST;
    key_param[5].enumerated = digest;
    key_param[6].tag = KM_TAG_MIN_MAC_LENGTH;
    key_param[6].integer = 64;
    paramset.length = 7;
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

    LOG_I("Sign and verify ...");
    CHECK_RESULT_OK(test_km_hmac_sign_verify(impl, &key_blob, digest));

end:
    keymaster_free_characteristics(&key_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}
