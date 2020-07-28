/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#include <TrustonicKeymaster4DeviceImpl.h>
#include <tlcTeeKeymaster_if.h>
#include "km_shared_util.h"
#include "cust_tee_keymaster_impl.h"
#include <sys/system_properties.h>
#include <time.h>

#undef  LOG_TAG
#define LOG_TAG "TrustonicKeymaster4DeviceImpl"

/* Assume date string is "YYYY-MM" or "YYYY-MM-DD"; convert to integer YYYYMMDD
   (where in the former case DD is 00). */
static bool int_from_datestr(
    uint32_t *D,
    const char *s,
    size_t s_len)
{
    uint32_t x;
    if (s_len < 7) return false;
    x = s[0] - 48;
    x *= 10; x += s[1] - 48;
    x *= 10; x += s[2] - 48;
    x *= 10; x += s[3] - 48;
    if (s[4] != '-') return false;
    x *= 10; x += s[5] - 48;
    x *= 10; x += s[6] - 48;
    if (s_len >= 10 && s[7] == '-') {
        x *= 10; x += s[8] - 48;
        x *= 10; x += s[9] - 48;
    } else {
        x *= 100;
    }
    *D = x;
    return true;
}

/* Read OS_VERSION, OS_PATCHLEVEL and VENDOR_PATCHLEVL from system properties.
 *
 * OS_VERSION should have the format XXYYZZ where XX, YY and ZZ are the major,
 * minor and sub-minor version numbers.
 *
 * OS_PATCHLEVEL should have the format YYYYMM (year and month).
 *
 * VENDOR_PATCHLEVEL should have the format YYYYMMDD (year, month and day).
 *
 * Returns 0 on success, non-zero error code on failure.
 */
static int get_version_info(
    bootimage_info_t *bootinfo)
{
    char os_version_str[PROP_VALUE_MAX];
    int os_version_strlen = __system_property_get("ro.build.version.release", os_version_str);
    char os_patchlevel_str[PROP_VALUE_MAX];
    int os_patchlevel_strlen = __system_property_get("ro.build.version.security_patch", os_patchlevel_str);
    char vendor_patchlevel_str[PROP_VALUE_MAX];
    int vendor_patchlevel_strlen = __system_property_get("ro.vendor.build.security_patch", vendor_patchlevel_str);

    /* Assume os_version_str is "X" or "X.Y" or "X.Y.Z" (etc) where each number is less than 100.
     * Convert to integer XXYYZZ. */
    int XX = 0, YY = 0, ZZ = 0;
    int i = 0;
    while ((i < os_version_strlen) && os_version_str[i] != '.') {
        XX *= 10; XX += (os_version_str[i] - 48);
        if (XX >= 100) return -1;
        i++;
    }
    i++; // skip the '.'
    while ((i < os_version_strlen) && os_version_str[i] != '.') {
        YY *= 10; YY += (os_version_str[i] - 48);
        if (YY >= 100) return -1;
        i++;
    }
    i++; // skip the '.'
    while ((i < os_version_strlen) && os_version_str[i] != '.') {
        ZZ *= 10; ZZ += (os_version_str[i] - 48);
        if (ZZ >= 100) return -1;
        i++;
    }
    bootinfo->os_version = 100*(100*XX +  YY) + ZZ;

    if (!int_from_datestr(&bootinfo->os_patchlevel,
        os_patchlevel_str, os_patchlevel_strlen))
    {
        bootinfo->os_patchlevel = 0;
    }
    bootinfo->os_patchlevel /= 100;

    if (!int_from_datestr(&bootinfo->vendor_patchlevel,
        vendor_patchlevel_str, vendor_patchlevel_strlen))
    {
        return -2;
    }

    /* The boot patch level is not available from system properties. Instead,
     * the bootloader reads it from boot.img and provides it to Keymaster (via
     * a driver). */
    bootinfo->boot_patchlevel = 0;

    return 0;
}

/**
 * Constructor
 */
