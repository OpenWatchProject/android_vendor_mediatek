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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef SOTER
#include "mtk_keymaster1.h"
#else
#include <hardware/keymaster1.h>
#endif

#include "test_km_ec.h"
#include "test_km_util.h"

#include <assert.h>

#undef  LOG_ANDROID
#undef  LOG_TAG
#define LOG_TAG "TlcTeeKeyMasterTest"
#include "log.h"

static keymaster_error_t test_km_ec_sign_verify(
    keymaster1_device_t *device,
    keymaster_key_blob_t *key_blob,
    uint32_t keylen,
    keymaster_digest_t digest)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[2];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message[1000];
    keymaster_blob_t messageblob = {0, 0};
    keymaster_blob_t sig_output = {0, 0};
    size_t message_consumed = 0;

    /* populate message */
    memset(message, 25, sizeof(message));

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_EC;
    key_param[1].tag = KM_TAG_DIGEST;
    key_param[1].enumerated = digest;
    paramset.length = 2;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_SIGN,
        key_blob,
        &paramset,
        NULL, // no out_params
        &handle));

    messageblob.data = &message[0];
    messageblob.data_length = 630;
    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &messageblob, // first part of message
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 630);

    messageblob.data = &message[630];
    messageblob.data_length = 370;
    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &messageblob, // last part of message
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 370);

    CHECK_RESULT_OK(device->finish(device,
        handle,
        NULL, // no params
        NULL, // no signature
        NULL, // no out_params
        &sig_output));
    CHECK_TRUE(sig_output.data_length <= 2*keylen + 9);

    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_VERIFY,
        key_blob,
        &paramset, // same as for KM_PURPOSE_SIGN
        NULL, // no out_params
        &handle));

    messageblob.data = &message[0];
    messageblob.data_length = 480;
    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &messageblob, // first part of message
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 480);

    messageblob.data = &message[480];
    messageblob.data_length = 520;
    CHECK_RESULT_OK(device->update(device,
        handle,
        NULL, // no params
        &messageblob, // last part of message
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 520);

    CHECK_RESULT_OK(device->finish(device,
        handle,
        NULL, // no params
        &sig_output,
        NULL, // no out_params
        NULL));

end:
    km_free_blob(&sig_output);

    return res;
}

static keymaster_error_t test_km_ec_sign_verify_empty_message(
    keymaster1_device_t *device,
    keymaster_key_blob_t *key_blob,
    uint32_t keylen,
    keymaster_digest_t digest)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[2];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message;
    keymaster_blob_t messageblob = {0, 0};
    keymaster_blob_t sig_output = {0, 0};
    size_t message_consumed = 0;

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_EC;
    key_param[1].tag = KM_TAG_DIGEST;
    key_param[1].enumerated = digest;
    paramset.length = 2;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_SIGN, key_blob, &paramset, NULL, &handle));

    messageblob.data = &message;
    messageblob.data_length = 0;
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &messageblob, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == 0);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, &sig_output));
    CHECK_TRUE(sig_output.data_length <= 2*keylen + 9);

    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_VERIFY, key_blob, &paramset, NULL, &handle));

    messageblob.data = &message;
    messageblob.data_length = 0;
    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &messageblob, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == 0);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, &sig_output, NULL, NULL));

end:
    km_free_blob(&sig_output);

    return res;
}

