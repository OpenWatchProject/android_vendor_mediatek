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
#ifdef SOTER
#include "mtk_keymaster_defs.h"
#else
#include <hardware/keymaster_defs.h>
#endif
#include "km_util.h"

keymaster_error_t get_enumerated_tag(
    const keymaster_key_param_set_t *params,
    keymaster_tag_t tag,
    uint32_t *value)
{
    if ( (params == NULL) || (value == NULL) ) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }
    for (size_t i = 0; i < params->length; i++) {
        if (params->params[i].tag == tag) {
            *value = params->params[i].enumerated;
            return KM_ERROR_OK;
        }
    }
    return KM_ERROR_INVALID_TAG;
}

keymaster_error_t get_integer_tag(
    const keymaster_key_param_set_t *params,
    keymaster_tag_t tag,
    uint32_t *value)
{
    if ( (params == NULL) || (value == NULL) ) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }
    for (size_t i = 0; i < params->length; i++) {
        if (params->params[i].tag == tag) {
            *value = params->params[i].integer;
            return KM_ERROR_OK;
        }
    }
    return KM_ERROR_INVALID_TAG;
}

keymaster_error_t get_long_integer_tag(
    const keymaster_key_param_set_t *params,
    keymaster_tag_t tag,
    uint64_t *value)
{
    if ( (params == NULL) || (value == NULL) ) {
        return KM_ERROR_UNEXPECTED_NULL_POINTER;
    }
    for (size_t i = 0; i < params->length; i++) {
        if (params->params[i].tag == tag) {
            *value = params->params[i].long_integer;
            return KM_ERROR_OK;
        }
    }
    return KM_ERROR_INVALID_TAG;
}

keymaster_error_t copy_to_scoped_buf(const uint8_t *buf, uint32_t size, scoped_buf_ptr_t& data)
{
    if(buf+size < buf)
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;

    if( (buf!=NULL) && (size!=0) ) {
        data.buf.reset(new (std::nothrow) uint8_t[size]);
        if(data.buf) {
            memcpy(data.buf.get(), buf, size);
            data.size = size;
            return KM_ERROR_OK;
        }
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }
    return KM_ERROR_OK;
}
