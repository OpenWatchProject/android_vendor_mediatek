/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <string.h>
#include "cpl_string.h"
#include "cpl_memory.h"
#include "gf_common.h"
#include "gf_tee_internal_api.h"
#include "gf_error.h"
#include "gf_sensor.h"
#include "gf_spi.h"
#include "gf_hw_common.h"
#include "gf_secure_object.h"
#include "milan_hv_bus.h"
#include "milan_hv_common.h"
#include "milan_hv_regs.h"
#include "milan_hv_crc.h"
#include "gf_type_define.h"
#include "gf_algo.h"
#include "gf_config.h"
#include "milan_hv_dac_adjust.h"
#include "gf_vfw.h"
#include "gf_hw.h"
#include "gf_nav.h"
#include "gf_base.h"

#define LOG_TAG "[gf_hw]"

/********************* Global Variable Definition Start *********************/
static uint16_t g_fdt_base_data[MILAN_HV_FDT_BASE_LEN];
static uint16_t *g_rawdata_pool = NULL;  // used during raw data split

extern gf_base_data_t *g_base_data;
/********************* Global Variable Definition End ***********************/

/******************* Internal Function Definition Start *********************/
static gf_error_t hw_init(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        /* vfw init */
        err = gf_vfw_init();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] vfw init failed", __func__);
            break;
        }

        gf_milan_hv_crc32_init_table();
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static uint8_t hw_get_key_state(const uint16_t *cur_frame, const uint16_t *base, uint32_t pixel_num,
        int32_t touch_threshold) {
    uint8_t result = 0;
    uint32_t i = 0;
    uint32_t count = 0;

    VOID_FUNC_ENTER();

    do {
        if ((NULL == base) || (NULL == cur_frame)) {
            GF_LOGD(LOG_TAG "[%s] no data", __func__);
            result = 1;
            break;
        }

        for (i = 0; i < pixel_num; i++) {
            if ((int32_t) base[i] - (int32_t) cur_frame[i] >= touch_threshold) {
                count++;
                if (count >= g_sensor.nav_finger_up_pixel_num) {
                    result = 1;
                    break;
                }
            }
        }
    } while (0);

    VOID_FUNC_EXIT();
    return result;
}

