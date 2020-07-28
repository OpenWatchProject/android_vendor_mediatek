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

#include "km_shared_util.h"

keymaster_error_t km_alloc(
    uint8_t **a,
    uint32_t l)
{
    *a = (uint8_t*)KM_ALLOC(l);
    if (!*a) {
        return KM_ERROR_MEMORY_ALLOCATION_FAILED;
    }
    memset(*a, 0, l);
    return KM_ERROR_OK;
}

void km_free(
    uint8_t *a)
{
    KM_FREE(a);
}

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
        case KM_ALGORITHM_TRIPLE_DES:
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

/* Read a length from position *p, with *len remaining, return the result in *l
 * and update *p and *len. Return true on success, false on failure or if length
 * exceeds 65535 or if length exceeds length of data remaining afterwards.
 */
static bool read_length(
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    uint8_t a;
    if (*len == 0) return false;
    a = (*p)[0]; *p += 1; *len -= 1;
    if (a < 0x80) {
        *l = a;
    } else if (a == 0x81) {
        if (*len == 0) return false;
        *l = (*p)[0]; *p += 1; *len -= 1;
    } else if (a == 0x82) {
        if (*len < 2) return false;
        *l = 256*(*p)[0] + (*p)[1]; *p += 2; *len -= 2;
    } else return false;
    return (*l <= *len);
}

static bool read_something_length(
    uint8_t tag,
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    if (*len == 0 || (*p)[0] != tag) return false;
    *p += 1; *len -= 1;
    return read_length(l, p, len);
}

bool der_read_integer_length(
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    if (!read_something_length(0x02, l, p, len)) {
        return false;
    }
    if (*l >= 1 && (*p)[0] == 0x00) {
        *p += 1; *len -= 1; *l -= 1;
    }
    return true;
}
bool der_read_bit_string_length(
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    return read_something_length(0x03, l, p, len);
}
bool der_read_octet_string_length(
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    return read_something_length(0x04, l, p, len);
}
bool der_read_oid_length(
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    return read_something_length(0x06, l, p, len);
}
bool der_read_sequence_length(
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    return read_something_length(0x30, l, p, len);
}

bool der_read_explicit_tag_and_length(
    int *tag,
    size_t *l,
    const uint8_t **p,
    size_t *len)
{
    uint8_t a = 0;
    if (*len == 0) return false;
    a = **p; (*p)++; (*len)--;
    if (a < 0xA0) return false;
    if (a < 0xBF) {
        *tag = a - 0xA0;
    } else {
        if (a != 0xBF || *len == 0) return false;
        a = **p; (*p)++; (*len)--;
        if (a < 128) {
            if (a < 31) return false;
            *tag = a;
        } else {
            *tag = 128 * (a - 128);
            if (*len == 0) return false;
            a = **p; (*p)++; (*len)--;
            *tag += a;
        }
    }
    return read_length(l, p, len);
}

#define CHK(a) CHECK_TRUE(KM_ERROR_INVALID_ARGUMENT, a)

/* OID 1.2.840.10045.2.1 */
static const uint8_t eckey_oid[] = {
    0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01};

/* OID 1.2.840.113549.1.1.1 */
static const uint8_t rsakey_oid[] = {
    0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01};

/* OID 1.3.132.0.33 */
static const uint8_t p224_oid[] = {
    0x2b, 0x81, 0x04, 0x00, 0x21};
/* OID 1.2.840.10045.3.1.7 */
static const uint8_t p256_oid[] = {
    0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07};
/* OID 1.3.132.0.34 */
static const uint8_t p384_oid[] = {
    0x2b, 0x81, 0x04, 0x00, 0x22};
/* OID 1.3.132.0.35 */
static const uint8_t p521_oid[] = {
    0x2b, 0x81, 0x04, 0x00, 0x23};

/**
 * Supported ECC curve types (from ectypes.h)
 */
typedef enum {
    ec_curve_nist_p192 = 1,
    ec_curve_nist_p224 = 2,
    ec_curve_nist_p256 = 3,
    ec_curve_nist_p384 = 4,
    ec_curve_nist_p521 = 5
} ec_curve_t;

