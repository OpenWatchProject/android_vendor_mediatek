/*
 * Copyright (c) 2015-2017 TRUSTONIC LIMITED
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

#include "test_km_rsa.h"
#include "test_km_util.h"

#include "log.h"

static keymaster_error_t test_km_rsa_sign_verify(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes,
    keymaster_padding_t padding,
    keymaster_digest_t digest)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message[1000];
    keymaster_blob_t messageblob = {0, 0};
    keymaster_blob_t sig_output = {0, 0};
    size_t message_consumed = 0;

    /* populate message */
    memset(message, 24, sizeof(message));

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_PADDING;
    key_param[1].enumerated = padding;
    key_param[2].tag = KM_TAG_DIGEST;
    key_param[2].enumerated = digest;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN,
        key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &handle));

    messageblob.data = &message[0];
    messageblob.data_length = 630;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &messageblob, // first part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 630);

    messageblob.data = &message[630];
    messageblob.data_length = 370;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &messageblob, // last part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 370);

    if ((keybytes == 64) &&
        (padding == KM_PAD_RSA_PSS) &&
        (digest == KM_DIGEST_SHA_2_256)) {
        /* In this case, an error is expected */
        /* TBUG-880: Google recommended that salt length = digest length */
        /* With RSA PSS, the modulus length must be >= (2 + salt length + digest length) */
        /* In this case, 64 is not >= (2+32+32) */
        CHECK_RESULT(KM_ERROR_INVALID_INPUT_LENGTH,
            impl->finish(
                handle,
                NULL, // no params
                NULL, // no input
                NULL, // no signature
                NULL, NULL,
                NULL, // no out_params
                &sig_output));
    } else {
        CHECK_RESULT_OK(impl->finish(
            handle,
            NULL, // no params
            NULL, // no input
            NULL, // no signature
            NULL, NULL,
            NULL, // no out_params
            &sig_output));
        CHECK_TRUE(sig_output.data_length == keybytes);
    }

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_VERIFY,
        key_blob,
        &paramset, // same as for KM_PURPOSE_SIGN
        NULL,
        NULL, // no out_params
        &handle));

    messageblob.data = &message[0];
    messageblob.data_length = 480;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &messageblob, // first part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 480);

    messageblob.data = &message[480];
    messageblob.data_length = 520;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &messageblob, // last part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 520);

    if ((keybytes == 64) &&
        (padding == KM_PAD_RSA_PSS) &&
        (digest == KM_DIGEST_SHA_2_256)) {
        /* TBUG-880 */
        CHECK_RESULT(KM_ERROR_VERIFICATION_FAILED,
            impl->finish(
                handle,
                NULL, // no params
                NULL, // no input
                &sig_output,
                NULL, NULL,
                NULL, // no out_params
                NULL));

    } else {
        CHECK_RESULT_OK(impl->finish(
            handle,
            NULL, // no params
            NULL, // no input
            &sig_output,
            NULL, NULL,
            NULL, // no out_params
            NULL));
    }

end:
    km_free_blob(&sig_output);

    return res;
}

static keymaster_error_t test_km_rsa_sign_verify_pkcs1_1_5_raw(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message[512];
    keymaster_blob_t messageblob = {message, 0};
    keymaster_blob_t sig_output = {0, 0};
    size_t message_consumed = 0;

    /* populate message */
    memset(message, 24, sizeof(message));

    LOG_I("Message of maximum allowed size...");
    messageblob.data_length = keybytes - 11; // maximum allowed

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_PADDING;
    key_param[1].enumerated = KM_PAD_RSA_PKCS1_1_5_SIGN;
    key_param[2].tag = KM_TAG_DIGEST;
    key_param[2].enumerated = KM_DIGEST_NONE;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == messageblob.data_length);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &sig_output));
    CHECK_TRUE(sig_output.data_length == keybytes);

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_VERIFY, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == messageblob.data_length);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, &sig_output, NULL, NULL, NULL, NULL));

    km_free_blob(&sig_output);

    LOG_I("Message exceeding maximum allowed size...");
    messageblob.data_length += 1;

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == messageblob.data_length);

    CHECK_RESULT(KM_ERROR_INVALID_INPUT_LENGTH,
        impl->finish(
            handle, NULL, NULL, NULL, NULL, NULL, NULL, &sig_output));

end:
    km_free_blob(&sig_output);

    return res;
}