keymaster_error_t test_km_ec_import_export(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[6];
    keymaster_key_param_set_t key_paramset = {key_param, 6};
    keymaster_key_blob_t key_blob = {NULL, 0};

    /* Test vector from Android M PDK: system/keymaster/ec_privkey_pk8.der */
    const uint8_t key[] = {
        0x30, 0x81, 0x87, 0x02, 0x01, 0x00, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86,
        0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
        0x03, 0x01, 0x07, 0x04, 0x6d, 0x30, 0x6b, 0x02, 0x01, 0x01, 0x04, 0x20,
        0x73, 0x7c, 0x2e, 0xcd, 0x7b, 0x8d, 0x19, 0x40, 0xbf, 0x29, 0x30, 0xaa,
        0x9b, 0x4e, 0xd3, 0xff, 0x94, 0x1e, 0xed, 0x09, 0x36, 0x6b, 0xc0, 0x32,
        0x99, 0x98, 0x64, 0x81, 0xf3, 0xa4, 0xd8, 0x59, 0xa1, 0x44, 0x03, 0x42,
        0x00, 0x04, 0xbf, 0x85, 0xd7, 0x72, 0x0d, 0x07, 0xc2, 0x54, 0x61, 0x68,
        0x3b, 0xc6, 0x48, 0xb4, 0x77, 0x8a, 0x9a, 0x14, 0xdd, 0x8a, 0x02, 0x4e,
        0x3b, 0xdd, 0x8c, 0x7d, 0xdd, 0x9a, 0xb2, 0xb5, 0x28, 0xbb, 0xc7, 0xaa,
        0x1b, 0x51, 0xf1, 0x4e, 0xbb, 0xbb, 0x0b, 0xd0, 0xce, 0x21, 0xbc, 0xc4,
        0x1c, 0x6e, 0xb0, 0x00, 0x83, 0xcf, 0x33, 0x76, 0xd1, 0x1f, 0xd4, 0x49,
        0x49, 0xe0, 0xb2, 0x18, 0x3b, 0xfe};

    const keymaster_blob_t key_data = {key, sizeof(key)};

    keymaster_blob_t export_data = {NULL, 0};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_EC;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 256;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_SIGN;
    key_param[4].tag = KM_TAG_PURPOSE;
    key_param[4].enumerated = KM_PURPOSE_VERIFY;
    key_param[5].tag = KM_TAG_DIGEST;
    key_param[5].enumerated = KM_DIGEST_SHA1;
    CHECK_RESULT_OK(device->import_key(device,
        &key_paramset, KM_KEY_FORMAT_PKCS8, &key_data, &key_blob, NULL));

    CHECK_RESULT_OK(test_km_ec_sign_verify(
        device, &key_blob, 32, KM_DIGEST_SHA1));

    CHECK_RESULT_OK(device->export_key(device,
        KM_KEY_FORMAT_X509, &key_blob, NULL, NULL, &export_data));

end:
    km_free_blob(&export_data);
    km_free_key_blob(&key_blob);

    return res;
}

