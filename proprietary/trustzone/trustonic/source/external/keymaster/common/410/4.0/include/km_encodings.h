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

#ifndef __KM_ENCODINGS_H__
#define __KM_ENCODINGS_H__

#include "keymaster_ta_defs.h"

/**
 * Encode public key data into X509 format.
 *
 * @param[out] export_data encoded public key data
 * @param key_type key type
 * @param key_size key size in bits
 * @param core_pub_data core public key data (metadata + raw data)
 * @param core_pub_data_len length of \p core_pub_data buffer
 *
 * @pre \p export_data points to a \p keymaster_blob_t
 * @post on success, caller assumes ownership of \p export_data->data
 * @post on error, no memory is allocated
 *
 * @return KM_ERROR_OK or error
 */
keymaster_error_t encode_x509(
    keymaster_blob_t *export_data,
    keymaster_algorithm_t key_type,
    uint32_t key_size,
    const uint8_t *core_pub_data,
    uint32_t core_pub_data_len);

#endif /* __KM_ENCODINGS_H__ */
