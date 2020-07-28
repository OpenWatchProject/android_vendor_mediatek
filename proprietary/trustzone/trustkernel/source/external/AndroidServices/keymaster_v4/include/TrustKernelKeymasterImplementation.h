/*
* Copyright (C) 2018 TrustKernel Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TRUSTKERNEL_KEYMASTER_IMPLEMENTAION_H_
#define TRUSTKERNEL_KEYMASTER_IMPLEMENTAION_H_

#include <android/log.h>

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <type_traits>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>

#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include <keymaster/keymaster_tags.h>

#include <tee_client_api.h>

namespace trustkernel {

struct EVP_PKEY_Delete {
    void operator()(EVP_PKEY* p) const { EVP_PKEY_free(p); }
};

typedef std::unique_ptr<EVP_PKEY, EVP_PKEY_Delete> Unique_EVP_PKEY;

struct PKCS8_PRIV_KEY_INFO_Delete {
    void operator()(PKCS8_PRIV_KEY_INFO* p) const { PKCS8_PRIV_KEY_INFO_free(p); }
};

typedef std::unique_ptr<PKCS8_PRIV_KEY_INFO, PKCS8_PRIV_KEY_INFO_Delete> Unique_PKCS8_PRIV_KEY_INFO;

struct RSA_Delete {
    void operator()(RSA* p) const { RSA_free(p); }
};

typedef std::unique_ptr<RSA, RSA_Delete> Unique_RSA;

struct EC_Delete {
    void operator()(EC_KEY* p) const { EC_KEY_free(p); }
};

struct BN_Delete {
    void operator()(BIGNUM* p) const { BN_free(p); }
};

struct Malloc_Free {
    void operator()(void* p) const { free(p); }
};

struct Characteristics_Delete {
    void operator()(keymaster_key_characteristics_t *character) const {
        keymaster_free_characteristics(character);
    }
};

typedef struct {
    keymaster_blob_t seed;
    uint8_t nonce[32];
} keymaster_hmac_sharing_parameters_t;

typedef struct __attribute__((__packed__)) {
    uint8_t version;
    uint64_t challenge;
    uint64_t user_id;
    uint64_t authenticator_id;
    uint32_t authenticator_type;
    uint64_t timestamp;
    uint8_t mac[32];
} keymaster_hardware_auth_token_t;

typedef struct {
    uint64_t challenge;
    uint64_t timestamp;

    keymaster_security_level_t security_level;
    keymaster_key_param_set_t parameters_verified;
    uint8_t mac[32];
} keymaster_verification_token_t;

class TrustKernelKeymasterImplementation {

private:
    int error_;
    TEEC_Context context;
    TEEC_Session session;

    std::unique_ptr<uint8_t, Malloc_Free> configureData;
    uint32_t configureDataSize;
    bool configured = false;

    std::unique_ptr<uint8_t, Malloc_Free> computeHmacData;
    uint32_t computeHmacDataSize;

    TEEC_Result OpenTEESession();
    void CloseTEESession();

    TEEC_Result reconfigure();

    bool invoke(const char *file, uint32_t lineno,
                uint32_t cmdId, uint32_t va, uint32_t vb,
                const void *inMem, uint32_t inMemSize, void *outMem, uint32_t outMemSize,
                uint32_t *outA = NULL, uint32_t *outB = NULL);

    bool invoke(const char *file, uint32_t lineno,
                uint32_t cmdId, uint32_t va, uint32_t vb,
                const void *inMem, uint32_t inMemSize, void *outMem, uint32_t *outMemSize,
                uint32_t *outA = NULL, uint32_t *outB = NULL);

    uint32_t marshal_size(const keymaster_key_param_set_t *params);
    void marshal_to(uint8_t *pMarshaled, const keymaster_key_param_set_t *params);
    uint8_t *marshal(const keymaster_key_param_set_t *params, uint32_t *pSize);

    keymaster_key_param_set_t unmarshal_param_set(uint8_t *marshalled, uint32_t marshalled_size, uint8_t **next_marshalled_position);

    keymaster_key_characteristics_t unmarshal_character(uint8_t *marshalled,
                                        uint32_t marshalled_size);

    int convert_to_evp(keymaster_algorithm_t algorithm);

public:

    TrustKernelKeymasterImplementation();

    ~TrustKernelKeymasterImplementation();

    keymaster_error_t get_hmac_sharing_parameters(
                    keymaster_hmac_sharing_parameters_t *params);

    keymaster_error_t compute_shared_hmac(
                    const uint8_t *params, size_t params_size,
                    keymaster_blob_t *sharing_check);

    keymaster_error_t verify_authorization(
                    uint64_t challenge,
                    const keymaster_key_param_set_t *attest_params,
                    const keymaster_hardware_auth_token_t *auth_token,
                    keymaster_verification_token_t *verify_token);

    keymaster_error_t add_rng_entropy(const uint8_t* data __unused,
             size_t data_length __unused);

    keymaster_error_t generate_key(const keymaster_key_param_set_t *params,
                                keymaster_key_blob_t *key_blob,
                                keymaster_key_characteristics_t *characteristics);

    keymaster_error_t get_key_characteristics(const keymaster_key_blob_t* key_blob,
                        const keymaster_blob_t *client_id,
                        const keymaster_blob_t *app_data,
                        keymaster_key_characteristics_t *characteristics);


    keymaster_error_t convert_pkcs8_blob_to_evp(
                    const uint8_t *key_data, size_t key_length,
                    keymaster_algorithm_t expected_algorithm,
                    std::unique_ptr<EVP_PKEY, EVP_PKEY_Delete>* pkey);

    keymaster_error_t import_key(
             const keymaster_key_param_set_t *params,
             keymaster_key_format_t key_format,
             const keymaster_blob_t *key_data,
             keymaster_key_blob_t *key_blob,
             keymaster_key_characteristics_t *characteristics);

    keymaster_error_t import_wrapped_key(
             const keymaster_key_blob_t *wrapped_key_data,
             const keymaster_key_blob_t *wrapping_key_data,
             const keymaster_key_blob_t *masking_key_data,
             const keymaster_key_param_set_t *params,
             uint64_t password_sid,
             uint64_t biometric_sid,
             keymaster_key_blob_t *key_blob,
             keymaster_key_characteristics_t *characteristics);

    keymaster_error_t export_key(
             keymaster_key_format_t export_format,
             const keymaster_key_blob_t *key_to_export,
             const keymaster_blob_t *client_id,
             const keymaster_blob_t *app_data,
             keymaster_blob_t *export_data);

    keymaster_error_t delete_key(const keymaster_key_blob_t* key);

    keymaster_error_t delete_all_keys();

    keymaster_error_t begin(keymaster_purpose_t purpose,
                const keymaster_key_blob_t* key,
                const keymaster_key_param_set_t* in_params,
                const keymaster_blob_t *auth_token_blob,
                keymaster_key_param_set_t* out_params,
                keymaster_operation_handle_t* operation_handle);

    keymaster_error_t update(
                keymaster_operation_handle_t operation_handle,
                const keymaster_key_param_set_t* in_params,
                const keymaster_blob_t* input,
                size_t* input_consumed,
                const keymaster_blob_t *auth_token_blob,
                keymaster_key_param_set_t* out_params,
                keymaster_blob_t* output);

    keymaster_error_t finish(
                keymaster_operation_handle_t operation_handle,
                const keymaster_key_param_set_t* in_params,
                const keymaster_blob_t* input,
                const keymaster_blob_t* signature,
                const keymaster_blob_t *auth_token_blob,
                keymaster_key_param_set_t* out_params,
                keymaster_blob_t* output);

    keymaster_error_t abort(keymaster_operation_handle_t operation_handle);

    keymaster_error_t attest_key(
                    const keymaster_key_blob_t *key_to_attest,
                    const keymaster_key_param_set_t *attest_params,
                    keymaster_cert_chain_t *cert_chain);

    keymaster_error_t configure(const keymaster_key_param_set_t *params);

    keymaster_error_t upgrade_key(
                    const keymaster_key_blob_t *key_to_upgrade,
                    const keymaster_key_param_set_t *upgrade_params,
                    keymaster_key_blob_t *upgraded_key);

    keymaster_error_t generate_attk_key_pair(const uint8_t copy_num);

    keymaster_error_t verify_attk_key_pair();

    keymaster_error_t export_attk_public_key(
                    const uint8_t *pub_key_data,
                    size_t *pub_key_data_length);

    keymaster_error_t get_device_id(
                        const uint8_t *device_id,
                        size_t *device_id_length);
};
}

#endif
