/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __MILAN_HV_BUS_H__
#define __MILAN_HV_BUS_H__

#include "gf_common.h"
#include "gf_error.h"

#include "milan_hv_common.h"
#include "milan_hv_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_milan_hv_write_cmd(milan_hv_cmd_t cmd);
gf_error_t gf_milan_hv_write_cmd_data(uint8_t cmd, uint8_t data);

gf_error_t gf_milan_hv_read_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len, uint32_t speed);
gf_error_t gf_milan_hv_write_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len);

gf_error_t gf_milan_hv_write_word_with_endian_exchange(uint16_t addr, uint16_t value);
gf_error_t gf_milan_hv_read_word_with_endian_exchange(uint16_t addr, uint16_t *value);

gf_error_t gf_milan_hv_read_bytes_with_endian_exchange(uint16_t addr, uint8_t *buf,
        uint32_t buf_len);

#ifdef __cplusplus
}
#endif

#endif  // __MILAN_HV_BUS_H__
