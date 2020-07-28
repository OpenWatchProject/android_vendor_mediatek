/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_IRQ_H__
#define __GF_IRQ_H__

#include "gf_common.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

void gf_irq_init_global_variable(void);

gf_error_t gf_irq(gf_irq_t *cmd);
gf_error_t gf_get_navigation_data(gf_irq_t *cmd);

#ifdef __cplusplus
}
#endif

#endif  // __GF_IRQ_H__
