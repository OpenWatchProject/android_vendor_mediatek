/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_CONFIG_H__
#define __GF_CONFIG_H__

#include <stdint.h>
#include "gf_error.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OSWEGO_M_VENDOR_CFG_COUNT_MAX  6
#define OSWEGO_M_PRODUCT_CFG_COUNT_MAX 3

/* fw and cfg binary length for oswego_m and oswego_s */
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

typedef struct {
    uint8_t *fw_data;
    uint32_t fw_data_len;

    uint8_t *cfg_data_eco_normal;
    uint8_t *cfg_data_eco_ff;
    uint8_t *cfg_data_mp_normal;
    uint8_t *cfg_data_mp_ff;
    uint32_t cfg_data_len;
} gf_oswego_s_fw_cfg_t;

typedef struct {
    uint8_t *fw_data;
    uint32_t fw_data_len;
    uint8_t *cfg_data;
    uint32_t cfg_data_len;
    uint8_t sensor_otp_type;
    uint16_t otp_version;
    uint16_t hbd_version;
    uint16_t hbd_otp_base;
} gf_milan_a_series_fw_cfg_t;

typedef union {
    gf_oswego_m_fw_cfg_t oswego_m;
    gf_oswego_s_fw_cfg_t oswego_s;
    gf_milan_a_series_fw_cfg_t milan_a_series;
} gf_fw_cfg_t;

gf_error_t gf_config_init(gf_config_t *config, gf_fw_cfg_t *fw_cfg);

#ifdef __cplusplus
}
#endif

#endif // __GF_CONFIG_H__
