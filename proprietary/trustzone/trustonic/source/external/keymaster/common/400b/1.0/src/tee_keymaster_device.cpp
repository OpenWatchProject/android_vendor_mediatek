/*
 * Copyright 2015 The Android Open Source Project
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

#ifdef SOTER
#include "mtk_keymaster_common.h"
#else
#include <hardware/keymaster_common.h>
#endif
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

    device_.flags = KEYMASTER_BLOBS_ARE_STANDALONE | KEYMASTER_SUPPORTS_EC;

    // The following functions are in the keymaster1_device_t definition but
    // should not be implemented.
    device_.generate_keypair    = NULL;
    device_.import_keypair      = NULL;
    device_.get_keypair_public  = NULL;
    device_.delete_keypair      = NULL;
    device_.delete_all          = NULL;
    device_.sign_data           = NULL;
    device_.verify_data         = NULL;

    // keymaster1 API
    device_.get_supported_algorithms = get_supported_algorithms;
    device_.get_supported_block_modes = get_supported_block_modes;
    device_.get_supported_padding_modes = get_supported_padding_modes;
    device_.get_supported_digests = get_supported_digests;
    device_.get_supported_import_formats = get_supported_import_formats;
    device_.get_supported_export_formats = get_supported_export_formats;
    device_.add_rng_entropy = add_rng_entropy;
    device_.generate_key = generate_key;
    device_.get_key_characteristics = get_key_characteristics;
    device_.import_key = import_key;
    device_.export_key = export_key;
    device_.delete_key = NULL;
    device_.delete_all_keys = NULL;
    device_.begin = begin;
    device_.update = update;
    device_.finish = finish;
    device_.abort = abort;
#ifdef SOTER
    device_.generate_attk_key_pair = generate_attk_key_pair;
    device_.verify_attk_key_pair = verify_attk_key_pair;
    device_.export_attk_public_key = export_attk_public_key;
    device_.get_device_id = get_device_id;
#endif
    device_.context = NULL;
}


hw_device_t* TeeKeymasterDevice::hw_device() {
    return &device_.common;
}


keymaster1_device_t* TeeKeymasterDevice::keymaster_device() {
    return &device_;
}

/* static */
static inline TeeKeymasterDevice* convert_device(const keymaster1_device_t* dev) {
    return reinterpret_cast<TeeKeymasterDevice*>(const_cast<keymaster1_device_t*>(dev));
}


