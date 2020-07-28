/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#ifndef __TEST_KM_UTIL_H__
#define __TEST_KM_UTIL_H__

#include <keymaster_ta_defs.h>
#include <assert.h>

#undef  ANDROID
#include "log.h"

#define BYTES_PER_BITS(nbits) (((nbits) + 7) / 8)

#define CHECK_RESULT(result, expr) \
    do { \
        keymaster_error_t res1 = (expr); \
        if (res1 != result) { \
            LOG_E("%s == %d, expected %d", #expr, res1, result); \
            res = KM_ERROR_UNKNOWN_ERROR; \
            goto end; \
        } \
    } while (0)

#define FREE(ptr) \
    do { \
        free(ptr); \
        ptr = NULL; \
    } while (0)

#define CHECK_RESULT_OK(expr) \
    do { \
        res = (expr); \
        if (res != KM_ERROR_OK) { \
            LOG_E("%s == %d\n", #expr, res); \
            goto end; \
        } \
    } while (0)

#define CHECK_TRUE(expr) \
    do { \
        if (!(expr)) { \
            LOG_E("'%s' is false", #expr); \
            res = KM_ERROR_UNKNOWN_ERROR; \
            goto end; \
        } \
    } while (false)

#define N_ARRAY_ELTS(array) (sizeof(array)/sizeof(*array))
#define SETPARAM(param_, i_, u_, tag_, value_) do { \
    assert((i_) < N_ARRAY_ELTS(param_)); \
    (param_)[(i_)].tag = (tag_); \
    (param_)[(i_)].u_ = (value_); \
    (i_)++; \
} while (0)

/**
 * Block length corresponding to hash.
 *
 * @param digest digest algorithm
 * @return hash output size in bits
 * @retval 0 invalid \p digest
 */
uint32_t block_length(
    keymaster_digest_t digest);

void km_free_blob(
    keymaster_blob_t *blob);

void km_free_key_blob(
    keymaster_key_blob_t *key_blob);

keymaster_error_t save_key_blob(
    const char *name, const keymaster_key_blob_t *blob);

struct sgentry {
    const void *p;
    size_t sz;
};

bool equal_sglists(const struct sgentry *a, size_t na,
    const struct sgentry *b, size_t nb);

void add_to_sglist(struct sgentry *sg, size_t *n, size_t max,
    const void *p, size_t sz);

#define ADD_TO_SGLIST(sg, p, sz) \
    add_to_sglist(sg, &n_##sg, N_ARRAY_ELTS((sg)), (p), (sz))

#define ADD_BLOB_TO_SGLIST(sg, blob) \
    ADD_TO_SGLIST(sg, (blob).data, (blob).data_length)

#define DECL_SGLIST(sg, n) \
    struct sgentry sg[n]; \
    size_t n_##sg = 0;

#endif /* __TEST_KM_UTIL_H__ */