keymaster_error_t test_km_ec_p521_interop(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[6];
    keymaster_key_param_set_t key_paramset = {key_param, 6};
    keymaster_key_param_t op_param[2];
    keymaster_key_param_set_t op_paramset = {op_param, 2};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_operation_handle_t handle;
    const char *message = "This is a test";
    keymaster_blob_t messageblob = {(const uint8_t*)message, strlen(message)};
    size_t message_consumed = 0;
    keymaster_blob_t sig = {0, 0};

    /* Test vector from Android M CTS: ec_key6_secp521r1_pkcs8.der */
    const uint8_t key[] = {
        0x30, 0x81, 0xED, 0x02, 0x01, 0x00, 0x30, 0x10, 0x06, 0x07, 0x2A, 0x86,
        0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x23,
        0x04, 0x81, 0xD5, 0x30, 0x81, 0xD2, 0x02, 0x01, 0x01, 0x04, 0x41, 0x11,
        0x45, 0x8C, 0x58, 0x6D, 0xB5, 0xDA, 0xA9, 0x2A, 0xFA, 0xB0, 0x3F, 0x4F,
        0xE4, 0x6A, 0xA9, 0xD9, 0xC3, 0xCE, 0x9A, 0x9B, 0x7A, 0x00, 0x6A, 0x83,
        0x84, 0xBE, 0xC4, 0xC7, 0x8E, 0x8E, 0x9D, 0x18, 0xD7, 0xD0, 0x8B, 0x5B,
        0xCF, 0xA0, 0xE5, 0x3C, 0x75, 0xB0, 0x64, 0xAD, 0x51, 0xC4, 0x49, 0xBA,
        0xE0, 0x25, 0x8D, 0x54, 0xB9, 0x4B, 0x1E, 0x88, 0x5D, 0xED, 0x08, 0xED,
        0x4F, 0xB2, 0x5C, 0xE9, 0xA1, 0x81, 0x89, 0x03, 0x81, 0x86, 0x00, 0x04,
        0x01, 0x49, 0xEC, 0x11, 0xC6, 0xDF, 0x0F, 0xA1, 0x22, 0xC6, 0xA9, 0xAF,
        0xD9, 0x75, 0x4A, 0x4F, 0xA9, 0x51, 0x3A, 0x62, 0x7C, 0xA3, 0x29, 0xE3,
        0x49, 0x53, 0x5A, 0x56, 0x29, 0x87, 0x5A, 0x8A, 0xDF, 0xBE, 0x27, 0xDC,
        0xB9, 0x32, 0xC0, 0x51, 0x98, 0x63, 0x77, 0x10, 0x8D, 0x05, 0x4C, 0x28,
        0xC6, 0xF3, 0x9B, 0x6F, 0x2C, 0x9A, 0xF8, 0x18, 0x02, 0xF9, 0xF3, 0x26,
        0xB8, 0x42, 0xFF, 0x2E, 0x5F, 0x3C, 0x00, 0xAB, 0x76, 0x35, 0xCF, 0xB3,
        0x61, 0x57, 0xFC, 0x08, 0x82, 0xD5, 0x74, 0xA1, 0x0D, 0x83, 0x9C, 0x1A,
        0x0C, 0x04, 0x9D, 0xC5, 0xE0, 0xD7, 0x75, 0xE2, 0xEE, 0x50, 0x67, 0x1A,
        0x20, 0x84, 0x31, 0xBB, 0x45, 0xE7, 0x8E, 0x70, 0xBE, 0xFE, 0x93, 0x0D,
        0xB3, 0x48, 0x18, 0xEE, 0x4D, 0x5C, 0x26, 0x25, 0x9F, 0x5C, 0x6B, 0x8E,
        0x28, 0xA6, 0x52, 0x95, 0x0F, 0x9F, 0x88, 0xD7, 0xB4, 0xB2, 0xC9, 0xD9};

    /* Signature generated by Bouncy Castle in CTS run */
    const uint8_t bc_sig[138] = {
        0x30, 0x81, 0x87, 0x02, 0x42, 0x01, 0x90, 0xef, 0xd1, 0x43, 0x8c, 0xf7,
        0xd2, 0x73, 0x19, 0x46, 0x09, 0xce, 0x74, 0x82, 0x10, 0x08, 0x3c, 0xb9,
        0x65, 0x1e, 0x04, 0xde, 0x34, 0x9d, 0x8f, 0x66, 0x2b, 0xf7, 0x2e, 0x62,
        0x10, 0xf4, 0x9c, 0xf7, 0xf5, 0x71, 0x9e, 0x6a, 0x5a, 0xeb, 0xce, 0xb7,
        0x28, 0xd8, 0xd5, 0xd1, 0x1e, 0x7f, 0x8b, 0x85, 0x29, 0xd7, 0x04, 0x88,
        0x2a, 0xf1, 0x99, 0x50, 0x71, 0x78, 0xd0, 0x14, 0xa9, 0x93, 0x68, 0x02,
        0x41, 0x5c, 0xc9, 0x15, 0xcc, 0xd2, 0x12, 0xf3, 0xdb, 0x42, 0x8a, 0x07,
        0x51, 0x85, 0x8b, 0xb9, 0x98, 0xc9, 0x27, 0xc1, 0x77, 0xb0, 0x7b, 0xdb,
        0xae, 0x5f, 0x5c, 0x42, 0xe8, 0x5e, 0x4b, 0x89, 0xed, 0x93, 0xba, 0x73,
        0x04, 0x98, 0x6a, 0xc7, 0x38, 0xb6, 0x9c, 0x6b, 0xa9, 0x13, 0x04, 0xc3,
        0x33, 0xaf, 0xcb, 0x85, 0x8d, 0xfa, 0x4a, 0x33, 0x8a, 0x09, 0xfc, 0xdb,
        0x5d, 0x3f, 0xcf, 0x49, 0x3a, 0x12};

    const keymaster_blob_t key_data = {key, sizeof(key)};
    const keymaster_blob_t bc_sig_blob = {bc_sig, sizeof(bc_sig)};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_EC;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 521;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_SIGN;
    key_param[4].tag = KM_TAG_PURPOSE;
    key_param[4].enumerated = KM_PURPOSE_VERIFY;
    key_param[5].tag = KM_TAG_DIGEST;
    key_param[5].enumerated = KM_DIGEST_NONE;
    CHECK_RESULT_OK(device->import_key(device,
        &key_paramset, KM_KEY_FORMAT_PKCS8, &key_data, &key_blob, NULL));

    op_param[0].tag = KM_TAG_ALGORITHM;
    op_param[0].enumerated = KM_ALGORITHM_EC;
    op_param[1].tag = KM_TAG_DIGEST;
    op_param[1].enumerated = KM_DIGEST_NONE;

    /* 1. Generate a signature ourselves. */

    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_SIGN, &key_blob, &op_paramset, NULL, &handle));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &messageblob, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == messageblob.data_length);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, NULL, NULL, &sig));

    /* 2. Verify BC's signature. */

    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_VERIFY, &key_blob, &op_paramset, NULL, &handle));

    CHECK_RESULT_OK(device->update(device,
        handle, NULL, &messageblob, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == messageblob.data_length);

    CHECK_RESULT_OK(device->finish(device,
        handle, NULL, &bc_sig_blob, NULL, NULL));

end:
    km_free_key_blob(&key_blob);
    km_free_blob(&sig);

    return res;
}

