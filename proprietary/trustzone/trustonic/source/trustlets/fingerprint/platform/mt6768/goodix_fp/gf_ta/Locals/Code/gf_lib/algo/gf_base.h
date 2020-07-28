/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

/**
 * @file   gf_base.h
 *
 * <Copyright goes here>
 */

#ifndef __TA_GF_BASE_H__
#define __TA_GF_BASE_H__

#include "gf_error.h"
#include "gf_type_define.h"

gf_error_t gf_base_init_finished(uint8_t *reset_flag);
void gf_base_save_calibration(void);

gf_error_t gf_base_navigate(uint8_t *reset_flag);
void gf_base_stop_navigation(void);

gf_error_t gf_base_update_finger_base(uint16_t *buf, uint32_t buf_len, uint8_t *reset_flag);
gf_error_t gf_base_update_nav_base(uint16_t *buf, uint32_t buf_len, uint8_t *reset_flag);

gf_error_t gf_base_on_temperature_change_irq(uint8_t *reset_flag);
gf_error_t gf_base_on_fdt_reverse_irq(uint8_t *reset_flag);
gf_error_t gf_base_on_down_irq(void);
gf_error_t gf_base_on_up_irq(uint8_t *reset_flag);
gf_error_t gf_base_on_pre_enroll(uint8_t *reset_flag);

/**
 * for Milan-EFGL, FN check temperature change update base
 */
gf_error_t gf_base_check_finger_long_press(uint8_t *reset_flag);
gf_error_t gf_base_do_finger_long_press_check(uint16_t *buf, uint32_t buf_len, uint8_t *reset_flag);
gf_error_t gf_base_on_image_irq(uint16_t *buf, uint32_t buf_len, uint8_t *reset_flag);
gf_error_t gf_base_on_nav_irq(uint16_t *buf, uint32_t buf_len, uint8_t *reset_flag);

uint8_t gf_base_is_finger_base_vaild(void);
uint8_t gf_base_is_nav_base_vaild(void);


gf_error_t gf_base_press_check_before_get_image(void);
gf_error_t gf_base_reset_base_status(void);

#endif  /* __TA_GF_BASE_H__ */

