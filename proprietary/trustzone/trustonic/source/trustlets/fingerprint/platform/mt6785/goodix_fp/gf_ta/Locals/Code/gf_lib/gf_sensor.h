/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_SENSOR_H__
#define __GF_SENSOR_H__

#include <stdint.h>
#include <gf_config.h>
#include <gf_common.h>
#include "gf_algo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gf_config_t config;
    gf_fw_cfg_t fw_cfg;

    gf_chip_type_t chip_type;
    uint32_t row;
    uint32_t col;
    uint32_t nav_row;
    uint32_t nav_col;
    uint32_t nav_gap;
    uint32_t nav_frame_len;

    uint32_t line_len;
    uint32_t line_aligned_len;
    uint32_t sensor_raw_data_len;
    uint32_t raw_data_len;
    uint32_t raw_data_is_encrypt;

    int32_t flag_chip_info;
    int32_t thr_select_bmp;

    uint8_t support_esd_check;

} gf_sensor_t;

extern gf_sensor_t g_sensor;

gf_error_t gf_sensor_info_init(void);

gf_error_t gf_sensor_init(void);

#ifdef __cplusplus
}
#endif

#endif // __GF_SENSOR_H__
