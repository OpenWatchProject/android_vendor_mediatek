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

#ifndef TRUSTONIC_TEE_KEYMASTER_IMPL_H_
#define TRUSTONIC_TEE_KEYMASTER_IMPL_H_

#include "tlcTeeKeymasterM_if.h"

class TrustonicTeeKeymasterImpl {
  public:
    TrustonicTeeKeymasterImpl();

    ~TrustonicTeeKeymasterImpl();

    keymaster_error_t get_supported_algorithms(
                        keymaster_algorithm_t**         algorithms,
                        size_t*                         algorithms_length);

    keymaster_error_t get_supported_block_modes(
                        keymaster_algorithm_t           algorithm,
                        keymaster_purpose_t             purpose,
                        keymaster_block_mode_t**        modes,
                        size_t*                         modes_length);

    keymaster_error_t get_supported_padding_modes(
                        keymaster_algorithm_t           algorithm,
                        keymaster_purpose_t             purpose,
                        keymaster_padding_t**           modes,
                        size_t*                         modes_length);

    keymaster_error_t get_supported_digests(
                        keymaster_algorithm_t           algorithm,
                        keymaster_purpose_t             purpose,
                        keymaster_digest_t**            digests,
                        size_t*                         digests_length);

    keymaster_error_t get_supported_import_formats(
                        keymaster_algorithm_t           algorithm,
                        keymaster_key_format_t**        formats,
                        size_t*                         formats_length);

    keymaster_error_t get_supported_export_formats(
                        keymaster_algorithm_t           algorithm,
                        keymaster_key_format_t**        formats,
                        size_t*                         formats_length);

    keymaster_error_t add_rng_entropy(
                        const uint8_t*                  data,
                        size_t                          data_length);

    keymaster_error_t generate_key(
                        const keymaster_key_param_set_t* params,
                        keymaster_key_blob_t*           key_blob,
                        keymaster_key_characteristics_t** characteristics);

    keymaster_error_t get_key_characteristics(
                        const keymaster_key_blob_t*     key_blob,
                        const keymaster_blob_t*         client_id,
                        const keymaster_blob_t*         app_data,
                        keymaster_key_characteristics_t** character);

    keymaster_error_t import_key(
                        const keymaster_key_param_set_t* params,
                        keymaster_key_format_t          key_format,
                        const keymaster_blob_t*         key_data,
                        keymaster_key_blob_t*           key_blob,
                        keymaster_key_characteristics_t** characteristics);

    keymaster_error_t export_key(
                        keymaster_key_format_t          export_format,
                        const keymaster_key_blob_t*     key_to_export,
                        const keymaster_blob_t*         client_id,
                        const keymaster_blob_t*         app_data,
                        keymaster_blob_t*               export_data);

    keymaster_error_t begin(
                        keymaster_purpose_t             purpose,
                        const keymaster_key_blob_t*     key,
                        const keymaster_key_param_set_t* params,
                        keymaster_key_param_set_t*      out_params,
                        keymaster_operation_handle_t*   operation_handle);

    keymaster_error_t update(
                        keymaster_operation_handle_t    operation_handle,
                        const keymaster_key_param_set_t* params,
                        const keymaster_blob_t*         input,
                        size_t*                         input_consumed,
                        keymaster_key_param_set_t*      out_params,
                        keymaster_blob_t*               output);

    keymaster_error_t finish(
                        keymaster_operation_handle_t    operation_handle,
                        const keymaster_key_param_set_t* params,
                        const keymaster_blob_t*         signature,
                        keymaster_key_param_set_t*      out_params,
                        keymaster_blob_t*               output);

    keymaster_error_t abort(
        keymaster_operation_handle_t operation_handle);

#ifdef SOTER
    keymaster_error_t delete_keypair(void);
    keymaster_error_t delete_all(void);

    keymaster_error_t generate_attk_key_pair(
            const uint8_t copy_num);

    keymaster_error_t verify_attk_key_pair(void);

    keymaster_error_t export_attk_public_key(
            const uint8_t** pub_key_data,
            size_t* pub_key_data_length);

    keymaster_error_t get_device_id(
            const uint8_t** device_id,
            size_t* device_id_length);
#endif

    TEE_SessionHandle session_handle_;
    int errcode;

};


#endif  //  TRUSTONIC_TEE_KEYMASTER_IMPL_H_
