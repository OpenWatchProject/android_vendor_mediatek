/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_TA_CHIP_SERIES_H__
#define __GF_TA_CHIP_SERIES_H__

#include "gf_common.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

void gf_ta_chip_series_init_global_variable(void);
void gf_ta_chip_series_cancel_operation(gf_operation_type_t cur_operation);
gf_error_t gf_ta_chip_series_invoke_cmd_entry_point(uint32_t cmd_id, void *buffer);

#ifdef __cplusplus
}
#endif

#endif  // __GF_TA_CHIP_SERIES_H__