static keymaster_error_t test_km_rsa_concurrent(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle[17];
    uint8_t message[100];
    keymaster_blob_t messageblob = {message, sizeof(message)};
    keymaster_blob_t sig_output[17];
    size_t message_consumed = 0;

    memset(sig_output, 0, sizeof(sig_output));

    /* populate message */
    memset(message, 24, sizeof(message));

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_PADDING;
    key_param[1].enumerated = KM_PAD_RSA_PKCS1_1_5_SIGN;
    key_param[2].tag = KM_TAG_DIGEST;
    key_param[2].enumerated = KM_DIGEST_MD5;
    paramset.length = 3;

    // start 16 RSA signature operations
    for (int i = 0; i < 16; i++) {
        CHECK_RESULT_OK(impl->begin(
            KM_PURPOSE_SIGN, key_blob, &paramset, NULL, NULL, &handle[i]));
    }
    // 17 is too many
    CHECK_RESULT(KM_ERROR_TOO_MANY_OPERATIONS, impl->begin(
        KM_PURPOSE_SIGN, key_blob, &paramset, NULL, NULL, &handle[16]));

    // update the 16 we've started
    for (int i = 0; i < 16; i++) {
        CHECK_RESULT_OK(impl->update(
            handle[i], NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
        CHECK_TRUE(message_consumed == sizeof(message));
    }

    // finish one of them
    CHECK_RESULT_OK(impl->finish(
        handle[0], NULL, NULL, NULL, NULL, NULL, NULL, &sig_output[0]));
    CHECK_TRUE(sig_output[0].data_length == keybytes);

    // now start another
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN, key_blob, &paramset, NULL, NULL, &handle[16]));

    // update it
    CHECK_RESULT_OK(impl->update(
        handle[16], NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == sizeof(message));

    // finish all of them
    for (int i = 1; i < 17; i++) {
        CHECK_RESULT_OK(impl->finish(
            handle[i], NULL, NULL, NULL, NULL, NULL, NULL, &sig_output[i]));
        CHECK_TRUE(sig_output[i].data_length == keybytes);
    }

end:
    for (int i = 0; i < 17; i++) {
        km_free_blob(&sig_output[i]);
    }

    return res;
}

static keymaster_error_t test_km_rsa_sign_verify_empty_message(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message;
    keymaster_blob_t messageblob = {0, 0};
    keymaster_blob_t sig_output = {0, 0};
    size_t message_consumed = 0;

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_PADDING;
    key_param[1].enumerated = KM_PAD_RSA_PSS;
    key_param[2].tag = KM_TAG_DIGEST;
    key_param[2].enumerated = KM_DIGEST_SHA_2_256;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN, key_blob, &paramset, NULL, NULL, &handle));

    messageblob.data = &message;
    messageblob.data_length = 0;
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == 0);

    if (keybytes == 64) {
        /* TBUG-880 */
        CHECK_RESULT(KM_ERROR_INVALID_INPUT_LENGTH,
            impl->finish(
                handle, NULL, NULL, NULL, NULL, NULL, NULL, &sig_output));
    } else {
        CHECK_RESULT_OK(impl->finish(
            handle, NULL, NULL, NULL, NULL, NULL, NULL, &sig_output));
        CHECK_TRUE(sig_output.data_length == keybytes);
    }

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_VERIFY, key_blob, &paramset, NULL, NULL, &handle));

    messageblob.data = &message;
    messageblob.data_length = 0;
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == 0);

    if (keybytes == 64) {
        /* TBUG-880 */
        CHECK_RESULT(KM_ERROR_VERIFICATION_FAILED,
            impl->finish(
                handle, NULL, NULL, &sig_output, NULL, NULL, NULL, NULL));
    } else {
        CHECK_RESULT_OK(impl->finish(
            handle, NULL, NULL, &sig_output, NULL, NULL, NULL, NULL));
    }

end:
    km_free_blob(&sig_output);

    return res;
}

static keymaster_error_t test_km_rsa_sign_verify_pkcs1_no_digest_too_long(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message[BYTES_PER_BITS(4096) - 10];
    keymaster_blob_t messageblob = {message, 0};
    keymaster_blob_t sig_output = {NULL, 0};
    size_t message_consumed = 0;

    memset(message, 0x80, sizeof(message));

    /* 1. Sign message of maximum length. */

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_PADDING;
    key_param[1].enumerated = KM_PAD_RSA_PKCS1_1_5_SIGN;
    key_param[2].tag = KM_TAG_DIGEST;
    key_param[2].enumerated = KM_DIGEST_NONE;
    paramset.length = 3;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_SIGN, key_blob, &paramset, NULL, NULL, &handle));

    messageblob.data_length = keybytes - 11;
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == keybytes - 11);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &sig_output));
    CHECK_TRUE(sig_output.data_length == keybytes);

    /* 2. Try to verify with message extended by one byte. */

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_VERIFY, key_blob, &paramset, NULL, NULL, &handle));

    messageblob.data_length = keybytes - 10;
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == keybytes - 10);

    CHECK_RESULT(KM_ERROR_INVALID_INPUT_LENGTH,
        impl->finish(
            handle, NULL, NULL, &sig_output, NULL, NULL, NULL, NULL));

end:
    km_free_blob(&sig_output);

    return res;
}

