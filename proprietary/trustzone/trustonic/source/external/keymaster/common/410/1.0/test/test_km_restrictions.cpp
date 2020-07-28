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

#include <assert.h>
#include <time.h>
#include <openssl/hmac.h>
#ifdef SOTER
#include "mtk_keymaster1.h"
#else
#include <hardware/keymaster1.h>
#endif
#include <hardware/hw_auth_token.h>
#include "test_km_restrictions.h"
#include "test_km_util.h"
#include "km_shared_util.h"

#undef  LOG_ANDROID
#undef  LOG_TAG
#define LOG_TAG "TlcTeeKeyMasterTest"
#include "log.h"

/**
 * Endianness of platform, initialized in \p main().
 */
bool am_big_endian;

#if 0
static uint32_t hton32(
    uint32_t a)
{
    if (am_big_endian) {
        return a;
    } else {
        uint32_t b = 0;
        for (int i = 0; i < 4; i++) {
            b |= ((a >> (8*i)) & 0xFF) << (8*(3-i));
        }
        return b;
    }
}

static uint64_t hton64(
    uint64_t a)
{
    if (am_big_endian) {
        return a;
    } else {
        uint64_t b = 0;
        for (int i = 0; i < 8; i++) {
            b |= ((a >> (8*i)) & 0xFF) << (8*(7-i));
        }
        return b;
    }
}

static void make_auth_token(
    hw_auth_token_t *auth_token,
    uint8_t version,
    uint64_t challenge,
    uint64_t user_id,
    uint32_t authenticator_type,
    bool old)
{
    assert(auth_token != NULL);
    auth_token->version = version;
    auth_token->challenge = challenge;
    auth_token->user_id = user_id;
    auth_token->authenticator_id = 0;
    auth_token->authenticator_type = hton32(authenticator_type);

    /* Timestamp */
    // FIXME gettimeofday() and tlApiGetSecureTimestamp() return completely
    // different things. We want secure time here.
    //
    // struct timeval tv;
    // gettimeofday(&tv, NULL);
    // auth_token->timestamp = hton64(tv.tv_sec * 1000000ull + tv.tv_usec);
    auth_token->timestamp = hton64(old ? 0ull : 0xFFFFFFFF00000000ull);

    /* HMAC using test key = [0x2A]*32. TODO integrated test with Gatekeeper. */
    uint8_t key[32];
    memset(key, 0x2A, 32);
    uint8_t data[37]; // size of packed hw_auth_token_t minus the HMAC
    uint8_t *pos;
    data[0] = auth_token->version;
    pos = data + 1;
    set_u64_increment_pos(&pos, auth_token->challenge);
    set_u64_increment_pos(&pos, auth_token->user_id);
    set_u64_increment_pos(&pos, auth_token->authenticator_id);
    set_u32_increment_pos(&pos, auth_token->authenticator_type);
    set_u64_increment_pos(&pos, auth_token->timestamp);
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    if (0 == HMAC_Init_ex(&ctx, key, 32, EVP_sha256(), NULL)) {
        assert(!"HMAC_Init_ex failed");
    }
    if (0 == HMAC_Update(&ctx, data, sizeof(data))) {
        assert(!"HMAC_Update failed");
    }
    unsigned int len = 32;
    if ((0 == HMAC_Final(&ctx, auth_token->hmac, &len)) || (len != 32)) {
        assert(!"HMAC_Final failed");
    }
    HMAC_CTX_cleanup(&ctx);
}

static void serialize_auth_token(
    uint8_t token_data[69],
    const hw_auth_token_t *auth_token)
{
    uint8_t *pos;
    token_data[0] = auth_token->version;
    pos = token_data + 1;
    set_u64_increment_pos(&pos, auth_token->challenge);
    set_u64_increment_pos(&pos, auth_token->user_id);
    set_u64_increment_pos(&pos, auth_token->authenticator_id);
    set_u32_increment_pos(&pos, auth_token->authenticator_type);
    set_u64_increment_pos(&pos, auth_token->timestamp);
    set_data_increment_pos(&pos, auth_token->hmac, 32);
}

