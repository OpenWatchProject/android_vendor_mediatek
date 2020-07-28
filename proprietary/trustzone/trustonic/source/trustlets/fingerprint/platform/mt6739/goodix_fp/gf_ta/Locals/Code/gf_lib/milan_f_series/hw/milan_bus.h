/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __MILAN_BUS_H__
#define __MILAN_BUS_H__

#include "gf_common.h"
#include "gf_error.h"

#include "milan_common.h"
#include "milan_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

void gf_milan_bus_init_global_variable(void);

gf_error_t gf_milan_reg_access(milan_reg_access_t *reg_data);

gf_error_t gf_milan_secspi_write_cmd(milan_cmd_t cmd);

gf_error_t gf_milan_secspi_write_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len);

gf_error_t gf_milan_secspi_write_word(uint16_t addr, uint16_t value);

gf_error_t gf_milan_secspi_read_bytes(uint16_t addr, uint8_t *buf, uint32_t buf_len);

gf_error_t gf_milan_secspi_read_word(uint16_t addr, uint16_t *value);

gf_error_t gf_milan_secspi_read_bytes_by_speed(uint16_t addr, uint8_t *buf,
        uint32_t buf_len, uint32_t speed);

#define MILAN_READ_REGISTER(addr, value_ptr)                                            \
    {                                                                                   \
        milan_reg_access_t reg;                                                         \
        MILAN_MK_REG_READ(reg, addr, value_ptr);                                        \
        err = gf_milan_reg_access(&reg);                                                \
        if (err != GF_SUCCESS) {                                                        \
            GF_LOGE(LOG_TAG "[%s], read register(%04x) error:%d", __func__, addr, err); \
            break;                                                                      \
        }                                                                               \
    }

#define MILAN_WRITE_REGISTER(addr, value_ptr)                                           \
    {                                                                                   \
        milan_reg_access_t reg;                                                         \
        MILAN_MK_REG_WRITE(reg, addr, value_ptr);                                       \
        err = gf_milan_reg_access(&reg);                                                \
        if (err != GF_SUCCESS) {                                                        \
            GF_LOGE(LOG_TAG "[%s] write register(%04x) error:%d", __func__, addr, err); \
            break;                                                                      \
        }                                                                               \
    }



#ifdef __cplusplus
}
#endif

#endif  // __MILAN_BUS_H__
