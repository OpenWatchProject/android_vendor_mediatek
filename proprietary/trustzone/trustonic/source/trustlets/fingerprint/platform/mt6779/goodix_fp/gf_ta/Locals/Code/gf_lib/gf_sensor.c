/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <stdlib.h>
#include "gf_tee_internal_api.h"
#include "gf_sensor.h"
#include "gf_error.h"
#include "gf_common.h"
#include "gf_status.h"
#include "cpl_string.h"
#include "gf_algo.h"

#define LOG_TAG "[gf_sensor] "

gf_sensor_t g_sensor;

/*
 * this function init chip specific params,
 * NOTE: nav col, nav step, it maybe override by chip config
 */
gf_error_t gf_sensor_info_init(void) {

    switch (g_sensor.config.sensor_type) {
        case GF_SENSOR_316M:
            g_sensor.chip_type = GF_OSWEGO_M;

            g_sensor.row = 68;
            g_sensor.col = 118;

            g_sensor.nav_row = 68;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;
            // g_sensor.flag_chip_info init by fw config
            // g_sensor.thr_select_bmp init by fw config
            break;

        case GF_SENSOR_318M:
        case GF_SENSOR_3118M:
            g_sensor.chip_type = GF_OSWEGO_M;

            g_sensor.row = 96;
            g_sensor.col = 96;

            g_sensor.nav_row = 96;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;
            // g_sensor.flag_chip_info init by fw config
            // g_sensor.thr_select_bmp init by fw config
            break;

        case GF_SENSOR_516M:
            g_sensor.chip_type = GF_OSWEGO_M;

            g_sensor.row = 68;
            g_sensor.col = 118;

            g_sensor.nav_row = 68;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;
            // g_sensor.flag_chip_info init by fw config
            // g_sensor.thr_select_bmp init by fw config
            break;

        case GF_SENSOR_518M:
        case GF_SENSOR_5118M:
            g_sensor.chip_type = GF_OSWEGO_M;

            g_sensor.row = 96;
            g_sensor.col = 96;

            g_sensor.nav_row = 96;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;
            // g_sensor.flag_chip_info init by fw config
            // g_sensor.thr_select_bmp init by fw config
            break;

        case GF_SENSOR_816M:
            g_sensor.chip_type = GF_OSWEGO_M;

            g_sensor.row = 68;
            g_sensor.col = 118;

            g_sensor.nav_row = 68;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;
            // g_sensor.flag_chip_info init by fw config
            // g_sensor.thr_select_bmp init by fw config
            break;

        case GF_SENSOR_316:
            g_sensor.chip_type = GF_OSWEGO_S;

            g_sensor.row = 68;
            g_sensor.col = 118;

            g_sensor.nav_row = 68;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;

            g_sensor.flag_chip_info = 0x04;
            g_sensor.thr_select_bmp = 400;
            break;

        case GF_SENSOR_318:
            g_sensor.chip_type = GF_OSWEGO_S;

            g_sensor.row = 96;
            g_sensor.col = 96;

            g_sensor.nav_row = 96;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;

            g_sensor.flag_chip_info = 0x04;
            g_sensor.thr_select_bmp = 300;
            break;

        case GF_SENSOR_516:
            g_sensor.chip_type = GF_OSWEGO_S;

            g_sensor.row = 68;
            g_sensor.col = 118;

            g_sensor.nav_row = 68;
            g_sensor.nav_col = 15;
            g_sensor.nav_gap = 0;

            g_sensor.flag_chip_info = 0x00;
            g_sensor.thr_select_bmp = 600;
            break;

        case GF_SENSOR_518:
            g_sensor.chip_type = GF_OSWEGO_S;

            g_sensor.row = 96;
            g_sensor.col = 96;
            g_sensor.nav_gap = 0;

            g_sensor.nav_row = 96;
            g_sensor.nav_col = 15;

            // g_sensor.flag_chip_info init by fw config
            // g_sensor.thr_select_bmp init by fw config
            break;

        case GF_SENSOR_3208: {

            // chip specific parameter

            g_sensor.chip_type = GF_MILAN_F;

            g_sensor.nav_row = 88;
            g_sensor.nav_col = 108;
            g_sensor.nav_gap = 2;
            g_sensor.row = 88;
            g_sensor.col = 108;
            g_sensor.thr_select_bmp = 800;
            g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 1, 0, g_sensor.row, g_sensor.col);
            break;
        }

        case GF_SENSOR_3206: {

            g_sensor.chip_type = GF_MILAN_G;

            g_sensor.nav_row = 54;
            g_sensor.nav_col = 16;
            g_sensor.nav_gap = 4;
            g_sensor.row = 54;
            g_sensor.col = 176;
            g_sensor.thr_select_bmp = 400;
            g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 1, 7, g_sensor.row, g_sensor.col);
            break;
        }

        case GF_SENSOR_3266: {

            g_sensor.chip_type = GF_MILAN_E;

            g_sensor.nav_row = 64;
            g_sensor.nav_col = 16;
            g_sensor.nav_gap = 4;
            g_sensor.row = 64;
            g_sensor.col = 176;
            g_sensor.thr_select_bmp = 400;
            g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 1, 6, g_sensor.row, g_sensor.col);
            break;
        }

        case GF_SENSOR_3288: {

            g_sensor.chip_type = GF_MILAN_L;

            g_sensor.nav_row = 132;
            g_sensor.nav_col = 16;
            g_sensor.nav_gap = 4;
            g_sensor.row = 112;
            g_sensor.col = 132;
            g_sensor.thr_select_bmp = 400;
            g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 1, 2, g_sensor.row, g_sensor.col);

            break;
        }
        case GF_SENSOR_3208FN: {

            g_sensor.chip_type = GF_MILAN_FN;

            g_sensor.nav_row = 88;
            g_sensor.nav_col = 16;
            g_sensor.nav_gap = 4;
            g_sensor.row = 88;
            g_sensor.col = 108;
            g_sensor.thr_select_bmp = 800;
            g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 1, 0, g_sensor.row, g_sensor.col);

            break;
        }
        case GF_SENSOR_5206:
            g_sensor.chip_type = GF_MILAN_A;

            g_sensor.row = 60;
            g_sensor.col = 128;
            g_sensor.thr_select_bmp = 600;

            if  (GF_CERAMIC == g_sensor.config.cover_type){
                g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 1, 0, 1, g_sensor.row, g_sensor.col);
            }else{
                g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 0, 1, g_sensor.row, g_sensor.col);
            }
            g_sensor.nav_row = 15;
            g_sensor.nav_col = 128;
            break;

        case GF_SENSOR_5216:
            g_sensor.chip_type = GF_MILAN_B;

            g_sensor.row = 60;
            g_sensor.col = 128;
            g_sensor.thr_select_bmp = 600;

            if  (GF_CERAMIC == g_sensor.config.cover_type){
                g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 1, 0, 1, g_sensor.row, g_sensor.col);
            }else{
                g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 0, 1, g_sensor.row, g_sensor.col);
            }
            g_sensor.nav_row = 15;
            g_sensor.nav_col = 128;
            break;

        case GF_SENSOR_5208:
            g_sensor.chip_type = GF_MILAN_A;

            g_sensor.row = 88;
            g_sensor.col = 108;
            g_sensor.thr_select_bmp = 800;
            g_sensor.flag_chip_info = GF_SET_CHIP_INFO(1, 0, 0, 8, g_sensor.row, g_sensor.col);
            g_sensor.nav_row = 16;
            g_sensor.nav_col = 108;
            break;

        default:
            return GF_ERROR_UNSUPPORT_SENSOR;
    }

    if ((gf_chip_type_t) GF_OSWEGO_M == g_sensor.chip_type
            || (gf_chip_type_t) GF_OSWEGO_S == g_sensor.chip_type
            || (gf_chip_type_t) GF_MILAN_A == g_sensor.chip_type
            || (gf_chip_type_t) GF_MILAN_B == g_sensor.chip_type) {
        g_sensor.support_esd_check = 1;
    } else if (IS_MILAN_F_SERIES_BY_CHIP_TYPE(g_sensor.chip_type)) {
        g_sensor.support_esd_check = 0;
    }

    if (96 == g_sensor.col) {
        if (g_sensor.config.enrolling_min_templates < 8) {
            g_sensor.config.enrolling_min_templates = 8;
        }
        if (g_sensor.config.enrolling_min_templates > 30) {
            g_sensor.config.enrolling_min_templates = 30;
        }
    } else if (118 == g_sensor.col) {
        if (g_sensor.config.enrolling_min_templates < 10) {
            g_sensor.config.enrolling_min_templates = 10;
        }
        if (g_sensor.config.enrolling_min_templates > 40) {
            g_sensor.config.enrolling_min_templates = 40;
        }
    } else if (108 == g_sensor.col) {
        if (g_sensor.config.enrolling_min_templates < 8) {
            g_sensor.config.enrolling_min_templates = 8;
        }
        if (g_sensor.config.enrolling_min_templates > 30) {
            g_sensor.config.enrolling_min_templates = 30;
        }
    } else if (176 == g_sensor.col) {
        if (g_sensor.config.enrolling_min_templates < 8) {
            g_sensor.config.enrolling_min_templates = 8;
        }
        if (g_sensor.config.enrolling_min_templates > 30) {
            g_sensor.config.enrolling_min_templates = 30;
        }
    } else if (128 == g_sensor.col) {
        if (g_sensor.config.enrolling_min_templates < 10) {
            g_sensor.config.enrolling_min_templates = 10;
        }
        if (g_sensor.config.enrolling_min_templates > 30) {
            g_sensor.config.enrolling_min_templates = 30;
        }
    }

    if (g_sensor.config.max_fingers_per_user > MAX_FINGERS_PER_USER) {
        g_sensor.config.max_fingers_per_user = MAX_FINGERS_PER_USER;
    } else if (g_sensor.config.max_fingers_per_user < 1) {
        g_sensor.config.max_fingers_per_user = 1;
    }
    if (g_sensor.config.max_fingers < g_sensor.config.max_fingers_per_user) {
        g_sensor.config.max_fingers = g_sensor.config.max_fingers_per_user;
    }

    g_sensor.line_len = g_sensor.row * 3 / 2;
    g_sensor.line_aligned_len = (g_sensor.line_len + 15) / 16 * 16;
    g_sensor.sensor_raw_data_len = g_sensor.line_aligned_len * g_sensor.col;
    g_sensor.nav_frame_len = g_sensor.line_aligned_len * g_sensor.nav_col;

    g_sensor.raw_data_len = g_sensor.row * g_sensor.col;

    return GF_SUCCESS;
}

gf_error_t gf_sensor_init(void) {
    gf_error_t err = GF_SUCCESS;

    cpl_memset((void*) &g_sensor, 0, sizeof(gf_sensor_t));

    do {
        err = gf_config_init(&g_sensor.config, &g_sensor.fw_cfg);
        if (err != GF_SUCCESS) {
            break;
        }

        err = gf_sensor_info_init();
    } while (0);

    return err;
}
