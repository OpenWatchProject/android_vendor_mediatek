/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_OSWEGO_M_BUS_H__
#define __GF_OSWEGO_M_BUS_H__

#include "gf_common.h"
#include "gf_error.h"

#define GF_SPI_COMPLETE_CHECK_RETRY_COUNT 1000

extern void gf_oswego_m_bus_init_global_variable(void);
extern gf_error_t gf_oswego_m_secspi_write_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_oswego_m_secspi_write_byte(uint16_t addr, uint8_t value);
extern gf_error_t gf_oswego_m_secspi_read_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_oswego_m_secspi_read_byte(uint16_t addr, uint8_t *value);
extern gf_error_t gf_oswego_m_secspi_read_bytes_high_speed(uint16_t addr, uint8_t *buf,
        uint32_t buf_len);
extern gf_error_t gf_oswego_m_secspi_read_flash(uint32_t flash_index, uint32_t offset,
        uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_oswego_m_secspi_write_flash(uint32_t flash_index, uint32_t offset,
        uint8_t *buf, uint32_t buf_len);
extern gf_error_t gf_oswego_m_secspi_read_flash_id(uint8_t *buf, uint32_t buf_len);
#endif  // __GF_OSWEGO_M_BUS_H__
