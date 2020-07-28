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

#ifndef TEE_KEYMASTER_DEVICE_H_
#define TEE_KEYMASTER_DEVICE_H_

#include <stdlib.h>

#ifdef SOTER
#include "mtk_keymaster1.h"
#else
#include <hardware/keymaster1.h>
#endif

#include <UniquePtr.h>


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
    keymaster1_device_t* keymaster_device();

    /*
     * These static methods are the functions referenced through the function pointers in
     * keymaster_device.
     */

    // Version 0.4 APIs.
    static keymaster_error_t get_supported_algorithms(
                        const keymaster1_device_t*      dev,
                        keymaster_algorithm_t**         algorithms,
                        size_t*                         algorithms_length);

    static keymaster_error_t get_supported_block_modes(
                        const keymaster1_device_t*      dev,
                        keymaster_algorithm_t           algorithm,
                        keymaster_purpose_t             purpose,
                        keymaster_block_mode_t**        modes,
                        size_t*                         modes_length);

    static keymaster_error_t get_supported_padding_modes(
                        const keymaster1_device_t*      dev,
                        keymaster_algorithm_t           algorithm,
                        keymaster_purpose_t             purpose,
                        keymaster_padding_t**           modes,
                        size_t*                         modes_length);

    static keymaster_error_t get_supported_digests(
                        const keymaster1_device_t*      dev,
                        keymaster_algorithm_t           algorithm,
                        keymaster_purpose_t             purpose,
                        keymaster_digest_t**            digests,
                        size_t*                         digests_length);

    static keymaster_error_t get_supported_import_formats(
                        const keymaster1_device_t*      dev,
                        keymaster_algorithm_t           algorithm,
                        keymaster_key_format_t**        formats,
                        size_t*                         formats_length);

    static keymaster_error_t get_supported_export_formats(
                        const keymaster1_device_t*      dev,
                        keymaster_algorithm_t           algorithm,
                        keymaster_key_format_t**        formats,
                        size_t*                         formats_length);

    static keymaster_error_t add_rng_entropy(
                        const keymaster1_device_t*      dev,
                        const uint8_t*                  data,
                        size_t                          data_length);

    static keymaster_error_t generate_key(
                        const keymaster1_device_t*      dev,
                        const keymaster_key_param_set_t* params,
                        keymaster_key_blob_t*           key_blob,
                        keymaster_key_characteristics_t** characteristics);

    static keymaster_error_t get_key_characteristics(
                        const keymaster1_device_t*      dev,
                        const keymaster_key_blob_t*     key_blob,
                        const keymaster_blob_t*         client_id,
                        const keymaster_blob_t*         app_data,
                        keymaster_key_characteristics_t** character);

    static keymaster_error_t import_key(
                        const keymaster1_device_t*      dev,
                        const keymaster_key_param_set_t* params,
                        keymaster_key_format_t          key_format,
                        const keymaster_blob_t*         key_data,
                        keymaster_key_blob_t*           key_blob,
                        keymaster_key_characteristics_t** characteristics);

    static keymaster_error_t export_key(
                        const keymaster1_device_t*      dev,
                        keymaster_key_format_t          export_format,
                        const keymaster_key_blob_t*     key_to_export,
                        const keymaster_blob_t*         client_id,
                        const keymaster_blob_t*         app_data,
                        keymaster_blob_t*               export_data);

    static keymaster_error_t begin(
                        const keymaster1_device_t*      dev,
                        keymaster_purpose_t             purpose,
                        const keymaster_key_blob_t*     key,
                        const keymaster_key_param_set_t* params,
                        keymaster_key_param_set_t*      out_params,
                        keymaster_operation_handle_t*   operation_handle);

    static keymaster_error_t update(
                        const keymaster1_device_t*      dev,
                        keymaster_operation_handle_t    operation_handle,
                        const keymaster_key_param_set_t* params,
                        const keymaster_blob_t*         input,
                        size_t*                         input_consumed,
                        keymaster_key_param_set_t*      out_params,
                        keymaster_blob_t*               output);

    static keymaster_error_t finish(
                        const keymaster1_device_t*      dev,
                        keymaster_operation_handle_t    operation_handle,
                        const keymaster_key_param_set_t* params,
                        const keymaster_blob_t*         signature,
                        keymaster_key_param_set_t*      out_params,
                        keymaster_blob_t*               output);

    static keymaster_error_t abort(
                        const keymaster1_device_t*      dev,
                        keymaster_operation_handle_t    operation_handle);

#ifdef SOTER
    static keymaster_error_t generate_attk_key_pair(
                        const keymaster1_device_t* dev,
                        const uint8_t copy_num);

	static keymaster_error_t verify_attk_key_pair(
                        const keymaster1_device_t* dev);

    static keymaster_error_t export_attk_public_key(
                        const keymaster1_device_t* dev,
                        const uint8_t** pub_key_data,
                        size_t* pub_key_data_length);

    static keymaster_error_t get_device_id(
                        const keymaster1_device_t* dev,
                        const uint8_t** device_id,
                        size_t* device_id_length);
#endif
  private:

    // Class is non-copyable and not default-constructible
    TeeKeymasterDevice();
    TeeKeymasterDevice(const TeeKeymasterDevice&);
    TeeKeymasterDevice& operator=(const TeeKeymasterDevice&);

#ifdef SOTER
    mtk_keymaster1_device_t               device_;
#else
    keymaster1_device_t                   device_;
#endif
  public:
    // Testing wants a sneaky way to get hold of this.
    UniquePtr<TrustonicTeeKeymasterImpl>  impl_;
};

#endif  // TEE_KEYMASTER_DEVICE_H_
