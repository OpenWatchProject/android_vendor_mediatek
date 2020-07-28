/*
 * Copyright (c) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef __UT_KEYMASTER_API_H__
#define __UT_KEYMASTER_API_H__

__BEGIN_DECLS

#include <stdbool.h>
#include <stdint.h>

#include <hardware/keymaster_defs.h>

keymaster_error_t ut_generate_key(const keymaster_key_param_set_t* params,
                                  keymaster_key_blob_t* key_blob,
                                  keymaster_key_characteristics_t* characteristics);

keymaster_error_t ut_get_supported_algorithms(keymaster_algorithm_t** algorithms,
                                              size_t* algorithms_length);

keymaster_error_t ut_get_supported_block_modes(keymaster_algorithm_t algorithm,
                                               keymaster_purpose_t purpose,
                                               keymaster_block_mode_t** modes,
                                               size_t* modes_length);

keymaster_error_t ut_get_supported_padding_modes(keymaster_algorithm_t algorithm,
                                                 keymaster_purpose_t purpose,
                                                 keymaster_padding_t** modes, size_t* modes_length);

keymaster_error_t ut_get_supported_digests(keymaster_algorithm_t algorithm,
                                           keymaster_purpose_t purpose,
                                           keymaster_digest_t** digests, size_t* digests_length);

keymaster_error_t ut_get_supported_import_formats(keymaster_algorithm_t algorithm,
                                                  keymaster_key_format_t** formats,
                                                  size_t* formats_length);

keymaster_error_t ut_get_supported_export_formats(keymaster_algorithm_t algorithm,
                                                  keymaster_key_format_t** formats,
                                                  size_t* formats_length);

keymaster_error_t ut_get_key_characteristics(keymaster_key_blob_t* key_blob,
                                             keymaster_blob_t* client_id,
                                             keymaster_blob_t* app_data,
                                             keymaster_key_characteristics_t* characteristics);

keymaster_error_t ut_import_key(const keymaster_key_param_set_t* params,
                                keymaster_key_format_t key_format, const keymaster_blob_t* key_data,
                                keymaster_key_blob_t* key_blob,
                                keymaster_key_characteristics_t* characteristics);

keymaster_error_t ut_export_key(keymaster_key_format_t export_format,
                                const keymaster_key_blob_t* key_to_export,
                                const keymaster_blob_t* client_id, const keymaster_blob_t* app_data,
                                keymaster_blob_t* export_data);
keymaster_error_t ut_delete_key(const keymaster_key_blob_t* key);

/**
 * ut_begin
 * @param  purpose          [in]
 * @param  key              [in]
 * @param  in_params        [in]
 * @param  out_params       [out]
 * @param  operation_handle [out]
 */
keymaster_error_t ut_begin(keymaster_purpose_t purpose, const keymaster_key_blob_t* key,
                           const keymaster_key_param_set_t* in_params,
                           keymaster_key_param_set_t* out_params,
                           keymaster_operation_handle_t* operation_handle);

/**
 * ut_update
 * @param  operation_handle [in]
 * @param  in_params        [in]
 * @param  input            [in]
 * @param  input_consumed   [in]
 * @param  out_params       [out]
 * @param  output           [out]
 */
keymaster_error_t ut_update(keymaster_operation_handle_t operation_handle,
                            const keymaster_key_param_set_t* in_params,
                            const keymaster_blob_t* input, size_t* input_consumed,
                            keymaster_key_param_set_t* out_params, keymaster_blob_t* output);

/**
 * ut_finish
 * @param   operation_handle[in]
 * @param   in_params       [in]
 * @param   signature       [in]
 * @param   out_params      [out]
 * @param   output          [out]
 */
keymaster_error_t ut_finish(keymaster_operation_handle_t operation_handle,
                            const keymaster_key_param_set_t* in_params,
                            const keymaster_blob_t* signature,
                            keymaster_key_param_set_t* out_params, keymaster_blob_t* output,
                            const keymaster_blob_t* input);

/**
 * ut_abort
 *
 * @param   operation_handle[in]
 */
keymaster_error_t ut_abort(keymaster_operation_handle_t operation_handle);


keymaster_error_t ut_attest_key(const keymaster_key_blob_t* key_to_attest,
                                const keymaster_key_param_set_t* attest_params,
                                keymaster_cert_chain_t* cert_chain);

keymaster_error_t ut_configure(const keymaster_key_param_set_t* params);

keymaster_error_t ut_upgrade_key(const keymaster_key_blob_t* key_to_upgrade,
                             const keymaster_key_param_set_t* upgrade_params,
                             keymaster_key_blob_t* upgraded_key);

keymaster_error_t ut_ree_import_attest_keybox(const unsigned char* peakb,
											const unsigned int peakb_len);


#ifdef SOTER
keymaster_error_t ut_generate_attk(const unsigned char copy_num);
keymaster_error_t ut_verify_attk();
keymaster_error_t ut_export_attk(const unsigned char** key_data, size_t* key_datalen);
keymaster_error_t ut_get_device_id(const unsigned char** device_id, size_t* device_idlen);
#endif

__END_DECLS

#endif //__UT_KEYMASTER_API_H__