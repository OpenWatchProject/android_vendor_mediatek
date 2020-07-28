/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/



#ifndef __GF_MILAN_A_SERIES_OTP_H__
#define __GF_MILAN_A_SERIES_OTP_H__
#include "gf_error.h"
#include "gf_type_define.h"
#include "gf_config.h"

gf_error_t gf_milan_a_series_update_config_from_otp(uint8_t **config);
gf_error_t gf_milan_a_series_init_sensor_type(uint8_t* otp_buffer);
gf_sensor_type_t gf_milan_a_series_get_sensor_type(void);
gf_error_t gf_milan_a_series_read_chip_otp(uint8_t *otp_buf);
gf_error_t gf_milan_a_series_otp_check_crc(uint8_t *otp_buffer, uint8_t* crc_buf);
void milan_a_series_dac_otp_update(uint8_t *otp_buffer, uint8_t *cfg_data, uint8_t type);
void milan_a_series_hbd_otp_update(uint8_t *otp_buffer, uint8_t *cfg_data,
        gf_milan_a_series_fw_cfg_t *fw_cfg);
void milan_a_series_temperature_otp_update(uint8_t *otp_buffer, uint8_t *config_buffer);
void milan_a_series_clock_otp_update(uint8_t *otp_buffer, uint8_t *cfg_data);
uint16_t milan_a_series_otp_get_checksum16(const uint8_t * usBuf, const uint16_t usLen);
gf_error_t gf_milan_a_series_vcm_cal(uint8_t *otp);
gf_error_t gf_milan_a_series_otp_get_sensor_id(uint8_t *buf, uint32_t buf_len);
gf_error_t gf_milan_a_series_otp_get_vendor_id(uint8_t *buf, uint32_t buf_len);


#endif //__GF_MILAN_A_SERIES_OTP_H__
