/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_HW_H__
#define __GF_HW_H__

#include "gf_error.h"
#include "gf_common.h"

gf_error_t gf_hw_chip_init(uint8_t *hal_otp_data, uint32_t *hal_otp_data_len, uint8_t *tee_otp_data,
        uint32_t *tee_otp_data_len);

gf_error_t gf_hw_get_image(uint8_t *origin_data, uint32_t *origin_data_len, uint16_t *raw_data,
        uint32_t *raw_data_len, uint8_t frame_num);

gf_error_t gf_hw_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len, uint16_t *raw_data,
        uint32_t *raw_data_len, uint8_t *frame_num, uint8_t *over_flag);

gf_error_t gf_hw_nav_complete(void);

gf_error_t gf_hw_get_nav_base_image(uint8_t *origin_data, uint32_t *origin_data_len,
        uint16_t *raw_data, uint32_t *raw_data_len);

gf_error_t gf_hw_enable_tx(void);
gf_error_t gf_hw_disable_tx(void);

gf_error_t gf_hw_check_temperature_change(uint32_t *is_temperature_change);

gf_error_t gf_hw_check_finger_touched(uint32_t *is_finger_touched);

gf_error_t gf_hw_update_fdt_base(uint16_t *buf, uint32_t buf_len);
gf_error_t gf_hw_get_fdt_base(uint16_t **buf, uint32_t *length);
gf_error_t gf_hw_get_fdt_threshold(uint16_t *threshold);
gf_error_t gf_hw_test_sensor_validity(void);

gf_error_t gf_hw_generate_image_irq_by_manual(void);
gf_error_t gf_hw_irq_preprocess(uint32_t *irq_type);
void gf_hw_clear_filed_down_base(void);

uint8_t gf_hw_is_enable_fdt_tx_strategy(void);
uint8_t gf_hw_is_enable_nav_tx_strategy(void);

gf_error_t gf_hw_get_nav_base_threshold(uint16_t *threshold);

void gf_hw_clear_saved_fdt_down_base(void);

gf_error_t gf_hw_switch_mode(uint32_t irq_type);

#endif /* __GF_HW_H__ */
