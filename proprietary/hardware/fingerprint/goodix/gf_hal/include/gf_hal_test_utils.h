/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_HAL_TEST_UTILS_H__
#define __GF_HAL_TEST_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define HAL_TEST_SIZEOF_INT64         (sizeof(uint32_t) + sizeof(int64_t))
#define HAL_TEST_SIZEOF_INT32         (sizeof(uint32_t) + sizeof(int32_t))
#define HAL_TEST_SIZEOF_INT16         (sizeof(uint32_t) + sizeof(int16_t))
#define HAL_TEST_SIZEOF_INT8          (sizeof(uint32_t) + sizeof(uint8_t))
#define HAL_TEST_SIZEOF_ARRAY(len)    (sizeof(uint32_t) + sizeof(int32_t) + (len))
#define HAL_TEST_SIZEOF_FLOAT         (sizeof(uint32_t) + sizeof(int32_t) + sizeof(float))
#define HAL_TEST_SIZEOF_DOUBLE        (sizeof(uint32_t) + sizeof(int32_t) + sizeof(double))

uint8_t *hal_test_encode_int8(uint8_t *buf, uint32_t token, int8_t value);
uint8_t *hal_test_encode_int16(uint8_t *buf, uint32_t token, int16_t value);
uint8_t *hal_test_encode_int32(uint8_t *buf, uint32_t token, int32_t value);
uint8_t *hal_test_encode_int64(uint8_t *buf, uint32_t token, int64_t value);
uint8_t *hal_test_encode_array(uint8_t *buf, uint32_t token, uint8_t *array, uint32_t size);
uint8_t *hal_test_encode_float(uint8_t *buf, uint32_t token, float value);
uint8_t *hal_test_encode_double(uint8_t *buf, uint32_t token, double value);
const uint8_t *hal_test_decode_uint32(uint32_t *value, const uint8_t *buf);
void hal_notify_test_memory_check(const char *func_name, uint8_t *start, uint8_t *end,
        uint32_t len);

#ifdef __cplusplus
}
#endif

#endif  // __GF_HAL_TEST_UTILS_H__
