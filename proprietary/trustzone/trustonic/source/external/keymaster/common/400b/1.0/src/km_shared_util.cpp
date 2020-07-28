/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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
#include "hardware/keymaster_defs.h"
#endif
#include "km_shared_util.h"

uint32_t get_u32(const uint8_t *pos) {
    uint32_t a = 0;
    for (int i = 0; i < 4; i++) {
        a |= ((uint32_t)pos[i] << (8*i));
    }
    return a;
}
uint64_t get_u64(const uint8_t *pos) {
    uint64_t a = 0;
    for (int i = 0; i < 8; i++) {
        a |= ((uint64_t)pos[i] << (8*i));
    }
    return a;
}
void set_u32(uint8_t *pos, uint32_t val) {
    for (int i = 0; i < 4; i++) {
        pos[i] = (val >> (8*i)) & 0xFF;
    }
}
void set_u64(uint8_t *pos, uint64_t val) {
    for (int i = 0; i < 8; i++) {
        pos[i] = (val >> (8*i)) & 0xFF;
    }
}
void set_u32_increment_pos(uint8_t **pos, uint32_t val) {
    set_u32(*pos, val);
    *pos += 4;
}
void set_u64_increment_pos(uint8_t **pos, uint64_t val) {
    set_u64(*pos, val);
    *pos += 8;
}
void set_data_increment_pos(uint8_t **pos, const uint8_t *src, uint32_t len) {
    memcpy(*pos, src, len);
    *pos += len;
}
void set_data_increment_src(uint8_t *dest, uint8_t **src, uint32_t len) {
    memcpy(dest, *src, len);
    *src += len;
}
void set_ptr_increment_src(uint8_t **ptr, uint8_t **src, uint32_t len) {
    *ptr = *src;
    *src += len;
}

bool check_algorithm_purpose(
    keymaster_algorithm_t algorithm,
    keymaster_purpose_t purpose)
{
    switch (algorithm) {
        case KM_ALGORITHM_AES:
            return ((purpose == KM_PURPOSE_ENCRYPT) ||
                    (purpose == KM_PURPOSE_DECRYPT));
        case KM_ALGORITHM_HMAC:
        case KM_ALGORITHM_EC:
            return ((purpose == KM_PURPOSE_SIGN) ||
                    (purpose == KM_PURPOSE_VERIFY));
        case KM_ALGORITHM_RSA:
            return ((purpose == KM_PURPOSE_ENCRYPT) ||
                    (purpose == KM_PURPOSE_DECRYPT) ||
                    (purpose == KM_PURPOSE_SIGN) ||
                    (purpose == KM_PURPOSE_VERIFY));
        default:
            return false;
    }
}
