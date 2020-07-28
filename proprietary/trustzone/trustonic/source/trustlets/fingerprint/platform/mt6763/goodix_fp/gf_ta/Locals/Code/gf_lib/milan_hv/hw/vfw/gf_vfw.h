/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_VFW_H__
#define __GF_VFW_H__

#include "gf_error.h"
#include "gf_common.h"
#include "milan_hv_common.h"

gf_error_t gf_vfw_init(void);
gf_error_t gf_vfw_init_otp(uint8_t *hal_otp_data, uint32_t *hal_otp_data_len, uint8_t *tee_otp_data,
        uint32_t *tee_otp_data_len);
gf_error_t gf_vfw_chip_init(void);
void gf_vfw_chip_handle_init(void);

gf_error_t gf_vfw_get_vendor_id(uint16_t *vendor_id);

gf_error_t gf_vfw_enable_tx(void);
gf_error_t gf_vfw_disable_tx(void);

gf_error_t gf_vfw_set_mode(gf_mode_t mode);
gf_error_t gf_vfw_get_mode(gf_mode_t *value);

gf_error_t gf_vfw_get_image(uint8_t *origin_data, uint32_t *origin_data_len, uint8_t frame_num);

gf_error_t gf_vfw_get_nav_base_image(uint8_t *origin_data, uint32_t *origin_data_len);
gf_error_t gf_vfw_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len);

gf_error_t gf_vfw_get_irq_type(uint16_t *irq_type);
gf_error_t gf_vfw_clear_irq(uint16_t irq_type);

gf_error_t gf_vfw_data_sample_resume(void);
gf_error_t gf_vfw_data_sample_suspend(void);

gf_error_t gf_vfw_update_fdt_base(uint16_t *buf);
gf_error_t gf_vfw_read_fdt_data_with_trigger(uint16_t *fdt_data);

gf_error_t gf_vfw_enable_spi_bypass_deglitch(void);

gf_error_t gf_vfw_check_temperature_change(uint32_t *is_temperature_change);

gf_error_t gf_vfw_check_temperature_change_on_reverse_irq(uint32_t *is_temperature_change);

gf_error_t gf_vfw_check_temperature_change_on_up_irq(uint32_t *is_temperature_change);

gf_error_t gf_vfw_check_finger_touched(uint32_t *is_finger_touched);

gf_error_t gf_vfw_generate_image_irq_by_manual(void);

const char* gf_vfw_strstate(milan_hv_chip_state_t state);

gf_error_t gf_vfw_update_fdt_down_base_with_reverse(void);

gf_error_t gf_vfw_get_otp_data(uint8_t *otp_data);

gf_error_t gf_vfw_switch_mode(void);

gf_error_t gf_vfw_get_fdt_threshold(uint16_t *threshold);

uint32_t gf_vfw_get_chip_id(void);

uint8_t gf_vfw_get_product_id(void);

uint32_t gf_vfw_get_image_double_rate_flag(void);
uint32_t gf_vfw_get_nav_double_rate_flag(void);

milan_hv_scan_mode_t gf_vfw_get_scan_mode(void);

void gf_vfw_set_general_reg(void);

milan_hv_chip_state_t gf_vfw_get_cur_chip_state(void);

milan_hv_chip_state_t gf_vfw_get_cur_chip_mode(void);

gf_mode_t gf_vfw_get_cur_mode(void);

void gf_vfw_set_dac_h(uint16_t dac_h);

uint16_t gf_vfw_get_orientation(void);

uint16_t gf_vfw_get_cur_facing(void);

gf_error_t gf_vfw_get_otp_info(milan_hv_otp_info_t *info);

gf_error_t gf_vfw_test_sensor_validity(void);
void gf_vfw_clear_filed_down_base(void);

uint8_t gf_vfw_is_enable_fdt_tx_strategy(void);
uint8_t gf_vfw_is_enable_nav_tx_strategy(void);

gf_error_t gf_vfw_get_nav_base_threshold(uint16_t *threshold);

void gf_vfw_clear_saved_fdt_down_base(void);

#endif /* __GF_VFW_H__ */
