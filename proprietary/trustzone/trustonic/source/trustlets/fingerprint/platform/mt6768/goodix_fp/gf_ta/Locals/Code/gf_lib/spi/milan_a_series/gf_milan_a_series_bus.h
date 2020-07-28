/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#ifndef __GF_MILAN_A_SERIES_BUS_H__
#define __GF_MILAN_A_SERIES_BUS_H__

#include "gf_common.h"
#include "gf_error.h"
gf_error_t gf_milan_a_series_secspi_write_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len);
gf_error_t gf_milan_a_series_secspi_write_byte(uint16_t addr, uint8_t value);
gf_error_t gf_milan_a_series_secspi_read_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len);
gf_error_t gf_milan_a_series_secspi_read_byte(uint16_t addr, uint8_t *value);
gf_error_t gf_milan_a_series_secspi_read_bytes_high_speed(uint16_t addr, uint8_t *buf,
        uint32_t buf_len);
gf_error_t gf_milan_a_series_secspi_read_flash(uint32_t flash_index, uint32_t offset, uint8_t *buf,
        uint32_t buf_len);
gf_error_t gf_milan_a_series_secspi_write_flash(uint32_t flash_index, uint32_t offset,
        uint8_t *buf, uint32_t buf_len);
gf_error_t gf_milan_a_series_secspi_read_word(uint16_t addr, uint16_t *value);
gf_error_t gf_milan_a_series_secspi_write_word(uint16_t addr, uint16_t value);
gf_error_t gf_milan_a_series_secspi_read_bytes_with_speed(uint16_t addr, uint8_t *buf, uint32_t buf_len, uint32_t speed) ;


#endif //__GF_MILAN_A_SERIES_BUS_H__