TrustonicKeymaster4DeviceImpl::TrustonicKeymaster4DeviceImpl()
    : session_handle_(NULL), errcode(0)
{
    int rc;

    bootimage_info_t bootinfo;
    rc = get_version_info(&bootinfo);
    if (rc) {
        LOG_E("Failed to read version info.");
        errcode = rc;
    } else {
        LOG_I("Read os_version = %u, os_patchlevel = %u, vendor_patchlevel = %u, boot_patchlevel = %u", bootinfo.os_version, bootinfo.os_patchlevel, bootinfo.vendor_patchlevel, bootinfo.boot_patchlevel);
        rc = TEE_Open(&session_handle_);
        if (rc) {
            LOG_E("Failed to open session to Keymaster TA.");
            session_handle_ = NULL;
            errcode = rc;
        } else {
            /* Configure the TA. */
            keymaster_key_param_t param[3] = {
                {KM_TAG_OS_VERSION, {bootinfo.os_version}},
                {KM_TAG_OS_PATCHLEVEL, {bootinfo.os_patchlevel}},
                {KM_TAG_VENDOR_PATCHLEVEL, {bootinfo.vendor_patchlevel}}
            };
            keymaster_key_param_set_t params = {param, 3};
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
TrustonicKeymaster4DeviceImpl::~TrustonicKeymaster4DeviceImpl()
{
    TEE_Close(session_handle_);
}


#define CHECK_SESSION(handle) \
    if (handle == NULL) { \
        LOG_E("%s: Invalid session handle", __func__); \
        return KM_ERROR_SECURE_HW_COMMUNICATION_FAILED; \
    }

static const char *km_name = "Kinibi Keymaster4";
static const char *km_author_name = "Trustonic";

void TrustonicKeymaster4DeviceImpl::get_hardware_info(
    keymaster_security_level_t *security_level,
    const char **keymaster_name,
    const char **keymaster_author_name)
{
    *security_level = KM_SECURITY_LEVEL_TRUSTED_ENVIRONMENT;
    *keymaster_name = km_name;
    *keymaster_author_name = km_author_name;
}

static const uint8_t dummy_nonce[32] = {0};
keymaster_error_t TrustonicKeymaster4DeviceImpl::get_hmac_sharing_parameters(
    keymaster_hmac_sharing_parameters_t *params)
{
    CHECK_SESSION(session_handle_);
    /* StrongBox integration not supported, but we must return OK */
    params->seed.data = NULL;
    params->seed.data_length = 0;
    memcpy(params->nonce, dummy_nonce, sizeof(dummy_nonce));
    return KM_ERROR_OK;
}

static const uint8_t dummy_sharing_check[32] = {0};
keymaster_error_t TrustonicKeymaster4DeviceImpl::compute_shared_hmac(
    const keymaster_hmac_sharing_parameters_set_t *params,
    keymaster_blob_t *sharing_check)
{
    CHECK_SESSION(session_handle_);
    /* StrongBox integration not supported, but we must return OK, as long as
     * the params match what GetHmacSharingParameters returned.
     */
    if (params->length != 1) { /* We are not alone? */
        return KM_ERROR_INVALID_ARGUMENT;
    }
    keymaster_hmac_sharing_parameters_t *params0 = &params->params[0];
    if ((params0->seed.data_length != 0) ||
        memcmp(params0->nonce, dummy_nonce, sizeof(dummy_nonce)))
    {
        return KM_ERROR_INVALID_ARGUMENT;
    }
    sharing_check->data = dummy_sharing_check;
    sharing_check->data_length = sizeof(dummy_sharing_check);
    return KM_ERROR_OK;
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::verify_authorization(
    keymaster_operation_handle_t operation_handle,
    const keymaster_key_param_set_t* parameters_to_verify,
    const keymaster_hw_auth_token_t *auth_token,
    keymaster_verification_token_t *token)
{
    CHECK_SESSION(session_handle_);
    (void)parameters_to_verify;
    (void)auth_token;
    memset(token, 0, sizeof(keymaster_verification_token_t));
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    token->challenge = operation_handle;
    token->timestamp = 1000 * spec.tv_sec + spec.tv_nsec / 1000000;
    token->security_level = KM_SECURITY_LEVEL_TRUSTED_ENVIRONMENT;
    return KM_ERROR_OK;
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::add_rng_entropy(
    const uint8_t* data,
    size_t data_length)
{
    CHECK_SESSION(session_handle_);
    return TEE_AddRngEntropy(session_handle_, data, data_length);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::generate_key(
    const keymaster_key_param_set_t* params,
    keymaster_key_blob_t* key_blob,
    keymaster_key_characteristics_t* characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_GenerateKey(session_handle_,
        params, key_blob, characteristics);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::get_key_characteristics(
    const keymaster_key_blob_t* key_blob,
    const keymaster_blob_t* client_id,
    const keymaster_blob_t* app_data,
    keymaster_key_characteristics_t* characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_GetKeyCharacteristics(session_handle_,
        key_blob, client_id, app_data, characteristics);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::import_key(
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

keymaster_error_t TrustonicKeymaster4DeviceImpl::export_key(
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

keymaster_error_t TrustonicKeymaster4DeviceImpl::attest_key(
    const keymaster_key_blob_t* key_to_attest,
    const keymaster_key_param_set_t* attest_params,
    keymaster_cert_chain_t* cert_chain)
{
    CHECK_SESSION(session_handle_);
    return TEE_AttestKey(session_handle_, key_to_attest, attest_params, cert_chain);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::upgrade_key(
    const keymaster_key_blob_t* key_to_upgrade,
    const keymaster_key_param_set_t* upgrade_params,
    keymaster_key_blob_t* upgraded_key)
{
    CHECK_SESSION(session_handle_);
    return TEE_UpgradeKey(session_handle_, key_to_upgrade, upgrade_params, upgraded_key);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::begin(
    keymaster_purpose_t purpose,
    const keymaster_key_blob_t* key,
    const keymaster_key_param_set_t* params,
    const keymaster_hw_auth_token_t *auth_token,
    keymaster_key_param_set_t* out_params,
    keymaster_operation_handle_t* operation_handle)
{
    CHECK_SESSION(session_handle_);
    return TEE_Begin(session_handle_,
        purpose, key, params, auth_token, out_params, operation_handle);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::update(
    keymaster_operation_handle_t operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t* input,
    const keymaster_hw_auth_token_t *auth_token,
    const keymaster_verification_token_t *verification_token,
    size_t* input_consumed,
    keymaster_key_param_set_t* out_params,
    keymaster_blob_t* output)
{
    CHECK_SESSION(session_handle_);
    (void)verification_token;
    return TEE_Update(session_handle_,
        operation_handle, params, input, auth_token, input_consumed, out_params, output);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::finish(
    keymaster_operation_handle_t operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t* input,
    const keymaster_blob_t* signature,
    const keymaster_hw_auth_token_t *auth_token,
    const keymaster_verification_token_t *verification_token,
    keymaster_key_param_set_t* out_params,
    keymaster_blob_t* output)
{
    CHECK_SESSION(session_handle_);
    (void)verification_token;
    return TEE_Finish(session_handle_,
        operation_handle, params, input, signature, auth_token, out_params, output);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::abort(
    keymaster_operation_handle_t operation_handle)
{
    CHECK_SESSION(session_handle_);
    return TEE_Abort(session_handle_, operation_handle);
}

keymaster_error_t TrustonicKeymaster4DeviceImpl::import_wrapped_key(
    const keymaster_blob_t* wrapped_key_data,
    const keymaster_key_blob_t* wrapping_key_blob,
    const uint8_t* masking_key,
    const keymaster_key_param_set_t* unwrapping_params,
    uint64_t password_sid,
    uint64_t biometric_sid,
    keymaster_key_blob_t* key_blob,
    keymaster_key_characteristics_t* key_characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_ImportWrappedKey(session_handle_, wrapped_key_data,
        wrapping_key_blob, masking_key, unwrapping_params, password_sid,
        biometric_sid, key_blob, key_characteristics);
}