static keymaster_error_t test_km_rsa_verify_empty_signature(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t message[BYTES_PER_BITS(4096) - 11];
    keymaster_blob_t messageblob = {message, 0};
    keymaster_blob_t sig = {NULL, 0};
    size_t message_consumed = 0;

    memset(message, 0x80, sizeof(message));

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_PADDING;
    key_param[1].enumerated = KM_PAD_RSA_PKCS1_1_5_SIGN;
    key_param[2].tag = KM_TAG_DIGEST;
    key_param[2].enumerated = KM_DIGEST_NONE;
    paramset.length = 3;

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_VERIFY, key_blob, &paramset, NULL, NULL, &handle));

    messageblob.data_length = keybytes - 11;
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &messageblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == keybytes - 11);

    CHECK_RESULT(KM_ERROR_VERIFICATION_FAILED,
        impl->finish(
            handle, NULL, NULL, &sig, NULL, NULL, NULL, NULL));

end:
    return res;
}

static keymaster_error_t test_km_rsa_encrypt_decrypt(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes,
    keymaster_padding_t padding,
    keymaster_digest_t digest, bool set_digest)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[4];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t plaintext[BYTES_PER_BITS(4096)]; // maximum key size
    keymaster_blob_t plainblob = {0, 0};
    keymaster_blob_t enc_output = {0, 0};
    keymaster_blob_t dec_output = {0, 0};
    size_t message_consumed = 0;

    uint32_t padlen = 0;
    if (padding == KM_PAD_RSA_OAEP) {
        padlen = 2 + 2*BYTES_PER_BITS(block_length(digest));
    } else if (padding == KM_PAD_RSA_PKCS1_1_5_ENCRYPT) {
        padlen = 11;
    }

    if (padlen > keybytes) {
        LOG_I("Invalid key size / digest combination: skipping test.");
        return KM_ERROR_OK;
    }
    uint32_t msgsize = keybytes - padlen;

    /* populate message */
    memset(plaintext, 25, sizeof(plaintext));
    paramset.length=0;

    key_param[paramset.length].tag = KM_TAG_ALGORITHM;
    key_param[paramset.length].enumerated = KM_ALGORITHM_RSA;
    paramset.length++;

    key_param[paramset.length].tag = KM_TAG_PADDING;
    key_param[paramset.length].enumerated = padding;
    paramset.length++;

    if (set_digest)
    {
        key_param[paramset.length].tag = KM_TAG_DIGEST;
        key_param[paramset.length].enumerated = digest;
        paramset.length++;
    }

    // Must be last one
    key_param[paramset.length].tag = KM_TAG_PURPOSE;
    key_param[paramset.length].enumerated = KM_PURPOSE_ENCRYPT;
    paramset.length++;

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT,
        key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &handle));

    plainblob.data = &plaintext[0];
    plainblob.data_length = 3;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &plainblob, // first part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == 3);

    plainblob.data = &plaintext[3];
    plainblob.data_length = msgsize - 3;
    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &plainblob, // last part of message
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == msgsize - 3);

    CHECK_RESULT_OK(impl->finish(
        handle,
        NULL, // no params
        NULL, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &enc_output));
    CHECK_TRUE(enc_output.data_length == keybytes);

    // Skip last param (KM_TAG_PURPOSE)
    paramset.length--;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT,
        key_blob,
        &paramset,
        NULL,
        NULL, // no out_params
        &handle));

    CHECK_RESULT_OK(impl->update(
        handle,
        NULL, // no params
        &enc_output,
        NULL, NULL,
        &message_consumed,
        NULL, // no out_params
        NULL));
    CHECK_TRUE(message_consumed == keybytes);

    CHECK_RESULT_OK(impl->finish(
        handle,
        NULL, // no params
        NULL, // no input
        NULL, // no signature
        NULL, NULL,
        NULL, // no out_params
        &dec_output));
    CHECK_TRUE(dec_output.data_length == msgsize);

    CHECK_TRUE(memcmp(plaintext, dec_output.data, msgsize) == 0);

end:
    km_free_blob(&enc_output);
    km_free_blob(&dec_output);

    return res;
}

static keymaster_error_t test_km_rsa_encrypt_decrypt_empty_message(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes,
    keymaster_padding_t padding)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[3];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_operation_handle_t handle;
    uint8_t plaintext = 0;
    keymaster_blob_t plainblob = {0, 0};
    keymaster_blob_t enc_output = {0, 0};
    keymaster_blob_t dec_output = {0, 0};
    size_t message_consumed = 0;

    paramset.length=0;

    key_param[paramset.length].tag = KM_TAG_ALGORITHM;
    key_param[paramset.length].enumerated = KM_ALGORITHM_RSA;
    paramset.length++;

    key_param[paramset.length].tag = KM_TAG_PADDING;
    key_param[paramset.length].enumerated = padding;
    paramset.length++;

    key_param[paramset.length].tag = KM_TAG_PURPOSE;
    key_param[paramset.length].enumerated = KM_PURPOSE_ENCRYPT;
    paramset.length++;

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, NULL, &handle));

    plainblob.data = &plaintext;
    plainblob.data_length = 0;
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &plainblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == 0);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &enc_output));
    CHECK_TRUE(enc_output.data_length == keybytes);

    // Skip last param (KM_TAG_PURPOSE)
    paramset.length--;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &enc_output, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == keybytes);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &dec_output));
    CHECK_TRUE(dec_output.data_length == 0);

