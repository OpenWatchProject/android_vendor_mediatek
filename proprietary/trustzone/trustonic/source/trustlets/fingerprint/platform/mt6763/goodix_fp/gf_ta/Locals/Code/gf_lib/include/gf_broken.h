/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_BROKEN_H__
#define __GF_BROKEN_H__

#include "gf_common.h"
#include "gf_error.h"

/**
 * [gf_broken_init_global_variable]:Initialize the global variable in this file
 */
void gf_broken_init_global_variable(void);

gf_error_t gf_broken_check_init(void);
gf_error_t gf_broken_check_on_image_irq(uint16_t *raw_data, uint32_t raw_len, gf_irq_t *cmd,
        gf_operation_type_t operation);
gf_error_t gf_broken_check_on_up_irq(uint16_t *raw_data, uint32_t raw_len, gf_irq_t *cmd);
gf_error_t gf_broken_check(gf_sensor_broken_check_t* broken_check, uint8_t *reset_flag,
        gf_operation_type_t operation);
gf_error_t gf_broken_check_destroy(void);

#endif  // __GF_BROKEN_H__