keymaster_error_t test_km_ec(
    keymaster1_device_t *device,
    uint32_t keysize)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[8];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    uint32_t keylen = BYTES_PER_BITS(keysize);

    LOG_I("Generate %d-bit key...", keysize);
    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_EC;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = keysize;
    key_param[2].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[2].boolean = true;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_SIGN;
    key_param[4].tag = KM_TAG_PURPOSE;
    key_param[4].enumerated = KM_PURPOSE_VERIFY;
    key_param[5].tag = KM_TAG_DIGEST;
    key_param[5].enumerated = KM_DIGEST_SHA1;
    key_param[6].tag = KM_TAG_DIGEST;
    key_param[6].enumerated = KM_DIGEST_SHA_2_512;
    key_param[7].tag = KM_TAG_DIGEST;
    key_param[7].enumerated = KM_DIGEST_NONE;
    paramset.length = 8;
    CHECK_RESULT_OK(device->generate_key(device,
        &paramset, &key_blob, NULL));

    LOG_I("ECDSA sign/verify with SHA1 hash...");
    CHECK_RESULT_OK(test_km_ec_sign_verify(
        device, &key_blob, keylen, KM_DIGEST_SHA1) );

    LOG_I("ECDSA sign/verify with SHA512 hash...");
    CHECK_RESULT_OK(test_km_ec_sign_verify(
        device, &key_blob, keylen, KM_DIGEST_SHA_2_512) );

    LOG_I("ECDSA sign/verify with SHA1 hash, empty message...");
    CHECK_RESULT_OK(test_km_ec_sign_verify_empty_message(
        device, &key_blob, keylen, KM_DIGEST_SHA1) );

    LOG_I("ECDSA sign/verify with no digest, empty message...");
    CHECK_RESULT_OK(test_km_ec_sign_verify_empty_message(
        device, &key_blob, keylen, KM_DIGEST_NONE) );

end:
    km_free_key_blob(&key_blob);

    return res;
}