end:
    km_free_blob(&enc_output);
    km_free_blob(&dec_output);

    return res;
}

static keymaster_error_t test_km_rsa_raw_too_large(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t param[4];
    keymaster_key_param_set_t paramset = {param, 0};
    keymaster_operation_handle_t handle;
    uint8_t msg[BYTES_PER_BITS(4096)]; // maximum key size
    keymaster_blob_t msg_blob = {msg, 0};
    keymaster_blob_t output = {0, 0};
    size_t message_consumed = 0;

    /* populate message with large number */
    memset(msg, 0xFF, keybytes);
    msg_blob.data_length = keybytes;

    /* Encrypt */
    param[0].tag = KM_TAG_ALGORITHM;
    param[0].enumerated = KM_ALGORITHM_RSA;
    param[1].tag = KM_TAG_PADDING;
    param[1].enumerated = KM_PAD_NONE;
    param[2].tag = KM_TAG_DIGEST;
    param[2].enumerated = KM_DIGEST_NONE;
    param[3].tag = KM_TAG_PURPOSE;
    param[3].enumerated = KM_PURPOSE_ENCRYPT;
    paramset.length = 4;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, NULL, &handle));
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &msg_blob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == keybytes);
    CHECK_RESULT(KM_ERROR_INVALID_ARGUMENT, impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &output));

    /* Decrypt */
    km_free_blob(&output);
    param[3].enumerated = KM_PURPOSE_DECRYPT;
    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, NULL, &handle));
    CHECK_RESULT_OK(impl->update(
        handle, NULL, &msg_blob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == keybytes);
    CHECK_RESULT(KM_ERROR_INVALID_ARGUMENT, impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &output));

end:
    km_free_blob(&output);
    return res;
}

static keymaster_error_t test_km_rsa_encrypt_raw_one(
    TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *key_blob,
    uint32_t keybytes)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t param[5];
    keymaster_key_param_set_t paramset = {param, 0};
    keymaster_operation_handle_t handle;
    uint8_t plaintext[1] = {1};
    keymaster_blob_t oneblob = {plaintext, 1}; // numeric value 1
    keymaster_blob_t enc_output = {NULL, 0};
    keymaster_blob_t dec_output = {NULL, 0};
    size_t message_consumed = 0;
    uint8_t one[BYTES_PER_BITS(4096)];

    memset(one, 0, keybytes-1);
    one[keybytes-1] = 1; // 1^e == 1

    param[0].tag = KM_TAG_ALGORITHM;
    param[0].enumerated = KM_ALGORITHM_RSA;
    param[1].tag = KM_TAG_PADDING;
    param[1].enumerated = KM_PAD_NONE;
    param[2].tag = KM_TAG_DIGEST;
    param[2].enumerated = KM_DIGEST_NONE;
    param[3].tag = KM_TAG_PURPOSE;
    param[3].enumerated = KM_PURPOSE_ENCRYPT;
    param[4].tag = KM_TAG_PURPOSE;
    param[4].enumerated = KM_PURPOSE_DECRYPT;
    paramset.length = 5;

    /* Encrypt {1} and check result. */

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_ENCRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &oneblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == 1);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &enc_output));
    CHECK_TRUE(enc_output.data_length == keybytes);
    CHECK_TRUE(memcmp(enc_output.data, one, keybytes) == 0);

    /* Decrypt {1} and check result. */

    CHECK_RESULT_OK(impl->begin(
        KM_PURPOSE_DECRYPT, key_blob, &paramset, NULL, NULL, &handle));

    CHECK_RESULT_OK(impl->update(
        handle, NULL, &oneblob, NULL, NULL, &message_consumed, NULL, NULL));
    CHECK_TRUE(message_consumed == 1);

    CHECK_RESULT_OK(impl->finish(
        handle, NULL, NULL, NULL, NULL, NULL, NULL, &dec_output));
    CHECK_TRUE(dec_output.data_length == keybytes);
    CHECK_TRUE(memcmp(dec_output.data, one, keybytes) == 0);

end:
    km_free_blob(&enc_output);
    km_free_blob(&dec_output);
    return res;
}

