/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
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

#include <string.h>

#include "tee_keymaster_device.h"
#include "test_km_aes.h"
#include "test_km_hmac.h"
#include "test_km_rsa.h"
#include "test_km_ec.h"
#include "test_km_restrictions.h"
#include "test_km_util.h"

#undef  LOG_ANDROID
#undef  LOG_TAG
#define LOG_TAG "TlcTeeKeyMasterTest"
#include "log.h"

extern bool am_big_endian;
extern struct keystore_module HAL_MODULE_INFO_SYM;

static keymaster_error_t isAlgorithmSupported(
        keymaster_algorithm_t*  algorithms,
        size_t                  len,
        keymaster_algorithm_t   algorithm)
{
    if ((algorithms == NULL) || (len == 0)) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    for (size_t i = 0; i < len; i++) {
        if (algorithms[i] == algorithm) {
            return KM_ERROR_OK;
        }
    }

    return KM_ERROR_UNSUPPORTED_ALGORITHM;
}


static keymaster_error_t isBlockModeSupported(
        keymaster_block_mode_t*  modes,
        size_t                   len,
        keymaster_block_mode_t   mode)
{
    if ((modes == NULL) || (len == 0)) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    for (size_t i = 0; i < len; i++) {
        if (modes[i] == mode) {
            return KM_ERROR_OK;
        }
    }

    return KM_ERROR_UNSUPPORTED_BLOCK_MODE;
}


static keymaster_error_t isPaddingSupported(
        keymaster_padding_t*    paddings,
        size_t                  len,
        keymaster_padding_t     padding)
{
    if ((paddings == NULL) || (len == 0)) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    for (size_t i = 0; i < len; i++) {
        if (paddings[i] == padding) {
            return KM_ERROR_OK;
        }
    }

    return KM_ERROR_UNSUPPORTED_PADDING_MODE;
}


static keymaster_error_t isDigestSupported(
        keymaster_digest_t*     digests,
        size_t                  len,
        keymaster_digest_t      digest)
{
    if ((digests == NULL) || (len == 0)) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    for (size_t i = 0; i < len; i++) {
        if (digests[i] == digest) {
            return KM_ERROR_OK;
        }
    }

    return KM_ERROR_UNSUPPORTED_DIGEST;
}


static keymaster_error_t isKeyFormatSupported(
        keymaster_key_format_t*  key_formats,
        size_t                   len,
        keymaster_key_format_t   key_format)
{
    if ((key_formats == NULL) || (len == 0)) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    for (size_t i = 0; i < len; i++) {
        if (key_formats[i] == key_format) {
            return KM_ERROR_OK;
        }
    }

    return KM_ERROR_UNSUPPORTED_KEY_FORMAT;
}

