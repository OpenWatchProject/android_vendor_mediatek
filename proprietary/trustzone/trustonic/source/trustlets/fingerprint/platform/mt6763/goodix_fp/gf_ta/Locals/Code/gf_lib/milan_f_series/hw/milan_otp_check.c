/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <string.h>

#include "gf_tee_internal_api.h"

#include "gf_error.h"

#include "cpl_string.h"
#include "gf_milan.h"

#include "milan_otp_check.h"

#define LOG_TAG "[milan_otp_check]"

gf_error_t milan_efglfn_calculate_otp_crc(uint8_t* otp_buf) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;
        uint8_t sum = 0;
        uint8_t crc = 0;
        uint8_t temp_data[OTP_LEN_IN_BYTES] = { 0 };

        if (NULL == otp_buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        /* FT not be written, {0x08 ~ 0x11, 0x1e} = 0 */
        for (i = 0x08; i <= 0x1E; ++i) {
            if (0 != otp_buf[i]) {
                break;
            }

            if (0x11 == i) {
                i = 0x1D;
            }
        }

        if (0x1E < i) {
            GF_LOGI(LOG_TAG "[%s] FT didn't be written", __func__);
            break;
        }

        /* checksum, 0x00 ~ 0x13, 0x1d, 0x1f */
        for (sum = 0, i = 0; i <= 0x13; ++i) {
            sum += otp_buf[i];
        }

        sum += otp_buf[0x1D];
        sum += otp_buf[0x1F];

        if (otp_buf[0x1E] == (uint8_t) (~sum + 1)) {
            GF_LOGD(LOG_TAG "[%s] OTP check, checksum check success", __func__);
            break;
        }

        /* unknown product CRC8 check, 0x00 ~ 0x13, 0x1d, 0x1f */
        // ((0 == otp_buf[0x1a]) && (0 == otp_buf[0x1b]) && (0 == otp_buf[0x1c]))
        cpl_memcpy(temp_data, otp_buf, 20);
        temp_data[20] = otp_buf[0x1D];
        temp_data[21] = otp_buf[0x1F];
        crc = milan_otp_crc8(temp_data, 22);
        if (crc == otp_buf[0x1e]) {
            GF_LOGD(LOG_TAG "[%s] OTP check, unknown product CRC8 check success", __func__);
            break;
        }

        /* TOP product CRC8 check, 0x00 ~ 0x13, 0x1c, 0x1d, 0x1f  */
        if (0xC0 == (otp_buf[0x1c] & 0xC0)) {
            cpl_memcpy(temp_data, otp_buf, 20);
            temp_data[20] = otp_buf[0x1C];
            temp_data[21] = otp_buf[0x1D];
            temp_data[22] = otp_buf[0x1F];
            crc = milan_otp_crc8(temp_data, 23);

            if (crc == otp_buf[0x1e]) {
                GF_LOGD(LOG_TAG "[%s] OTP check, TOP product CRC8 check success", __func__);
                break;
            }
        }

        /* open market product CRC8 check, 0x00 ~ 0x13, 0x1a, 0x1b, 0x1c, 0x1d, 0x1f */
        if (0xC0 != (otp_buf[0x1c] & 0xC0)) {
            cpl_memcpy(temp_data, otp_buf, 20);
            cpl_memcpy(&temp_data[20], &otp_buf[0x1A], 4);

            temp_data[24] = otp_buf[0x1F];

            crc = milan_otp_crc8(temp_data, 25);
            if (crc == otp_buf[0x1e]) {
                GF_LOGD(LOG_TAG "[%s] OTP check, open market product CRC8 check success", __func__);
                break;
            }
        }

        err = GF_ERROR_SENSOR_TEST_FAILED;
        GF_LOGE(LOG_TAG "[%s] OTP crc check failed", __func__);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t milan_kjh_calculate_otp_crc(uint8_t* otp_buf) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();
    do {
        uint32_t i = 0;
        uint8_t crc = 0;
        uint8_t temp_data[OTP_LEN_IN_BYTES] = { 0 };
        if (NULL == otp_buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        /* FT not be written, {0x08 ~ 0x11, 0x1e} = 0 */
        for (i = 0x08; i <= 0x1E; ++i) {
            if (0 != otp_buf[i]) {
                break;
            }

            if (0x11 == i) {
                i = 0x1D;
            }
        }

        if (0x1E < i) {
            GF_LOGI(LOG_TAG "[%s] FT didn't be written", __func__);
            break;
        }

        // CRC8 check, 0x00 ~ 0x13, 0x19~0x1D和0x1F

        cpl_memcpy(temp_data, otp_buf, 20);
        cpl_memcpy(&temp_data[20], &otp_buf[0x19], 5);

        temp_data[25] = otp_buf[0x1F];

        crc = milan_otp_crc8(temp_data, 26);
        if (crc == otp_buf[0x1e]) {
            GF_LOGD(LOG_TAG "[%s] OTP check, CRC8 check success", __func__);
            break;
        }


        err = GF_ERROR_SENSOR_TEST_FAILED;
        GF_LOGE(LOG_TAG "[%s] OTP crc check failed", __func__);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t milan_efglfn_check_ft_test_info(uint8_t* otp_buf) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        uint32_t i = 0;

        if (NULL == otp_buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        /* FT not be wroten, 0x14 and 0x15 check */
        /* {0x08 ~ 0x11, 0x14 ~ 0x1c, 0x1e} = 0 */
        for (i = 0x08; i <= 0x1E; ++i) {
            if (0 != otp_buf[i]) {
                break;
            }

            if (0x11 == i) {
                i = 0x13;
            } else if (0x1C == i) {
                i = 0x1D;
            }
        }

        if (0x1E < i) {
            GF_LOGI(LOG_TAG "[%s] FT didn't be written in OTP", __func__);
            break;
        }

        if ((((otp_buf[0x08] & 0x3E) >> 1) == 16)
                && ((((otp_buf[0x09] & 0x03) << 2) + ((otp_buf[0x08] & 0xC0) >> 6)) < 4)) {
            GF_LOGI(LOG_TAG "[%s] sensor test information didn't be written in OTP", __func__);
            break;
        }

        if ((0 != otp_buf[0x14]) && (0 != otp_buf[0x15])) {
            GF_LOGI(LOG_TAG "[%s] 0x14 and 0x15 check success, both non-zero", __func__);
            break;
        }

        err = GF_ERROR_SENSOR_TEST_FAILED;
        GF_LOGE(LOG_TAG "[%s] otp check failed, otp[0x14]=0x%02X, otp[0x15]=0x%02X", __func__,
                otp_buf[0x14], otp_buf[0x15]);
    } while (0);

    FUNC_EXIT(err);

    return err;
}

gf_error_t milan_kjh_check_ft_test_info(uint8_t* otp_buf) {
    gf_error_t err = GF_SUCCESS;

    FUNC_ENTER();

    do {
        uint32_t i = 0;

        if (NULL == otp_buf) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter", __func__);
            break;
        }

        /* FT not be wroten, 0x14 and 0x15 check */
        /* {0x08 ~ 0x11, 0x1e} = 0 */
        for (i = 0x08; i <= 0x1E; ++i) {
            if (0 != otp_buf[i]) {
                break;
            }

            if (0x11 == i) {
                i = 0x1D;
            }
        }

        if (0x1E < i) {
            GF_LOGI(LOG_TAG "[%s] FT didn't be written in OTP", __func__);
            break;
        }

        if ((0 != otp_buf[0x14]) && (0 != otp_buf[0x15])) {
            GF_LOGI(LOG_TAG "[%s] 0x14 and 0x15 check success, both non-zero", __func__);
            break;
        }

        err = GF_ERROR_SENSOR_TEST_FAILED;
        GF_LOGE(LOG_TAG "[%s] otp check failed, otp[0x14]=0x%02X, otp[0x15]=0x%02X", __func__,
                otp_buf[0x14], otp_buf[0x15]);
    } while (0);

    FUNC_EXIT(err);

    return err;
}