static gf_error_t hw_milan_e_raw_data_split(uint16_t *des, uint8_t *raw_data,
        uint32_t raw_data_len, uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps,
        uint32_t scan_mode) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint32_t j = 0;
        uint32_t one_col_bytes = 0;  // effect data bytes for one column
        uint32_t extra_one_col_bytes = 0;  // data bytes for one column which contain dummy data.
        // effect data bytes for one frame after removing dummy data
        uint32_t effect_data_bytes = 0;
        uint8_t *temp = NULL;  // the raw data after removing dummy bytes
        uint8_t *src = raw_data;  // original raw data from chip

        if (NULL == raw_data || NULL == des) {
            GF_LOGE(LOG_TAG "[%s] bad params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (NULL == g_rawdata_pool) {
            g_rawdata_pool = (uint16_t *) gf_malloc(g_sensor.row * g_sensor.col * sizeof(uint16_t));
            if (NULL == g_rawdata_pool) {
                GF_LOGE(LOG_TAG "[%s] allocated memory failed", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
        }

        temp = (uint8_t *) g_rawdata_pool;
        GF_LOGD(LOG_TAG "[%s] is_double_rate=%u, raw_data_len=%u", __func__, is_double_rate,
                raw_data_len);

        GF_LOGD(LOG_TAG "[%s] col_num=%u, col_gaps=%u, scan_mode=%u", __func__, col_num, col_gaps,
                scan_mode);

        GF_LOGD(LOG_TAG "[%s] g_sensor.row=%u, g_sensor.col=%u", __func__, g_sensor.row,
                g_sensor.col);

        one_col_bytes = g_sensor.row * 3 / 2;
        extra_one_col_bytes = raw_data_len / col_num;
        effect_data_bytes = one_col_bytes * col_num;

        if (is_double_rate == 0) {
            uint16_t *des_temp = des;

            for (i = 0; i < col_num; i++) {  // remove dummy bytes
                cpl_memcpy(temp, src, one_col_bytes);
                src += extra_one_col_bytes;
                temp += one_col_bytes;
            }

            temp = (uint8_t *) g_rawdata_pool;
            for (i = 0; i < effect_data_bytes;) {
                *des_temp++ = ((temp[0] & 0x0F) << 8) + temp[1];
                *des_temp++ = (temp[3] << 4) + (temp[0] >> 4);
                *des_temp++ = ((temp[5] & 0x0F) << 8) + (temp[2]);
                *des_temp++ = (temp[4] << 4) + (temp[5] >> 4);
                temp += 6;
                i += 6;
            }
        } else {
            uint32_t two_cols_valid_data_bytes = one_col_bytes * 2;
            uint32_t two_cols_extra_data_bytes = extra_one_col_bytes * 2;

            for (i = 0; i < col_num / 2; i++) {  // remove dummy bytes
                cpl_memcpy(temp, src, two_cols_valid_data_bytes);
                src += two_cols_extra_data_bytes;
                temp += two_cols_valid_data_bytes;
            }

            temp = (uint8_t *) g_rawdata_pool;

            for (i = 0; i < effect_data_bytes;) {
                if (i % one_col_bytes == 0) {  // calculate the start address for column
                    switch (scan_mode) {
                        // Scan Mode0(TA0, TA1, TA3 Mode0): 0/88, 1/89\u2026\u2026
                        case SCAN_MODE0: {
                            j = i / one_col_bytes;
                            j = (j % 2) * (col_num / 2) + j / 2;
                            j = j * g_sensor.row;
                            break;
                        }

                        // Scan Mode1(TA3 Mode1):0/172, 1/173, 2/174, 3/175, 4/168, 5/169, 5/170,
                        // 7/171,\u2026\u2026
                        case SCAN_MODE1: {
                            j = i / one_col_bytes;
                            if ((j % 2) != 0) {
                                j = (((g_sensor.col - 1) / 4) - (j / 2) * col_gaps / 4) * 4
                                        + (j / 2) * col_gaps % 4;
                                j = j / col_gaps;
                            } else {
                                j = j / 2;
                            }
                            j = j * g_sensor.row;
                            break;
                        }

                        // Scan Mode2(TA3 Mode2):87/91, 86/90, 85/89, 84/88, \u2026\u20263/175,
                        // 2/174, 1/173, 0/172
                        case SCAN_MODE2: {
                            j = i / one_col_bytes;
                            j = col_num - 1 - j;
                            if ((j % 2) == 0) {
                                j = (((g_sensor.col - 1) / 4) - (j / 2) * col_gaps / 4) * 4
                                        + (j / 2) * col_gaps % 4;
                                j = j / col_gaps;
                            } else {
                                j = j / 2;
                            }
                            j = j * g_sensor.row;
                            break;
                        }

                        // Scan Mode3(TA2)\uff1a0/4, 1/5, 2/6, 3/7, 8/12, 9/13, 10/14,
                        // 11/15,\u2026\u2026
                        case SCAN_MODE3: {
                            j = i / one_col_bytes;  // k = 8*(k/8)+4*(k%2)+(k%8)/2;
                            j = j / 2 + (j % 2) * (4 / col_gaps)
                                    + (j / (8 / col_gaps)) * (4 / col_gaps);
                            j = j * g_sensor.row;
                            break;
                        }

                        default: {
                            GF_LOGE(LOG_TAG "[%s] unknown scan mode=%u", __func__, scan_mode);
                            break;
                        }
                    }
                }
                des[j] = ((temp[i] & 0x0F) << 8) + temp[i + 1];
                des[j + 1] = (temp[i + 3] << 4) + (temp[i] >> 4);
                des[j + 2] = ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);
                des[j + 3] = (temp[i + 4] << 4) + (temp[i + 5] >> 4);
                j = j + 4;
                i = i + 6;
            }
        }

        if (g_sensor.col == col_num) {
            cpl_memcpy(g_rawdata_pool, des, g_sensor.row * g_sensor.col * sizeof(uint16_t));

            for (i = 0; i < g_sensor.row; i++) {
                for (j = 0; j < g_sensor.col; j++) {
                    des[i * g_sensor.col + j] = g_rawdata_pool[j * g_sensor.row + i];
                }
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t hw_milan_j_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode) {
    gf_error_t err = GF_SUCCESS;

    uint32_t i = 0;
    uint32_t k = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t effect_data_byte = 0;

    int32_t GroupNum = (g_sensor.col / 4);
    int32_t PixelNumPerGroup = (g_sensor.row * 4);

    uint8_t *src = raw_data;

    uint32_t one_col_byte = g_sensor.row * 3 / 2;
    uint32_t extra_one_col_byte = raw_data_len / col_num;
    uint32_t frame_pixel_half = (col_num * g_sensor.row / 2);

    UNUSED_VAR(col_gaps);

    FUNC_ENTER();

    do {
        uint8_t *temp = NULL;
        if (NULL == raw_data || NULL == des) {
            GF_LOGE(LOG_TAG "%s, bad params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (g_sensor.col != col_num && g_sensor.nav_col != col_num) {
            GF_LOGE(LOG_TAG "%s, invalid col_num col_num = %d", __func__, col_num);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGD(LOG_TAG "[%s] is_double_rate:%d, scan_mode:%d",
                __func__, is_double_rate, scan_mode);

        if (g_rawdata_pool == NULL) {
            g_rawdata_pool = (uint16_t *) gf_malloc(g_sensor.row * g_sensor.col * sizeof(uint16_t));

            if (NULL == g_rawdata_pool) {
                GF_LOGE(LOG_TAG "%s, out of memory", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
        }

        temp = (uint8_t *) g_rawdata_pool;
        if (is_double_rate == 0) {
            for (i = 0; i < raw_data_len; i++) {
                if (i % extra_one_col_byte < one_col_byte) {
                    temp[effect_data_byte++] = src[i];
                }
            }
        } else {
            for (i = 0; i < raw_data_len; i++) {
                if (i % (extra_one_col_byte * 2) < one_col_byte * 2) {
                    temp[effect_data_byte++] = src[i];
                }
            }
        }

        temp = (uint8_t*) g_rawdata_pool;
        if (is_double_rate == 0) {
            for (i = 0; i < effect_data_byte;) {
                if (g_sensor.col == col_num) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];
                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = (temp[i + 3] << 4)
                            + (temp[i + 0] >> 4);
                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);
                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = (temp[i + 4] << 4)
                            + (temp[i + 5] >> 4);
                    k++;
                } else {
                    des[k++] = ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];
                    des[k++] = (temp[i + 3] << 4) + (temp[i + 0] >> 4);
                    des[k++] = ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);
                    des[k++] = (temp[i + 4] << 4) + (temp[i + 5] >> 4);
                }

                i = i + 6;
            }
        } else if (0 == scan_mode) {
            k2 = col_num * g_sensor.row / 2;

            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = (temp[i + 3] << 4)
                            + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = (temp[i + 4] << 4)
                            + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] = (temp[i + 3]
                            << 4) + (temp[i + 0] >> 4);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] = (temp[i + 4]
                            << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (1 == scan_mode) {
            /*scan mode 2*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = (temp[i + 3] << 4)
                            + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] = (temp[i + 4] << 4)
                            + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 3]
                            << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 4]
                            << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (2 == scan_mode) {
            /*scan mode 3*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1)
                            + k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1)
                            + k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 3]
                            << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1)
                            + k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1)
                            + k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 4]
                            << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1)
                            - g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1)
                            + k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1)
                            - g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1)
                            + k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 3]
                            << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1)
                            - g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1)
                            + k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1)
                            - g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1)
                            + k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 4]
                            << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (3 == scan_mode) {
            /*scan mode 4*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (int) (one_col_byte * 2) < one_col_byte) {
                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 3]
                            << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 4]
                            << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 0]
                            & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 3]
                            << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = ((temp[i + 5]
                            & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup
                            + k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] = (temp[i + 4]
                            << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t hw_milan_fn_raw_data_split(uint16_t *des, uint8_t *raw_data,
        uint32_t raw_data_len, uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps,
        uint32_t scan_mode) {
    gf_error_t err = GF_SUCCESS;

    uint32_t i = 0;
    uint32_t j = 0;

    uint8_t *src = raw_data;

    UNUSED_VAR(is_double_rate);
    UNUSED_VAR(col_gaps);
    UNUSED_VAR(scan_mode);

    FUNC_ENTER();

    do {
        if (NULL == raw_data || NULL == des) {
            GF_LOGE(LOG_TAG "[%s] bad params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (g_sensor.nav_col != col_num && g_sensor.col != col_num) {
            GF_LOGE(LOG_TAG "[%s] invalid col_num col_num=%u", __func__, col_num);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (i = 0; i < raw_data_len; i += 6, j += 4) {
            des[j] = ((src[i] & 0x0F) << 8) + (src[i + 1]);
            des[j + 1] = ((src[i + 3]) << 4) + ((src[i] & 0xF0) >> 4);
            des[j + 2] = ((src[i + 5] & 0x0F) << 8) + src[i + 2];
            des[j + 3] = (src[i + 4] << 4) + ((src[i + 5] & 0xF0) >> 4);
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

static gf_error_t hw_raw_data_split(uint16_t *raw_data, uint8_t *origin_data,
        uint32_t origin_data_len, uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps) {
    gf_error_t err = GF_SUCCESS;
    uint32_t chip_id = 0;
    milan_hv_scan_mode_t scan_mode = 0;

    FUNC_ENTER();

    chip_id = gf_vfw_get_chip_id();
    scan_mode = gf_vfw_get_scan_mode();

    switch (chip_id >> 8) {
        case MILAN_E_HV_CHIP_ID: {
            err = hw_milan_e_raw_data_split(raw_data, origin_data, origin_data_len, col_num,
                    is_double_rate, col_gaps, scan_mode);
            break;
        }

        case MILAN_FN_HV_CHIP_ID: {
            err = hw_milan_fn_raw_data_split(raw_data, origin_data, origin_data_len, col_num,
                    is_double_rate, col_gaps, scan_mode);
            break;
        }

        case MILAN_J_HV_CHIP_ID: {
            err = hw_milan_j_raw_data_split(raw_data, origin_data, origin_data_len, col_num,
                    is_double_rate, col_gaps, scan_mode);
            break;
        }

        default: {
            GF_LOGE(LOG_TAG "[%s] un-supported chip, chip_id=0x%x", __func__, chip_id);
            err = GF_ERROR_UNSUPPORT_CHIP;
            break;
        }
    }

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hw_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len,
        uint16_t *raw_data) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t crc32_from_host = 0xFFFFFFFF;  // CRC32 Value calculate by Host
        uint32_t crc32_from_chip = 0;  // CRC32 Value general by Chip
        uint32_t data_len = 0;
        err = gf_vfw_get_nav_image(origin_data, origin_data_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] get nav image failed", __func__);
            break;
        }

        data_len = *origin_data_len - 4;

        crc32_from_host = gf_milan_hv_get_crc32(origin_data, data_len);
        crc32_from_chip = (origin_data[data_len + 2] << 24)
                + (origin_data[data_len + 3] << 16) + (origin_data[data_len] << 8)
                + (origin_data[data_len + 1]);

        if (crc32_from_host != crc32_from_chip) {
            GF_LOGE(LOG_TAG "[%s] crc32 check failed, crc32_from_host=%u, crc32_from_chip=%u",
                    __func__, crc32_from_host, crc32_from_chip);
            err = GF_ERROR_SPI_RAW_DATA_CRC_FAILED;
            break;
        }
        GF_LOGI(LOG_TAG "[%s] crc32 check success", __func__);

        err = hw_raw_data_split(raw_data, origin_data, data_len, g_sensor.nav_col,
                gf_vfw_get_nav_double_rate_flag(), g_sensor.nav_gap);

        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] split raw data failed", __func__);
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hw_parse_hardware_irq_type(uint16_t original_irq_type, uint32_t *irq_type) {
    gf_error_t err = GF_SUCCESS;
    milan_hv_chip_state_t cur_chip_state = 0;

    FUNC_ENTER();

    do {
        uint32_t chip_id = 0;
        cur_chip_state = gf_vfw_get_cur_chip_state();
        chip_id = gf_vfw_get_chip_id();

        if (original_irq_type == HV_TMR_IRQ_MNT_MASK) {
            *irq_type = GF_IRQ_TMR_IRQ_MNT_MASK;
            break;
        }

        if (original_irq_type == HV_FDT_DOWN_IRQ_MASK) {
            *irq_type = GF_IRQ_FINGER_DOWN_MASK;
            break;
        }

        if (original_irq_type == HV_DATA_INT_MASK) {
            if ((cur_chip_state & CHIP_MODE_IMAGE) > 0) {
                *irq_type = GF_IRQ_IMAGE_MASK;
            } else {
                *irq_type = GF_IRQ_NAV_MASK;
            }
            break;
        }

        if (original_irq_type == HV_FDT_IRQ1_MASK) {
            *irq_type = GF_IRQ_FDT_REVERSE_MASK;
            break;
        }

        if (original_irq_type == HV_FDT_IRQ_UP_MASK) {
            *irq_type = GF_IRQ_FINGER_UP_MASK;
            break;
        }

        /* IRQ bit[10], when milan J hv means temperature irq, for milan E/FN hv means reset irq */
        if ((original_irq_type == MILAN_J_HV_TEMPERATURE_IRQ_MASK)
                && (!MILAN_HV_CHIP_ID_IS_EQUAL(chip_id, MILAN_J_HV_CHIP_ID))) {
            *irq_type = GF_IRQ_RESET_MASK;
            break;
        }

        if ((original_irq_type == MILAN_J_HV_RESET_IRQ_MASK)
                && (MILAN_HV_CHIP_ID_IS_EQUAL(chip_id, MILAN_J_HV_CHIP_ID))) {
            *irq_type = GF_IRQ_RESET_MASK;
            break;
        }

        GF_LOGD(LOG_TAG "[%s] invalid original irq type=0x%x",
                __func__, original_irq_type);
    } while (0);

    GF_LOGI(LOG_TAG "[%s] original_irq_type=0x%04X, parsed irq type=0x%04X", __func__,
            original_irq_type, *irq_type);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hw_dac_dynamic_adjust(uint16_t *raw_data) {
    gf_error_t err = GF_SUCCESS;
    milan_hv_otp_info_t otp_info;
    uint16_t pre_dac_h = 0;
    uint16_t post_dac_h = 0;
    uint16_t reg_dac_data = 0;

    FUNC_ENTER();

    do {
        err = gf_vfw_get_otp_info(&otp_info);
        if (GF_SUCCESS != err) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        if (1 == otp_info.valid) {
            pre_dac_h = otp_info.dac_h;
            reg_dac_data = otp_info.dac_h << 4;

            GF_LOGI(LOG_TAG "[%s] Before adjust DAC, dac_h=0x%04X", __func__,
                    otp_info.dac_h);

            HVDacDynamicAdjust(raw_data, g_base_data->finger_base_rawdata, g_sensor.row,
                    g_sensor.col, &reg_dac_data, otp_info.otp_tcode, otp_info.dac_h);

            otp_info.dac_h = (reg_dac_data >> 4) & 0x1FF;
            post_dac_h = otp_info.dac_h;

            GF_LOGI(LOG_TAG "[%s] After adjust DAC, dac_h=0x%04X", __func__, otp_info.dac_h);

            if (post_dac_h == pre_dac_h) {
                GF_LOGI(LOG_TAG "[%s] DAC not Adjust", __func__);
            } else {
                gf_vfw_set_dac_h(post_dac_h);
                GF_LOGI(LOG_TAG "[%s] DAC Adjust, new dac_h=0x%04X", __func__, otp_info.dac_h);
            }
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

static gf_error_t hw_switch_mode(void) {
    gf_error_t err = GF_SUCCESS;
    milan_hv_chip_state_t chip_mode = gf_vfw_get_cur_chip_mode();

    FUNC_ENTER();

    if ((chip_mode != CHIP_MODE_IMAGE) && (chip_mode != CHIP_MODE_NAV)) {
        GF_LOGI(LOG_TAG "[%s] Will switch mode. cur_chip_state=%s",
                __func__, gf_vfw_strstate(gf_vfw_get_cur_chip_state()));
        err = gf_vfw_switch_mode();
    }

    FUNC_EXIT(err);
    return err;
}

/********************* Internal Function Definition End *********************/

/********************* External Function Definition Start *********************/
gf_error_t gf_hw_detect_sensor(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t chip_id = 0;
        gf_vfw_chip_handle_init();

        chip_id = gf_vfw_get_chip_id();

        GF_LOGI(LOG_TAG "[%s] chip id=0x%08X", __func__, chip_id);

        if (MILAN_HV_CHIP_ID_IS_EQUAL(chip_id, MILAN_E_HV_CHIP_ID)
                || MILAN_HV_CHIP_ID_IS_EQUAL(chip_id, MILAN_J_HV_CHIP_ID)
                || MILAN_HV_CHIP_ID_IS_EQUAL(chip_id, MILAN_FN_HV_CHIP_ID)) {
            GF_LOGI(LOG_TAG "[%s] milan hv sensor detected", __func__);
        } else {
            GF_LOGE(LOG_TAG "[%s] un-supported chip id", __func__);
            err = GF_ERROR_SENSOR_NOT_AVAILABLE;
            break;
        }

        err = hw_init();
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_chip_init(uint8_t *hal_otp_data, uint32_t *hal_otp_data_len, uint8_t *tee_otp_data,
        uint32_t *tee_otp_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t chip_id = 0;
        err = gf_vfw_init_otp(hal_otp_data, hal_otp_data_len, tee_otp_data, tee_otp_data_len);
        GF_ERROR_BREAK(err);

        chip_id = gf_vfw_get_chip_id();

        switch (chip_id >> 8) {
            uint8_t product_id = 0;
            case MILAN_E_HV_CHIP_ID:
                product_id = gf_vfw_get_product_id();

                if (GF5266_PRODUCT_ID == product_id || GF5266ZN2_PRODUCT_ID == product_id
                        || GF5266ZN1_PRODUCT_ID == product_id
                        || GF5266BN1_PRODUCT_ID == product_id) {
                    g_config.chip_type = GF_CHIP_5266;
                } else {
                    g_config.chip_type = GF_CHIP_8206;
                }
                break;

            case MILAN_FN_HV_CHIP_ID:
                g_config.chip_type = GF_CHIP_5288;
                break;

            case MILAN_J_HV_CHIP_ID:
                g_config.chip_type = GF_CHIP_6226;
                break;

            default: {
                GF_LOGE(LOG_TAG "[%s] un-supported chip id", __func__);
                err = GF_ERROR_SENSOR_NOT_AVAILABLE;
                FUNC_EXIT(err);
                return err;
            }
        }
        err = gf_sensor_info_init_by_chip_type(g_config.chip_type);
        GF_ERROR_BREAK(err);

        err = gf_vfw_chip_init();
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_chip_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t chip_id = 0;

    FUNC_ENTER();

    do {
        if (NULL == buf || buf_len < sizeof(uint32_t)) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter, buf=%p, buf_len=%u", __func__, buf, buf_len);
            break;
        }
        chip_id = gf_vfw_get_chip_id();

        cpl_memcpy(buf, (uint8_t *) (&chip_id), sizeof(uint32_t));
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_vendor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint16_t vendor_id = 0;

    FUNC_ENTER();

    do {
        if (NULL == buf || buf_len < sizeof(uint16_t)) {
            GF_LOGE(LOG_TAG "[%s] bad parameter, buf=%p, buf_len=%u", __func__, buf, buf_len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_vfw_get_vendor_id(&vendor_id);
        GF_ERROR_BREAK(err);

        cpl_memcpy(buf, (uint8_t *) (&vendor_id), sizeof(uint16_t));
        GF_LOGD(LOG_TAG "[%s] vendor_id=0x%04X", __func__, vendor_id);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_sensor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == buf || buf_len > MILAN_HV_OTP_DATA_LEN) {
            GF_LOGE(LOG_TAG "[%s] bad parameter, buf=%p, buf_len=%u", __func__, buf, buf_len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_vfw_get_otp_data(buf);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_otp_data(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == buf || buf_len < MILAN_HV_OTP_DATA_LEN) {
            GF_LOGE(LOG_TAG "[%s] bad parameter, buf=%p, buf_len=%u", __func__, buf, buf_len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_vfw_get_otp_data(buf);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_set_mode(gf_mode_t mode) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_vfw_set_mode(mode);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_mode(gf_mode_t *value) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_vfw_get_mode(value);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_image(uint8_t *origin_data, uint32_t *origin_data_len, uint16_t *raw_data,
        uint32_t *raw_data_len, uint8_t frame_num) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *tmp_origin_data = NULL;
    uint16_t *tmp_raw_data = NULL;

    uint8_t i = 0;

    FUNC_ENTER();

    do {
    uint32_t per_origin_data_len_no_crc = 0;
    gf_mode_t cur_mode = MODE_NONE;
    uint32_t per_raw_data_len = 0;
        if (NULL == origin_data || NULL == origin_data_len
                || NULL == raw_data || NULL == raw_data_len
                || frame_num > MAX_CONTINUE_FRAME_NUM || 0 == frame_num) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter, frame_num=%d", __func__, frame_num);
            break;
        }

        err = gf_vfw_get_image(origin_data, origin_data_len, frame_num);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get image fail", __func__);
            break;
        }

        cur_mode = gf_vfw_get_cur_mode();

        per_origin_data_len_no_crc = (*origin_data_len / frame_num) - CRC32_SIZE;
        per_raw_data_len = g_sensor.row * g_sensor.col;

        *raw_data_len = per_raw_data_len * frame_num;

        for (i = 0; i < frame_num; i++) {
            uint32_t crc32_from_host = 0xFFFFFFFF;  // CRC32 Value calculate by Host
            uint32_t crc32_from_chip = 0;  // CRC32 Value general by Chip
            tmp_origin_data = origin_data + i * (per_origin_data_len_no_crc + CRC32_SIZE);
            tmp_raw_data = raw_data + i * per_raw_data_len;

            crc32_from_host = gf_milan_hv_get_crc32(tmp_origin_data, per_origin_data_len_no_crc);
            crc32_from_chip = (tmp_origin_data[per_origin_data_len_no_crc + 2] << 24)
                    + (tmp_origin_data[per_origin_data_len_no_crc + 3] << 16)
                    + (tmp_origin_data[per_origin_data_len_no_crc] << 8)
                    + (tmp_origin_data[per_origin_data_len_no_crc + 1]);

            if (crc32_from_host != crc32_from_chip) {
                GF_LOGE(LOG_TAG "[%s] crc32 check failed[%d], crc32_from_host=%u, "
                        "crc32_from_chip=%u", __func__, i, crc32_from_host, crc32_from_chip);
                err = GF_ERROR_SPI_RAW_DATA_CRC_FAILED;
                break;
            }
            GF_LOGI(LOG_TAG "[%s] crc32 check success[%d]", __func__, i);

            /* decipher and do data re-order */
            err = hw_raw_data_split(tmp_raw_data, tmp_origin_data, per_origin_data_len_no_crc,
                    g_sensor.col, gf_vfw_get_image_double_rate_flag(), 1);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] split raw_data failed[%d], err=%s, errno=%d", __func__, i,
                        gf_strerror(err), err);
                break;
            }
            GF_LOGI(LOG_TAG "[%s] split raw_data success[%d]", __func__, i);
        }

        GF_ERROR_BREAK(err);

        if ((1 == g_config.support_hv_dac_adjust)
                && ((MODE_IMAGE == cur_mode) || (MODE_FF == cur_mode))) {
            hw_dac_dynamic_adjust(raw_data);
        }

        GF_LOGD(LOG_TAG "[%s] raw_data_len=%u, origin_data_len=%u", __func__, *raw_data_len,
                *origin_data_len);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_nav_image(uint8_t *origin_data, uint32_t *origin_data_len, uint16_t *raw_data,
        uint32_t *raw_data_len, uint8_t *frame_num, uint8_t *over_flag) {
    gf_error_t err = GF_SUCCESS;

    int32_t i = 0;
    uint16_t *base_data = NULL;
    uint32_t base_len = 0;


    FUNC_ENTER();

    do {
        uint16_t *tmp_raw_data = NULL;
        *over_flag = 0;
        *frame_num = 0;
        *origin_data_len = 0;
        *raw_data_len = g_sensor.nav_row * g_sensor.nav_col;

        gf_nav_get_base(&base_data, &base_len);

        tmp_raw_data = raw_data;

        for (i = 0; i < g_config.nav_config.maxNvgFrameNum;) {
            err = hw_get_nav_image(origin_data, origin_data_len, tmp_raw_data);
            if (GF_SUCCESS != err) {
                GF_LOGE(LOG_TAG "[%s] get %dth frame failed, err=%s, errno=%d", __func__, i + 1,
                        gf_strerror(err), err);
                break;
            } else {
                i++;
                GF_LOGD(LOG_TAG "[%s] get %dth frame success", __func__, i);

                if (0 == hw_get_key_state(tmp_raw_data, base_data, *raw_data_len,
                        g_sensor.nav_finger_up_pixel_diff)) {
                    // finger leaving is detected when get NAV image, so detect finger down directly
                    GF_LOGI(LOG_TAG "[%s] finger leaving, set to fdt down", __func__);

                    *over_flag = 1;

                    GF_LOGD(LOG_TAG "[%s] set to fdt down", __func__);
                    err = gf_hw_set_mode(MODE_NAV);
                    if (GF_SUCCESS != err) {
                        GF_LOGE(LOG_TAG "[%s] set fdt down failed", __func__);
                    }

                    break;
                }

                tmp_raw_data += NAV_BUFFER_LEN;
            }
        }

        *frame_num = i;

        if (i > 0) {
            err = GF_SUCCESS;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_nav_base_image(uint8_t *origin_data, uint32_t *origin_data_len,
        uint16_t *raw_data, uint32_t *raw_data_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t crc32_from_chip = 0;  // CRC32 Value general by Chip
        uint32_t crc32_from_host = 0xFFFFFFFF;  // CRC32 Value calculate by Host
        uint32_t data_len = 0;
        err = gf_vfw_get_nav_base_image(origin_data, origin_data_len);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read nav base failed", __func__);
            break;
        }

        data_len = *origin_data_len - 4;

        crc32_from_host = gf_milan_hv_get_crc32(origin_data, data_len);
        crc32_from_chip = (origin_data[data_len + 2] << 24)
                + (origin_data[data_len + 3] << 16) + (origin_data[data_len] << 8)
                + (origin_data[data_len + 1]);

        if (crc32_from_host != crc32_from_chip) {
            GF_LOGE(LOG_TAG "[%s] crc32 check failed, crc32_from_host=%u, crc32_from_chip=%u",
                    __func__, crc32_from_host, crc32_from_chip);
            err = GF_ERROR_SPI_RAW_DATA_CRC_FAILED;
            break;
        }
        GF_LOGI(LOG_TAG "[%s] crc32 check success", __func__);

        /* decipher and do data re-order */
        err = hw_raw_data_split(raw_data, origin_data, data_len, g_sensor.nav_col,
                gf_vfw_get_nav_double_rate_flag(), g_sensor.nav_gap);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] split raw_data failed, err=%s, errno=%d", __func__,
                    gf_strerror(err), err);
            break;
        }

        // g_milan_hv_chip_handle.suspend_data_sampling_flag = 1;
        *raw_data_len = g_sensor.nav_row * g_sensor.nav_col;

        GF_LOGD(LOG_TAG "[%s] raw_data_len=%u, origin_data_len=%u", __func__, *raw_data_len,
                *origin_data_len);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_orientation(uint16_t *orientation, uint16_t *facing) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == orientation || NULL == facing) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameters, orientation=%p, facing=%p",
                    __func__, (void *)orientation, (void *)facing);
            break;
        }

        *orientation = gf_vfw_get_orientation();
        *facing = gf_vfw_get_cur_facing();
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_read_register(uint32_t addr, uint8_t *content, uint32_t read_len) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (content == NULL) {
            GF_LOGE(LOG_TAG "[%s] content container is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        if (read_len > GF_MAX_SPI_RW_LEN) {
            GF_LOGE(LOG_TAG "[%s] illegal buffer size", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_milan_hv_read_bytes((uint16_t) addr, content, read_len, GF_SPI_SPEED_LOW);
        GF_LOGD(LOG_TAG "[%s] READ 0x%04x(%d bytes)", __func__, addr, read_len);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_write_register(uint32_t addr, uint8_t *content, uint32_t write_len) {
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;

    FUNC_ENTER();

    do {
        if (content == NULL) {
            GF_LOGE(LOG_TAG "[%s] content container is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (write_len > GF_MAX_SPI_RW_LEN) {
            GF_LOGE(LOG_TAG "[%s] illegal buffer size", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (i = 0; i < write_len; i++) {
            GF_LOGD(LOG_TAG "[%s] WRITE 0x%02X ", __func__, content[i]);
        }

        err = gf_milan_hv_write_bytes((uint16_t) addr, content, write_len);
        GF_LOGD(LOG_TAG "[%s] WRITE 0x%04x(%d bytes)", __func__, addr, write_len);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_irq_preprocess(uint32_t *irq_type) {
    gf_error_t err = GF_SUCCESS;
    uint32_t temperature_change_flag = 0;

    FUNC_ENTER();

    do {
        if (NULL == irq_type) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] irq_type is NULL", __func__);
            break;
        }

        if (*irq_type == GF_IRQ_FINGER_DOWN_MASK) {
            gf_hw_check_temperature_change(&temperature_change_flag);
            if (1 == temperature_change_flag) {
                GF_LOGI(LOG_TAG "[%s] down irq, caused by temperature", __func__);
                *irq_type = GF_IRQ_TEMPERATURE_CHANGE_MASK;
            }
            break;
        }

        if (*irq_type == GF_IRQ_FINGER_UP_MASK) {
            err = gf_vfw_check_temperature_change_on_up_irq(&temperature_change_flag);
            if (1 == temperature_change_flag) {
                GF_LOGI(LOG_TAG "[%s] up irq, caused by temperature", __func__);
                *irq_type = GF_IRQ_TEMPERATURE_CHANGE_MASK;
            }
            break;
        }

        // when check as temperature update all base
        if (*irq_type == GF_IRQ_FDT_REVERSE_MASK) {
            // is startup base ready
            if (0 == gf_base_is_finger_base_vaild()) {
                *irq_type = GF_IRQ_TEMPERATURE_CHANGE_MASK;
            } else {
                // check if maybe temperature
                gf_hw_check_temperature_change(&temperature_change_flag);
                if (1 == temperature_change_flag) {
                    // check if this is really temperature case
                    err = gf_vfw_check_temperature_change_on_reverse_irq(&temperature_change_flag);
                    if (1 == temperature_change_flag) {
                        *irq_type = GF_IRQ_TEMPERATURE_CHANGE_MASK;
                        GF_LOGI(LOG_TAG "[%s] reverse irq, caused by temperature", __func__);
                    }
                } else {
                    GF_LOGI(LOG_TAG "[%s] reverse irq only update down base", __func__);
                    err = gf_vfw_update_fdt_down_base_with_reverse();
                }
            }
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_irq_type(uint32_t *irq_type) {
    gf_error_t err = GF_SUCCESS;
    uint16_t original_irq_type = 0;

    FUNC_ENTER();

    do {
        if (NULL == irq_type) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] irq_type is NULL", __func__);
            break;
        }

        *irq_type = 0;

        err = gf_vfw_get_irq_type(&original_irq_type);
        GF_ERROR_BREAK(err);

        hw_parse_hardware_irq_type(original_irq_type, irq_type);

        // clear irq immediately
        if (0 == *irq_type) {
            err = gf_vfw_clear_irq(0xFFFF);
            GF_LOGD(LOG_TAG "[%s] write 0xFFFF to clear invalide irq", __func__);
        } else {
            GF_LOGD(LOG_TAG "[%s] clear %s", __func__, gf_strirq(*irq_type));
            err = gf_vfw_clear_irq(original_irq_type);
        }

        if (*irq_type == GF_IRQ_RESET_MASK) {
            GF_LOGI(LOG_TAG "[%s] handle GF_IRQ_RESET_MASK", __func__);
            gf_vfw_set_general_reg();
            gf_vfw_enable_spi_bypass_deglitch();
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] irq_type=%s, irq_type_code=0x%08X",
            __func__, gf_strirq(*irq_type), *irq_type);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_switch_mode(uint32_t irq_type) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    if (!(irq_type & GF_IRQ_FDT_REVERSE_MASK)) {
        err = hw_switch_mode();
    }

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_clear_irq(uint32_t irq_type) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] clear irq when get irq, do nothing here", __func__);
    UNUSED_VAR(irq_type);
    return err;
}

gf_error_t gf_hw_nav_complete(void) {
    gf_error_t err = GF_SUCCESS;
    gf_mode_t mode = MODE_NONE;
    milan_hv_chip_state_t chip_state = CHIP_MODE_IDLE;

    FUNC_ENTER();

    do {
        err = gf_vfw_get_mode(&mode);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] get mode fail", __func__);
            break;
        }

        if (MODE_NAV != mode) {
            err = GF_ERROR_GENERIC;
            GF_LOGE(LOG_TAG "[%s] mode=%s, is not nav mode", __func__, mode);
            break;
        }

        chip_state = gf_vfw_get_cur_chip_mode();
        if (CHIP_MODE_NAV != chip_state) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] cur_chip_state=%s, is not nav", __func__,
                    gf_vfw_strstate(chip_state));
            break;
        }

        GF_LOGD(LOG_TAG "[%s] set to fdt up", __func__);
        err = gf_hw_data_sample_suspend();
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] set nav fdt up failed", __func__);
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_data_sample_resume(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_vfw_data_sample_resume();
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_data_sample_suspend(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_vfw_data_sample_suspend();
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_update_fdt_base(uint16_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    UNUSED_VAR(buf_len);

    FUNC_ENTER();

    err = gf_vfw_update_fdt_base(buf);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_fdt_base(uint16_t **buf, uint32_t *length) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        cpl_memset(g_fdt_base_data, 0, sizeof(g_fdt_base_data));

        err = gf_vfw_read_fdt_data_with_trigger(g_fdt_base_data);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read FDT data failed", __func__);
            break;
        }

        *buf = g_fdt_base_data;
        *length = MILAN_HV_FDT_BASE_LEN;
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_fdt_threshold(uint16_t *threshold) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_vfw_get_fdt_threshold(threshold);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_get_nav_base_threshold(uint16_t *threshold) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_vfw_get_nav_base_threshold(threshold);
    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_test_sensor_validity(void) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    err = gf_vfw_test_sensor_validity();
    FUNC_EXIT(err);
    return err;
}

void gf_hw_init_global_variable(void) {
    // TODO(goodix): add global variable initialize if necessary
}

gf_error_t gf_hw_check_temperature_change(uint32_t *is_temperature_change) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == is_temperature_change) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_vfw_check_temperature_change(is_temperature_change);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_check_finger_touched(uint32_t *is_finger_touched) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        if (NULL == is_finger_touched) {
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_vfw_check_finger_touched(is_finger_touched);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t gf_hw_generate_image_irq_by_manual(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_vfw_generate_image_irq_by_manual();
    FUNC_EXIT(err);

    return err;
}

gf_error_t gf_hw_enable_tx(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_vfw_enable_tx();
    FUNC_EXIT(err);
    return err;
}
gf_error_t gf_hw_disable_tx(void) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_vfw_disable_tx();
    FUNC_EXIT(err);
    return err;
}

void gf_hw_destroy(void) {
}

gf_error_t gf_hw_chip_config_init(void) {
    return GF_SUCCESS;
}

void gf_hw_clear_filed_down_base(void) {
    VOID_FUNC_ENTER();
    // gf_vfw_clear_filed_down_base();
    VOID_FUNC_EXIT();
}

uint8_t gf_hw_is_enable_fdt_tx_strategy(void) {
    return gf_vfw_is_enable_fdt_tx_strategy();
}
uint8_t gf_hw_is_enable_nav_tx_strategy(void) {
    return gf_vfw_is_enable_nav_tx_strategy();
}

void gf_hw_clear_saved_fdt_down_base(void) {
    VOID_FUNC_ENTER();
    gf_vfw_clear_saved_fdt_down_base();
    VOID_FUNC_EXIT();
}

/********************* External Function Definition End *********************/
