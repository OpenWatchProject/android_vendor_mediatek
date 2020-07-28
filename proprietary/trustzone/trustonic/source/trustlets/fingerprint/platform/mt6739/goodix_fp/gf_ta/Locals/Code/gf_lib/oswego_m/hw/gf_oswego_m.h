/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_OSWEGO_M_H__
#define __GF_OSWEGO_M_H__

#include "gf_error.h"

extern gf_error_t gf_oswego_m_fw_cfg_init(void);
extern gf_error_t gf_oswego_m_chip_init(uint8_t *download_fw_flag, uint8_t *download_cfg_flag);

extern gf_error_t gf_oswego_m_get_fw_version(uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_oswego_m_get_production_date(uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_oswego_m_write_product_info(uint8_t product_cfg_idx);

extern gf_error_t gf_oswego_m_esd_check(uint8_t *result);

extern gf_error_t gf_oswego_m_pre_get_image(void);
extern gf_error_t gf_oswego_m_get_image(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_oswego_m_post_get_image(uint16_t *raw_data, uint32_t *raw_data_len);

extern gf_error_t gf_oswego_m_pre_get_nav_image(void);
extern gf_error_t gf_oswego_m_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_oswego_m_post_get_nav_image(uint16_t *raw_data, uint32_t *raw_data_len,
        uint8_t *frame_num, uint8_t *over_flag);

extern gf_error_t gf_oswego_m_pre_get_nav_base(void);
extern gf_error_t gf_oswego_m_get_nav_base(uint8_t *origin_data, uint32_t *origin_data_len);
extern gf_error_t gf_oswego_m_post_get_nav_base(uint16_t *raw_data, uint32_t *raw_data_len);

extern gf_error_t gf_oswego_m_get_spi_speed(uint32_t irq_type, uint32_t *speed);

extern gf_error_t gf_oswego_m_download_fw(uint8_t *reset_flag);
extern gf_error_t gf_oswego_m_download_cfg(gf_config_type_t config_type);

extern gf_error_t gf_oswego_m_test_erase_fw(void);
extern gf_error_t gf_oswego_m_test_download_fw(uint8_t *fw_data, uint32_t fw_data_len);
extern gf_error_t gf_oswego_m_test_download_cfg(uint8_t *cfg_data, uint32_t cfg_data_len);

#endif  // __GF_OSWEGO_M_H__

