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

#include <hardware/keymaster_common.h>

#include <tee_keymaster_device.h>
#include <trustonic_tee_keymaster_impl.h>
#include <tlcTeeKeymasterN_if.h>
#include "km_shared_util.h"
#include "cust_tee_keymaster_impl.h"
#include <sys/system_properties.h>

#undef  LOG_TAG
#define LOG_TAG "TrustonicTeeKeymasterImpl"
#include "log.h"

/* Read version info from system properties.
 * Returns 0 on success, non-zero error code on failure.
 */
static int get_version_info(
    uint32_t *os_version, uint32_t *os_patchlevel)
{
    char verstr[PROP_VALUE_MAX];
    char patstr[PROP_VALUE_MAX];
    int verstrlen = __system_property_get("ro.build.version.release", verstr);
    int patstrlen = __system_property_get("ro.build.version.security_patch", patstr);

    /* Assume verstr is "X" or "X.Y" or "X.Y.Z" (etc) where each number is less than 100.
     * Convert to integer XXYYZZ. */
    int XX = 0, YY = 0, ZZ = 0;
    int i = 0;
    while ((i < verstrlen) && verstr[i] != '.') {
        XX *= 10; XX += (verstr[i] - 48);
        if (XX >= 100) return -1;
        i++;
    }
    i++; // skip the '.'
    while ((i < verstrlen) && verstr[i] != '.') {
        YY *= 10; YY += (verstr[i] - 48);
        if (YY >= 100) return -1;
        i++;
    }
    i++; // skip the '.'
    while ((i < verstrlen) && verstr[i] != '.') {
        ZZ *= 10; ZZ += (verstr[i] - 48);
        if (ZZ >= 100) return -1;
        i++;
    }
    *os_version = 100*(100*XX +  YY) + ZZ;

    /* Assume patstr is "YYYY-MM" or "YYYY-MM-DD".
     * Convert to integer YYYYMM. */
    if (patstrlen < 7) return -2;
    int D = patstr[0] - 48;
    D *= 10; D += patstr[1] - 48;
    D *= 10; D += patstr[2] - 48;
    D *= 10; D += patstr[3] - 48;
    if (patstr[4] != '-') return -2;
    D *= 10; D += patstr[5] - 48;
    D *= 10; D += patstr[6] - 48;
    *os_patchlevel = D;

    return 0;
}

/**
 * Constructor
 */
TrustonicTeeKeymasterImpl::TrustonicTeeKeymasterImpl()
    : session_handle_(NULL), errcode(0)
{
    int rc;

    uint32_t os_version, os_patchlevel;
    rc = get_version_info(&os_version, &os_patchlevel);
    if (rc) {
        LOG_E("Failed to read version info.");
        errcode = rc;
    } else {
        LOG_I("Read os_version = %u, os_patchlevel = %u", os_version, os_patchlevel);
        rc = TEE_Open(&session_handle_);
        if (rc) {
            LOG_E("Failed to open session to Keymaster TA.");
            session_handle_ = NULL;
            errcode = rc;
        } else {
            /* Configure the TA. */
            keymaster_key_param_t param[2] = {
                {KM_TAG_OS_VERSION, {os_version}},
                {KM_TAG_OS_PATCHLEVEL, {os_patchlevel}}
            };
            keymaster_key_param_set_t params = {param, 2};
            rc = HAL_Configure(session_handle_);
            if (rc) {
                LOG_E("HAL_Configure() failed.");
                TEE_Close(session_handle_);
                session_handle_ = NULL;
                errcode = rc;
            } else {
                rc = TEE_Configure(session_handle_, &params);
                if (rc) {
                    LOG_E("TEE_Configure() failed.");
                    TEE_Close(session_handle_);
                    session_handle_ = NULL;
                    errcode = rc;
                }
            }
        }
    }
}

/**
 * Destructor
 */
TrustonicTeeKeymasterImpl::~TrustonicTeeKeymasterImpl()
{
    TEE_Close(session_handle_);
}


#define CHECK_SESSION(handle) \
    if (handle == NULL) { \
        LOG_E("%s: Invalid session handle", __func__); \
        return KM_ERROR_SECURE_HW_COMMUNICATION_FAILED; \
    }\

keymaster_error_t TrustonicTeeKeymasterImpl::configure(
    const keymaster_key_param_set_t* params)
{
    /* TODO Remove from API for Keymaster3. */
    (void)params;
    return KM_ERROR_OK;
}

