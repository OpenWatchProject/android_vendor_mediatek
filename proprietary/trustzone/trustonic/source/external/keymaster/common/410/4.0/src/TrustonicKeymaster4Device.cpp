/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
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

#define LOG_TAG "android.hardware.keymaster@4.0-impl"

#include "TrustonicKeymaster4Device.h"

#include <log/log.h>

#include "keymaster_ta_defs.h"

namespace android {
namespace hardware {
namespace keymaster {
namespace V4_0 {
namespace implementation {

TrustonicKeymaster4Device::~TrustonicKeymaster4Device() {}

// Conversion utilities

// -- Enum conversions

// ---- tag
inline Tag convert_tag(const keymaster_tag_t value) {
    switch (value) {
        case KM_TAG_PURPOSE: return Tag::PURPOSE;
        case KM_TAG_ALGORITHM: return Tag::ALGORITHM;
        case KM_TAG_KEY_SIZE: return Tag::KEY_SIZE;
        case KM_TAG_BLOCK_MODE: return Tag::BLOCK_MODE;
        case KM_TAG_DIGEST: return Tag::DIGEST;
        case KM_TAG_PADDING: return Tag::PADDING;
        case KM_TAG_CALLER_NONCE: return Tag::CALLER_NONCE;
        case KM_TAG_MIN_MAC_LENGTH: return Tag::MIN_MAC_LENGTH;
        case KM_TAG_EC_CURVE: return Tag::EC_CURVE;
        case KM_TAG_RSA_PUBLIC_EXPONENT: return Tag::RSA_PUBLIC_EXPONENT;
        case KM_TAG_INCLUDE_UNIQUE_ID: return Tag::INCLUDE_UNIQUE_ID;
        case KM_TAG_BLOB_USAGE_REQUIREMENTS: return Tag::BLOB_USAGE_REQUIREMENTS;
        case KM_TAG_BOOTLOADER_ONLY: return Tag::BOOTLOADER_ONLY;
        case KM_TAG_ROLLBACK_RESISTANCE: return Tag::ROLLBACK_RESISTANCE;
        case KM_TAG_HARDWARE_TYPE: return Tag::HARDWARE_TYPE;
        case KM_TAG_ACTIVE_DATETIME: return Tag::ACTIVE_DATETIME;
        case KM_TAG_ORIGINATION_EXPIRE_DATETIME: return Tag::ORIGINATION_EXPIRE_DATETIME;
        case KM_TAG_USAGE_EXPIRE_DATETIME: return Tag::USAGE_EXPIRE_DATETIME;
        case KM_TAG_MIN_SECONDS_BETWEEN_OPS: return Tag::MIN_SECONDS_BETWEEN_OPS;
        case KM_TAG_MAX_USES_PER_BOOT: return Tag::MAX_USES_PER_BOOT;
        case KM_TAG_USER_ID: return Tag::USER_ID;
        case KM_TAG_USER_SECURE_ID: return Tag::USER_SECURE_ID;
        case KM_TAG_NO_AUTH_REQUIRED: return Tag::NO_AUTH_REQUIRED;
        case KM_TAG_USER_AUTH_TYPE: return Tag::USER_AUTH_TYPE;
        case KM_TAG_AUTH_TIMEOUT: return Tag::AUTH_TIMEOUT;
        case KM_TAG_ALLOW_WHILE_ON_BODY: return Tag::ALLOW_WHILE_ON_BODY;
        case KM_TAG_TRUSTED_USER_PRESENCE_REQUIRED: return Tag::TRUSTED_USER_PRESENCE_REQUIRED;
        case KM_TAG_TRUSTED_CONFIRMATION_REQUIRED: return Tag::TRUSTED_CONFIRMATION_REQUIRED;
        case KM_TAG_UNLOCKED_DEVICE_REQUIRED: return Tag::UNLOCKED_DEVICE_REQUIRED;
        case KM_TAG_APPLICATION_ID: return Tag::APPLICATION_ID;
        case KM_TAG_APPLICATION_DATA: return Tag::APPLICATION_DATA;
        case KM_TAG_CREATION_DATETIME: return Tag::CREATION_DATETIME;
        case KM_TAG_ORIGIN: return Tag::ORIGIN;
        case KM_TAG_ROOT_OF_TRUST: return Tag::ROOT_OF_TRUST;
        case KM_TAG_OS_VERSION: return Tag::OS_VERSION;
        case KM_TAG_OS_PATCHLEVEL: return Tag::OS_PATCHLEVEL;
        case KM_TAG_UNIQUE_ID: return Tag::UNIQUE_ID;
        case KM_TAG_ATTESTATION_CHALLENGE: return Tag::ATTESTATION_CHALLENGE;
        case KM_TAG_ATTESTATION_APPLICATION_ID: return Tag::ATTESTATION_APPLICATION_ID;
        case KM_TAG_ATTESTATION_ID_BRAND: return Tag::ATTESTATION_ID_BRAND;
        case KM_TAG_ATTESTATION_ID_DEVICE: return Tag::ATTESTATION_ID_DEVICE;
        case KM_TAG_ATTESTATION_ID_PRODUCT: return Tag::ATTESTATION_ID_PRODUCT;
        case KM_TAG_ATTESTATION_ID_SERIAL: return Tag::ATTESTATION_ID_SERIAL;
        case KM_TAG_ATTESTATION_ID_IMEI: return Tag::ATTESTATION_ID_IMEI;
        case KM_TAG_ATTESTATION_ID_MEID: return Tag::ATTESTATION_ID_MEID;
        case KM_TAG_ATTESTATION_ID_MANUFACTURER: return Tag::ATTESTATION_ID_MANUFACTURER;
        case KM_TAG_ATTESTATION_ID_MODEL: return Tag::ATTESTATION_ID_MODEL;
        case KM_TAG_VENDOR_PATCHLEVEL: return Tag::VENDOR_PATCHLEVEL;
        case KM_TAG_BOOT_PATCHLEVEL: return Tag::BOOT_PATCHLEVEL;
        case KM_TAG_ASSOCIATED_DATA: return Tag::ASSOCIATED_DATA;
        case KM_TAG_NONCE: return Tag::NONCE;
        case KM_TAG_MAC_LENGTH: return Tag::MAC_LENGTH;
        case KM_TAG_RESET_SINCE_ID_ROTATION: return Tag::RESET_SINCE_ID_ROTATION;
        case KM_TAG_CONFIRMATION_TOKEN: return Tag::CONFIRMATION_TOKEN;
        default: return Tag::INVALID;
    }
}

// -- Blob conversions

// ---- keymaster_key_blob
inline hidl_vec<uint8_t> convert_key_blob(const keymaster_key_blob_t& blob) {
    hidl_vec<uint8_t> result;
    result.setToExternal(const_cast<uint8_t*>(blob.key_material), blob.key_material_size);
    return result;
}
inline keymaster_key_blob_t convert_key_blob(const hidl_vec<uint8_t>& blob) {
    keymaster_key_blob_t result = {blob.data(), blob.size()};
    return result;
}

// ---- keymaster_blob
inline hidl_vec<uint8_t> convert_blob(const keymaster_blob_t& blob) {
    hidl_vec<uint8_t> result;
    result.setToExternal(const_cast<uint8_t*>(blob.data), blob.data_length);
    return result;
}
inline keymaster_blob_t convert_blob(const hidl_vec<uint8_t>& blob) {
    keymaster_blob_t result = {blob.data(), blob.size()};
    return result;
}

// -- Certificate chain conversion

inline static hidl_vec<hidl_vec<uint8_t>>
convert_cert_chain(const keymaster_cert_chain_t& cert_chain) {
    hidl_vec<hidl_vec<uint8_t>> result;
    if (!cert_chain.entry_count || !cert_chain.entries)
        return result;

    result.resize(cert_chain.entry_count);
    for (size_t i = 0; i < cert_chain.entry_count; ++i) {
        result[i] = convert_blob(cert_chain.entries[i]);
    }

    return result;
}

// -- Parameter set conversions

static inline hidl_vec<KeyParameter> convert_key_param_set(const keymaster_key_param_set_t& set) {
    hidl_vec<KeyParameter> result;
    const size_t n_params = set.length;
    result.resize(n_params);
    const keymaster_key_param_t* params = set.params;
    size_t j = 0;
    for (size_t i = 0; i < n_params; ++i) {
        const keymaster_key_param_t& p = params[i];
        const keymaster_tag_t tag = p.tag;
        const Tag hidl_tag = convert_tag(tag);
        if (hidl_tag != Tag::INVALID) {
            KeyParameter *r = &result[j];
            r->tag = hidl_tag;
            switch (keymaster_tag_get_type(tag)) {
            case KM_ENUM:
            case KM_ENUM_REP:
            {
                const uint32_t v = p.enumerated;
                switch (tag) {
                case KM_TAG_ALGORITHM:
                    r->f.algorithm = Algorithm(v);
                    break;
                case KM_TAG_BLOCK_MODE:
                    r->f.blockMode = BlockMode(v);
                    break;
                case KM_TAG_PADDING:
                    r->f.paddingMode = PaddingMode(v);
                    break;
                case KM_TAG_DIGEST:
                    r->f.digest = Digest(v);
                    break;
                case KM_TAG_EC_CURVE:
                    r->f.ecCurve = EcCurve(v);
                    break;
                case KM_TAG_ORIGIN:
                    r->f.origin = KeyOrigin(v);
                    break;
                case KM_TAG_BLOB_USAGE_REQUIREMENTS:
                    r->f.keyBlobUsageRequirements = KeyBlobUsageRequirements(v);
                    break;
                case KM_TAG_PURPOSE:
                    r->f.purpose = KeyPurpose(v);
                    break;
                case KM_TAG_KDF:
                    r->f.keyDerivationFunction = KeyDerivationFunction(v);
                    break;
                case KM_TAG_USER_AUTH_TYPE:
                    r->f.hardwareAuthenticatorType = HardwareAuthenticatorType(v);
                    break;
                case KM_TAG_HARDWARE_TYPE:
                    r->f.hardwareType = SecurityLevel(v);
                    break;
                default:
                    /* Shouldn't get here. */
                    break;
                }
                break;
            }
            case KM_UINT:
            case KM_UINT_REP:
                r->f.integer = p.integer;
                break;
            case KM_ULONG:
            case KM_ULONG_REP:
                r->f.longInteger = p.long_integer;
                break;
            case KM_DATE:
                r->f.dateTime = p.date_time;
                break;
            case KM_BOOL:
                r->f.boolValue = true;
                break;
            case KM_BIGNUM:
            case KM_BYTES:
                r->blob.setToExternal(const_cast<uint8_t*>(p.blob.data), p.blob.data_length);
                break;
            default:
                /* Shouldn't get here. */
                break;
            }
            j++;
        }
    }
    result.resize(j);
    return result;
}
class KmKeyParamSet : public keymaster_key_param_set_t {
  public:
    KmKeyParamSet(const hidl_vec<KeyParameter>& keyParams) {
        size_t n_params = keyParams.size();
        params = new keymaster_key_param_t[n_params];
        size_t j = 0;
        for (size_t i = 0; i < n_params; ++i) {
            const KeyParameter& p = keyParams[i];
            const keymaster_tag_t tag = keymaster_tag_t(p.tag);
            if (tag != KM_TAG_INVALID) {
                keymaster_key_param_t *q = &params[j];
                q->tag = tag;
                switch (keymaster_tag_get_type(tag)) {
                case KM_ENUM:
                case KM_ENUM_REP:
                    switch (tag) {
                        case KM_TAG_ALGORITHM:
                            q->enumerated = keymaster_algorithm_t(p.f.algorithm);
                            break;
                        case KM_TAG_BLOCK_MODE:
                            q->enumerated = keymaster_block_mode_t(p.f.blockMode);
                            break;
                        case KM_TAG_PADDING:
                            q->enumerated = keymaster_padding_t(p.f.paddingMode);
                            break;
                        case KM_TAG_DIGEST:
                            q->enumerated = keymaster_digest_t(p.f.digest);
                            break;
                        case KM_TAG_EC_CURVE:
                            q->enumerated = keymaster_ec_curve_t(p.f.ecCurve);
                            break;
                        case KM_TAG_ORIGIN:
                            q->enumerated = keymaster_key_origin_t(p.f.origin);
                            break;
                        case KM_TAG_BLOB_USAGE_REQUIREMENTS:
                            q->enumerated = keymaster_key_blob_usage_requirements_t(p.f.keyBlobUsageRequirements);
                            break;
                        case KM_TAG_PURPOSE:
                            q->enumerated = keymaster_purpose_t(p.f.purpose);
                            break;
                        case KM_TAG_KDF:
                            q->enumerated = keymaster_kdf_t(p.f.keyDerivationFunction);
                            break;
                        case KM_TAG_USER_AUTH_TYPE:
                            q->enumerated = uint32_t(p.f.hardwareAuthenticatorType);
                            break;
                        case KM_TAG_HARDWARE_TYPE:
                            q->enumerated = keymaster_security_level_t(p.f.hardwareType);
                            break;
                        default:
                            break;
                    }
                    break;
                case KM_UINT:
                case KM_UINT_REP:
                    q->integer = p.f.integer;
                    break;
                case KM_ULONG:
                case KM_ULONG_REP:
                    q->long_integer = p.f.longInteger;
                    break;
                case KM_DATE:
                    q->date_time = p.f.dateTime;
                    break;
                case KM_BOOL:
                    q->boolean = true;
                    break;
                case KM_BIGNUM:
                case KM_BYTES:
                    q->blob.data = p.blob.size() ? &p.blob[0] : NULL;
                    q->blob.data_length = p.blob.size();
                    break;
                default:
                    break;
                }
                j++;
            }
        }
        length = j;
    }
    KmKeyParamSet(KmKeyParamSet&& other) : keymaster_key_param_set_t{other.params, other.length} {
        other.length = 0;
        other.params = nullptr;
    }
    KmKeyParamSet(const KmKeyParamSet&) = delete;
    ~KmKeyParamSet() { delete[] params; }
};

// -- Other conversions

inline HmacSharingParameters convert_hmac_sharing_parameters(
    const keymaster_hmac_sharing_parameters_t& params)
{
    HmacSharingParameters result;
    result.seed = convert_blob(params.seed);
    result.nonce = params.nonce;
    return result;
}

inline VerificationToken convert_verification_token(
    const keymaster_verification_token_t& token)
{
    VerificationToken result;
    result.challenge = token.challenge;
    result.timestamp = token.timestamp;
    result.parametersVerified = convert_key_param_set(token.parameters_verified);
    result.securityLevel = SecurityLevel(token.security_level);
    result.mac = convert_blob(token.mac);
    return result;
}

inline KeyCharacteristics convert_key_characteristics(
    const keymaster_key_characteristics_t& characteristics)
{
    KeyCharacteristics result;
    result.softwareEnforced = convert_key_param_set(characteristics.sw_enforced);
    result.hardwareEnforced = convert_key_param_set(characteristics.hw_enforced);
    return result;
}

class KmHwAuthToken : public keymaster_hw_auth_token_t {
public:
    KmHwAuthToken(const HardwareAuthToken& authToken) {
        challenge = authToken.challenge;
        user_id = authToken.userId;
        authenticator_id = authToken.authenticatorId;
        authenticator_type = keymaster_hw_auth_type_t(ntohl(static_cast<uint32_t>(authToken.authenticatorType)));
        timestamp = ntohq(authToken.timestamp);
        mac = convert_blob(authToken.mac);
    }
};

class KmVerificationToken : public keymaster_verification_token_t {
public:
    KmVerificationToken(const VerificationToken& verificationToken) {
        challenge = verificationToken.challenge;
        timestamp = verificationToken.timestamp;
        KmKeyParamSet ps(verificationToken.parametersVerified);
        parameters_verified = ps;
        security_level = keymaster_security_level_t(verificationToken.securityLevel);
        mac = convert_blob(verificationToken.mac);
    }
};

class KmHmacSharingParametersSet : public keymaster_hmac_sharing_parameters_set_t {
public:
    KmHmacSharingParametersSet(const hidl_vec<HmacSharingParameters>& hmacSharingParams) {
        params = new keymaster_hmac_sharing_parameters_t[hmacSharingParams.size()];
        length = hmacSharingParams.size();
        for (size_t i = 0; i < hmacSharingParams.size(); ++i) {
            params[i].seed = convert_blob(hmacSharingParams[i].seed);
            memcpy(params[i].nonce, hmacSharingParams[i].nonce.data(), 32); // FIXME make more robust
        }
    }
};

// Methods from ::android::hardware::keymaster::V4_0::IKeymasterDevice

Return<void> TrustonicKeymaster4Device::getHardwareInfo(
    IKeymasterDevice::getHardwareInfo_cb _hidl_cb)
{
    keymaster_security_level_t _security_level = KM_SECURITY_LEVEL_SOFTWARE;
    const char *_keymaster_name = 0;
    const char *_keymaster_author_name = 0;
    impl_->get_hardware_info(
        &_security_level, &_keymaster_name, &_keymaster_author_name);
    const SecurityLevel securityLevel = SecurityLevel(_security_level);
    const hidl_string keymasterName(_keymaster_name);
    const hidl_string keymasterAuthorName(_keymaster_author_name);
    _hidl_cb(securityLevel, keymasterName, keymasterAuthorName);
    return Void();
}

Return<void> TrustonicKeymaster4Device::getHmacSharingParameters(
    IKeymasterDevice::getHmacSharingParameters_cb _hidl_cb)
{
    keymaster_hmac_sharing_parameters_t _params = {};
    keymaster_error_t ret = impl_->get_hmac_sharing_parameters(
        &_params);
    HmacSharingParameters params = convert_hmac_sharing_parameters(_params);
    _hidl_cb(ErrorCode(ret), params);
    return Void();
}

Return<void> TrustonicKeymaster4Device::computeSharedHmac(
    const hidl_vec<HmacSharingParameters>& params,
    IKeymasterDevice::computeSharedHmac_cb _hidl_cb)
{
    const KmHmacSharingParametersSet _params(params);
    keymaster_blob_t _sharing_check = {};
    keymaster_error_t ret = impl_->compute_shared_hmac(
        &_params, &_sharing_check);
    hidl_vec<uint8_t> sharingCheck = convert_blob(_sharing_check);
    _hidl_cb(ErrorCode(ret), sharingCheck);
    return Void();
}

Return<void> TrustonicKeymaster4Device::verifyAuthorization(
    uint64_t operationHandle,
    const hidl_vec<KeyParameter>& parametersToVerify,
    const HardwareAuthToken& authToken,
    IKeymasterDevice::verifyAuthorization_cb _hidl_cb)
{
    const KmKeyParamSet _key_params(parametersToVerify);
    const KmHwAuthToken _auth_token(authToken);
    keymaster_verification_token_t _token = {};
    keymaster_error_t ret = impl_->verify_authorization(
        operationHandle, &_key_params, &_auth_token, &_token);
    VerificationToken token = convert_verification_token(_token);
    _hidl_cb(ErrorCode(ret), token);
    return Void();
}

Return<ErrorCode> TrustonicKeymaster4Device::addRngEntropy(
    const hidl_vec<uint8_t>& data)
{
    return ErrorCode(impl_->add_rng_entropy(
        data.data(), data.size()));
}

Return<void> TrustonicKeymaster4Device::generateKey(
    const hidl_vec<KeyParameter>& keyParams,
    IKeymasterDevice::generateKey_cb _hidl_cb)
{
    const KmKeyParamSet _key_params(keyParams);
    keymaster_key_blob_t _key_blob = {};
    keymaster_key_characteristics_t _characteristics = {};
    keymaster_error_t ret = impl_->generate_key(
        &_key_params, &_key_blob, &_characteristics);
    hidl_vec<uint8_t> keyBlob = convert_key_blob(_key_blob);
    KeyCharacteristics keyCharacteristics = convert_key_characteristics(_characteristics);
    _hidl_cb(ErrorCode(ret), keyBlob, keyCharacteristics);
    return Void();
}

Return<void> TrustonicKeymaster4Device::importKey(
    const hidl_vec<KeyParameter>& keyParams,
    KeyFormat keyFormat,
    const hidl_vec<uint8_t>& keyData,
    IKeymasterDevice::importKey_cb _hidl_cb)
{
    const KmKeyParamSet _key_params(keyParams);
    const keymaster_blob_t _key_data = convert_blob(keyData);
    keymaster_key_blob_t _key_blob = {};
    keymaster_key_characteristics_t _characteristics = {};
    keymaster_error_t ret = impl_->import_key(
        &_key_params, keymaster_key_format_t(keyFormat), &_key_data, &_key_blob, &_characteristics);
    hidl_vec<uint8_t> keyBlob = convert_key_blob(_key_blob);
    KeyCharacteristics keyCharacteristics = convert_key_characteristics(_characteristics);
    _hidl_cb(ErrorCode(ret), keyBlob, keyCharacteristics);
    return Void();
}

Return<void> TrustonicKeymaster4Device::importWrappedKey(
    const hidl_vec<uint8_t>& wrappedKeyData,
    const hidl_vec<uint8_t>& wrappingKeyBlob,
    const hidl_vec<uint8_t>& maskingKey,
    const hidl_vec<KeyParameter>& unwrappingParams,
    uint64_t passwordSid,
    uint64_t biometricSid,
    IKeymasterDevice::importWrappedKey_cb _hidl_cb)
{
    const keymaster_blob_t _wrapped_key_data = convert_blob(wrappedKeyData);
    const keymaster_key_blob_t _wrapping_key_blob = convert_key_blob(wrappingKeyBlob);
    const KmKeyParamSet _unwrapping_params(unwrappingParams);
    keymaster_key_blob_t _key_blob = {};
    keymaster_key_characteristics_t _key_characteristics = {};
    keymaster_error_t ret;
    if (maskingKey.size() != 32) {
        ret = KM_ERROR_INVALID_ARGUMENT;
    } else {
        ret = impl_->import_wrapped_key(
            &_wrapped_key_data, &_wrapping_key_blob, maskingKey.data(), &_unwrapping_params, passwordSid, biometricSid, &_key_blob, &_key_characteristics);
    }
    hidl_vec<uint8_t> keyBlob = convert_key_blob(_key_blob);
    KeyCharacteristics keyCharacteristics = convert_key_characteristics(_key_characteristics);
    _hidl_cb(ErrorCode(ret), keyBlob, keyCharacteristics);
    return Void();
}

Return<void> TrustonicKeymaster4Device::getKeyCharacteristics(
    const hidl_vec<uint8_t>& keyBlob,
    const hidl_vec<uint8_t>& clientId,
    const hidl_vec<uint8_t>& appData,
    IKeymasterDevice::getKeyCharacteristics_cb _hidl_cb)
{
    const keymaster_key_blob_t _key_blob = convert_key_blob(keyBlob);
    const keymaster_blob_t _client_id = convert_blob(clientId);
    const keymaster_blob_t _app_data = convert_blob(appData);
    keymaster_key_characteristics_t _characteristics = {};
    keymaster_error_t ret = impl_->get_key_characteristics(
        &_key_blob, &_client_id, &_app_data, &_characteristics);
    KeyCharacteristics keyCharacteristics = convert_key_characteristics(_characteristics);
    _hidl_cb(ErrorCode(ret), keyCharacteristics);
    return Void();
}

Return<void> TrustonicKeymaster4Device::exportKey(
    KeyFormat keyFormat,
    const hidl_vec<uint8_t>& keyBlob,
    const hidl_vec<uint8_t>& clientId,
    const hidl_vec<uint8_t>& appData,
    IKeymasterDevice::exportKey_cb _hidl_cb)
{
    const keymaster_key_blob_t _key_to_export = convert_key_blob(keyBlob);
    const keymaster_blob_t _client_id = convert_blob(clientId);
    const keymaster_blob_t _app_data = convert_blob(appData);
    keymaster_blob_t _export_data = {};
    keymaster_error_t ret = impl_->export_key(
        keymaster_key_format_t(keyFormat), &_key_to_export, &_client_id, &_app_data, &_export_data);
    hidl_vec<uint8_t> keyMaterial = convert_blob(_export_data);
    _hidl_cb(ErrorCode(ret), keyMaterial);
    return Void();
}

Return<void> TrustonicKeymaster4Device::attestKey(
    const hidl_vec<uint8_t>& keyToAttest,
    const hidl_vec<KeyParameter>& attestParams,
    IKeymasterDevice::attestKey_cb _hidl_cb)
{
    const keymaster_key_blob_t _key_to_attest = convert_key_blob(keyToAttest);
    const KmKeyParamSet _attest_params(attestParams);
    keymaster_cert_chain_t _cert_chain = {};
    keymaster_error_t ret = impl_->attest_key(
        &_key_to_attest, &_attest_params, &_cert_chain);
    hidl_vec<hidl_vec<uint8_t>> certChain = convert_cert_chain(_cert_chain);
    _hidl_cb(ErrorCode(ret), certChain);
    return Void();
}

Return<void> TrustonicKeymaster4Device::upgradeKey(
    const hidl_vec<uint8_t>& keyBlobToUpgrade,
    const hidl_vec<KeyParameter>& upgradeParams,
    IKeymasterDevice::upgradeKey_cb _hidl_cb)
{
    const keymaster_key_blob_t _key_to_upgrade = convert_key_blob(keyBlobToUpgrade);
    const KmKeyParamSet _upgrade_params(upgradeParams);
    keymaster_key_blob_t _upgraded_key = {};
    keymaster_error_t ret = impl_->upgrade_key(
        &_key_to_upgrade, &_upgrade_params, &_upgraded_key);
    hidl_vec<uint8_t> upgradedKeyBlob = convert_key_blob(_upgraded_key);
    _hidl_cb(ErrorCode(ret), upgradedKeyBlob);
    return Void();
}

Return<ErrorCode> TrustonicKeymaster4Device::deleteKey(
    const hidl_vec<uint8_t>& keyBlob)
{
    (void)keyBlob;
    return ErrorCode::UNIMPLEMENTED;
}

Return<ErrorCode> TrustonicKeymaster4Device::deleteAllKeys()
{
    return ErrorCode::UNIMPLEMENTED;
}

Return<ErrorCode> TrustonicKeymaster4Device::destroyAttestationIds()
{
    return ErrorCode::UNIMPLEMENTED;
}

Return<void> TrustonicKeymaster4Device::begin(
    KeyPurpose purpose,
    const hidl_vec<uint8_t>& keyBlob,
    const hidl_vec<KeyParameter>& inParams,
    const HardwareAuthToken& authToken,
    IKeymasterDevice::begin_cb _hidl_cb)
{
    const keymaster_key_blob_t _key = convert_key_blob(keyBlob);
    const KmKeyParamSet _in_params(inParams);
    const KmHwAuthToken _auth_token(authToken);
    keymaster_key_param_set_t _out_params = {};
    uint64_t operation_handle = 0;
    keymaster_error_t ret = impl_->begin(
        keymaster_purpose_t(purpose), &_key, &_in_params, &_auth_token, &_out_params, &operation_handle);
    hidl_vec<KeyParameter> outParams = convert_key_param_set(_out_params);
    _hidl_cb(ErrorCode(ret), outParams, operation_handle);
    return Void();
}

Return<void> TrustonicKeymaster4Device::update(
    uint64_t operationHandle,
    const hidl_vec<KeyParameter>& inParams,
    const hidl_vec<uint8_t>& input,
    const HardwareAuthToken& authToken,
    const VerificationToken& verificationToken,
    IKeymasterDevice::update_cb _hidl_cb)
{
    const KmKeyParamSet _in_params(inParams);
    const keymaster_blob_t _input = convert_blob(input);
    const KmHwAuthToken _auth_token(authToken);
    const KmVerificationToken _verification_token(verificationToken);
    size_t input_consumed = 0;
    keymaster_key_param_set_t _out_params = {};
    keymaster_blob_t _output = {};
    keymaster_error_t ret = impl_->update(
        operationHandle, &_in_params, &_input, &_auth_token, &_verification_token, &input_consumed, &_out_params, &_output);
    hidl_vec<KeyParameter> outParams = convert_key_param_set(_out_params);
    hidl_vec<uint8_t> output = convert_blob(_output);
    _hidl_cb(ErrorCode(ret), input_consumed, outParams, output);
    return Void();
}

Return<void> TrustonicKeymaster4Device::finish(
    uint64_t operationHandle,
    const hidl_vec<KeyParameter>& inParams,
    const hidl_vec<uint8_t>& input,
    const hidl_vec<uint8_t>& signature,
    const HardwareAuthToken& authToken,
    const VerificationToken& verificationToken,
    IKeymasterDevice::finish_cb _hidl_cb)
{
    const KmKeyParamSet _in_params(inParams);
    const keymaster_blob_t _input = convert_blob(input);
    const keymaster_blob_t _signature = convert_blob(signature);
    const KmHwAuthToken _auth_token(authToken);
    const KmVerificationToken _verification_token(verificationToken);
    keymaster_key_param_set_t _out_params = {};
    keymaster_blob_t _output = {};
    keymaster_error_t ret = impl_->finish(
        operationHandle, &_in_params, &_input, &_signature, &_auth_token, &_verification_token, &_out_params, &_output);
    hidl_vec<KeyParameter> outParams = convert_key_param_set(_out_params);
    hidl_vec<uint8_t> output = convert_blob(_output);
    _hidl_cb(ErrorCode(ret), outParams, output);
    return Void();
}

Return<ErrorCode> TrustonicKeymaster4Device::abort(
    uint64_t operationHandle)
{
    return ErrorCode(impl_->abort(
        operationHandle));
}

IKeymasterDevice* HIDL_FETCH_IKeymasterDevice(const char* name) {
    TrustonicKeymaster4DeviceImpl *impl = nullptr;
    ALOGI("Fetching keymaster device name %s", name);
    if (name && strcmp(name, "softwareonly") == 0) {
        // FIXME
        // return ::keymaster::V4_0::ng::CreateKeymasterDevice();
    } else if (name && strcmp(name, "default") == 0) {
        return new TrustonicKeymaster4Device(impl);
    }
    return nullptr;
}

}  // namespace implementation
}  // namespace V4_0
}  // namespace keymaster
}  // namespace hardware
}  // namespace android