/* Static */
keymaster_error_t TeeKeymasterDevice::get_supported_algorithms(
    const keymaster1_device_t*      dev,
    keymaster_algorithm_t**         algorithms,
    size_t*                         algorithms_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }
    return convert_device(dev)->impl_->get_supported_algorithms(
        algorithms, algorithms_length);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::get_supported_block_modes(
    const struct keymaster1_device* dev,
    keymaster_algorithm_t           algorithm,
    keymaster_purpose_t             purpose,
    keymaster_block_mode_t**        modes,
    size_t*                         modes_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_supported_block_modes(
        algorithm, purpose, modes, modes_length);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::get_supported_padding_modes(
    const keymaster1_device_t*      dev,
    keymaster_algorithm_t           algorithm,
    keymaster_purpose_t             purpose,
    keymaster_padding_t**           modes,
    size_t*                         modes_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_supported_padding_modes(
        algorithm, purpose, modes, modes_length);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::get_supported_digests(
    const keymaster1_device_t*      dev,
    keymaster_algorithm_t           algorithm,
    keymaster_purpose_t             purpose,
    keymaster_digest_t**            digests,
    size_t*                         digests_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_supported_digests(
        algorithm, purpose, digests, digests_length);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::get_supported_import_formats(
    const keymaster1_device_t*      dev,
    keymaster_algorithm_t           algorithm,
    keymaster_key_format_t**        formats,
    size_t*                         formats_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_supported_import_formats(
        algorithm, formats, formats_length);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::get_supported_export_formats(
    const keymaster1_device_t*       dev,
    keymaster_algorithm_t            algorithm,
    keymaster_key_format_t**         formats,
    size_t*                          formats_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_supported_export_formats(
        algorithm, formats, formats_length);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::add_rng_entropy(
    const keymaster1_device_t*      dev,
    const uint8_t*                  data,
    size_t                          data_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->add_rng_entropy(
        data, data_length);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::generate_key(
    const struct keymaster1_device*     dev,
    const keymaster_key_param_set_t*    params,
    keymaster_key_blob_t*               key_blob,
    keymaster_key_characteristics_t**   characteristics)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->generate_key(
        params, key_blob, characteristics);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::get_key_characteristics(
    const keymaster1_device_t*      dev,
    const keymaster_key_blob_t*     key_blob,
    const keymaster_blob_t*         client_id,
    const keymaster_blob_t*         app_data,
    keymaster_key_characteristics_t** characteristics)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_key_characteristics(
        key_blob, client_id, app_data, characteristics);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::import_key(
    const keymaster1_device_t*      dev,
    const keymaster_key_param_set_t* params,
    keymaster_key_format_t          key_format,
    const keymaster_blob_t*         key_data,
    keymaster_key_blob_t*           key_blob,
    keymaster_key_characteristics_t** characteristics)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->import_key(
        params, key_format, key_data, key_blob, characteristics);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::export_key(
    const keymaster1_device_t*      dev,
    keymaster_key_format_t          export_format,
    const keymaster_key_blob_t*     key_to_export,
    const keymaster_blob_t*         client_id,
    const keymaster_blob_t*         app_data,
    keymaster_blob_t*               export_data)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->export_key(
        export_format, key_to_export, client_id, app_data, export_data);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::begin(
    const keymaster1_device_t*      dev,
    keymaster_purpose_t             purpose,
    const keymaster_key_blob_t*     key,
    const keymaster_key_param_set_t* params,
    keymaster_key_param_set_t*      out_params,
    keymaster_operation_handle_t*   operation_handle)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->begin(
        purpose, key, params, out_params, operation_handle);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::update(
    const keymaster1_device_t*      dev,
    keymaster_operation_handle_t    operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t*         input,
    size_t*                         input_consumed,
    keymaster_key_param_set_t*      out_params,
    keymaster_blob_t*               output)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->update(
        operation_handle, params, input, input_consumed, out_params, output);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::finish(
    const keymaster1_device_t*      dev,
    keymaster_operation_handle_t    operation_handle,
    const keymaster_key_param_set_t* params,
    const keymaster_blob_t*         signature,
    keymaster_key_param_set_t*      out_params,
    keymaster_blob_t*               output)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->finish(
        operation_handle, params, signature, out_params, output);
}


/* Static */
keymaster_error_t TeeKeymasterDevice::abort(
    const keymaster1_device_t*      dev,
    keymaster_operation_handle_t    operation_handle)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->abort(
        operation_handle);
}

#ifdef SOTER
keymaster_error_t TeeKeymasterDevice::generate_attk_key_pair(
    const struct keymaster1_device* dev,
    const uint8_t copy_num)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->generate_attk_key_pair(copy_num);
}

keymaster_error_t TeeKeymasterDevice::verify_attk_key_pair(
    const struct keymaster1_device* dev)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->verify_attk_key_pair();
}

keymaster_error_t TeeKeymasterDevice::export_attk_public_key(
    const struct keymaster1_device* dev,
    const uint8_t** pub_key_data,
    size_t* pub_key_data_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->export_attk_public_key(
            pub_key_data, pub_key_data_length);
}

keymaster_error_t TeeKeymasterDevice::get_device_id(
    const struct keymaster1_device* dev,
    const uint8_t** device_id,
    size_t* device_id_length)
{
    if (dev == NULL)
    {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }

    return convert_device(dev)->impl_->get_device_id(
            device_id, device_id_length);
}
#endif
