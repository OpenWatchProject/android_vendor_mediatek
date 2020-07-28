/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_FW_CFG_H__
#define __GF_FW_CFG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OSWEGO_M_VENDOR_CFG_COUNT_MAX  6
#define OSWEGO_M_PRODUCT_CFG_COUNT_MAX 3

/* fw and cfg binary length for oswego_m */
#define GF_OSWEGO_M_FW_LENGTH (4+8+2+(32+4+2+4)*1024)   /* 43022 bytes */
#define GF_OSWEGO_M_CFG_LENGTH 249

typedef struct {
    uint16_t orientation;
    uint16_t facing;
    uint8_t data[GF_OSWEGO_M_CFG_LENGTH];
} gf_oswego_m_cfg_data_t;

typedef struct {
    uint8_t vendor_id;
    gf_oswego_m_cfg_data_t cfg;
} gf_oswego_m_cfg_by_vendor_id_t;

typedef struct {
    uint8_t product_cfg_idx;
    gf_oswego_m_cfg_data_t cfg;
} gf_oswego_m_cfg_by_product_idx_t;

typedef struct {
    gf_oswego_m_cfg_by_vendor_id_t   *cfg_data_by_vendor_id[OSWEGO_M_VENDOR_CFG_COUNT_MAX];
    gf_oswego_m_cfg_by_product_idx_t *cfg_data_by_product_idx[OSWEGO_M_PRODUCT_CFG_COUNT_MAX];

    uint8_t *cfg_data_pixel_test;
    uint32_t cfg_data_len;

    uint8_t *fw_data;
    uint32_t fw_data_len;
} gf_oswego_m_fw_cfg_t;

extern gf_oswego_m_fw_cfg_t g_oswego_m_fw_cfg;

void gf_fw_cfg_init(void);

#ifdef __cplusplus
}
#endif

#endif  // __GF_FW_CFG_H__

