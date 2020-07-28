/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "gf_vfw_milan_j_hv.h"
#include "milan_hv_crc8.h"
#include "gf_common.h"
#include "cpl_string.h"
#include "gf_log.h"
#include "milan_hv_regs.h"
#include "milan_hv_bus.h"

#define MILAN_J_HV_OTP_CRC_LEN 31

#define LOG_TAG "[gf_vfw_milan_j_hv]"

/* 0x00~0x18, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F.  31 Bytes */
uint8_t vfw_milan_j_hv_get_otp_data_crc(uint8_t* otp_buf) {
    uint8_t crc = 0;
    uint8_t temp_data[GF_SENSOR_OTP_INFO_LEN] = { 0 };

    cpl_memcpy(temp_data, otp_buf, 25);
    cpl_memcpy(&temp_data[25], &otp_buf[0x1A], 6);

    crc = gf_milan_hv_get_crc8(temp_data, MILAN_J_HV_OTP_CRC_LEN);

    return crc;
}

gf_error_t vfw_milan_j_hv_set_general_reg(void) {
    gf_error_t err = GF_SUCCESS;

    do {
        uint16_t reg_data = 0x2116;
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_PMU_CTRL, reg_data);
        if (GF_SUCCESS != err) {
            break;
        }

        reg_data = 0x030A;
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL5, reg_data);
        if (GF_SUCCESS != err) {
            break;
        }

        reg_data = 0x0903;
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_CLOCK_DIV_CTRL, reg_data);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t vfw_milan_j_hv_update_dac_to_regs(uint16_t dac_groups[]) {
    gf_error_t err = GF_SUCCESS;
    uint16_t reg_data = 0;
    FUNC_ENTER();

    do {
        if (NULL == dac_groups) {
            GF_LOGE(LOG_TAG "[%s] invalid params", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGI(LOG_TAG "[%s] new dac_groups[0]=0x%04X, dac_groups[1]=0x%04X, dac_groups[2]=0x%04X,"
                " dac_groups[3]=0x%04X", __func__, dac_groups[0], dac_groups[1], dac_groups[2],
                dac_groups[3]);

        err = gf_milan_hv_read_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL0, &reg_data);
        if (GF_SUCCESS != err) {
            break;
        }

        reg_data = ((reg_data & 0xe00f) | (dac_groups[0] << 4) | 0x8);

        // set dac_group[0]: reg[0220]
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_CTRL0, reg_data);
        if (GF_SUCCESS != err) {
            break;
        }
        // set dac_group[1]: reg[0236]
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_DAC_P2,
                dac_groups[1]);
        if (err != GF_SUCCESS) {
            break;
        }

        // set dac_group[2]: reg[0238]
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_DAC_P3,
                dac_groups[2]);
        if (err != GF_SUCCESS) {
            break;
        }

        // set dac_group[3]: reg[023a]
        err = gf_milan_hv_write_word_with_endian_exchange(MILAN_HV_REG_RG_PIXEL_DAC_P4,
                dac_groups[3]);
    } while (0);

    FUNC_EXIT(err);
    return err;
}