static keymaster_error_t decode_pkcs8_ec(
    uint8_t **key_data,
    size_t *key_data_len,
    const uint8_t *data,
    size_t data_len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    size_t sublen, seq_len, oid_len, curve_oid_len, os_len, bs_len;
    const uint8_t *curve_oid;
    size_t curve_len_bits = 0, curve_len = 0;
    ec_curve_t curve;

    // SEQUENCE
    CHK(der_read_sequence_length(&seq_len, &data, &data_len)
        && seq_len == data_len);

    // INTEGER (0)
    CHK(data_len >= 3 && data[0] == 0x02 && data[1] == 0x01 && data[2] == 0x00);
    data += 3; data_len -= 3;

    // SEQUENCE
    CHK(der_read_sequence_length(&seq_len, &data, &data_len));
    // 1. ecPublicKey OID
    CHK(der_read_oid_length(&oid_len, &data, &data_len));
    CHK(memcmp(data, eckey_oid, oid_len) == 0);
    data += oid_len; data_len -= oid_len;
    // 2. curve OID
    CHK(der_read_oid_length(&curve_oid_len, &data, &data_len));
    curve_oid = data;
    if (curve_oid_len == sizeof(p224_oid) && memcmp(data, p224_oid, curve_oid_len) == 0) {
        curve = ec_curve_nist_p224;
        curve_len_bits = 224;
    } else if (curve_oid_len == sizeof(p256_oid) && memcmp(data, p256_oid, curve_oid_len) == 0) {
        curve = ec_curve_nist_p256;
        curve_len_bits = 256;
    } else if (curve_oid_len == sizeof(p384_oid) && memcmp(data, p384_oid, curve_oid_len) == 0) {
        curve = ec_curve_nist_p384;
        curve_len_bits = 384;
    } else if (curve_oid_len == sizeof(p521_oid) && memcmp(data, p521_oid, curve_oid_len) == 0) {
        curve = ec_curve_nist_p521;
        curve_len_bits = 521;
    } else {
        CHECK_RESULT_OK(KM_ERROR_INVALID_ARGUMENT);
    }
    curve_len = (curve_len_bits+7)/8;
    data += curve_oid_len; data_len -= curve_oid_len;

    // Now we know the curve, allocate the buffer and start filling it
    *key_data_len = 24 + 3*curve_len;
    CHECK_RESULT_OK(km_alloc(key_data, *key_data_len));
    set_u32(*key_data, KM_ALGORITHM_EC);
    set_u32(*key_data + 4, curve_len_bits);
    set_u32(*key_data + 8, curve);
    set_u32(*key_data + 12, curve_len);
    set_u32(*key_data + 16, curve_len);
    set_u32(*key_data + 20, curve_len);

    // OCTET STRING
    CHK(der_read_octet_string_length(&os_len, &data, &data_len)
        && os_len == data_len);
    // SEQUENCE
    CHK(der_read_sequence_length(&seq_len, &data, &data_len)
        && seq_len == data_len);
    // 1. INTEGER (1)
    CHK(data_len >= 3 && data[0] == 0x02 && data[1] == 0x01 && data[2] == 0x01);
    data += 3; data_len -= 3;
    // 2. OCTET STRING (private key)
    CHK(der_read_octet_string_length(&os_len, &data, &data_len)
        && os_len == curve_len);
    memcpy(*key_data + 24 + 2 * curve_len, data, curve_len); // private key
    data += curve_len; data_len -= curve_len;
    // 3. Optional Constructed [0] ...
    if (data_len >= 1 && data[0] == 0xa0) {
        data += 1; data_len -= 1;
        CHK(read_length(&sublen, &data, &data_len));
        // ... curve OID (again)
        CHK(der_read_oid_length(&oid_len, &data, &data_len));
        // Must agree with what we read earlier:
        CHK(oid_len == curve_oid_len && memcmp(data, curve_oid, oid_len) == 0);
        data += oid_len; data_len -= oid_len;
    }
    // 4. Optional Constructed [1] (actually we require this) ...
    CHK(data_len >= 1 && data[0] == 0xa1);
    data += 1; data_len -= 1;
    CHK(read_length(&sublen, &data, &data_len)
        && sublen == data_len);
    // ... BIT STRING
    CHK(der_read_bit_string_length(&bs_len, &data, &data_len)
        && bs_len == data_len);
    CHK(data_len >= 2 && data[0] == 0x00 && data[1] == 0x04); // uncompressed
    data += 2; data_len -= 2;
    CHK(data_len == 2 * curve_len);
    memcpy(*key_data + 24, data, curve_len); // x
    memcpy(*key_data + 24 + curve_len, data + curve_len, curve_len); // y

end:
    return ret;
}

