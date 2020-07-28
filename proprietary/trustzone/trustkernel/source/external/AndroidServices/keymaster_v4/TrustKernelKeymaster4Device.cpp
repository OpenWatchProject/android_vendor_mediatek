/*
 **
 ** Copyright 2017, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#define LOG_TAG "trustkernel.hardware.keymaster@4.0"
#include <log/log.h>
#include <stdlib.h>
#include <cutils/properties.h>

#include "include/TrustKernelKeymaster4Device.h"

#include <keymaster/android_keymaster.h>
#include <keymaster/android_keymaster_messages.h>
#include <keymaster/keymaster_configuration.h>
#include <keymaster/keymaster_enforcement.h>
#include <keymaster/authorization_set.h>
#include <keymasterV4_0/keymaster_utils.h>

#include <authorization_set.h>

namespace android {
namespace hardware {
inline static bool operator<(const hidl_vec<uint8_t>& a, const hidl_vec<uint8_t>& b) {
    return memcmp(a.data(), b.data(), std::min(a.size(), b.size())) == -1;
}
}
}

namespace trustkernel {
namespace {

inline keymaster_tag_t legacy_enum_conversion(const Tag value) {
    return keymaster_tag_t(value);
}

inline Tag legacy_enum_conversion(const keymaster_tag_t value) {
    return Tag(value);
}

inline keymaster_purpose_t legacy_enum_conversion(const KeyPurpose value) {
    return static_cast<keymaster_purpose_t>(value);
}

inline keymaster_key_format_t legacy_enum_conversion(const KeyFormat value) {
    return static_cast<keymaster_key_format_t>(value);
}

inline SecurityLevel legacy_enum_conversion(const keymaster_security_level_t value) {
    return static_cast<SecurityLevel>(value);
}

inline hw_authenticator_type_t legacy_enum_conversion(const HardwareAuthenticatorType value) {
    return static_cast<hw_authenticator_type_t>(value);
}

inline ErrorCode legacy_enum_conversion(const keymaster_error_t value) {
    return static_cast<ErrorCode>(value);
}

inline keymaster_tag_type_t typeFromTag(const keymaster_tag_t tag) {
    return keymaster_tag_get_type(tag);
}

class KmParamSet : public keymaster_key_param_set_t {
  public:
    KmParamSet(const hidl_vec<KeyParameter>& keyParams) {
        params = new keymaster_key_param_t[keyParams.size()];
        length = keyParams.size();
        for (size_t i = 0; i < keyParams.size(); ++i) {
            auto tag = legacy_enum_conversion(keyParams[i].tag);
            switch (typeFromTag(tag)) {
            case KM_ENUM:
            case KM_ENUM_REP:
                params[i] = keymaster_param_enum(tag, keyParams[i].f.integer);
                break;
            case KM_UINT:
            case KM_UINT_REP:
                params[i] = keymaster_param_int(tag, keyParams[i].f.integer);
                break;
            case KM_ULONG:
            case KM_ULONG_REP:
                params[i] = keymaster_param_long(tag, keyParams[i].f.longInteger);
                break;
            case KM_DATE:
                params[i] = keymaster_param_date(tag, keyParams[i].f.dateTime);
                break;
            case KM_BOOL:
                if (keyParams[i].f.boolValue)
                    params[i] = keymaster_param_bool(tag);
                else
                    params[i].tag = KM_TAG_INVALID;
                break;
            case KM_BIGNUM:
            case KM_BYTES:
                params[i] =
                    keymaster_param_blob(tag, &keyParams[i].blob[0], keyParams[i].blob.size());
                break;
            case KM_INVALID:
            default:
                params[i].tag = KM_TAG_INVALID;
                /* just skip */
                break;
            }
        }
    }
    KmParamSet(KmParamSet&& other) : keymaster_key_param_set_t{other.params, other.length} {
        other.length = 0;
        other.params = nullptr;
    }
    KmParamSet(const KmParamSet&) = delete;
    ~KmParamSet() { delete[] params; }
};

