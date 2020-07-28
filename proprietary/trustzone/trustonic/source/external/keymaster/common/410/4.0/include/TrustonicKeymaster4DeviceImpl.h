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

#ifndef TRUSTONIC_TEE_KEYMASTER_IMPL_H_
#define TRUSTONIC_TEE_KEYMASTER_IMPL_H_

#include "tlcTeeKeymaster_if.h"

class TrustonicKeymaster4DeviceImpl {
  public:
    TrustonicKeymaster4DeviceImpl();

    ~TrustonicKeymaster4DeviceImpl();

    void get_hardware_info(
        keymaster_security_level_t *security_level,
        const char **keymaster_name,
        const char **keymaster_author_name);

    keymaster_error_t get_hmac_sharing_parameters(
        keymaster_hmac_sharing_parameters_t *params);

    keymaster_error_t compute_shared_hmac(
        const keymaster_hmac_sharing_parameters_set_t *params,
        keymaster_blob_t *sharing_check);

    keymaster_error_t verify_authorization(
        keymaster_operation_handle_t operation_handle,
        const keymaster_key_param_set_t* parameters_to_verify,
        const keymaster_hw_auth_token_t *auth_token,
        keymaster_verification_token_t *token);

    keymaster_error_t add_rng_entropy(
        const uint8_t* data,
        size_t data_length);

    keymaster_error_t generate_key(
        const keymaster_key_param_set_t* params,
        keymaster_key_blob_t* key_blob,
        keymaster_key_characteristics_t* characteristics);

    keymaster_error_t get_key_characteristics(
        const keymaster_key_blob_t* key_blob,
        const keymaster_blob_t* client_id,
        const keymaster_blob_t* app_data,
        keymaster_key_characteristics_t* characteristics);

    keymaster_error_t import_key(
        const keymaster_key_param_set_t* params,
        keymaster_key_format_t key_format,
        const keymaster_blob_t* key_data,
        keymaster_key_blob_t* key_blob,
        keymaster_key_characteristics_t* characteristics);

    keymaster_error_t export_key(
        keymaster_key_format_t export_format,
        const keymaster_key_blob_t* key_to_export,
        const keymaster_blob_t* client_id,
        const keymaster_blob_t* app_data,
        keymaster_blob_t* export_data);

    keymaster_error_t attest_key(
        const keymaster_key_blob_t* key_to_attest,
        const keymaster_key_param_set_t* attest_params,
        keymaster_cert_chain_t* cert_chain);

    keymaster_error_t upgrade_key(
        const keymaster_key_blob_t* key_to_upgrade,
        const keymaster_key_param_set_t* upgrade_params,
        keymaster_key_blob_t* upgraded_key);

    keymaster_error_t begin(
        keymaster_purpose_t purpose,
        const keymaster_key_blob_t* key,
        const keymaster_key_param_set_t* params,
        const keymaster_hw_auth_token_t *auth_token,
        keymaster_key_param_set_t* out_params,
        keymaster_operation_handle_t* operation_handle);

    keymaster_error_t update(
        keymaster_operation_handle_t operation_handle,
        const keymaster_key_param_set_t* params,
        const keymaster_blob_t* input,
        const keymaster_hw_auth_token_t *auth_token,
        const keymaster_verification_token_t *verification_token,
        size_t* input_consumed,
        keymaster_key_param_set_t* out_params,
        keymaster_blob_t* output);

    keymaster_error_t finish(
        keymaster_operation_handle_t operation_handle,
        const keymaster_key_param_set_t* params,
        const keymaster_blob_t* input,
        const keymaster_blob_t* signature,
        const keymaster_hw_auth_token_t *auth_token,
        const keymaster_verification_token_t *verification_token,
        keymaster_key_param_set_t* out_params,
        keymaster_blob_t* output);

    keymaster_error_t abort(
        keymaster_operation_handle_t operation_handle);

    keymaster_error_t import_wrapped_key(
        const keymaster_blob_t* wrapped_key_data,
        const keymaster_key_blob_t* wrapping_key_blob,
        const uint8_t* masking_key,
        const keymaster_key_param_set_t* unwrapping_params,
        uint64_t password_sid,
        uint64_t biometric_sid,
        keymaster_key_blob_t* key_blob,
        keymaster_key_characteristics_t* key_characteristics);

    TEE_SessionHandle session_handle_;
    int errcode;

};


#endif  //  TRUSTONIC_TEE_KEYMASTER_IMPL_H_
