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

/**
 * Definition of the machine-specific integer types
 **/
#ifndef __TEE_TYPE_H__
#define __TEE_TYPE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifndef NULL
#  ifdef __cplusplus
#     define NULL  0
#  else
#     define NULL  ((void *)0)
#  endif
#endif

#define IN
#define OUT

/*
 * Definition of other common types
 */
typedef uint32_t TEE_Result;
typedef TEE_Result TEEC_Result;

/** Definition of an UUID (from RFC 4122 http://www.ietf.org/rfc/rfc4122.txt) */
typedef struct {
    uint32_t timeLow;
    uint16_t timeMid;
    uint16_t timeHiAndVersion;
    uint8_t clockSeqAndNode[8];
} TEE_UUID;
typedef TEE_UUID TEEC_UUID;

/** Type definition for a TEE Identity */
typedef struct {
    uint32_t login;
    TEE_UUID uuid;
} TEE_Identity;

typedef struct __TEE_PropSetHandle {
    unsigned char reserved;
} __TEE_PropSetHandle;

typedef struct __TEE_PropSetHandle* TEE_PropSetHandle;

#if TBASE_API_LEVEL >= 7
typedef struct __TEE_TASessionHandle* TEE_TASessionHandle;
#endif

/* Property Sets Pseudo Handles */
#define TEE_PROPSET_CURRENT_TA               (TEE_PropSetHandle)0xFFFFFFFF
#define TEE_PROPSET_CURRENT_CLIENT           (TEE_PropSetHandle)0xFFFFFFFE
#define TEE_PROPSET_TEE_IMPLEMENTATION       (TEE_PropSetHandle)0xFFFFFFFD

#if TBASE_API_LEVEL >= 7
/* Relative to GP Time library */
typedef struct TEE_Time {
    uint32_t seconds;
    uint32_t millis;
} TEE_Time;
#endif

#if TBASE_API_LEVEL >= 7
/*
 * Data types for TEE Arithmetical API (TEE core API 1.1. section 8.3)
 */

/** Big-integer representation. */
typedef uint32_t TEE_BigInt;

/** Context for fast modular multiplication operations. */
typedef uint32_t TEE_BigIntFMMContext;

/** Big-integer representation in a fast-modular-multiplication context. */
typedef uint32_t TEE_BigIntFMM;
#endif

#if defined(_MSC_VER)
#define __func__ __FUNCTION__
#endif

#endif /* __TEE_TYPE_H__ */