inline hidl_vec<uint8_t> kmBlob2hidlVec(const keymaster_key_blob_t& blob) {
    hidl_vec<uint8_t> result;
    result.setToExternal(const_cast<unsigned char*>(blob.key_material), blob.key_material_size);
    return result;
}

inline hidl_vec<uint8_t> kmBlob2hidlVec(const keymaster_blob_t& blob) {
    hidl_vec<uint8_t> result;
    result.setToExternal(const_cast<unsigned char*>(blob.data), blob.data_length);
    return result;
}
/*
inline hidl_vec<uint8_t> kmBuffer2hidlVec(const ::keymaster::Buffer& buf) {
    hidl_vec<uint8_t> result;
    result.setToExternal(const_cast<unsigned char*>(buf.peek_read()), buf.available_read());
    return result;
}
*/

inline static hidl_vec<hidl_vec<uint8_t>>
kmCertChain2Hidl(const keymaster_cert_chain_t& cert_chain) {
    hidl_vec<hidl_vec<uint8_t>> result;
    if (!cert_chain.entry_count || !cert_chain.entries)
        return result;

    result.resize(cert_chain.entry_count);
    for (size_t i = 0; i < cert_chain.entry_count; ++i) {
        result[i] = kmBlob2hidlVec(cert_chain.entries[i]);
    }

    return result;
}

static inline hidl_vec<KeyParameter> kmParamSet2Hidl(const keymaster_key_param_set_t& set) {
    hidl_vec<KeyParameter> result;
    if (set.length == 0 || set.params == nullptr)
        return result;

    result.resize(set.length);
    keymaster_key_param_t* params = set.params;
    for (size_t i = 0; i < set.length; ++i) {
        auto tag = params[i].tag;
        result[i].tag = legacy_enum_conversion(tag);
        switch (typeFromTag(tag)) {
        case KM_ENUM:
        case KM_ENUM_REP:
            result[i].f.integer = params[i].enumerated;
            break;
        case KM_UINT:
        case KM_UINT_REP:
            result[i].f.integer = params[i].integer;
            break;
        case KM_ULONG:
        case KM_ULONG_REP:
            result[i].f.longInteger = params[i].long_integer;
            break;
        case KM_DATE:
            result[i].f.dateTime = params[i].date_time;
            break;
        case KM_BOOL:
            result[i].f.boolValue = params[i].boolean;
            break;
        case KM_BIGNUM:
        case KM_BYTES:
            result[i].blob.setToExternal(const_cast<unsigned char*>(params[i].blob.data),
                                         params[i].blob.data_length);
            break;
        case KM_INVALID:
        default:
            params[i].tag = KM_TAG_INVALID;
            /* just skip */
            break;
        }
    }
    return result;
}

}  // anonymous namespace

using ::keymaster::TypedTag;
using ::keymaster::TypedEnumTag;

typedef enum {
    KM_TAG_BOOT_VBMETA_DIGEST = KM_BYTES | 20000,
    KM_TAG_BOOT_VBMETA_DEVICE_STATE = KM_UINT | 20001,
    KM_TAG_BOOT_VERIFIED_BOOT_STATE = KM_ENUM | 20002,
} keymaster_trustkernel_extension_tag_t;

#define DECLARE_KEYMASTER_EXTENSION_TAG(type, name) \
    extern TypedTag<type, (keymaster_tag_t) KM_##name> name

#define DECLARE_KEYMASTER_ENUM_EXTENSION_TAG(type, name, enumtype) \
    extern TypedEnumTag<type, (keymaster_tag_t) KM_##name, enumtype> name