static keymaster_error_t decode_pkcs8_rsa(
    uint8_t **key_data,
    size_t *key_data_len,
    const uint8_t *data,
    size_t data_len)
{
    keymaster_error_t ret = KM_ERROR_OK;
    size_t seq_len, oid_len, os_len;
    const uint8_t *n, *e, *d, *p, *q, *dp, *dq, *qinv;
    uint8_t *pos;
    size_t n_len, e_len, d_len, p_len, q_len, dp_len, dq_len, qinv_len;

    // SEQUENCE
    CHK(der_read_sequence_length(&seq_len, &data, &data_len)
        && seq_len == data_len);

    // INTEGER (0)
    CHK(data_len >= 3 && data[0] == 0x02 && data[1] == 0x01 && data[2] == 0x00);
    data += 3; data_len -= 3;

    // SEQUENCE
    CHK(der_read_sequence_length(&seq_len, &data, &data_len));
    // 1. rsaEncryption OID
    CHK(der_read_oid_length(&oid_len, &data, &data_len));
    CHK(memcmp(data, rsakey_oid, oid_len) == 0);
    data += oid_len; data_len -= oid_len;
    // 2. NULL
    CHK(data_len >= 2 && data[0] == 0x05 && data[1] == 0x00);
    data += 2; data_len -= 2;

    // OCTET STRING
    CHK(der_read_octet_string_length(&os_len, &data, &data_len)
        && os_len == data_len);
    // SEQUENCE
    CHK(der_read_sequence_length(&seq_len, &data, &data_len)
        && seq_len == data_len);
    // INTEGER (0)
    CHK(data_len >= 3 && data[0] == 0x02 && data[1] == 0x01 && data[2] == 0x00);
    data += 3; data_len -= 3;
    // INTEGER (n)
    CHK(der_read_integer_length(&n_len, &data, &data_len));
    n = data; data += n_len; data_len -= n_len;
    // INTEGER (e)
    CHK(der_read_integer_length(&e_len, &data, &data_len));
    e = data; data += e_len; data_len -= e_len;
    // INTEGER (d)
    CHK(der_read_integer_length(&d_len, &data, &data_len));
    d = data; data += d_len; data_len -= d_len;
    // INTEGER (p)
    CHK(der_read_integer_length(&p_len, &data, &data_len));
    p = data; data += p_len; data_len -= p_len;
    // INTEGER (e)
    CHK(der_read_integer_length(&q_len, &data, &data_len));
    q = data; data += q_len; data_len -= q_len;
    // INTEGER (dp)
    CHK(der_read_integer_length(&dp_len, &data, &data_len));
    dp = data; data += dp_len; data_len -= dp_len;
    // INTEGER (dq)
    CHK(der_read_integer_length(&dq_len, &data, &data_len));
    dq = data; data += dq_len; data_len -= dq_len;
    // INTEGER (qinv)
    CHK(der_read_integer_length(&qinv_len, &data, &data_len));
    qinv = data; data += qinv_len; data_len -= qinv_len;

    CHK(data_len == 0);

    // Allocate and fill the buffer
    *key_data_len = 44 +
        n_len + e_len + d_len + p_len + q_len + dp_len + dq_len + qinv_len;
    CHECK_RESULT_OK(km_alloc(key_data, *key_data_len));
    pos = *key_data;
    set_u32_increment_pos(&pos, KM_ALGORITHM_RSA);
    set_u32_increment_pos(&pos, 8 * n_len);
    set_u32_increment_pos(&pos, 8 * n_len);
    set_u32_increment_pos(&pos, n_len);
    set_u32_increment_pos(&pos, e_len);
    set_u32_increment_pos(&pos, d_len);
    set_u32_increment_pos(&pos, p_len);
    set_u32_increment_pos(&pos, q_len);
    set_u32_increment_pos(&pos, dp_len);
    set_u32_increment_pos(&pos, dq_len);
    set_u32_increment_pos(&pos, qinv_len);
    set_data_increment_pos(&pos, n, n_len);
    set_data_increment_pos(&pos, e, e_len);
    set_data_increment_pos(&pos, d, d_len);
    set_data_increment_pos(&pos, p, p_len);
    set_data_increment_pos(&pos, q, q_len);
    set_data_increment_pos(&pos, dp, dp_len);
    set_data_increment_pos(&pos, dq, dq_len);
    set_data_increment_pos(&pos, qinv, qinv_len);

end:
    return ret;
}

keymaster_error_t decode_key_data(
    uint8_t **key_data,
    size_t *key_data_len,
    keymaster_key_format_t key_format,
    keymaster_algorithm_t key_type,
    const uint8_t *data,
    size_t data_len)
{
    keymaster_error_t ret = KM_ERROR_OK;

    switch (key_format) {
        case KM_KEY_FORMAT_PKCS8:
            if (key_type == KM_ALGORITHM_EC) {
                CHECK_RESULT_OK(decode_pkcs8_ec(key_data, key_data_len, data, data_len));
            } else if (key_type == KM_ALGORITHM_RSA) {
                CHECK_RESULT_OK(decode_pkcs8_rsa(key_data, key_data_len, data, data_len));
            }
            else {
                CHECK_RESULT_OK(KM_ERROR_INCOMPATIBLE_ALGORITHM);
            }
            break;
        case KM_KEY_FORMAT_RAW:
            /* data contains the raw data for a symmetric key */
            CHECK_TRUE(KM_ERROR_INCOMPATIBLE_ALGORITHM,
                key_type == KM_ALGORITHM_AES ||
                key_type == KM_ALGORITHM_TRIPLE_DES ||
                key_type == KM_ALGORITHM_HMAC);
            *key_data_len = 4 + 4 + data_len;
            CHECK_RESULT_OK(km_alloc(key_data, *key_data_len));
            set_u32(*key_data, key_type);
            set_u32(*key_data + 4, 8 * data_len);
            memcpy(*key_data + 8, data, data_len);
            break;
        default:
            CHECK_RESULT_OK(KM_ERROR_UNSUPPORTED_KEY_FORMAT);
    }

end:
    return ret;
}

uint32_t ec_bitlen(
    keymaster_ec_curve_t curve)
{
    switch (curve) {
        case KM_EC_CURVE_P_224:
            return 224;
        case KM_EC_CURVE_P_256:
            return 256;
        case KM_EC_CURVE_P_384:
            return 384;
        case KM_EC_CURVE_P_521:
            return 521;
        default:
            return 0;
    }
}