static keymaster_error_t test_km_restrictions_auth_token(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t generate_param[7];
    keymaster_key_param_set_t generate_paramset = {generate_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t *pkey_characteristics = NULL;
    static const uint64_t user_id = 0x123456789abcdef0ull;
    hw_auth_token_t auth_token;
    uint8_t token_data[69];
    keymaster_blob_t token_blob = {token_data, 69};
    keymaster_key_param_t begin_param[4];
    keymaster_key_param_set_t begin_paramset = {begin_param, 0};
    keymaster_operation_handle_t handle;
    keymaster_key_param_set_t begin_out_paramset = {NULL, 0};
    uint8_t iv[16];
    keymaster_key_param_t update_param[1];
    keymaster_key_param_set_t update_paramset = {update_param, 0};
    uint8_t plaintext_data[32];
    keymaster_blob_t plaintext = {plaintext_data, 32};
    keymaster_blob_t ciphertext = {NULL, 0};
    size_t input_consumed = 0;
    keymaster_blob_t decrypted = {NULL, 0};

    LOG_I("Auth-token tests ...");

    LOG_I("Generate key ...");
    generate_param[0].tag = KM_TAG_ALGORITHM;
    generate_param[0].enumerated = KM_ALGORITHM_AES;
    generate_param[1].tag = KM_TAG_KEY_SIZE;
    generate_param[1].integer = 128;
    generate_param[2].tag = KM_TAG_PURPOSE;
    generate_param[2].enumerated = KM_PURPOSE_ENCRYPT;
    generate_param[3].tag = KM_TAG_PURPOSE;
    generate_param[3].enumerated = KM_PURPOSE_DECRYPT;
    generate_param[4].tag = KM_TAG_BLOCK_MODE;
    generate_param[4].enumerated = KM_MODE_CBC;
    generate_param[5].tag = KM_TAG_USER_SECURE_ID;
    generate_param[5].long_integer = user_id;
    generate_param[6].tag = KM_TAG_USER_AUTH_TYPE;
    generate_param[6].enumerated = HW_AUTH_PASSWORD;
    generate_paramset.length = 7;
    CHECK_RESULT_OK(device->generate_key(device,
        &generate_paramset, &key_blob, &pkey_characteristics));
    print_characteristics(pkey_characteristics);

    LOG_I("Begin encrypt ...");
    begin_param[0].tag = KM_TAG_BLOCK_MODE;
    begin_param[0].enumerated = KM_MODE_CBC;
    begin_param[1].tag = KM_TAG_AUTH_TOKEN;
    begin_param[1].blob = token_blob;
    begin_param[2].tag = KM_TAG_USER_SECURE_ID;
    begin_param[2].long_integer = user_id;
    begin_paramset.length = 3;

    LOG_I("unauthorized ...");
    /* Bad version */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION + 1, 0, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->begin(device, KM_PURPOSE_ENCRYPT, &key_blob,
            &begin_paramset, &begin_out_paramset, &handle));
    /* Bad user_id */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION , 0, user_id + 1,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->begin(device, KM_PURPOSE_ENCRYPT, &key_blob,
            &begin_paramset, &begin_out_paramset, &handle));
    /* Bad authenticator_type */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION , 0, user_id,
        HW_AUTH_FINGERPRINT, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->begin(device, KM_PURPOSE_ENCRYPT, &key_blob,
            &begin_paramset, &begin_out_paramset, &handle));
    /* Bad hmac */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION , 0, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    token_data[68] ^= 1;
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->begin(device, KM_PURPOSE_ENCRYPT, &key_blob,
            &begin_paramset, &begin_out_paramset, &handle));

    LOG_I("authorized ...");
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION, 0, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT_OK(device->begin(device, KM_PURPOSE_ENCRYPT, &key_blob,
        &begin_paramset, &begin_out_paramset, &handle));
    CHECK_TRUE(begin_out_paramset.length == 1);
    CHECK_TRUE(begin_out_paramset.params[0].tag == KM_TAG_NONCE);
    CHECK_TRUE(begin_out_paramset.params[0].blob.data_length == 16);
    memcpy(iv, begin_out_paramset.params[0].blob.data, 16);

    LOG_I("Update encrypt ...");
    update_param[0].tag = KM_TAG_AUTH_TOKEN;
    update_param[0].blob = token_blob;
    update_paramset.length = 1;
    memset(plaintext_data, 3, 32);

    LOG_I("unauthorized ...");
    /* Bad version */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION + 1, 0, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->update(device, handle, &update_paramset,
            &plaintext, &input_consumed, NULL, &ciphertext));
    /* Bad user_id */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION , 0, user_id + 1,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->update(device, handle, &update_paramset,
            &plaintext, &input_consumed, NULL, &ciphertext));
    /* Bad authenticator_type */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION , 0, user_id,
        HW_AUTH_FINGERPRINT, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->update(device, handle, &update_paramset,
            &plaintext, &input_consumed, NULL, &ciphertext));
    /* Bad hmac */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION , 0, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    token_data[68] ^= 1;
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->update(device, handle, &update_paramset,
            &plaintext, &input_consumed, NULL, &ciphertext));

    LOG_I("authorized ...");
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION, handle, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT_OK(device->update(device, handle, &update_paramset,
        &plaintext, &input_consumed, NULL, &ciphertext));
    CHECK_TRUE(input_consumed == 32);
    CHECK_TRUE(ciphertext.data_length == 32);

    LOG_I("Finish encrypt ...");
    /* Use the same params (i.e. just the auth token) */
    CHECK_RESULT_OK(device->finish(device, handle, &update_paramset,
        NULL, NULL, NULL));

    LOG_I("Begin decrypt ...");
    /* Use the same params as previously with the addition of the IV. Note that
     * this includes the auth_token used for the update() and finish() calls
     * above. This should be OK as the 'challenge' is ignored for a begin().
     */
    begin_param[3].tag = KM_TAG_NONCE;
    begin_param[3].blob.data_length = 16;
    begin_param[3].blob.data = iv;
    begin_paramset.length = 4;
    CHECK_RESULT_OK(device->begin(device, KM_PURPOSE_DECRYPT, &key_blob,
        &begin_paramset, NULL, &handle));

    LOG_I("Update decrypt ...");
    /* Need a new auth token with the correct 'challenge'. */
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION, handle, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT_OK(device->update(device, handle, &update_paramset,
        &ciphertext, &input_consumed, NULL, &decrypted));
    CHECK_TRUE(input_consumed == 32);
    CHECK_TRUE(decrypted.data_length == 32);

    LOG_I("Finish decrypt ...");
    /* Use the same params (i.e. just the auth token) */
    CHECK_RESULT_OK(device->finish(device, handle, &update_paramset,
        NULL, NULL, NULL));

    CHECK_TRUE(memcmp(plaintext.data, decrypted.data, 32) == 0);

