/*
 * Copyright 2015-2016 The Android Open Source Project
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

#include <assert.h>

#include <hardware/keymaster_common.h>

#include <tee_keymaster_device.h>
#include <trustonic_tee_keymaster_impl.h>
#include <module.h>
#include "buildTag.h"
#include "km_util.h"

extern "C" {

extern struct keystore_module HAL_MODULE_INFO_SYM;

/******************************************************************************/
__attribute__((visibility("default")))
int kinibi_keymaster_open( const struct hw_module_t* module, const char* id,
                            struct hw_device_t** device)
{
    int ret = 0;
    LOG_I(MOBICORE_COMPONENT_BUILD_TAG);

    if( id == NULL )
        return -EINVAL;

    if( memcmp(id, KEYSTORE_KEYMASTER, strlen(KEYSTORE_KEYMASTER)) != 0)
        return -EINVAL;

    // Make sure we initialize only with known module
    if( (module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
        (module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
        (module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
        (0!=memcmp(module->name, HAL_MODULE_INFO_SYM.common.name, sizeof(KEYMASTER_HARDWARE_MODULE_TEE_NAME)-1)) )
    {
        return -EINVAL;
    }

    TeeKeymasterDevice* keymaster =
        new (std::nothrow) TeeKeymasterDevice(&HAL_MODULE_INFO_SYM.common);
    if (0 == keymaster)
    {
        // Heap exhausted
        return -ENOMEM;
    }

    if ((ret = keymaster->impl_->errcode) != 0) {
        delete keymaster;
        return ret;
    }
    assert(keymaster->impl_->session_handle_);

    *device = reinterpret_cast<hw_device_t*>(keymaster);
    return ret;
}

/******************************************************************************/
__attribute__((visibility("default")))
int kinibi_keymaster_close(hw_device_t *hw)
{
    if(hw != NULL)
    {
        TeeKeymasterDevice* km_device = reinterpret_cast<TeeKeymasterDevice*>(hw);
        delete km_device;
    }
    return 0;
}
} // extern "C"

/******************************************************************************/

TeeKeymasterDevice::TeeKeymasterDevice(hw_module_t* module) :
        impl_(new TrustonicTeeKeymasterImpl())
{
    memset(&device_, 0, sizeof(device_));

    device_.common.tag = HARDWARE_DEVICE_TAG;
    device_.common.version = 1;
    device_.common.module = module;
    device_.common.close = &kinibi_keymaster_close;

    device_.flags = 0;

    // keymaster2 API
    device_.configure = configure;
    device_.add_rng_entropy = add_rng_entropy;
    device_.generate_key = generate_key;
    device_.get_key_characteristics = get_key_characteristics;
    device_.import_key = import_key;
    device_.export_key = export_key;
    device_.attest_key = attest_key;
    device_.upgrade_key = upgrade_key;
    device_.delete_key = delete_key;
    device_.delete_all_keys = delete_all_keys;
    device_.begin = begin;
    device_.update = update;
    device_.finish = finish;
    device_.abort = abort;

    device_.context = NULL;
}


hw_device_t* TeeKeymasterDevice::hw_device() {
    return &device_.common;
}


keymaster2_device_t* TeeKeymasterDevice::keymaster_device() {
    return &device_;
}

static inline TeeKeymasterDevice* convert_device(const keymaster2_device_t* dev) {
    return reinterpret_cast<TeeKeymasterDevice*>(const_cast<keymaster2_device_t*>(dev));
}

keymaster_error_t TeeKeymasterDevice::configure(
    const keymaster2_device_t* dev,
    const keymaster_key_param_set_t* params)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->configure(
        params);
}

keymaster_error_t TeeKeymasterDevice::add_rng_entropy(
    const keymaster2_device_t* dev,
    const uint8_t* data,
    size_t data_length)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->add_rng_entropy(
        data, data_length);
}

keymaster_error_t TeeKeymasterDevice::generate_key(
    const struct keymaster2_device* dev,
    const keymaster_key_param_set_t* params,
    keymaster_key_blob_t* key_blob,
    keymaster_key_characteristics_t* characteristics)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->generate_key(
        params, key_blob, characteristics);
}

keymaster_error_t TeeKeymasterDevice::get_key_characteristics(
    const keymaster2_device_t* dev,
    const keymaster_key_blob_t* key_blob,
    const keymaster_blob_t* client_id,
    const keymaster_blob_t* app_data,
    keymaster_key_characteristics_t* characteristics)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_key_characteristics(
        key_blob, client_id, app_data, characteristics);
}

keymaster_error_t TeeKeymasterDevice::import_key(
    const keymaster2_device_t* dev,
    const keymaster_key_param_set_t* params,
    keymaster_key_format_t key_format,
    const keymaster_blob_t* key_data,
    keymaster_key_blob_t* key_blob,
    keymaster_key_characteristics_t* characteristics)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->import_key(
        params, key_format, key_data, key_blob, characteristics);
}

keymaster_error_t TeeKeymasterDevice::export_key(
    const keymaster2_device_t* dev,
    keymaster_key_format_t export_format,
    const keymaster_key_blob_t* key_to_export,
    const keymaster_blob_t* client_id,
    const keymaster_blob_t* app_data,
    keymaster_blob_t* export_data)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->export_key(
        export_format, key_to_export, client_id, app_data, export_data);
}

keymaster_error_t TeeKeymasterDevice::attest_key(
    const keymaster2_device_t* dev,
    const keymaster_key_blob_t* key_to_attest,
    const keymaster_key_param_set_t* attest_params,
    keymaster_cert_chain_t* cert_chain)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->attest_key(
        key_to_attest, attest_params, cert_chain);
}

keymaster_error_t TeeKeymasterDevice::upgrade_key(
    const keymaster2_device_t* dev,
    const keymaster_key_blob_t* key_to_upgrade,
    const keymaster_key_param_set_t* upgrade_params,
    keymaster_key_blob_t* upgraded_key)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->upgrade_key(
        key_to_upgrade, upgrade_params, upgraded_key);
}

keymaster_error_t TeeKeymasterDevice::delete_key(
    const struct keymaster2_device* dev,
    const keymaster_key_blob_t* key)
{
    if (dev == NULL || key == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->delete_key(key);
}

keymaster_error_t TeeKeymasterDevice::delete_all_keys(
    const struct keymaster2_device* dev)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->delete_all_keys();
}

keymaster_error_t TeeKeymasterDevice::begin(
    const keymaster2_device_t* dev,
    keymaster_purpose_t purpose,
    const keymaster_key_blob_t* key,
    const keymaster_key_param_set_t* params,
    keymaster_key_param_set_t* out_params,
    keymaster_operation_handle_t* operation_handle)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->begin(
        purpose, key, params, out_params, operation_handle);
}

keymaster_error_t TeeKeymasterDevice::update(
    const keymaster2_device_t* dev,
    keymaster_operation_handle_t operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t* input,
    size_t* input_consumed,
    keymaster_key_param_set_t* out_params,
    keymaster_blob_t* output)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->update(
        operation_handle, params, input, input_consumed, out_params, output);
}

keymaster_error_t TeeKeymasterDevice::finish(
    const keymaster2_device_t* dev,
    keymaster_operation_handle_t operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t* input,
    const keymaster_blob_t* signature,
    keymaster_key_param_set_t* out_params,
    keymaster_blob_t* output)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->finish(
        operation_handle, params, input, signature, out_params, output);
}

keymaster_error_t TeeKeymasterDevice::abort(
    const keymaster2_device_t* dev,
    keymaster_operation_handle_t operation_handle)
{
    if (dev == NULL) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->abort(
        operation_handle);
}