keymaster_error_t test_km_rsa_import_export(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[6];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t key_characteristics;

    memset(&key_characteristics, 0, sizeof(keymaster_key_characteristics_t));

    /* Test vector from Android M PDK: system/keymaster/rsa_privkey_pk8.der */
    const uint8_t key[] = {
        0x30, 0x82, 0x02, 0x75, 0x02, 0x01, 0x00, 0x30, 0x0d, 0x06, 0x09, 0x2a,
        0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04, 0x82,
        0x02, 0x5f, 0x30, 0x82, 0x02, 0x5b, 0x02, 0x01, 0x00, 0x02, 0x81, 0x81,
        0x00, 0xc6, 0x09, 0x54, 0x09, 0x04, 0x7d, 0x86, 0x34, 0x81, 0x2d, 0x5a,
        0x21, 0x81, 0x76, 0xe4, 0x5c, 0x41, 0xd6, 0x0a, 0x75, 0xb1, 0x39, 0x01,
        0xf2, 0x34, 0x22, 0x6c, 0xff, 0xe7, 0x76, 0x52, 0x1c, 0x5a, 0x77, 0xb9,
        0xe3, 0x89, 0x41, 0x7b, 0x71, 0xc0, 0xb6, 0xa4, 0x4d, 0x13, 0xaf, 0xe4,
        0xe4, 0xa2, 0x80, 0x5d, 0x46, 0xc9, 0xda, 0x29, 0x35, 0xad, 0xb1, 0xff,
        0x0c, 0x1f, 0x24, 0xea, 0x06, 0xe6, 0x2b, 0x20, 0xd7, 0x76, 0x43, 0x0a,
        0x4d, 0x43, 0x51, 0x57, 0x23, 0x3c, 0x6f, 0x91, 0x67, 0x83, 0xc3, 0x0e,
        0x31, 0x0f, 0xcb, 0xd8, 0x9b, 0x85, 0xc2, 0xd5, 0x67, 0x71, 0x16, 0x97,
        0x85, 0xac, 0x12, 0xbc, 0xa2, 0x44, 0xab, 0xda, 0x72, 0xbf, 0xb1, 0x9f,
        0xc4, 0x4d, 0x27, 0xc8, 0x1e, 0x1d, 0x92, 0xde, 0x28, 0x4f, 0x40, 0x61,
        0xed, 0xfd, 0x99, 0x28, 0x07, 0x45, 0xea, 0x6d, 0x25, 0x02, 0x03, 0x01,
        0x00, 0x01, 0x02, 0x81, 0x80, 0x1b, 0xe0, 0xf0, 0x4d, 0x9c, 0xae, 0x37,
        0x18, 0x69, 0x1f, 0x03, 0x53, 0x38, 0x30, 0x8e, 0x91, 0x56, 0x4b, 0x55,
        0x89, 0x9f, 0xfb, 0x50, 0x84, 0xd2, 0x46, 0x0e, 0x66, 0x30, 0x25, 0x7e,
        0x05, 0xb3, 0xce, 0xab, 0x02, 0x97, 0x2d, 0xfa, 0xbc, 0xd6, 0xce, 0x5f,
        0x6e, 0xe2, 0x58, 0x9e, 0xb6, 0x79, 0x11, 0xed, 0x0f, 0xac, 0x16, 0xe4,
        0x3a, 0x44, 0x4b, 0x8c, 0x86, 0x1e, 0x54, 0x4a, 0x05, 0x93, 0x36, 0x57,
        0x72, 0xf8, 0xba, 0xf6, 0xb2, 0x2f, 0xc9, 0xe3, 0xc5, 0xf1, 0x02, 0x4b,
        0x06, 0x3a, 0xc0, 0x80, 0xa7, 0xb2, 0x23, 0x4c, 0xf8, 0xae, 0xe8, 0xf6,
        0xc4, 0x7b, 0xbf, 0x4f, 0xd3, 0xac, 0xe7, 0x24, 0x02, 0x90, 0xbe, 0xf1,
        0x6c, 0x0b, 0x3f, 0x7f, 0x3c, 0xdd, 0x64, 0xce, 0x3a, 0xb5, 0x91, 0x2c,
        0xf6, 0xe3, 0x2f, 0x39, 0xab, 0x18, 0x83, 0x58, 0xaf, 0xcc, 0xcd, 0x80,
        0x81, 0x02, 0x41, 0x00, 0xe4, 0xb4, 0x9e, 0xf5, 0x0f, 0x76, 0x5d, 0x3b,
        0x24, 0xdd, 0xe0, 0x1a, 0xce, 0xaa, 0xf1, 0x30, 0xf2, 0xc7, 0x66, 0x70,
        0xa9, 0x1a, 0x61, 0xae, 0x08, 0xaf, 0x49, 0x7b, 0x4a, 0x82, 0xbe, 0x6d,
        0xee, 0x8f, 0xcd, 0xd5, 0xe3, 0xf7, 0xba, 0x1c, 0xfb, 0x1f, 0x0c, 0x92,
        0x6b, 0x88, 0xf8, 0x8c, 0x92, 0xbf, 0xab, 0x13, 0x7f, 0xba, 0x22, 0x85,
        0x22, 0x7b, 0x83, 0xc3, 0x42, 0xff, 0x7c, 0x55, 0x02, 0x41, 0x00, 0xdd,
        0xab, 0xb5, 0x83, 0x9c, 0x4c, 0x7f, 0x6b, 0xf3, 0xd4, 0x18, 0x32, 0x31,
        0xf0, 0x05, 0xb3, 0x1a, 0xa5, 0x8a, 0xff, 0xdd, 0xa5, 0xc7, 0x9e, 0x4c,
        0xce, 0x21, 0x7f, 0x6b, 0xc9, 0x30, 0xdb, 0xe5, 0x63, 0xd4, 0x80, 0x70,
        0x6c, 0x24, 0xe9, 0xeb, 0xfc, 0xab, 0x28, 0xa6, 0xcd, 0xef, 0xd3, 0x24,
        0xb7, 0x7e, 0x1b, 0xf7, 0x25, 0x1b, 0x70, 0x90, 0x92, 0xc2, 0x4f, 0xf5,
        0x01, 0xfd, 0x91, 0x02, 0x40, 0x23, 0xd4, 0x34, 0x0e, 0xda, 0x34, 0x45,
        0xd8, 0xcd, 0x26, 0xc1, 0x44, 0x11, 0xda, 0x6f, 0xdc, 0xa6, 0x3c, 0x1c,
        0xcd, 0x4b, 0x80, 0xa9, 0x8a, 0xd5, 0x2b, 0x78, 0xcc, 0x8a, 0xd8, 0xbe,
        0xb2, 0x84, 0x2c, 0x1d, 0x28, 0x04, 0x05, 0xbc, 0x2f, 0x6c, 0x1b, 0xea,
        0x21, 0x4a, 0x1d, 0x74, 0x2a, 0xb9, 0x96, 0xb3, 0x5b, 0x63, 0xa8, 0x2a,
        0x5e, 0x47, 0x0f, 0xa8, 0x8d, 0xbf, 0x82, 0x3c, 0xdd, 0x02, 0x40, 0x1b,
        0x7b, 0x57, 0x44, 0x9a, 0xd3, 0x0d, 0x15, 0x18, 0x24, 0x9a, 0x5f, 0x56,
        0xbb, 0x98, 0x29, 0x4d, 0x4b, 0x6a, 0xc1, 0x2f, 0xfc, 0x86, 0x94, 0x04,
        0x97, 0xa5, 0xa5, 0x83, 0x7a, 0x6c, 0xf9, 0x46, 0x26, 0x2b, 0x49, 0x45,
        0x26, 0xd3, 0x28, 0xc1, 0x1e, 0x11, 0x26, 0x38, 0x0f, 0xde, 0x04, 0xc2,
        0x4f, 0x91, 0x6d, 0xec, 0x25, 0x08, 0x92, 0xdb, 0x09, 0xa6, 0xd7, 0x7c,
        0xdb, 0xa3, 0x51, 0x02, 0x40, 0x77, 0x62, 0xcd, 0x8f, 0x4d, 0x05, 0x0d,
        0xa5, 0x6b, 0xd5, 0x91, 0xad, 0xb5, 0x15, 0xd2, 0x4d, 0x7c, 0xcd, 0x32,
        0xcc, 0xa0, 0xd0, 0x5f, 0x86, 0x6d, 0x58, 0x35, 0x14, 0xbd, 0x73, 0x24,
        0xd5, 0xf3, 0x36, 0x45, 0xe8, 0xed, 0x8b, 0x4a, 0x1c, 0xb3, 0xcc, 0x4a,
        0x1d, 0x67, 0x98, 0x73, 0x99, 0xf2, 0xa0, 0x9f, 0x5b, 0x3f, 0xb6, 0x8c,
        0x88, 0xd5, 0xe5, 0xd9, 0x0a, 0xc3, 0x34, 0x92, 0xd6};

    const keymaster_blob_t key_data = {key, sizeof(key)};

    keymaster_blob_t export_data = {NULL, 0};

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[1].boolean = true;
    key_param[2].tag = KM_TAG_PURPOSE;
    key_param[2].enumerated = KM_PURPOSE_SIGN;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_VERIFY;
    key_param[4].tag = KM_TAG_PADDING;
    key_param[4].enumerated = KM_PAD_RSA_PSS;
    key_param[5].tag = KM_TAG_DIGEST;
    key_param[5].enumerated = KM_DIGEST_SHA1;
    paramset.length = 6;
    CHECK_RESULT_OK(impl->import_key(
        &paramset, KM_KEY_FORMAT_PKCS8, &key_data, &key_blob, &key_characteristics));

    CHECK_RESULT_OK(test_km_rsa_sign_verify(impl,
        &key_blob, 128, KM_PAD_RSA_PSS, KM_DIGEST_SHA1));

    CHECK_RESULT_OK(impl->export_key(
        KM_KEY_FORMAT_X509, &key_blob, NULL, NULL, &export_data));

end:
    keymaster_free_characteristics(&key_characteristics);
    km_free_blob(&export_data);
    km_free_key_blob(&key_blob);

    return res;
}