keymaster_error_t TrustonicTeeKeymasterImpl::add_rng_entropy(
    const uint8_t* data,
    size_t data_length)
{
    CHECK_SESSION(session_handle_);
    return TEE_AddRngEntropy(session_handle_, data, data_length);
}

keymaster_error_t TrustonicTeeKeymasterImpl::generate_key(
    const keymaster_key_param_set_t* params,
    keymaster_key_blob_t* key_blob,
    keymaster_key_characteristics_t* characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_GenerateKey(session_handle_,
        params, key_blob, characteristics);
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_key_characteristics(
    const keymaster_key_blob_t* key_blob,
    const keymaster_blob_t* client_id,
    const keymaster_blob_t* app_data,
    keymaster_key_characteristics_t* characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_GetKeyCharacteristics(session_handle_,
        key_blob, client_id, app_data, characteristics);
}

keymaster_error_t TrustonicTeeKeymasterImpl::import_key(
    const keymaster_key_param_set_t* params,
    keymaster_key_format_t key_format,
    const keymaster_blob_t* key_data,
    keymaster_key_blob_t* key_blob,
    keymaster_key_characteristics_t* characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_ImportKey(session_handle_,
        params, key_format, key_data, key_blob, characteristics);
}

keymaster_error_t TrustonicTeeKeymasterImpl::export_key(
    keymaster_key_format_t export_format,
    const keymaster_key_blob_t* key_to_export,
    const keymaster_blob_t* client_id,
    const keymaster_blob_t* app_data,
    keymaster_blob_t* export_data)
{
    CHECK_SESSION(session_handle_);
    return TEE_ExportKey(session_handle_,
        export_format, key_to_export, client_id, app_data, export_data);
}

keymaster_error_t TrustonicTeeKeymasterImpl::attest_key(
    const keymaster_key_blob_t* key_to_attest,
    const keymaster_key_param_set_t* attest_params,
    keymaster_cert_chain_t* cert_chain)
{
    CHECK_SESSION(session_handle_);
    return TEE_AttestKey(session_handle_, key_to_attest, attest_params, cert_chain);
}

keymaster_error_t TrustonicTeeKeymasterImpl::upgrade_key(
    const keymaster_key_blob_t* key_to_upgrade,
    const keymaster_key_param_set_t* upgrade_params,
    keymaster_key_blob_t* upgraded_key)
{
    CHECK_SESSION(session_handle_);
    return TEE_UpgradeKey(session_handle_, key_to_upgrade, upgrade_params, upgraded_key);
}

keymaster_error_t TrustonicTeeKeymasterImpl::delete_key(
    const keymaster_key_blob_t* key)
{
    CHECK_SESSION(session_handle_);
    return TEE_DeleteKey(session_handle_, key);
}

keymaster_error_t TrustonicTeeKeymasterImpl::delete_all_keys()
{
    CHECK_SESSION(session_handle_);
    return TEE_DeleteAllKeys(session_handle_);
}

keymaster_error_t TrustonicTeeKeymasterImpl::begin(
    keymaster_purpose_t purpose,
    const keymaster_key_blob_t* key,
    const keymaster_key_param_set_t* params,
    keymaster_key_param_set_t* out_params,
    keymaster_operation_handle_t* operation_handle)
{
    CHECK_SESSION(session_handle_);
    return TEE_Begin(session_handle_,
        purpose, key, params, out_params, operation_handle);
}

keymaster_error_t TrustonicTeeKeymasterImpl::update(
    keymaster_operation_handle_t operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t* input,
    size_t* input_consumed,
    keymaster_key_param_set_t* out_params,
    keymaster_blob_t* output)
{
    CHECK_SESSION(session_handle_);
    return TEE_Update(session_handle_,
        operation_handle, params, input, input_consumed, out_params, output);
}

keymaster_error_t TrustonicTeeKeymasterImpl::finish(
    keymaster_operation_handle_t operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t* input,
    const keymaster_blob_t* signature,
    keymaster_key_param_set_t* out_params,
    keymaster_blob_t* output)
{
    CHECK_SESSION(session_handle_);
    return TEE_Finish(session_handle_,
        operation_handle, params, input, signature, out_params, output);
}

keymaster_error_t TrustonicTeeKeymasterImpl::abort(
    keymaster_operation_handle_t operation_handle)
{
    CHECK_SESSION(session_handle_);
    return TEE_Abort(session_handle_, operation_handle);
}
