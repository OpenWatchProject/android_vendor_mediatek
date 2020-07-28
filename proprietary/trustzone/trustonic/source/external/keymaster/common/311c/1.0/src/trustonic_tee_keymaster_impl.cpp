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

#ifdef SOTER
#include "mtk_keymaster_common.h"
#else
#include <hardware/keymaster_common.h>
#endif
#include <tee_keymaster_device.h>
#include <trustonic_tee_keymaster_impl.h>
#include <tlcTeeKeymasterM_if.h>
#include "km_shared_util.h"

#undef  LOG_TAG
#define LOG_TAG "TrustonicTeeKeymasterImpl"
#include "log.h"

// Supported algorithms
static const keymaster_algorithm_t supported_algorithms[]   = {
    KM_ALGORITHM_RSA,
    KM_ALGORITHM_EC,
    KM_ALGORITHM_AES,
    KM_ALGORITHM_HMAC
};

// Supported block modes
static const keymaster_block_mode_t supported_block_modes[] = {
    KM_MODE_ECB,
    KM_MODE_CBC,
    KM_MODE_CTR,
    KM_MODE_GCM
};

// Supported padding modes
static const keymaster_padding_t supported_aes_padding_modes[] = {
    KM_PAD_NONE,
    KM_PAD_PKCS7
};
static const keymaster_padding_t supported_rsa_sig_padding[] = {
    KM_PAD_NONE,
    KM_PAD_RSA_PKCS1_1_5_SIGN,
    KM_PAD_RSA_PSS
};
static const keymaster_padding_t supported_rsa_crypt_padding[] = {
    KM_PAD_NONE,
    KM_PAD_RSA_OAEP,
    KM_PAD_RSA_PKCS1_1_5_ENCRYPT
};

// Supported digests
static const keymaster_digest_t supported_hmac_digests[] = {
    KM_DIGEST_SHA1,
    KM_DIGEST_SHA_2_224,
    KM_DIGEST_SHA_2_256,
    KM_DIGEST_SHA_2_384,
    KM_DIGEST_SHA_2_512
};
static const keymaster_digest_t supported_rsa_sig_digests[] = {
    KM_DIGEST_NONE,
    KM_DIGEST_MD5,
    KM_DIGEST_SHA1,
    KM_DIGEST_SHA_2_224,
    KM_DIGEST_SHA_2_256,
    KM_DIGEST_SHA_2_384,
    KM_DIGEST_SHA_2_512
};
static const keymaster_digest_t supported_rsa_crypt_digests[] = {
    KM_DIGEST_NONE,
    KM_DIGEST_MD5,
    KM_DIGEST_SHA1,
    KM_DIGEST_SHA_2_224,
    KM_DIGEST_SHA_2_256,
    KM_DIGEST_SHA_2_384,
    KM_DIGEST_SHA_2_512
};
static const keymaster_digest_t supported_ec_digests[] = {
    KM_DIGEST_NONE,
    KM_DIGEST_SHA1,
    KM_DIGEST_SHA_2_224,
    KM_DIGEST_SHA_2_256,
    KM_DIGEST_SHA_2_384,
    KM_DIGEST_SHA_2_512
};

// Key formats
static const keymaster_key_format_t public_key_export_formats[]     = { KM_KEY_FORMAT_X509 };
static const keymaster_key_format_t asymmetric_key_import_formats[] = { KM_KEY_FORMAT_PKCS8 };
static const keymaster_key_format_t symmetric_key_import_formats[]  = { KM_KEY_FORMAT_RAW };

/**
 * Constructor
 */
TrustonicTeeKeymasterImpl::TrustonicTeeKeymasterImpl()
    : session_handle_(NULL), errcode(0)
{
    do {
        int rc = TEE_Open(&session_handle_);
        if (rc) {
            LOG_E("Failed to open session to Keymaster TA.");
            session_handle_ = NULL;
            LOG_E("Waiting for mcDriverDaemon ...");
            sleep(1);
            //errcode = rc;
        } else {
            break;
        }
    } while(true);
}


/**
 * Destructor
 */
