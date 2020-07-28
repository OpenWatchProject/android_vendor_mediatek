/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_TA_TEST_H__
#define __GF_TA_TEST_H__

#include "gf_error.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

void gf_ta_test_init_global_variable(void);

gf_error_t gf_ta_invoke_test_cmd_entry_point(uint32_t cmd_id, void *buffer, uint32_t buffer_len);

#ifdef __cplusplus
}
#endif

#endif  // __GF_TA_TEST_H__
