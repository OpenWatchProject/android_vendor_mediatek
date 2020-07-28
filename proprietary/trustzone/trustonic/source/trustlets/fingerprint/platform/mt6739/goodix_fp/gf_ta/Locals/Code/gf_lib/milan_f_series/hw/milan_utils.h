/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __MILAN_UTILS_H__
#define __MILAN_UTILS_H__

#include "milan_common.h"

gf_error_t milan_f_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
gf_error_t milan_e_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
gf_error_t milan_g_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
gf_error_t milan_l_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
gf_error_t milan_fn_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
gf_error_t milan_k_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
gf_error_t milan_j_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
gf_error_t milan_h_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
#endif  // __MILAN_UTILS_H__
