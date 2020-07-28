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

#ifndef TEE_KEYMASTER_DEVICE_H_
#define TEE_KEYMASTER_DEVICE_H_

#include <stdlib.h>

#include <api_keymaster2.h>

#include <memory>

class TrustonicTeeKeymasterImpl;

/**
 * TEE based Keymaster implementation.
 *
 * IMPORTANT MAINTAINER NOTE: Pointers to instances of this class must be castable to hw_device_t
 * and keymaster_device. This means it must remain a standard layout class (no virtual functions and
 * no data members which aren't standard layout), and device_ must be the first data member.
 * Assertions in the constructor validate compliance with those constraints.
 */
class TeeKeymasterDevice {
  public:
    TeeKeymasterDevice(hw_module_t* module);

    hw_device_t* hw_device();
    keymaster2_device_t* keymaster_device();

    /*
     * These static methods are the functions referenced through the function pointers in
     * keymaster_device.
     */

    static keymaster_error_t configure(
        const struct keymaster2_device* dev,
        const keymaster_key_param_set_t* params);

    static keymaster_error_t add_rng_entropy(
        const keymaster2_device_t* dev,
        const uint8_t* data,
        size_t data_length);

    static keymaster_error_t generate_key(
        const keymaster2_device_t* dev,
        const keymaster_key_param_set_t* params,
        keymaster_key_blob_t* key_blob,
        keymaster_key_characteristics_t* characteristics);

    static keymaster_error_t get_key_characteristics(
        const keymaster2_device_t* dev,
        const keymaster_key_blob_t* key_blob,
        const keymaster_blob_t* client_id,
        const keymaster_blob_t* app_data,
        keymaster_key_characteristics_t* character);

    static keymaster_error_t import_key(
        const keymaster2_device_t* dev,
        const keymaster_key_param_set_t* params,
        keymaster_key_format_t key_format,
        const keymaster_blob_t* key_data,
        keymaster_key_blob_t* key_blob,
        keymaster_key_characteristics_t* characteristics);

    static keymaster_error_t export_key(
        const keymaster2_device_t* dev,
        keymaster_key_format_t export_format,
        const keymaster_key_blob_t* key_to_export,
        const keymaster_blob_t* client_id,
        const keymaster_blob_t* app_data,
        keymaster_blob_t* export_data);

    static keymaster_error_t attest_key(
        const struct keymaster2_device* dev,
        const keymaster_key_blob_t* key_to_attest,
        const keymaster_key_param_set_t* attest_params,
        keymaster_cert_chain_t* cert_chain);

    static keymaster_error_t upgrade_key(
        const struct keymaster2_device* dev,
        const keymaster_key_blob_t* key_to_upgrade,
        const keymaster_key_param_set_t* upgrade_params,
        keymaster_key_blob_t* upgraded_key);

    static keymaster_error_t delete_key(
        const struct keymaster2_device* dev,
        const keymaster_key_blob_t* key);

    static keymaster_error_t delete_all_keys(
        const struct keymaster2_device* dev);

    static keymaster_error_t begin(
        const keymaster2_device_t* dev,
        keymaster_purpose_t purpose,
        const keymaster_key_blob_t* key,
        const keymaster_key_param_set_t* params,
        keymaster_key_param_set_t* out_params,
        keymaster_operation_handle_t* operation_handle);

    static keymaster_error_t update(
        const keymaster2_device_t* dev,
        keymaster_operation_handle_t operation_handle,
        const keymaster_key_param_set_t* params,
        const keymaster_blob_t* input,
        size_t* input_consumed,
        keymaster_key_param_set_t* out_params,
        keymaster_blob_t* output);

    static keymaster_error_t finish(
        const keymaster2_device_t* dev,
        keymaster_operation_handle_t operation_handle,
        const keymaster_key_param_set_t* params,
        const keymaster_blob_t* input,
        const keymaster_blob_t* signature,
        keymaster_key_param_set_t* out_params,
        keymaster_blob_t* output);

    static keymaster_error_t abort(
        const keymaster2_device_t* dev,
        keymaster_operation_handle_t operation_handle);

  private:
    // Class is non-copyable and not default-constructible
    TeeKeymasterDevice();
    TeeKeymasterDevice(const TeeKeymasterDevice&);
    TeeKeymasterDevice& operator=(const TeeKeymasterDevice&);

    keymaster2_device_t device_;

  public:
    // Testing wants a sneaky way to get hold of this.
    std::unique_ptr<TrustonicTeeKeymasterImpl> impl_;
};

#endif  // TEE_KEYMASTER_DEVICE_H_
