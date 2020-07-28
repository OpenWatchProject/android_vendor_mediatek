/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_HAL_TEST_H__
#define __GF_HAL_TEST_H__

#include "gf_error.h"
#include "gf_fingerprint.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_hal_test_cmd(void* dev, uint32_t cmd_id, const uint8_t* param, uint32_t param_len);
gf_error_t gf_hal_test_cancel(void *dev);
gf_error_t gf_hal_test_prior_cancel(void *dev);

#ifdef __cplusplus
}
#endif

#endif  // __GF_HAL_TEST_H__