TrustonicTeeKeymasterImpl::~TrustonicTeeKeymasterImpl()
{
    TEE_Close(session_handle_);
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_supported_algorithms(
    keymaster_algorithm_t**         algorithms,
    size_t*                         algorithms_length)
{
    if ((algorithms == NULL) || (algorithms_length == NULL)) {
        return KM_ERROR_OUTPUT_PARAMETER_NULL;
    }
    *algorithms = reinterpret_cast<keymaster_algorithm_t*>(malloc(sizeof(supported_algorithms)));
    if (*algorithms == NULL) {
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }
    memcpy(*algorithms, &supported_algorithms[0], sizeof(supported_algorithms));
    *algorithms_length = sizeof(supported_algorithms) / sizeof(keymaster_algorithm_t);

    return KM_ERROR_OK;
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_supported_block_modes(
    keymaster_algorithm_t           algorithm,
    keymaster_purpose_t             purpose,
    keymaster_block_mode_t**        modes,
    size_t*                         modes_length)
{
    if ((modes == NULL) || (modes_length == NULL)) {
        return KM_ERROR_OUTPUT_PARAMETER_NULL;
    }

    if (!check_algorithm_purpose(algorithm, purpose)) {
        return KM_ERROR_UNSUPPORTED_PURPOSE;
    }

    *modes = NULL;
    *modes_length = 0;
    if (algorithm == KM_ALGORITHM_AES) {
        *modes = reinterpret_cast<keymaster_block_mode_t*>(malloc(sizeof(supported_block_modes)));
        if (*modes == NULL) {
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
        }
        memcpy(*modes, &supported_block_modes[0], sizeof(supported_block_modes));
        *modes_length = sizeof(supported_block_modes) / sizeof(keymaster_block_mode_t);
    }

    return KM_ERROR_OK;
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_supported_padding_modes(
    keymaster_algorithm_t           algorithm,
    keymaster_purpose_t             purpose,
    keymaster_padding_t**           modes,
    size_t*                         modes_length)
{
    const keymaster_padding_t* supported_padding_modes = NULL;
    size_t               supported_padding_modes_length = 0;

    if ((modes == NULL) || (modes_length == NULL)) {
        return KM_ERROR_OUTPUT_PARAMETER_NULL;
    }

    if (!check_algorithm_purpose(algorithm, purpose)) {
        return KM_ERROR_UNSUPPORTED_PURPOSE;
    }

    switch (purpose) {
        case KM_PURPOSE_SIGN:
        case KM_PURPOSE_VERIFY:
            if (algorithm == KM_ALGORITHM_RSA) {
                supported_padding_modes = &supported_rsa_sig_padding[0];
                supported_padding_modes_length = sizeof(supported_rsa_sig_padding);
            }
            break;
        case KM_PURPOSE_ENCRYPT:
        case KM_PURPOSE_DECRYPT:
            if (algorithm == KM_ALGORITHM_RSA) {
                    supported_padding_modes = &supported_rsa_crypt_padding[0];
                    supported_padding_modes_length = sizeof(supported_rsa_crypt_padding);
            } else if (algorithm == KM_ALGORITHM_AES) {
                    supported_padding_modes = &supported_aes_padding_modes[0];
                    supported_padding_modes_length = sizeof(supported_aes_padding_modes);
            }
            break;
        default: /* shouldn't get here */
            break;
    }

    *modes = NULL;
    *modes_length = 0;
    if (supported_padding_modes != NULL) {
        *modes = reinterpret_cast<keymaster_padding_t*>(malloc(supported_padding_modes_length));
        if (*modes == NULL) {
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
        }

        memcpy(*modes, supported_padding_modes, supported_padding_modes_length);
        *modes_length = supported_padding_modes_length / sizeof(keymaster_padding_t);
    }

    return KM_ERROR_OK;
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_supported_digests(
    keymaster_algorithm_t           algorithm,
    keymaster_purpose_t             purpose,
    keymaster_digest_t**            digests,
    size_t*                         digests_length)
{
    const keymaster_digest_t *supported_digests = NULL;
    size_t supported_digests_length = 0;

    if ((digests == NULL) || (digests_length == NULL)) {
        return KM_ERROR_OUTPUT_PARAMETER_NULL;
    }

    if (!check_algorithm_purpose(algorithm, purpose)) {
        return KM_ERROR_UNSUPPORTED_PURPOSE;
    }

    switch (algorithm) {
        case KM_ALGORITHM_RSA:
            if ((purpose == KM_PURPOSE_SIGN) ||
                (purpose == KM_PURPOSE_VERIFY))
            {
                supported_digests = &supported_rsa_sig_digests[0];
                supported_digests_length = sizeof(supported_rsa_sig_digests);
            } else if ((purpose == KM_PURPOSE_ENCRYPT) ||
                       (purpose == KM_PURPOSE_DECRYPT))
            {
                supported_digests = &supported_rsa_crypt_digests[0];
                supported_digests_length = sizeof(supported_rsa_crypt_digests);
            }
            break;
        case KM_ALGORITHM_EC:
            if ((purpose == KM_PURPOSE_SIGN) ||
                (purpose == KM_PURPOSE_VERIFY))
            {
                supported_digests = &supported_ec_digests[0];
                supported_digests_length = sizeof(supported_ec_digests);
            }
            break;
        case KM_ALGORITHM_HMAC:
            if ((purpose == KM_PURPOSE_SIGN) ||
                (purpose == KM_PURPOSE_VERIFY))
            {
                supported_digests = &supported_hmac_digests[0];
                supported_digests_length = sizeof(supported_hmac_digests);
            }
            break;
        default: /* shouldn't get here */
            break;
    }

    *digests = NULL;
    *digests_length = 0;

    if (supported_digests != NULL) {
        *digests = reinterpret_cast<keymaster_digest_t*>(malloc(supported_digests_length));
        if (*digests == NULL) {
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
        }

        memcpy(*digests, supported_digests, supported_digests_length);
        *digests_length = supported_digests_length / sizeof(keymaster_digest_t);
    }

    return KM_ERROR_OK;
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_supported_import_formats(
    keymaster_algorithm_t           algorithm,
    keymaster_key_format_t**        formats,
    size_t*                         formats_length)
{
    const keymaster_key_format_t* supported_key_formats = NULL;
    size_t                 supported_key_formats_length = 0;

    if ((formats == NULL) || (formats_length == NULL)) {
        return KM_ERROR_OUTPUT_PARAMETER_NULL;
    }

    switch (algorithm) {
        case KM_ALGORITHM_RSA:
        case KM_ALGORITHM_EC:
            supported_key_formats = &asymmetric_key_import_formats[0];
            supported_key_formats_length = sizeof(asymmetric_key_import_formats);
            break;
        case KM_ALGORITHM_AES:
        case KM_ALGORITHM_HMAC:
            supported_key_formats = &symmetric_key_import_formats[0];
            supported_key_formats_length = sizeof(symmetric_key_import_formats);
            break;
        default:
            break;
    }

    *formats = NULL;
    *formats_length = 0;
    if (supported_key_formats != NULL) {
        *formats = reinterpret_cast<keymaster_key_format_t*>(malloc(supported_key_formats_length));
        if (*formats == NULL) {
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
        }
        memcpy(*formats, supported_key_formats, supported_key_formats_length);
        *formats_length = supported_key_formats_length / sizeof(keymaster_key_format_t);
    }

    return KM_ERROR_OK;
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_supported_export_formats(
    keymaster_algorithm_t            algorithm,
    keymaster_key_format_t**         formats,
    size_t*                          formats_length)
{
    const keymaster_key_format_t* supported_key_formats = NULL;
    size_t                 supported_key_formats_length = 0;

    if ((formats == NULL) || (formats_length == NULL)) {
        return KM_ERROR_OUTPUT_PARAMETER_NULL;
    }

    switch (algorithm)
    {
        case KM_ALGORITHM_RSA:
        case KM_ALGORITHM_EC:
            supported_key_formats = &public_key_export_formats[0];
            supported_key_formats_length = sizeof(public_key_export_formats);
            break;
        default:
            break;
    }

    *formats = NULL;
    *formats_length = 0;
    if (supported_key_formats != NULL) {
        *formats = reinterpret_cast<keymaster_key_format_t*>(malloc(supported_key_formats_length));
        if (*formats == NULL) {
            return KM_ERROR_MEMORY_ALLOCATION_FAILED;
        }
        memcpy(*formats, supported_key_formats, supported_key_formats_length);
        *formats_length = supported_key_formats_length / sizeof(keymaster_key_format_t);
    }

    return KM_ERROR_OK;
}

#define CHECK_SESSION(handle) \
        if (handle == NULL) { \
            LOG_E("%s: Invalid session handle", __func__); \
            return KM_ERROR_SECURE_HW_COMMUNICATION_FAILED; \
        }\

keymaster_error_t TrustonicTeeKeymasterImpl::add_rng_entropy(
    const uint8_t*                  data,
    size_t                          data_length)
{
    CHECK_SESSION(session_handle_);
    return TEE_AddRngEntropy(session_handle_, data, data_length);
}

keymaster_error_t TrustonicTeeKeymasterImpl::generate_key(
    const keymaster_key_param_set_t*    params,
    keymaster_key_blob_t*               key_blob,
    keymaster_key_characteristics_t**   characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_GenerateKey(session_handle_,
        params, key_blob, characteristics);
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_key_characteristics(
    const keymaster_key_blob_t*     key_blob,
    const keymaster_blob_t*         client_id,
    const keymaster_blob_t*         app_data,
    keymaster_key_characteristics_t** characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_GetKeyCharacteristics(session_handle_,
        key_blob, client_id, app_data, characteristics);
}

keymaster_error_t TrustonicTeeKeymasterImpl::import_key(
    const keymaster_key_param_set_t* params,
    keymaster_key_format_t          key_format,
    const keymaster_blob_t*         key_data,
    keymaster_key_blob_t*           key_blob,
    keymaster_key_characteristics_t** characteristics)
{
    CHECK_SESSION(session_handle_);
    return TEE_ImportKey(session_handle_,
        params, key_format, key_data, key_blob, characteristics);
}

keymaster_error_t TrustonicTeeKeymasterImpl::export_key(
    keymaster_key_format_t          export_format,
    const keymaster_key_blob_t*     key_to_export,
    const keymaster_blob_t*         client_id,
    const keymaster_blob_t*         app_data,
    keymaster_blob_t*               export_data)
{
    CHECK_SESSION(session_handle_);
    return TEE_ExportKey(session_handle_,
        export_format, key_to_export, client_id, app_data, export_data);
}

keymaster_error_t TrustonicTeeKeymasterImpl::begin(
    keymaster_purpose_t             purpose,
    const keymaster_key_blob_t*     key,
    const keymaster_key_param_set_t* params,
    keymaster_key_param_set_t*      out_params,
    keymaster_operation_handle_t*   operation_handle)
{
    CHECK_SESSION(session_handle_);
    return TEE_Begin(session_handle_,
        purpose, key, params, out_params, operation_handle);
}

keymaster_error_t TrustonicTeeKeymasterImpl::update(
    keymaster_operation_handle_t    operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t*         input,
    size_t*                         input_consumed,
    keymaster_key_param_set_t*      out_params,
    keymaster_blob_t*               output)
{
    CHECK_SESSION(session_handle_);
    return TEE_Update(session_handle_,
        operation_handle, params, input, input_consumed, out_params, output);
}

keymaster_error_t TrustonicTeeKeymasterImpl::finish(
    keymaster_operation_handle_t    operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t*         signature,
    keymaster_key_param_set_t*      out_params,
    keymaster_blob_t*               output)
{
    CHECK_SESSION(session_handle_);
    return TEE_Finish(session_handle_,
        operation_handle, params, signature, out_params, output);
}

keymaster_error_t TrustonicTeeKeymasterImpl::abort(
    keymaster_operation_handle_t    operation_handle)
{
    CHECK_SESSION(session_handle_);
    return TEE_Abort(session_handle_, operation_handle);
}

#ifdef SOTER
keymaster_error_t TrustonicTeeKeymasterImpl::generate_attk_key_pair(
	const uint8_t copy_num)
{
    keymaster_error_t ret = KM_ERROR_OK;
    keymaster_key_param_t key_params[8];
    keymaster_key_param_set_t paramset = {key_params, 0};

	LOG_I("Generate ATTK KEY Pair...");

	key_params[0].tag =             KM_TAG_ALGORITHM;
	key_params[0].enumerated =      KM_ALGORITHM_RSA;
	key_params[1].tag =             KM_TAG_KEY_SIZE;
	key_params[1].integer =         2048;
	key_params[2].tag =             KM_TAG_RSA_PUBLIC_EXPONENT;
	key_params[2].long_integer =    65537;
	key_params[3].tag =             KM_TAG_PURPOSE;
	key_params[3].enumerated =      KM_PURPOSE_SIGN;
	key_params[4].tag =             KM_TAG_PURPOSE;
	key_params[4].enumerated =      KM_PURPOSE_VERIFY;
	key_params[5].tag =             KM_TAG_DIGEST;
	key_params[5].enumerated =      KM_DIGEST_SHA_2_256;
	key_params[6].tag =             KM_TAG_NO_AUTH_REQUIRED;
	key_params[6].boolean =         true;
	key_params[7].tag =             KM_TAG_PADDING;
    key_params[7].enumerated =      KM_PAD_RSA_PSS;

	paramset.length = 8;

	ret = TEE_GenerateATTKKeyPair(session_handle_,
		&paramset, copy_num);

	LOG_I("Generate ATTK KEY Pair Done...");

	return ret;
}

keymaster_error_t TrustonicTeeKeymasterImpl::verify_attk_key_pair(void)
{
    return TEE_VerifyATTKeyPair(session_handle_);
}

keymaster_error_t TrustonicTeeKeymasterImpl::export_attk_public_key(
	const uint8_t** pub_key_data,
	size_t* pub_key_data_length)
{
	return TEE_ExportATTKPublicKey(session_handle_,
		pub_key_data, pub_key_data_length);
}

keymaster_error_t TrustonicTeeKeymasterImpl::get_device_id(
	const uint8_t** device_id,
	size_t* device_id_length)
{
	return TEE_GetDeviceID(session_handle_,
		device_id, device_id_length);
}
#endif