keymaster_error_t test_km_rsa_generate_export(
    TrustonicKeymaster4DeviceImpl *impl)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[6];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t key_characteristics;
    keymaster_blob_t export_data = {NULL, 0};
    uint8_t client_id_bytes[7];
    uint8_t app_data_bytes[14];
    keymaster_blob_t client_id = {client_id_bytes, sizeof(client_id_bytes)};
    keymaster_blob_t app_data = {app_data_bytes, sizeof(app_data_bytes)};

    memset(&key_characteristics, 0, sizeof(keymaster_key_characteristics_t));

    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = 2048;
    key_param[2].tag = KM_TAG_PURPOSE;
    key_param[2].enumerated = KM_PURPOSE_ENCRYPT;
    key_param[3].tag = KM_TAG_PURPOSE;
    key_param[3].enumerated = KM_PURPOSE_DECRYPT;
    key_param[4].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[4].boolean = true;
    key_param[5].tag = KM_TAG_RSA_PUBLIC_EXPONENT;
    key_param[5].long_integer = 0x10001;
    paramset.length = 6;
    CHECK_RESULT_OK(impl->generate_key(
        &paramset, &key_blob, NULL));

    memset(client_id_bytes, 0x01, sizeof(client_id_bytes));
    memset(app_data_bytes, 0x02, sizeof(app_data_bytes));
    CHECK_RESULT_OK(impl->get_key_characteristics(
        &key_blob, &client_id, &app_data, &key_characteristics));

    CHECK_RESULT_OK(impl->export_key(
        KM_KEY_FORMAT_X509, &key_blob, &client_id, &app_data, &export_data));

