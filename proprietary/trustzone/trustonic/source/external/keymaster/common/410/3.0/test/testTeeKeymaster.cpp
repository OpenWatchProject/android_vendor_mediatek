/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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
#include <string.h>

#include "tee_keymaster_device.h"
#include "test_km_aes.h"
#include "test_km_hmac.h"
#include "test_km_rsa.h"
#include "test_km_ec.h"
#include "test_km_compat.h"
#include "test_km_restrictions.h"
#include "test_km_attestation.h"
#include "test_km_verbind.h"
#include "test_km_util.h"

//#undef  LOG_ANDROID
//#undef  LOG_TAG
//#define LOG_TAG "TlcTeeKeyMasterTest"

#include "log.h"

extern struct keystore_module HAL_MODULE_INFO_SYM;

/**
 * Smoke test
 *
 * @param device device
 *
 * @return KM_ERROR_OK or error
 */
static keymaster_error_t test_api(
    keymaster2_device_t *device)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t key_params[7];
    keymaster_key_blob_t key_blob = {0, 0};
    keymaster_operation_handle_t operation_handle;
    keymaster_key_characteristics_t key_characteristics;
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
        &paramset, &key_blob, &key_characteristics));
    CHECK_RESULT_OK(save_key_blob("rsa-v2.blob", &key_blob));

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
        operation_handle, NULL, NULL, NULL, NULL, &rsa_output_data_blob));
    LOG_I("Data length: %zu", rsa_output_data_blob.data_length);

end:
    keymaster_free_characteristics(&key_characteristics);
    km_free_key_blob(&key_blob);

    return res;
}

int main(void)
{
    keymaster_error_t res = KM_ERROR_OK;
    TeeKeymasterDevice *device = new TeeKeymasterDevice(&HAL_MODULE_INFO_SYM.common);
    keymaster2_device_t *keymaster_device = device->keymaster_device();
    uint32_t rsa_key_sizes_to_test[] = {512, 1024, 4096};
    size_t n_rsa_key_sizes =
        sizeof(rsa_key_sizes_to_test)/sizeof(*rsa_key_sizes_to_test);
    uint32_t keysize;
    bool quick;

    { const char *p = getenv("KM_TEST_QUICK");
        quick = p && (*p == '1' || *p == 'y' || *p == 't'); }
    if (quick) n_rsa_key_sizes--;

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
    LOG_I("Testing version binding...");
    CHECK_RESULT_OK(test_km_verbind(keymaster_device));

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
    for (uint32_t i = 0; i < n_rsa_key_sizes; i++) {
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

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing attestation...");
    CHECK_RESULT_OK(test_km_attestation(keymaster_device));

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    LOG_I("Testing blob compatibility...");
    CHECK_RESULT_OK(test_km_compat());
end:
    return res;
}
