/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_SENSOR_H__
#define __GF_SENSOR_H__

#include <stdint.h>
#include "gf_error.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t row;
    uint32_t col;
    uint32_t nav_row;
    uint32_t nav_col;
    uint32_t nav_gap;
    uint32_t nav_finger_up_pixel_diff;
    uint32_t nav_finger_up_pixel_num;

    uint32_t raw_data_len;
    uint32_t raw_data_is_encrypt;

    int32_t flag_chip_info;
    int32_t thr_select_bmp;

    uint8_t support_esd_check;
    uint16_t pixel_open_threshold;
} gf_sensor_t;

extern gf_sensor_t g_sensor;

gf_error_t gf_sensor_info_init_by_chip_type(gf_chip_type_t chip_type);

gf_error_t gf_sensor_init(void);

#ifdef __cplusplus
}
#endif

#endif  // __GF_SENSOR_H__
