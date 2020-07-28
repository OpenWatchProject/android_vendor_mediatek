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

#ifndef __TLCTEEKEYMASTERM_IF_H__
#define __TLCTEEKEYMASTERM_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SOTER
#include "mtk_keymaster_defs.h"
#endif

#include <stdint.h>
#include <stdbool.h>

typedef void *TEE_SessionHandle;

/**
 * Open session to the TEE Keymaster trusted application
 *
 * @param  pSessionHandle  [out] Return pointer to the session handle
 *
 * @return Zero, or negative @c errno value
 */
int TEE_Open(
    TEE_SessionHandle                 *sessionHandle);

/**
 * Close session to the TEE Keymaster trusted application
 *
 * @param  sessionHandle  [in] Session handle
 */
void TEE_Close(
    TEE_SessionHandle                 sessionHandle);

keymaster_error_t TEE_AddRngEntropy(
    TEE_SessionHandle                 session_handle,
    const uint8_t*                    data,
    uint32_t                          dataLength);

keymaster_error_t TEE_GenerateKey(
    TEE_SessionHandle                 session_handle,
    const keymaster_key_param_set_t*  params,
    keymaster_key_blob_t*             key_blob,
    keymaster_key_characteristics_t** characteristics);

keymaster_error_t TEE_GetKeyCharacteristics(
    TEE_SessionHandle                 session_handle,
    const keymaster_key_blob_t*       key_blob,
    const keymaster_blob_t*           client_id,
    const keymaster_blob_t*           app_data,
    keymaster_key_characteristics_t** characteristics);

keymaster_error_t TEE_ImportKey(
    TEE_SessionHandle                 session_handle,
    const keymaster_key_param_set_t*  params,
    keymaster_key_format_t            key_format,
    const keymaster_blob_t*           key_data,
    keymaster_key_blob_t*             key_blob,
    keymaster_key_characteristics_t** characteristics);

keymaster_error_t TEE_ExportKey(
    TEE_SessionHandle                 session_handle,
    keymaster_key_format_t            export_format,
    const keymaster_key_blob_t*       key_to_export,
    const keymaster_blob_t*           client_id,
    const keymaster_blob_t*           app_data,
    keymaster_blob_t*                 export_data);

keymaster_error_t TEE_Begin(
    TEE_SessionHandle                 session_handle,
    keymaster_purpose_t               purpose,
    const keymaster_key_blob_t*       key,
    const keymaster_key_param_set_t*  params,
    keymaster_key_param_set_t*        out_params,
    keymaster_operation_handle_t*     operation_handle);

keymaster_error_t TEE_Update(
    TEE_SessionHandle                session_handle,
    keymaster_operation_handle_t      operation_handle,
    const keymaster_key_param_set_t*  params,
    const keymaster_blob_t*           input,
    size_t*                           input_consumed,
    keymaster_key_param_set_t*        out_params,
    keymaster_blob_t*                 output);

keymaster_error_t TEE_Finish(
    TEE_SessionHandle                 session_handle,
    keymaster_operation_handle_t      operation_handle,
    const keymaster_key_param_set_t*  params,
    const keymaster_blob_t*           signature,
    keymaster_key_param_set_t*        out_params,
    keymaster_blob_t*                 output);

keymaster_error_t TEE_Abort(
    TEE_SessionHandle                 session_handle,
    keymaster_operation_handle_t      operation_handle);

#ifdef SOTER
keymaster_error_t TEE_GenerateATTKKeyPair(
    TEE_SessionHandle                   sessionHandle,
    const keymaster_key_param_set_t*    params,
    const uint8_t                       copy_num);

keymaster_error_t TEE_VerifyATTKeyPair(
    TEE_SessionHandle                   sessionHandle);

keymaster_error_t TEE_ExportATTKPublicKey(
    TEE_SessionHandle                   sessionHandle,
    const uint8_t**                     pub_key_data,
    size_t*                             pub_key_data_length);

keymaster_error_t TEE_GetDeviceID(
    TEE_SessionHandle                   sessionHandle,
    const uint8_t**                     device_id,
    size_t*                             device_id_length);
#endif
#ifdef __cplusplus
}
#endif

#endif /* __TLCTEEKEYMASTERM_IF_H__ */
