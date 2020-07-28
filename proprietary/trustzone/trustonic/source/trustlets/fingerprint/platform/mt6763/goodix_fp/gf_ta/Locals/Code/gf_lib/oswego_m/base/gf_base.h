/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_BASE_H__
#define __GF_BASE_H__

#include "gf_error.h"
#include "gf_type_define.h"

void gf_base_init_global_variable(void);
gf_error_t gf_base_init_finished(uint8_t *reset_flag);
void gf_base_save_calibration(void);

gf_error_t gf_base_navigate(uint8_t *reset_flag);

gf_error_t gf_base_update_finger_base(uint16_t *buf, uint32_t buf_len, uint8_t *reset_flag);
gf_error_t gf_base_update_nav_base(uint16_t *buf, uint32_t buf_len, uint8_t *reset_flag);

gf_error_t gf_base_on_up_irq(uint8_t *reset_flag);
gf_error_t gf_base_on_pre_enroll(uint8_t *reset_flag);

uint8_t gf_base_is_finger_base_vaild(void);
uint8_t gf_base_is_nav_base_vaild(void);

#endif  // __GF_BASE_H__