end:
    keymaster_free_characteristics(&key_characteristics);
    km_free_blob(&export_data);
    km_free_key_blob(&key_blob);

    return res;
}

keymaster_error_t test_km_rsa(
    TrustonicKeymaster4DeviceImpl *impl,
    uint32_t keysize,
    uint64_t e)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_param[18];
    keymaster_key_param_set_t paramset = {key_param, 0};
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_key_characteristics_t key_characteristics;
    uint32_t keybytes = BYTES_PER_BITS(keysize);

    memset(&key_characteristics, 0, sizeof(keymaster_key_characteristics_t));

    LOG_I("Generate %d-bit key, public exponent 0x%08x%08x...", keysize, (uint32_t)(e>>32), (uint32_t)e);
    key_param[0].tag = KM_TAG_ALGORITHM;
    key_param[0].enumerated = KM_ALGORITHM_RSA;
    key_param[1].tag = KM_TAG_KEY_SIZE;
    key_param[1].integer = keysize;
    key_param[2].tag = KM_TAG_RSA_PUBLIC_EXPONENT;
    key_param[2].long_integer = e;
    key_param[3].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_param[3].boolean = true;
    key_param[4].tag = KM_TAG_PURPOSE;
    key_param[4].enumerated = KM_PURPOSE_SIGN;
    key_param[5].tag = KM_TAG_PURPOSE;
    key_param[5].enumerated = KM_PURPOSE_VERIFY;
    key_param[6].tag = KM_TAG_PURPOSE;
    key_param[6].enumerated = KM_PURPOSE_ENCRYPT;
    key_param[7].tag = KM_TAG_PURPOSE;
    key_param[7].enumerated = KM_PURPOSE_DECRYPT;
    key_param[8].tag = KM_TAG_PADDING;
    key_param[8].enumerated = KM_PAD_NONE;
    key_param[9].tag = KM_TAG_PADDING;
    key_param[9].enumerated = KM_PAD_RSA_PSS;
    key_param[10].tag = KM_TAG_PADDING;
    key_param[10].enumerated = KM_PAD_RSA_PKCS1_1_5_SIGN;
    key_param[11].tag = KM_TAG_PADDING;
    key_param[11].enumerated = KM_PAD_RSA_OAEP;
    key_param[12].tag = KM_TAG_PADDING;
    key_param[12].enumerated = KM_PAD_RSA_PKCS1_1_5_ENCRYPT;
    key_param[13].tag = KM_TAG_DIGEST;
    key_param[13].enumerated = KM_DIGEST_NONE;
    key_param[14].tag = KM_TAG_DIGEST;
    key_param[14].enumerated = KM_DIGEST_SHA1;
    key_param[15].tag = KM_TAG_DIGEST;
    key_param[15].enumerated = KM_DIGEST_SHA_2_256;
    key_param[16].tag = KM_TAG_DIGEST;
    key_param[16].enumerated = KM_DIGEST_NONE;
    key_param[17].tag = KM_TAG_DIGEST;
    key_param[17].enumerated = KM_DIGEST_MD5;
    paramset.length = 18;
    CHECK_RESULT_OK(impl->generate_key(
        &paramset, &key_blob, &key_characteristics));

    LOG_I("Sign/verify with KM_PAD_RSA_PSS and SHA1...");
    CHECK_RESULT_OK( test_km_rsa_sign_verify(
        impl, &key_blob, keybytes, KM_PAD_RSA_PSS, KM_DIGEST_SHA1) );

    LOG_I("Sign/verify with KM_PAD_RSA_PSS and SHA256...");
    CHECK_RESULT_OK( test_km_rsa_sign_verify(
        impl, &key_blob, keybytes, KM_PAD_RSA_PSS, KM_DIGEST_SHA_2_256) );

    LOG_I("Sign/verify with KM_PAD_RSA_PKCS1_1_5_SIGN and SHA1...");
    CHECK_RESULT_OK( test_km_rsa_sign_verify(
        impl, &key_blob, keybytes, KM_PAD_RSA_PKCS1_1_5_SIGN, KM_DIGEST_SHA1) );

    LOG_I("Sign/verify with KM_PAD_RSA_PKCS1_1_5_SIGN and SHA256...");
    CHECK_RESULT_OK( test_km_rsa_sign_verify(
        impl, &key_blob, keybytes, KM_PAD_RSA_PKCS1_1_5_SIGN, KM_DIGEST_SHA_2_256) );

    LOG_I("Sign/verify with KM_PAD_RSA_PKCS1_1_5_SIGN and no digest [sic]...");
    CHECK_RESULT_OK( test_km_rsa_sign_verify_pkcs1_1_5_raw(
        impl, &key_blob, keybytes) );

    LOG_I("Sign/verify with empty message...");
    CHECK_RESULT_OK( test_km_rsa_sign_verify_empty_message(
        impl, &key_blob, keybytes) );

    LOG_I("Sign/verify with KM_PAD_RSA_PKCS1_1_5_SIGN and no digest, message too long...");
    CHECK_RESULT_OK(test_km_rsa_sign_verify_pkcs1_no_digest_too_long(
        impl, &key_blob, keybytes));

    LOG_I("Verify with empty signature...");
    CHECK_RESULT_OK(test_km_rsa_verify_empty_signature(
        impl, &key_blob, keybytes));

    LOG_I("Encrypt/decrypt with KM_PAD_RSA_OAEP and SHA1...");
    CHECK_RESULT_OK( test_km_rsa_encrypt_decrypt(
        impl, &key_blob, keybytes, KM_PAD_RSA_OAEP, KM_DIGEST_SHA1, true) );
    LOG_I("Encrypt/decrypt with KM_PAD_RSA_OAEP and SHA256...");
    CHECK_RESULT_OK( test_km_rsa_encrypt_decrypt(
        impl, &key_blob, keybytes, KM_PAD_RSA_OAEP, KM_DIGEST_SHA_2_256, true) );

    LOG_I("Encrypt/decrypt with KM_PAD_RSA_PKCS1_1_5_ENCRYPT...");
    CHECK_RESULT_OK( test_km_rsa_encrypt_decrypt(
        impl, &key_blob, keybytes, KM_PAD_RSA_PKCS1_1_5_ENCRYPT, KM_DIGEST_NONE, true) );

    LOG_I("Encrypt/decrypt with KM_PAD_NONE...");
    CHECK_RESULT_OK( test_km_rsa_encrypt_decrypt(
        impl, &key_blob, keybytes, KM_PAD_NONE, KM_DIGEST_NONE, true) );

    LOG_I("Encrypt/decrypt with KM_PAD_NONE and no digest provided...");
    CHECK_RESULT_OK( test_km_rsa_encrypt_decrypt(
        impl, &key_blob, keybytes, KM_PAD_NONE, KM_DIGEST_NONE, false));

    LOG_I("Encrypt with KM_PAD_NONE, message numerically larger than modulus...");
    CHECK_RESULT_OK(test_km_rsa_raw_too_large(
        impl, &key_blob, keybytes));

    LOG_I("Encrypt/decrypt 1 with KM_PAD_NONE...");
    CHECK_RESULT_OK(test_km_rsa_encrypt_raw_one(
        impl, &key_blob, keybytes));

    LOG_I("Encrypt/decrypt with KM_PAD_RSA_PKCS1_1_5_ENCRYPT and empty message...");
    CHECK_RESULT_OK( test_km_rsa_encrypt_decrypt_empty_message(
        impl, &key_blob, keybytes, KM_PAD_RSA_PKCS1_1_5_ENCRYPT) );

    LOG_I("Concurrent RSA operations...");
    CHECK_RESULT_OK(test_km_rsa_concurrent(
        impl, &key_blob, keybytes));

end:
    keymaster_free_characteristics(&key_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}
