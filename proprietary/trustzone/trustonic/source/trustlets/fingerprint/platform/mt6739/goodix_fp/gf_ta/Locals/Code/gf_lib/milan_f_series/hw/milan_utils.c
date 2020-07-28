/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "milan_utils.h"
#include "gf_tee_internal_api.h"
#include "cpl_memory.h"
#include "cpl_string.h"
#include "gf_sensor.h"

#define LOG_TAG "[milan_utils]"

static uint16_t *g_rawdata_pool = NULL;

gf_error_t milan_f_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode) {
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

        if (g_sensor.col != col_num) {
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

gf_error_t milan_e_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint32_t j = 0;
        uint8_t *temp = NULL;  // the raw data after removing dummy bytes
        uint8_t *src = raw_data;  // original raw data from chip
        uint32_t one_col_bytes = 0;  // effect data bytes for one column
        uint32_t extra_one_col_bytes = 0;  // data bytes for one column which contain dummy data.
        // effect data bytes for one frame after removing dummy data
        uint32_t effect_data_bytes = 0;

        if (NULL == raw_data || NULL == des) {
            GF_LOGE(LOG_TAG "[%s] bad params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (NULL == g_rawdata_pool) {
            g_rawdata_pool = (uint16_t *) CPL_MEM_MALLOC(
                    g_sensor.row * g_sensor.col * sizeof(uint16_t));
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
                        case SCAN_MODE0: {  // Scan Mode0(TA0, TA1, TA3 Mode0): 0/88, 1/89……
                            j = i / one_col_bytes;
                            j = (j % 2) * (col_num / 2) + j / 2;
                            j = j * g_sensor.row;
                            break;
                        }

                        // Scan Mode1(TA3 Mode1):0/172, 1/173, 2/174, 3/175, 4/168,
                        // 5/169, 5/170, 7/171,……
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

                        // Scan Mode2(TA3 Mode2):87/91, 86/90, 85/89, 84/88,
                        // ……3/175, 2/174, 1/173, 0/172
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

                        // Scan Mode3(TA2)：0/4, 1/5, 2/6, 3/7, 8/12, 9/13, 10/14, 11/15,……
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

gf_error_t milan_g_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint32_t j = 0;
        uint8_t *temp = NULL;
        uint32_t one_col_bytes = 0;
        uint32_t extra_one_col_bytes = 0;
        uint32_t effect_data_bytes = 0;
        uint8_t *src = raw_data;

        if (NULL == raw_data || NULL == des) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameters", __func__);
            break;
        }

        if (NULL == g_rawdata_pool) {
            g_rawdata_pool = (uint16_t *) CPL_MEM_MALLOC(
                    g_sensor.col * g_sensor.row * sizeof(uint16_t));
            if (NULL == g_rawdata_pool) {
                err = GF_ERROR_OUT_OF_MEMORY;
                GF_LOGE(LOG_TAG "[%s] allocate memory failed", __func__);
                break;
            }
        }

        temp = (uint8_t*) g_rawdata_pool;
        GF_LOGD(LOG_TAG "[%s] is_double_rate=%u, raw_data_len=%u", __func__, is_double_rate,
                raw_data_len);
        GF_LOGD(LOG_TAG "[%s] col_num=%u, col_gaps=%u, scan_mode=%u", __func__, col_num, col_gaps,
                scan_mode);

        one_col_bytes = g_sensor.row * 3 / 2 + 1;
        extra_one_col_bytes = raw_data_len / col_num;
        effect_data_bytes = one_col_bytes * col_num;

        if (is_double_rate == 0) {
            for (i = 0; i < col_num; i++) {  // remove the dummy bytes
                cpl_memcpy(temp, src, one_col_bytes);
                src += extra_one_col_bytes;
                temp += one_col_bytes;
            }

            temp = (uint8_t*) g_rawdata_pool;
            for (i = 0, j = 0; i < effect_data_bytes;) {
                if (j % 54 == 52) {
                    des[j] = ((temp[i] & 0x0F) << 8) + temp[i + 1];
                    des[j + 1] = (temp[i + 3] << 4) + (temp[i] >> 4);
                    j = j + 2;
                    i = i + 4;
                } else {
                    des[j] = ((temp[i] & 0x0F) << 8) + temp[i + 1];
                    des[j + 1] = (temp[i + 3] << 4) + (temp[i] >> 4);
                    des[j + 2] = ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);
                    des[j + 3] = (temp[i + 4] << 4) + (temp[i + 5] >> 4);
                    j = j + 4;
                    i = i + 6;
                }
            }
        } else {
            uint32_t two_cols_valid_data_bytes = one_col_bytes * 2;
            uint32_t two_cols_extra_data_bytes = extra_one_col_bytes * 2;

            for (i = 0; i < col_num / 2; i++) {
                cpl_memcpy(temp, src, two_cols_valid_data_bytes);
                src += two_cols_extra_data_bytes;
                temp += two_cols_valid_data_bytes;
            }

            temp = (uint8_t*) g_rawdata_pool;
            for (i = 0; i < effect_data_bytes;) {
                if (i % one_col_bytes == 0) {
                    switch (scan_mode) {
                        case SCAN_MODE0: {
                            j = i / one_col_bytes;
                            j = (j % 2) * (g_sensor.col / 2) + j / 2;
                            j = j * g_sensor.row;
                            break;
                        }

                        case SCAN_MODE1: {
                            j = i / one_col_bytes;
                            if ((j % 2) != 0) {
                                j = (((g_sensor.col - 1) / 4) - (j / 2) * col_gaps / 4) * 4
                                        + (j / 2) * col_gaps % 4;
                                j = j / col_gaps;
                            } else {
                                j = j / 2;
                            }
                            j = j * g_sensor.row;  // 当前列起始索引
                            break;
                        }

                        case SCAN_MODE2: {
                            j = i / one_col_bytes;  // 计算列索引
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

                        default: {
                            GF_LOGE(LOG_TAG "[%s] unsupport scan mode=%u", __func__, scan_mode);
                            break;
                        }
                    }
                }

                if (j % 54 == 52) {
                    des[j] = ((temp[i] & 0x0F) << 8) + temp[i + 1];
                    des[j + 1] = (temp[i + 3] << 4) + (temp[i] >> 4);
                    j = j + 2;
                    i = i + 4;
                } else {
                    des[j] = ((temp[i] & 0x0F) << 8) + temp[i + 1];
                    des[j + 1] = (temp[i + 3] << 4) + (temp[i] >> 4);
                    des[j + 2] = ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);
                    des[j + 3] = (temp[i + 4] << 4) + (temp[i + 5] >> 4);
                    j = j + 4;
                    i = i + 6;
                }
            }
        }

        if (g_sensor.col == col_num) {
            cpl_memcpy(g_rawdata_pool, (uint8_t*) des,
                    g_sensor.row * g_sensor.col * sizeof(uint16_t));

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

gf_error_t milan_l_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode) {
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

gf_error_t milan_fn_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
        uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode) {
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

gf_error_t milan_k_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
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
    uint8_t *temp = NULL;

    if (g_rawdata_pool == NULL) {
        g_rawdata_pool = (uint16_t *) CPL_MEM_MALLOC(
                g_sensor.row * g_sensor.col * sizeof(uint16_t));
    }

    temp = (uint8_t*) g_rawdata_pool;

    UNUSED_VAR(col_gaps);

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] raw_data_len = %d, col_num=%d, is_double_rate=%d",
            __func__, raw_data_len, col_num, is_double_rate);

    do {
        if (NULL == g_rawdata_pool) {
            GF_LOGE(LOG_TAG "[%s] out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        if (NULL == raw_data || NULL == des) {
            GF_LOGE(LOG_TAG "[%s] bad params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (g_sensor.col != col_num && g_sensor.nav_col != col_num) {
            GF_LOGE(LOG_TAG "[%s] invalid col_num col_num=%u", __func__, col_num);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGD(LOG_TAG "[%s] is_double_rate=%u, scan_mode=%u", __func__, is_double_rate,
                scan_mode);
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
                if (col_num == g_sensor.col) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

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
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (1 == scan_mode) {
            /*scan mode 2*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (2 == scan_mode) {
            /*scan mode 3*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                            k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                            k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup)+1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                            k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (3 == scan_mode) {
            /*scan mode 4*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (int)(one_col_byte * 2) < one_col_byte) {
                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
            } else {
                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        }
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t milan_j_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
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
    uint8_t *temp = NULL;

    if (g_rawdata_pool == NULL) {
        g_rawdata_pool = (uint16_t *) CPL_MEM_MALLOC(
                g_sensor.row * g_sensor.col * sizeof(uint16_t));
    }

    temp = (uint8_t*) g_rawdata_pool;

    UNUSED_VAR(col_gaps);

    FUNC_ENTER();

    GF_LOGD(LOG_TAG "[%s] raw_data_len = %d, col_num = %d, is_double_rate = %d",
            __func__, raw_data_len, col_num, is_double_rate);

    do {
        if (NULL == g_rawdata_pool) {
            GF_LOGE(LOG_TAG "%s, out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

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

        GF_LOGD(LOG_TAG "[%s] is_double_rate:%d, scan_mode:%d", __func__, is_double_rate,
                scan_mode);
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
                if (col_num == g_sensor.col) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

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
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (1 == scan_mode) {
            /*scan mode 2*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (2 == scan_mode) {
            /*scan mode 3*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup)+1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (3 == scan_mode) {
            /*scan mode 4*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (int)(one_col_byte * 2) < one_col_byte) {
                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
            } else {
                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "%s, exit", __func__);

    return err;
}

gf_error_t milan_h_raw_data_split(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
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
    uint8_t *temp = NULL;

    if (g_rawdata_pool == NULL) {
        g_rawdata_pool = (uint16_t *) CPL_MEM_MALLOC(
                g_sensor.row * g_sensor.col * sizeof(uint16_t));
    }

    temp = (uint8_t*) g_rawdata_pool;

    UNUSED_VAR(col_gaps);

    GF_LOGD(LOG_TAG "%s, enter", __func__);

    do {
        if (NULL == g_rawdata_pool) {
            GF_LOGE(LOG_TAG "%s, out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

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

        GF_LOGD(LOG_TAG "[%s] is_double_rate:%d, scan_mode:%d", __func__, is_double_rate,
                scan_mode);
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
                if (col_num == g_sensor.col) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

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
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    des[(k2 % g_sensor.row) * g_sensor.col + (k2 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (1 == scan_mode) {
            /*scan mode 2*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    des[(k % g_sensor.row) * g_sensor.col + (k / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (GroupNum - 1 - k2 / PixelNumPerGroup) * PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (2 == scan_mode) {
            /*scan mode 3*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (one_col_byte * 2) < one_col_byte) {
                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = frame_pixel_half - g_sensor.row * ((k / g_sensor.row) + 1) +
                            k % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
                } else {
                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup) + 1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = frame_pixel_half + PixelNumPerGroup * ((k2 / PixelNumPerGroup)+1) -
                            g_sensor.row * (((k2 % PixelNumPerGroup) / g_sensor.row) + 1) +
                                    k2 % g_sensor.row;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        } else if (3 == scan_mode) {
            /*scan mode 4*/
            for (i = 0; i < effect_data_byte;) {
                if (i % (int)(one_col_byte * 2) < one_col_byte) {
                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k++;

                    k3 = (k / PixelNumPerGroup) * PixelNumPerGroup * 2 + k % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k++;

                    i = i + 6;
            } else {
                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 0] & 0x0F) << 8) + temp[i + 1];

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 3] << 4) + (temp[i + 0] >> 4);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            ((temp[i + 5] & 0x0F) << 8) + (temp[i + 2]);

                    k2++;

                    k3 = (k2 / PixelNumPerGroup) * PixelNumPerGroup * 2 + PixelNumPerGroup +
                            k2 % PixelNumPerGroup;

                    des[(k3 % g_sensor.row) * g_sensor.col + (k3 / g_sensor.row)] =
                            (temp[i + 4] << 4) + (temp[i + 5] >> 4);

                    k2++;

                    i = i + 6;
                }
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "%s, exit", __func__);

    return err;
}