DECLARE_KEYMASTER_EXTENSION_TAG(KM_BYTES, TAG_BOOT_VBMETA_DIGEST);
DECLARE_KEYMASTER_EXTENSION_TAG(KM_UINT, TAG_BOOT_VBMETA_DEVICE_STATE);
DECLARE_KEYMASTER_ENUM_EXTENSION_TAG(KM_ENUM, TAG_BOOT_VERIFIED_BOOT_STATE, keymaster_verified_boot_t);

TrustKernelKeymaster4Device::TrustKernelKeymaster4Device()
    : impl_(new ::trustkernel::TrustKernelKeymasterImplementation()),
    securityLevel_(SecurityLevel::TRUSTED_ENVIRONMENT)
{
    char str_vbmeta_digest[PROPERTY_VALUE_MAX];
    property_get("ro.boot.vbmeta.digest", str_vbmeta_digest, "");

    char str_vbmeta_device_state[PROPERTY_VALUE_MAX];
    uint32_t vbmeta_device_state;
    property_get("ro.boot.vbmeta.device_state", str_vbmeta_device_state, "");
    if (!strcmp(str_vbmeta_device_state, "unlocked")) {
        vbmeta_device_state = 0;
    } else {
        vbmeta_device_state = 1;
    }

    char str_verified_boot_state[PROPERTY_VALUE_MAX];
    keymaster_verified_boot_t verified_boot_state;
    property_get("ro.boot.verifiedbootstate", str_verified_boot_state, "");
    if (!strcmp(str_verified_boot_state, "green")) {
        verified_boot_state = KM_VERIFIED_BOOT_VERIFIED;
    } else if (!strcmp(str_verified_boot_state, "yellow")) {
        verified_boot_state = KM_VERIFIED_BOOT_SELF_SIGNED;
    } else if (!strcmp(str_verified_boot_state, "orange")) {
        verified_boot_state = KM_VERIFIED_BOOT_UNVERIFIED;
    } else {
        verified_boot_state = KM_VERIFIED_BOOT_FAILED;
    }

    ::keymaster::AuthorizationSet config_params(::keymaster::AuthorizationSetBuilder()
            .Authorization(::keymaster::TAG_OS_VERSION, ::keymaster::GetOsVersion())
            .Authorization(::keymaster::TAG_OS_PATCHLEVEL, ::keymaster::GetOsPatchlevel())
            .Authorization(::trustkernel::TAG_BOOT_VBMETA_DIGEST,
                keymaster_blob_t { .data = (uint8_t *) str_vbmeta_digest, .data_length = strlen(str_vbmeta_digest) })
            .Authorization(::trustkernel::TAG_BOOT_VBMETA_DEVICE_STATE, vbmeta_device_state)
            .Authorization(::trustkernel::TAG_BOOT_VERIFIED_BOOT_STATE, verified_boot_state));

    impl_->configure(&config_params);
}

TrustKernelKeymaster4Device::~TrustKernelKeymaster4Device() {}

Return<void> TrustKernelKeymaster4Device::getHardwareInfo(getHardwareInfo_cb _hidl_cb) {
    _hidl_cb(securityLevel_,
             "HardwareKeymasterDevice", "TrustKernel");
    return Void();
}

