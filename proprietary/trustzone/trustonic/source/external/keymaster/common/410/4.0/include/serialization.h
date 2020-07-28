/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
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

#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__

#include "keymaster_ta_defs.h"
#include "km_shared_util.h"
#include "km_util.h"

/**
 * Serialize key parameters.
 *
 * The format of the returned buffer is
 *
 * n (uint32_t) | param_1 (pbuf) | ... | param_n (pbuf)
 *
 * where a 'pbuf' is
 *
 * tag (uint32_t <= keymaster_tag_t) | val
 *
 * where 'val' is
 *
 * int_type OR bool_type OR longint_type OR blob_type
 *
 * where
 *
 * int_type = uint32_t
 * bool_type = uint32_t (value 0 or 1)
 * longint_type = uint64_t
 * blob_type = K | byte_1 (uint8_t) | ... | byte_K (uint8_t)
 *
 * \param[out] buf serialized parameters
 * \param[out] buflen length of \p buf
 * \param params set of parameters to serialize
 * \param add_time if true, add KM_TAG_CREATION_DATETIME parameter
 * \param key_size if non-zero and KM_TAG_KEY_SIZE not present in params, add it
 * \param rsa_pubexp if non-zero and KM_TAG_RSA_PUBLIC_EXPONENT not present in params, add it
 *
 * \post On error, no memory is allocated and *buf == NULL.
 *
 * \return KM_ERROR_OK or error
 */
keymaster_error_t km_serialize_params(
    scoped_buf_ptr_t& serialized_data,
    const keymaster_key_param_set_t *params,
    bool add_time,
    uint32_t key_size,
    uint64_t rsa_pubexp);

/**
 * Deserialize a parameter set.
 *
 * @param[out] param_set patameter set
 * @param[in,out] pos pointer to current position in buffer
 * @param[in,out] remain length of buffer remaining
 *
 * @return KM_ERROR_OK or error
 */
keymaster_error_t deserialize_param_set(
    keymaster_key_param_set_t *param_set,
    uint8_t **pos,
    uint32_t *remain);

/**
 * Deserialize key characteristics.
 *
 * This function allocates memory in \p characteristics, which the caller should
 * free using keymaster_free_characteristics().
 *
 * The expected format of the serialized characteristics is
 *
 * hw_enforced (params) | sw_enforced (params)
 *
 * where 'params' is serialized as for \p km_serialize_params().
 *
 * \param[out] characteristics deserialized characteristics
 * \param buffer serialized key characteristics
 * \param buffer_length length of buffer
 *
 * \return KM_ERROR_OK or error
 */
keymaster_error_t km_deserialize_characteristics(
    keymaster_key_characteristics_t *characteristics,
    const uint8_t *buffer,
    uint32_t buffer_length);

/**
 * Deserialize an attestation.
 *
 * The format of the buffer is
 *
 * n (uint32_t) | cert_1 (certbuf) | ... | cert_n (certbuf)
 *
 * where a 'certbuf' is
 *
 * K (uint32_t) | byte_1 (uint8_t) | ... | byte_K (uint8_t)
 * where K != 0
 *
 * @param[out] cert_chain deserialized attestation
 * @param buffer serialized attestation
 * @param buffer_length length of buffer (may be longer than the actual data)
 *
 * @return KM_ERROR_OK or error
 */
keymaster_error_t km_deserialize_attestation(
    keymaster_cert_chain_t *cert_chain,
    const uint8_t *buffer,
    uint32_t buffer_length);

#endif /* __SERIALIZATION_H__ */