/**
 * Smoke test
 *
 * @param device device
 *
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t test_api(
    keymaster1_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_algorithm_t *algorithms = NULL;
    keymaster_block_mode_t *block_modes = NULL;
    keymaster_padding_t *paddings = NULL;
    keymaster_digest_t *digests = NULL;
    keymaster_key_format_t *key_formats = NULL;
    size_t len;
    keymaster_key_param_t key_params[7];
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_operation_handle_t operation_handle;
    keymaster_key_characteristics_t *pkey_characteristics = NULL;
    uint8_t test_entropy[] = { 0x65, 0x6e, 0x74, 0x72, 0x6f, 0x70, 0x79 };
    uint8_t rsa_input_data[] = {0x69, 0x6e, 0x70, 0x75, 0x74, 0x20, 0x64, 0x61, 0x74, 0x61};
    uint8_t *ptr_rsa_output_data = NULL;;
    size_t rsa_input_data_len = sizeof(rsa_input_data);
    size_t input_consumed;
    keymaster_key_param_set_t paramset = {key_params, 0};
    keymaster_blob_t rsa_input_data_blob = {rsa_input_data, rsa_input_data_len};
    keymaster_blob_t rsa_output_data_blob = {ptr_rsa_output_data, 0};

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing get_supported_algorithms API...");
    CHECK_RESULT_OK(device->get_supported_algorithms(device, &algorithms, &len));
    CHECK_RESULT_OK(isAlgorithmSupported(algorithms, len, KM_ALGORITHM_RSA));
    CHECK_RESULT_OK(isAlgorithmSupported(algorithms, len, KM_ALGORITHM_EC));
    CHECK_RESULT_OK(isAlgorithmSupported(algorithms, len, KM_ALGORITHM_AES));
    CHECK_RESULT_OK(isAlgorithmSupported(algorithms, len, KM_ALGORITHM_HMAC));
    FREE(algorithms);
    CHECK_RESULT(KM_ERROR_OUTPUT_PARAMETER_NULL,
        device->get_supported_algorithms(device, NULL, NULL));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing get_supported_block_modes API...");
    CHECK_RESULT_OK(device->get_supported_block_modes(device,
        KM_ALGORITHM_AES, KM_PURPOSE_ENCRYPT, &block_modes, &len));
    CHECK_RESULT_OK(isBlockModeSupported(block_modes, len, KM_MODE_ECB));
    CHECK_RESULT_OK(isBlockModeSupported(block_modes, len, KM_MODE_CBC));
    CHECK_RESULT_OK(isBlockModeSupported(block_modes, len, KM_MODE_CTR));
    FREE(block_modes);
    CHECK_RESULT_OK(device->get_supported_block_modes(device,
        KM_ALGORITHM_AES, KM_PURPOSE_DECRYPT, &block_modes, &len));
    CHECK_RESULT_OK(isBlockModeSupported(block_modes, len, KM_MODE_ECB));
    CHECK_RESULT_OK(isBlockModeSupported(block_modes, len, KM_MODE_CBC));
    CHECK_RESULT_OK(isBlockModeSupported(block_modes, len, KM_MODE_CTR));
    FREE(block_modes);
    CHECK_RESULT_OK(device->get_supported_block_modes(device,
        KM_ALGORITHM_RSA, KM_PURPOSE_SIGN, &block_modes, &len));
    CHECK_TRUE(len == 0);
    FREE(block_modes);
    CHECK_RESULT_OK(device->get_supported_block_modes(device,
        KM_ALGORITHM_EC, KM_PURPOSE_SIGN, &block_modes, &len));
    CHECK_TRUE(len == 0);
    FREE(block_modes);
    CHECK_RESULT_OK(device->get_supported_block_modes(device,
        KM_ALGORITHM_HMAC, KM_PURPOSE_SIGN, &block_modes, &len));
    CHECK_TRUE(len == 0);
    FREE(block_modes);
    CHECK_RESULT(KM_ERROR_OUTPUT_PARAMETER_NULL,
        device->get_supported_block_modes(device, KM_ALGORITHM_HMAC, KM_PURPOSE_SIGN, NULL, NULL));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing get_supported_padding_modes API...");
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_AES, KM_PURPOSE_ENCRYPT, &paddings, &len));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_NONE));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_PKCS7));
    FREE(paddings);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_AES, KM_PURPOSE_DECRYPT, &paddings, &len));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_NONE));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_PKCS7));
    FREE(paddings);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_RSA, KM_PURPOSE_SIGN, &paddings, &len));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_PKCS1_1_5_SIGN));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_PSS));
    FREE(paddings);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_RSA, KM_PURPOSE_VERIFY, &paddings, &len));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_PKCS1_1_5_SIGN));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_PSS));
    FREE(paddings);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_RSA, KM_PURPOSE_ENCRYPT, &paddings, &len));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_NONE));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_OAEP));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_PKCS1_1_5_ENCRYPT));
    FREE(paddings);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_RSA, KM_PURPOSE_DECRYPT, &paddings, &len));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_NONE));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_OAEP));
    CHECK_RESULT_OK(isPaddingSupported(paddings, len, KM_PAD_RSA_PKCS1_1_5_ENCRYPT));
    FREE(paddings);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_EC, KM_PURPOSE_SIGN, &paddings, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_EC, KM_PURPOSE_VERIFY, &paddings, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_HMAC, KM_PURPOSE_SIGN, &paddings, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT_OK(device->get_supported_padding_modes(device,
        KM_ALGORITHM_HMAC, KM_PURPOSE_VERIFY, &paddings, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT(KM_ERROR_OUTPUT_PARAMETER_NULL,
        device->get_supported_padding_modes(device, KM_ALGORITHM_HMAC, KM_PURPOSE_VERIFY, NULL, NULL));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing get_supported_digests API...");
    CHECK_RESULT_OK(device->get_supported_digests(device,
        KM_ALGORITHM_RSA, KM_PURPOSE_SIGN, &digests, &len));
    CHECK_RESULT_OK(isDigestSupported(digests, len, KM_DIGEST_SHA_2_256));
    FREE(digests);
    CHECK_RESULT_OK(device->get_supported_digests(device,
        KM_ALGORITHM_HMAC, KM_PURPOSE_SIGN, &digests, &len));
    CHECK_RESULT_OK(isDigestSupported(digests, len, KM_DIGEST_SHA1));
    CHECK_RESULT_OK(isDigestSupported(digests, len, KM_DIGEST_SHA_2_224));
    CHECK_RESULT_OK(isDigestSupported(digests, len, KM_DIGEST_SHA_2_256));
    CHECK_RESULT_OK(isDigestSupported(digests, len, KM_DIGEST_SHA_2_384));
    CHECK_RESULT_OK(isDigestSupported(digests, len, KM_DIGEST_SHA_2_512));
    CHECK_RESULT(KM_ERROR_UNSUPPORTED_DIGEST,
        isDigestSupported(digests, len, KM_DIGEST_NONE));
    FREE(digests);
    CHECK_RESULT_OK(device->get_supported_digests(device,
        KM_ALGORITHM_EC, KM_PURPOSE_SIGN, &digests, &len));
    CHECK_RESULT_OK(isDigestSupported(digests, len, KM_DIGEST_SHA1));
    FREE(digests);
    CHECK_RESULT_OK(device->get_supported_digests(device,
        KM_ALGORITHM_AES, KM_PURPOSE_ENCRYPT, &digests, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT_OK(device->get_supported_digests(device,
        KM_ALGORITHM_AES, KM_PURPOSE_DECRYPT, &digests, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT(KM_ERROR_OUTPUT_PARAMETER_NULL,
        device->get_supported_digests(device, KM_ALGORITHM_AES, KM_PURPOSE_DECRYPT, NULL, NULL));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing get_supported_import_formats API...");
    CHECK_RESULT_OK(device->get_supported_import_formats(device,
        KM_ALGORITHM_RSA, &key_formats, &len));
    CHECK_RESULT_OK(isKeyFormatSupported(key_formats, len, KM_KEY_FORMAT_PKCS8));
    FREE(key_formats);
    CHECK_RESULT_OK(device->get_supported_import_formats(device,
        KM_ALGORITHM_EC, &key_formats, &len));
    CHECK_RESULT_OK(isKeyFormatSupported(key_formats, len, KM_KEY_FORMAT_PKCS8));
    FREE(key_formats);
    CHECK_RESULT_OK(device->get_supported_import_formats(device,
        KM_ALGORITHM_AES, &key_formats, &len));
    CHECK_RESULT_OK(isKeyFormatSupported(key_formats, len, KM_KEY_FORMAT_RAW));
    FREE(key_formats);
    CHECK_RESULT_OK(device->get_supported_import_formats(device,
        KM_ALGORITHM_HMAC, &key_formats, &len));
    CHECK_RESULT_OK(isKeyFormatSupported(key_formats, len, KM_KEY_FORMAT_RAW));
    FREE(key_formats);
    CHECK_RESULT(KM_ERROR_OUTPUT_PARAMETER_NULL,
        device->get_supported_import_formats(device, KM_ALGORITHM_RSA, NULL, NULL));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing get_supported_export_formats API...");
    CHECK_RESULT_OK(device->get_supported_export_formats(device,
        KM_ALGORITHM_RSA, &key_formats, &len));
    CHECK_RESULT_OK(isKeyFormatSupported(key_formats, len, KM_KEY_FORMAT_X509));
    FREE(key_formats);
    CHECK_RESULT_OK(device->get_supported_export_formats(device,
        KM_ALGORITHM_EC, &key_formats, &len));
    CHECK_RESULT_OK(isKeyFormatSupported(key_formats, len, KM_KEY_FORMAT_X509));
    FREE(key_formats);
    CHECK_RESULT_OK(device->get_supported_export_formats(device,
        KM_ALGORITHM_AES, &key_formats, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT_OK(device->get_supported_export_formats(device,
        KM_ALGORITHM_HMAC, &key_formats, &len));
    CHECK_TRUE(len == 0);
    CHECK_RESULT(KM_ERROR_OUTPUT_PARAMETER_NULL,
        device->get_supported_export_formats(device, KM_ALGORITHM_RSA, NULL, NULL));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing add_rng_entropy API...");
    CHECK_RESULT_OK(device->add_rng_entropy(device,
        &test_entropy[0], sizeof(test_entropy)));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing generate_key API...");
    key_params[0].tag = KM_TAG_ALGORITHM;
    key_params[0].enumerated = KM_ALGORITHM_RSA;
    key_params[1].tag = KM_TAG_KEY_SIZE;
    key_params[1].integer = 1024;
    key_params[2].tag = KM_TAG_RSA_PUBLIC_EXPONENT;
    key_params[2].long_integer = 65537;
    key_params[3].tag = KM_TAG_PURPOSE;
    key_params[3].enumerated = KM_PURPOSE_SIGN;
    key_params[4].tag = KM_TAG_NO_AUTH_REQUIRED;
    key_params[4].boolean = true;
    key_params[5].tag = KM_TAG_PADDING;
    key_params[5].enumerated = KM_PAD_RSA_PSS;
    key_params[6].tag = KM_TAG_DIGEST;
    key_params[6].enumerated = KM_DIGEST_SHA1;
    paramset.length = 7;
    CHECK_RESULT_OK(device->generate_key(device,
        &paramset, &key_blob, &pkey_characteristics));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing begin API...");
    key_params[0].tag = KM_TAG_ALGORITHM;
    key_params[0].enumerated = KM_ALGORITHM_RSA;
    key_params[1].tag = KM_TAG_PADDING;
    key_params[1].enumerated = KM_PAD_RSA_PSS;
    key_params[2].tag = KM_TAG_DIGEST;
    key_params[2].enumerated = KM_DIGEST_SHA1;
    key_params[3].tag = KM_TAG_PURPOSE;
    key_params[3].enumerated = KM_PURPOSE_SIGN;
    paramset.length = 4;
    CHECK_RESULT_OK(device->begin(device,
        KM_PURPOSE_SIGN, &key_blob, &paramset, NULL, &operation_handle));

    LOG_I("Testing update API...");
    CHECK_RESULT_OK(device->update(device,
        operation_handle, NULL, &rsa_input_data_blob, &input_consumed, NULL, NULL));

    LOG_I("Testing finish API...");
    CHECK_RESULT_OK(device->finish(device,
        operation_handle, NULL, NULL, NULL, &rsa_output_data_blob));
    LOG_I("Data length: %zu", rsa_output_data_blob.data_length);

end:
    keymaster_free_characteristics(pkey_characteristics);
    free(pkey_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}

int main(void)
{
    keymaster_error_t res = KM_ERROR_OK;
    TeeKeymasterDevice *device = new TeeKeymasterDevice(&HAL_MODULE_INFO_SYM.common);
    keymaster1_device_t *keymaster_device = device->keymaster_device();
    uint32_t rsa_key_sizes_to_test[] = {512, 1024, 4096};
    uint32_t keysize;

    /* Determine endianness of platform. */
    {const int a = 1; am_big_endian = (*((char*)&a) == 0);}

    LOG_I("Keymaster Module");
    LOG_I("Name: %s", keymaster_device->common.module->name);
    LOG_I("Author: %s", keymaster_device->common.module->author);
    LOG_I("API version: %d" ,keymaster_device->common.module->module_api_version);

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("API smoke test...");
    CHECK_RESULT_OK(test_api(keymaster_device));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing AES...");
    for (uint32_t keysize = 128; keysize <= 256; keysize += 64) {
        CHECK_RESULT_OK(test_km_aes(keymaster_device, keysize));
    }

    LOG_I("AES import and KAT...");
    CHECK_RESULT_OK(test_km_aes_import(keymaster_device));

    LOG_I("Try importing AES key with bad length...");
    CHECK_RESULT_OK(test_km_aes_import_bad_length(keymaster_device));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing HMAC: KM_DIGEST_SHA1...");
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA1, 140));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA1, 160));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA1, 180));
    LOG_I("Testing HMAC: KM_DIGEST_SHA_2_224...");
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_224, 200));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_224, 224));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_224, 248));
    LOG_I("Testing HMAC: KM_DIGEST_SHA_2_256...");
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_256, 224));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_256, 256));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_256, 288));
    LOG_I("Testing HMAC: KM_DIGEST_SHA_2_384...");
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_384, 360));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_384, 384));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_384, 408));
    LOG_I("Testing HMAC: KM_DIGEST_SHA_2_512...");
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_512, 500));
    CHECK_RESULT_OK(test_km_hmac(keymaster_device, KM_DIGEST_SHA_2_512, 512));

    LOG_I("HMAC import and KAT...");
    CHECK_RESULT_OK(test_km_hmac_import(keymaster_device));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("RSA import and export...");
    CHECK_RESULT_OK(test_km_rsa_import_export(keymaster_device));
    CHECK_RESULT_OK(test_km_rsa_generate_export(keymaster_device));

    LOG_I("Testing RSA...");
    for (uint32_t i = 0; i < sizeof(rsa_key_sizes_to_test)/sizeof(uint32_t); i++) {
        keysize = rsa_key_sizes_to_test[i];
        CHECK_RESULT_OK(test_km_rsa(keymaster_device, keysize, 65537));
        CHECK_RESULT_OK(test_km_rsa(keymaster_device, keysize, 17));
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing ECDSA (P192)...");
    CHECK_RESULT_OK(test_km_ec(keymaster_device, 192));
    LOG_I("Testing ECDSA (P224)...");
    CHECK_RESULT_OK(test_km_ec(keymaster_device, 224));
    LOG_I("Testing ECDSA (P256)...");
    CHECK_RESULT_OK(test_km_ec(keymaster_device, 256));
    LOG_I("Testing ECDSA (P384)...");
    CHECK_RESULT_OK(test_km_ec(keymaster_device, 384));
    LOG_I("Testing ECDSA (P521)...");
    CHECK_RESULT_OK(test_km_ec(keymaster_device, 521));

    LOG_I("EC import and export...");
    CHECK_RESULT_OK(test_km_ec_import_export(keymaster_device));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing key restrictions...");
    CHECK_RESULT_OK(test_km_restrictions(keymaster_device));
end:
    return res;
}