Return<void>
TrustKernelKeymaster4Device::getHmacSharingParameters(getHmacSharingParameters_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_hmac_sharing_parameters_t in_params;

    ::android::hardware::keymaster::V4_0::HmacSharingParameters params;

    static_assert(sizeof(in_params.nonce) == params.nonce.size(),
        "Nonce sizes don't match");

    error = impl_->get_hmac_sharing_parameters(&in_params);
    if (error == KM_ERROR_OK) {
        params.seed = kmBlob2hidlVec(in_params.seed);
        memcpy(params.nonce.data(), in_params.nonce, params.nonce.size());
    }

    _hidl_cb(legacy_enum_conversion(error), params);
    free((void *) in_params.seed.data);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::computeSharedHmac(
    const hidl_vec<::android::hardware::keymaster::V4_0::HmacSharingParameters>& params,
    computeSharedHmac_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_blob_t sharing_check_blob;

    hidl_vec<uint8_t> sharing_check;
    size_t size = 0;

    std::vector<hidl_vec<uint8_t>> bin_params(params.size());
    for (size_t i = 0; i < params.size(); ++i) {
        size += 4 + params[i].seed.size() + params[i].nonce.size();
        bin_params[i].resize(params[i].seed.size() + params[i].nonce.size());
        memcpy(bin_params[i].data(), params[i].seed.data(), params[i].seed.size());
        memcpy(bin_params[i].data() + params[i].seed.size(), params[i].nonce.data(), params[i].nonce.size());
    }
    std::sort(bin_params.begin(), bin_params.end());
    uint8_t *buf = new uint8_t[size], *p = buf;
    if (buf == NULL) {
        _hidl_cb(legacy_enum_conversion(KM_ERROR_MEMORY_ALLOCATION_FAILED), sharing_check);
        return Void();
    }
    for (size_t i = 0; i < bin_params.size(); i++) {
        uint32_t s = bin_params[i].size();
        memcpy(p, &s, sizeof(s));
        p += sizeof(s);
        memcpy(p, bin_params[i].data(), bin_params[i].size());
        p += bin_params[i].size();
    }

    error = impl_->compute_shared_hmac(buf, size,
            &sharing_check_blob);
    if (error == KM_ERROR_OK) {
        sharing_check = kmBlob2hidlVec(sharing_check_blob);
    }
    delete[] buf;

    _hidl_cb(legacy_enum_conversion(error), sharing_check);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::verifyAuthorization(
    uint64_t challenge, const hidl_vec<KeyParameter>& parametersToVerify,
    const ::android::hardware::keymaster::V4_0::HardwareAuthToken& authToken,
    verifyAuthorization_cb _hidl_cb)
{
    keymaster_error_t error;
    KmParamSet km_param_set(parametersToVerify);
    keymaster_hardware_auth_token_t auth_token;
    keymaster_verification_token_t verify_token;

    auth_token.version = 0;
    auth_token.challenge = authToken.challenge;
    auth_token.user_id = authToken.userId;
    auth_token.authenticator_id = authToken.authenticatorId;
    auth_token.authenticator_type = legacy_enum_conversion(authToken.authenticatorType);
    auth_token.timestamp = authToken.timestamp;
    if (authToken.mac.size() >= sizeof(auth_token.mac))
        memcpy(auth_token.mac, authToken.mac.data(), sizeof(auth_token.mac));


    ::android::hardware::keymaster::V4_0::VerificationToken token;

    error = impl_->verify_authorization(challenge, &km_param_set, &auth_token, &verify_token);
    if (error == KM_ERROR_OK) {
        token.challenge = verify_token.challenge;
        token.timestamp = verify_token.timestamp;
        token.parametersVerified = kmParamSet2Hidl(verify_token.parameters_verified);
        token.securityLevel = legacy_enum_conversion(verify_token.security_level);
        token.mac = hidl_vec<uint8_t>(verify_token.mac, verify_token.mac + sizeof(verify_token.mac));
    }

    _hidl_cb(legacy_enum_conversion(error), token);

    return Void();
}

Return<ErrorCode> TrustKernelKeymaster4Device::addRngEntropy(const hidl_vec<uint8_t>& data) {
    if (data.size() == 0)
        return ErrorCode::OK;

    return legacy_enum_conversion(
        impl_->add_rng_entropy(data.data(), data.size()));
}

Return<void> TrustKernelKeymaster4Device::generateKey(const hidl_vec<KeyParameter>& keyParams,
                                                  generateKey_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_key_blob_t key_blob;
    KmParamSet km_param_set(keyParams);
    keymaster_key_characteristics_t characteristics;

    error = impl_->generate_key(&km_param_set, &key_blob, &characteristics);

    KeyCharacteristics resultCharacteristics;
    hidl_vec<uint8_t> resultKeyBlob;
    if (error == KM_ERROR_OK) {
        resultKeyBlob = kmBlob2hidlVec(key_blob);
        resultCharacteristics.hardwareEnforced = kmParamSet2Hidl(characteristics.hw_enforced);
        resultCharacteristics.softwareEnforced = kmParamSet2Hidl(characteristics.sw_enforced);
    }
    _hidl_cb(legacy_enum_conversion(error), resultKeyBlob, resultCharacteristics);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::getKeyCharacteristics(const hidl_vec<uint8_t>& keyBlob,
                                                            const hidl_vec<uint8_t>& clientId,
                                                            const hidl_vec<uint8_t>& appData,
                                                            getKeyCharacteristics_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_key_characteristics_t characteristics;

    keymaster_key_blob_t key_blob =
        { keyBlob.data(), keyBlob.size() };

    keymaster_blob_t client_id =
        { clientId.data(), clientId.size() };

    keymaster_blob_t app_data =
        { appData.data(), appData.size() };

    error = impl_->get_key_characteristics(&key_blob, &client_id, &app_data, &characteristics);

    KeyCharacteristics resultCharacteristics;
    if (error == KM_ERROR_OK) {
        resultCharacteristics.hardwareEnforced = kmParamSet2Hidl(characteristics.hw_enforced);
        resultCharacteristics.softwareEnforced = kmParamSet2Hidl(characteristics.sw_enforced);
    }
    _hidl_cb(legacy_enum_conversion(error), resultCharacteristics);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::importKey(const hidl_vec<KeyParameter>& params,
                                                KeyFormat keyFormat,
                                                const hidl_vec<uint8_t>& keyData,
                                                importKey_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_key_characteristics_t characteristics;
    keymaster_key_blob_t key_blob;

    KmParamSet km_param_set(params);
    keymaster_blob_t key_data = { keyData.data(), keyData.size() };

    error = impl_->import_key(&km_param_set, legacy_enum_conversion(keyFormat),
        &key_data, &key_blob, &characteristics);

    KeyCharacteristics resultCharacteristics;
    hidl_vec<uint8_t> resultKeyBlob;
    if (error == KM_ERROR_OK) {
        resultKeyBlob = kmBlob2hidlVec(key_blob);
        resultCharacteristics.hardwareEnforced = kmParamSet2Hidl(characteristics.hw_enforced);
        resultCharacteristics.softwareEnforced = kmParamSet2Hidl(characteristics.sw_enforced);
    }
    _hidl_cb(legacy_enum_conversion(error), resultKeyBlob, resultCharacteristics);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::importWrappedKey(
    const hidl_vec<uint8_t>& wrappedKeyData, const hidl_vec<uint8_t>& wrappingKeyBlob,
    const hidl_vec<uint8_t>& maskingKey, const hidl_vec<KeyParameter>& unwrappingParams,
    uint64_t passwordSid, uint64_t biometricSid, importWrappedKey_cb _hidl_cb)
{
    keymaster_error_t error;

    keymaster_key_blob_t out_key_blob;
    keymaster_key_characteristics_t out_characteristics;

    keymaster_key_blob_t wrapped_key = {
        wrappedKeyData.data(), wrappedKeyData.size()
    };

    keymaster_key_blob_t wrapping_key = {
        wrappingKeyBlob.data(), wrappingKeyBlob.size()
    };

    keymaster_key_blob_t masking_key = {
        maskingKey.data(), maskingKey.size()
    };

    KmParamSet additional_params(unwrappingParams);

    error = impl_->import_wrapped_key(&wrapped_key,
                                &wrapping_key,
                                &masking_key,
                                &additional_params,
                                passwordSid, biometricSid,
                                &out_key_blob, &out_characteristics);

    KeyCharacteristics resultCharacteristics;
    hidl_vec<uint8_t> resultKeyBlob;
    if (error == KM_ERROR_OK) {
        resultKeyBlob = kmBlob2hidlVec(out_key_blob);
        resultCharacteristics.hardwareEnforced =
            kmParamSet2Hidl(out_characteristics.hw_enforced);
        resultCharacteristics.softwareEnforced =
            kmParamSet2Hidl(out_characteristics.sw_enforced);
    }

    _hidl_cb(legacy_enum_conversion(error), resultKeyBlob, resultCharacteristics);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::exportKey(KeyFormat exportFormat,
                                                const hidl_vec<uint8_t>& keyBlob,
                                                const hidl_vec<uint8_t>& clientId,
                                                const hidl_vec<uint8_t>& appData,
                                                exportKey_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_blob_t export_data;

    keymaster_key_blob_t key_blob = {
        keyBlob.data(), keyBlob.size()
    };

    keymaster_blob_t client_id = {
        clientId.data(), clientId.size()
    };

    keymaster_blob_t app_data = {
        appData.data(), appData.size()
    };

    error = impl_->export_key(legacy_enum_conversion(exportFormat),
        &key_blob, &client_id, &app_data, &export_data);

    hidl_vec<uint8_t> resultKeyBlob;
    if (error == KM_ERROR_OK) {
        resultKeyBlob = kmBlob2hidlVec(export_data);
    }
    _hidl_cb(legacy_enum_conversion(error), resultKeyBlob);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::attestKey(const hidl_vec<uint8_t>& keyToAttest,
                                                const hidl_vec<KeyParameter>& attestParams,
                                                attestKey_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_cert_chain_t cert_chain;

    keymaster_key_blob_t key_blob = {
        keyToAttest.data(), keyToAttest.size()
    };
    KmParamSet km_param_set(attestParams);

    error = impl_->attest_key(&key_blob, &km_param_set, &cert_chain);

    hidl_vec<hidl_vec<uint8_t>> resultCertChain;
    if (error == KM_ERROR_OK) {
        resultCertChain = kmCertChain2Hidl(cert_chain);
    }
    _hidl_cb(legacy_enum_conversion(error), resultCertChain);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::upgradeKey(const hidl_vec<uint8_t>& keyBlobToUpgrade,
                                                 const hidl_vec<KeyParameter>& upgradeParams,
                                                 upgradeKey_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_key_blob_t upgraded_key_blob;

    keymaster_key_blob_t key_blob = { keyBlobToUpgrade.data(), keyBlobToUpgrade.size() };
    KmParamSet upgrade_params(upgradeParams);

    error = impl_->upgrade_key(&key_blob, &upgrade_params, &upgraded_key_blob);

    if (error == KM_ERROR_OK) {
        _hidl_cb(ErrorCode::OK, kmBlob2hidlVec(upgraded_key_blob));
    } else {
        _hidl_cb(legacy_enum_conversion(error), hidl_vec<uint8_t>());
    }
    return Void();
}

Return<ErrorCode> TrustKernelKeymaster4Device::deleteKey(const hidl_vec<uint8_t>& keyBlob)
{
    keymaster_error_t error;

    keymaster_key_blob_t key_blob = {
        keyBlob.data(), keyBlob.size()
    };

    error = impl_->delete_key(&key_blob);

    return legacy_enum_conversion(error);
}

Return<ErrorCode> TrustKernelKeymaster4Device::deleteAllKeys()
{
    return legacy_enum_conversion(impl_->delete_all_keys());
}

Return<ErrorCode> TrustKernelKeymaster4Device::destroyAttestationIds() {
    return ErrorCode::UNIMPLEMENTED;
}

Return<void> TrustKernelKeymaster4Device::begin(KeyPurpose purpose, const hidl_vec<uint8_t>& key,
                                            const hidl_vec<KeyParameter>& inParams,
                                            const HardwareAuthToken& authToken,
                                            begin_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_key_param_set_t output_params;
    keymaster_operation_handle_t handle;
    auto auth_token = ::android::hardware::keymaster::V4_0::support::authToken2HidlVec(authToken);
    keymaster_blob_t auth_token_blob = {
        auth_token.data(), auth_token.size()
    };

    KmParamSet in_params(inParams);
    keymaster_key_blob_t key_blob = {
        key.data(), key.size()
    };

    error = impl_->begin(legacy_enum_conversion(purpose), &key_blob,
        &in_params, &auth_token_blob, &output_params, &handle);

    hidl_vec<KeyParameter> resultParams;
    if (error == KM_ERROR_OK) {
        resultParams = kmParamSet2Hidl(output_params);
    }

    _hidl_cb(legacy_enum_conversion(error), resultParams, handle);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::update(uint64_t operationHandle,
                                             const hidl_vec<KeyParameter>& inParams,
                                             const hidl_vec<uint8_t>& input,
                                             const HardwareAuthToken& authToken,
                                             const VerificationToken& /* verificationToken */,
                                             update_cb _hidl_cb)
{
    keymaster_error_t error;

    keymaster_blob_t output;
    keymaster_key_param_set_t output_params;
    size_t resultConsumed = 0;

    KmParamSet input_params(inParams);
    keymaster_operation_handle_t operation_handle = operationHandle;
    keymaster_blob_t input_blob = { input.data(), input.size() };

    auto auth_token = ::android::hardware::keymaster::V4_0::support::authToken2HidlVec(authToken);
    keymaster_blob_t auth_token_blob = {
        auth_token.data(), auth_token.size()
    };

    error = impl_->update(operation_handle, &input_params,
        &input_blob, &resultConsumed, &auth_token_blob, &output_params, &output);

    hidl_vec<KeyParameter> resultParams;
    hidl_vec<uint8_t> resultBlob;
    if (error == KM_ERROR_OK) {
        resultParams = kmParamSet2Hidl(output_params);
        resultBlob = kmBlob2hidlVec(output);
    }
    _hidl_cb(legacy_enum_conversion(error), resultConsumed, resultParams, resultBlob);
    return Void();
}

Return<void> TrustKernelKeymaster4Device::finish(uint64_t operationHandle,
                                             const hidl_vec<KeyParameter>& inParams,
                                             const hidl_vec<uint8_t>& input,
                                             const hidl_vec<uint8_t>& signature,
                                             const HardwareAuthToken& authToken,
                                             const VerificationToken& /* verificationToken */,
                                             finish_cb _hidl_cb)
{
    keymaster_error_t error;
    keymaster_key_param_set_t output_params;
    keymaster_blob_t output_data;

    keymaster_operation_handle_t operation_handle = operationHandle;
    KmParamSet input_params(inParams);

    auto auth_token = ::android::hardware::keymaster::V4_0::support::authToken2HidlVec(authToken);
    keymaster_blob_t auth_token_blob = {
        auth_token.data(), auth_token.size()
    };

    keymaster_blob_t input_data = {
        input.data(), input.size()
    };

    keymaster_blob_t signature_blob = {
        signature.data(), signature.size()
    };

    error = impl_->finish(operation_handle, &input_params,
        &input_data, &signature_blob, &auth_token_blob, &output_params, &output_data);

    hidl_vec<KeyParameter> resultParams;
    hidl_vec<uint8_t> resultBlob;
    if (error == KM_ERROR_OK) {
        resultParams = kmParamSet2Hidl(output_params);
        resultBlob = kmBlob2hidlVec(output_data);
    }
    _hidl_cb(legacy_enum_conversion(error), resultParams, resultBlob);
    return Void();
}

Return<ErrorCode> TrustKernelKeymaster4Device::abort(uint64_t operationHandle) {
    return legacy_enum_conversion(
        impl_->abort((keymaster_operation_handle_t) operationHandle));
}

IKeymasterDevice* CreateKeymasterDevice() {
    return new TrustKernelKeymaster4Device();
}

}  // namespace trustkernel
