/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_HW_COMMON_H__
#define __GF_HW_COMMON_H__

#include "gf_error.h"
#include "gf_common.h"

extern void gf_hw_endian_exchange(uint8_t *buf, int32_t len);
extern void gf_hw_init_global_variable(void);

extern gf_error_t gf_hw_chip_config_init(void);
extern void gf_hw_destroy(void);

extern gf_error_t gf_hw_enable_encrypt(uint32_t flag);
extern gf_error_t gf_hw_get_chip_id(uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_hw_get_vendor_id(uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_hw_get_sensor_id(uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_hw_get_otp_data(uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_hw_get_orientation(uint16_t *orientation, uint16_t *facing);

extern gf_error_t gf_hw_detect_sensor(void);

extern gf_error_t gf_hw_set_mode(gf_mode_t mode);
extern gf_error_t gf_hw_get_mode(gf_mode_t *mode);

extern gf_error_t gf_hw_data_sample_suspend(void);
extern gf_error_t gf_hw_data_sample_resume(void);

extern gf_error_t gf_hw_get_irq_type(uint32_t *irq_type);
extern gf_error_t gf_hw_clear_irq(uint32_t irq_type);

extern gf_error_t gf_hw_read_register(uint32_t addr, uint8_t* content, uint32_t read_len);
extern gf_error_t gf_hw_write_register(uint32_t addr, uint8_t* content, uint32_t write_len);

#endif  // __GF_HW_COMMON_H__

