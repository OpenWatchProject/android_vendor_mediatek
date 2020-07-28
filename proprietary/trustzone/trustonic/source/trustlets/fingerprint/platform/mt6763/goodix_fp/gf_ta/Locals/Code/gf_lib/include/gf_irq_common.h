/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_IRQ_COMMON_H__
#define __GF_IRQ_COMMON_H__

#include <stdint.h>

#include "gf_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPDATE_BASE_THRESHOLD   20

void gf_irq_common_reset(uint32_t irq_type);
gf_error_t irq_test_data_noise(gf_irq_t *irq);

#ifdef __cplusplus
}
#endif

#endif  // __GF_IRQ_COMMON_H__
