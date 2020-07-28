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

#ifndef __KM_UTIL_H__
#define __KM_UTIL_H__

#ifdef SOTER
#include "mtk_keymaster_defs.h"
#else
#include <hardware/keymaster_defs.h>
#endif
#include <memory>
#define LOG_TAG "TlcTeeKeyMaster"
#include "log.h"

template<typename T> struct scoped_data_ptr_t
{
    scoped_data_ptr_t()
    : size(0) {}

    scoped_data_ptr_t(const scoped_data_ptr_t<T>&) = delete;
    scoped_data_ptr_t<T>& operator=(const scoped_data_ptr_t<T>&) = delete;

    std::unique_ptr<T> buf;
    uint32_t           size;
};

typedef scoped_data_ptr_t<uint8_t[]> scoped_buf_ptr_t;

/**
 * Set \p ret to the return value of \p expr; if it is not \p KM_ERROR_OK then
 * log the error and go to \p end,
 */
#define CHECK_RESULT_OK(expr) \
    do { \
        ret = (expr); \
        if (ret != KM_ERROR_OK) { \
            LOG_E("%s:%d: %s == %d", __func__, __LINE__, #expr, ret); \
            goto end; \
        } \
    } while (false)

#define CHECK_TRUE(errcode, expr) \
    do { \
        if (!(expr)) { \
            LOG_E("%s:%d: '%s' is false\n", __func__, __LINE__, #expr); \
            ret = (errcode); \
            goto end; \
        } \
    } while (false)

#define CHECK_NOT_NULL(expr) \
    CHECK_TRUE(KM_ERROR_UNEXPECTED_NULL_POINTER, (expr) != NULL)

/**
 * Get value of enumerated tag from key parameters
 */
keymaster_error_t get_enumerated_tag(
    const keymaster_key_param_set_t *params,
    keymaster_tag_t tag,
    uint32_t *value);

/**
 * Get value of integer tag from key parameters
 */
keymaster_error_t get_integer_tag(
    const keymaster_key_param_set_t *params,
    keymaster_tag_t tag,
    uint32_t *value);

/**
 * Get value of long integer tag from key parameters
 */
keymaster_error_t get_long_integer_tag(
    const keymaster_key_param_set_t *params,
    keymaster_tag_t tag,
    uint64_t *value);

/**
 * Data copy to memory with scope lifetime.
 *
 * If either buf or size is equal to NULL/0 function
 * won't change state of data and it will return KM_ERROR_OK.
 */
keymaster_error_t copy_to_scoped_buf(const uint8_t *buf, uint32_t size, scoped_buf_ptr_t& data);

#endif /* __KM_UTIL_H__ */