end:
    keymaster_free_characteristics(pkey_characteristics);
    free(pkey_characteristics);
    km_free_key_blob(&key_blob);
    keymaster_free_param_set(&begin_out_paramset);
    km_free_blob(&ciphertext);
    km_free_blob(&decrypted);

    return res;
}

static keymaster_error_t test_km_restrictions_timeout(
    keymaster1_device_t *device)
{ // TODO
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t generate_param[8];
    keymaster_key_param_set_t generate_paramset = {generate_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t *pkey_characteristics = NULL;
    static const uint64_t user_id = 0x123456789abcdef0ull;
    hw_auth_token_t auth_token;
    uint8_t token_data[69];
    keymaster_blob_t token_blob = {token_data, 69};
    keymaster_key_param_t begin_param[4];
    keymaster_key_param_set_t begin_paramset = {begin_param, 0};
    keymaster_operation_handle_t handle;
    keymaster_operation_handle_t handle1;

    LOG_I("Timeout tests ...");

    LOG_I("Generate key (timeout = 3s) ...");
    generate_param[0].tag = KM_TAG_ALGORITHM;
    generate_param[0].enumerated = KM_ALGORITHM_AES;
    generate_param[1].tag = KM_TAG_KEY_SIZE;
    generate_param[1].integer = 128;
    generate_param[2].tag = KM_TAG_PURPOSE;
    generate_param[2].enumerated = KM_PURPOSE_ENCRYPT;
    generate_param[3].tag = KM_TAG_PURPOSE;
    generate_param[3].enumerated = KM_PURPOSE_DECRYPT;
    generate_param[4].tag = KM_TAG_BLOCK_MODE;
    generate_param[4].enumerated = KM_MODE_ECB;
    generate_param[5].tag = KM_TAG_USER_SECURE_ID;
    generate_param[5].long_integer = user_id;
    generate_param[6].tag = KM_TAG_USER_AUTH_TYPE;
    generate_param[6].enumerated = HW_AUTH_ANY;
    generate_param[7].tag = KM_TAG_AUTH_TIMEOUT;
    generate_param[7].integer = 3; // seconds
    generate_paramset.length = 8;
    CHECK_RESULT_OK(device->generate_key(device,
        &generate_paramset, &key_blob, &pkey_characteristics));
    print_characteristics(pkey_characteristics);

    LOG_I("Begin encrypt with new auth token ...");
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION, 0, user_id,
        HW_AUTH_PASSWORD, false);
    serialize_auth_token(token_data, &auth_token);
    begin_param[0].tag = KM_TAG_BLOCK_MODE;
    begin_param[0].enumerated = KM_MODE_ECB;
    begin_param[1].tag = KM_TAG_AUTH_TOKEN;
    begin_param[1].blob = token_blob;
    begin_param[2].tag = KM_TAG_USER_SECURE_ID;
    begin_param[2].long_integer = user_id;
    begin_paramset.length = 3;
    CHECK_RESULT_OK(device->begin(device, KM_PURPOSE_ENCRYPT, &key_blob,
        &begin_paramset, NULL, &handle));

    // FIXME We can't do this test until we have secure time in the auth token.
    // LOG_I("Sleep (4s) ...");
    // sleep(4);

    LOG_I("Begin encrypt with old auth token ...");
    make_auth_token(&auth_token, HW_AUTH_TOKEN_VERSION, 0, user_id,
        HW_AUTH_PASSWORD, true);
    serialize_auth_token(token_data, &auth_token);
    CHECK_RESULT(KM_ERROR_KEY_USER_NOT_AUTHENTICATED,
        device->begin(device, KM_PURPOSE_ENCRYPT, &key_blob,
            &begin_paramset, NULL, &handle1));

    CHECK_RESULT_OK(device->abort(device, handle));

end:
    keymaster_free_characteristics(pkey_characteristics);
    free(pkey_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}
#endif

static keymaster_error_t test_km_restrictions_app_id_and_data(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_params[7];
    keymaster_key_param_set_t generate_paramset = {key_params, 0};
    const uint8_t id_bytes[] = {1, 2, 3};
    const uint8_t data_bytes[] = {4, 5, 6, 7};
    keymaster_blob_t application_id = {id_bytes, sizeof(id_bytes)};
    keymaster_blob_t application_data = {data_bytes, sizeof(data_bytes)};
    const uint8_t wrong_id_bytes[] = {3, 2, 1};
    const uint8_t wrong_data_bytes[] = {4, 5, 6, 7, 8};
    keymaster_blob_t wrong_application_id = {wrong_id_bytes, sizeof(wrong_id_bytes)};
    keymaster_blob_t wrong_application_data = {wrong_data_bytes, sizeof(wrong_data_bytes)};
    keymaster_key_blob_t key_blob = {NULL, 0};
    keymaster_key_characteristics_t *pkey_characteristics = NULL;
    keymaster_blob_t export_data = {NULL, 0};
    keymaster_operation_handle_t handle;
    keymaster_key_param_t op_params[4];
    keymaster_key_param_set_t begin_paramset = {op_params, 0};

    /* generate key */
    key_params[0].tag = KM_TAG_ALGORITHM;
    key_params[0].enumerated = KM_ALGORITHM_EC;
    key_params[1].tag = KM_TAG_KEY_SIZE;
    key_params[1].integer = 256;
    key_params[2].tag = KM_TAG_PURPOSE;
    key_params[2].enumerated = KM_PURPOSE_SIGN;
    key_params[3].tag = KM_TAG_DIGEST;
    key_params[3].enumerated = KM_DIGEST_SHA_2_256;
    key_params[4].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_params[4].boolean = true;
    key_params[5].tag = KM_TAG_APPLICATION_ID;
    key_params[5].blob = application_id;
    key_params[6].tag = KM_TAG_APPLICATION_DATA;
    key_params[6].blob = application_data;
    generate_paramset.length = 7;
    CHECK_RESULT_OK(device->generate_key(device,
        &generate_paramset, &key_blob, NULL));

    /* get_key_characteristics (good case) */
    CHECK_RESULT_OK(device->get_key_characteristics(device,
        &key_blob, &application_id, &application_data, &pkey_characteristics));

    /* get_key_characteristics (bad cases) */
    keymaster_free_characteristics(pkey_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->get_key_characteristics(device,
            &key_blob, &application_id, NULL, &pkey_characteristics));
    keymaster_free_characteristics(pkey_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->get_key_characteristics(device,
            &key_blob, NULL, &application_data, &pkey_characteristics));
    keymaster_free_characteristics(pkey_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->get_key_characteristics(device,
            &key_blob, &application_id, &wrong_application_data, &pkey_characteristics));
    keymaster_free_characteristics(pkey_characteristics);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->get_key_characteristics(device,
            &key_blob, &wrong_application_id, &application_data, &pkey_characteristics));

    /* export_key (good case) */
    CHECK_RESULT_OK(device->export_key(device,
        KM_KEY_FORMAT_X509, &key_blob, &application_id, &application_data, &export_data));

    /* export_key (bad cases) */
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->export_key(device,
            KM_KEY_FORMAT_X509, &key_blob, &application_id, NULL, &export_data));
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->export_key(device,
            KM_KEY_FORMAT_X509, &key_blob, NULL, &application_data, &export_data));
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->export_key(device,
            KM_KEY_FORMAT_X509, &key_blob, &application_id, &wrong_application_data, &export_data));
    km_free_blob(&export_data);
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->export_key(device,
            KM_KEY_FORMAT_X509, &key_blob, &wrong_application_id, &application_data, &export_data));

    /* begin (good case) */
    op_params[0].tag = KM_TAG_ALGORITHM;
    op_params[0].enumerated = KM_ALGORITHM_EC;
    op_params[1].tag = KM_TAG_DIGEST;
    op_params[1].enumerated = KM_DIGEST_SHA_2_256;
    op_params[2].tag = KM_TAG_APPLICATION_ID;
    op_params[2].blob = application_id;
    op_params[3].tag = KM_TAG_APPLICATION_DATA;
    op_params[3].blob = application_data;
    begin_paramset.length = 4;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, &handle));
    CHECK_RESULT_OK(device->abort(device,
        handle));

    /* begin (bad cases) */
    begin_paramset.length = 3;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->begin(device,
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, &handle));
    op_params[2].tag = KM_TAG_APPLICATION_DATA;
    op_params[2].blob = application_data;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->begin(device,
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, &handle));
    op_params[3].tag = KM_TAG_APPLICATION_ID;
    op_params[3].blob = wrong_application_id;
    begin_paramset.length = 4;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->begin(device,
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, &handle));
    op_params[2].blob = wrong_application_data;
    op_params[3].blob = application_id;
    CHECK_RESULT( KM_ERROR_INVALID_KEY_BLOB,
        device->begin(device,
            KM_PURPOSE_SIGN, &key_blob, &begin_paramset, NULL, &handle));

end:
    km_free_blob(&export_data);
    keymaster_free_characteristics(pkey_characteristics);
    free(pkey_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}

keymaster_error_t test_km_restrictions(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;

    /* FIXME: Test currently uses old 'dummy' HMAC key, so fails.
     * Need integration test with Gatekeeper.
     */
    // CHECK_RESULT_OK(test_km_restrictions_auth_token(device));
    // CHECK_RESULT_OK(test_km_restrictions_timeout(device));
    CHECK_RESULT_OK(test_km_restrictions_app_id_and_data(device));

end:
    return res;
}
