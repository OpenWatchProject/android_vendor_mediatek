/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include "gf_milan_a_series_otp.h"
#include "gf_tee_internal_api.h"
#include "gf_common.h"
#include "gf_sensor.h"
#include "gf_milan_a_series_regs.h"
#include "cpl_string.h"
#include "cpl_memory.h"
#include "gf_milan_a_series_bus.h"
#include "gf_milan_a_series_common.h"

/*********************************************macro *********************************/
#define SIZE_CODE (5 * 1024)
#define SIZE_CONFIG 256
#define SIZE_HEADER 12
#define CP_OTP_NUM 12
#define MT_OTP_NUM 12

#define GF11_PATCH_LEN 512
#define GF11_PATCH_WITH_ID_LEN 524
#define MASK_ID_REG_ADDR 0x0603

#define OSC80M_OTP_ADD     0x12
#define OSC80M_CONFIG_ADD  150

#define OSC1M_OTP_ADD      0x13
#define OSC1M_CONFIG_ADD   151

#define GSC_LED0_OTP_ADD      0x18
#define GSC_LED0_CONFIG_ADD   124

#define GSC_BASE_L_OTP_ADD    0x19
#define GSC_BASE_H_OTP_ADD    0x1A
#define GSC_BASE_CONFIG_ADD   112

#define HBD_CANCEL_OTP_ADD    0x1B
#define HBD_CANCEL_CONFIG_ADD 134
#define HBD_STEP_CONFIG_ADD   140

#define HBD_LED0_OTP_ADD    0x1C
#define HBD_LED0_CONFIG_ADD 142

#define TEMPER_OTP_ADD     0x34
#define TEMPER_CONFIG_ADD  152

#define DAC_OTP_BASE     0x20
#define DAC_OTP_SIZE     21

#define PIXEL_KEY_VCMCAL_ADDR_START 180
#define PIXEL_KEY_VCMCAL_ADDR_END 190

#define VERSION_OTP_ADD 0x2D
#define VENDOR_ID_ADDR   0x14

#define SENSOR_OTP_TYPE_OFFSET 0x39
#define SENSOR_OTP_TYPE_STANDARD 0x25

#define LOG_TAG "[gf_milan_a_series_otp] "

/*********************************************module variable *********************************/
/* config offset from document */
static uint8_t dac_cfg_offset_table[DAC_OTP_SIZE] = { 86, 87, 88, 89, 91, 92, 93, 94, 95, 96, 97,
        98, 99, 100, 90, 104, 105, 106, 107, 108, 109 };
extern uint8_t otp_crc[MILAN_A_SERIES_OTP_BUF_LEN];
static uint8_t CRC_OFFSET = 0x1F;
static uint8_t HV_LV_SWITCH_OFFSET = 0x3f;

// X^8 + X^2 + X^1 + 1
static const uint8_t CRC8_TAB[256] = { 0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F,
        0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48,
        0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
        0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, 0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82,
        0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC,
        0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE, 0xAB,
        0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E,
        0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59,
        0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E,
        0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, 0xF9,
        0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
        0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43,
        0x44, 0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A,
        0x33, 0x34, 0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A,
        0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F,
        0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98,
        0x9F, 0x8A, 0x8D, 0x84, 0x83, 0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1,
        0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3 };

static gf_sensor_type_t g_sensor_type = GF_SENSOR_5206;

/*********************************************implementation *********************************/
static uint8_t milan_a_series_otp_get_crc8(uint8_t *ucPtr, uint8_t ucLen) {
    uint8_t ucIndex;
    uint8_t ucCRC8 = 0;
    uint8_t i = 0;

    for (i = 0; i < ucLen; i++) {
        ucIndex = ucCRC8 ^ (*(ucPtr + i));
        ucCRC8 = CRC8_TAB[ucIndex];
    }

    return (~ucCRC8);
}

uint16_t milan_a_series_otp_get_checksum16(const uint8_t * usBuf,  const uint16_t usLen) {
    uint16_t ii;
    uint16_t usCheckSum;
    uint16_t tmp_cfg[MILAN_A_SERIES_CONFIG_MAX_LEN] = {0};
    usCheckSum = 0xA5A5;

    cpl_memcpy(tmp_cfg, usBuf, MILAN_A_SERIES_CONFIG_MAX_LEN);
    for (ii = 0; ii < usLen; ii++) {
        usCheckSum = usCheckSum + tmp_cfg[ii];
    }

    usCheckSum = 0 - usCheckSum;
    return usCheckSum;
}

/**
 * @brief
 *  update clock and wdt
 */
void milan_a_series_clock_otp_update(uint8_t *otp_buffer, uint8_t *cfg_data) {

    cfg_data[OSC80M_CONFIG_ADD] = otp_buffer[OSC80M_OTP_ADD];
    cfg_data[OSC1M_CONFIG_ADD] = otp_buffer[OSC1M_OTP_ADD];
}
/*
 static void milan_a_series_vbg_otp_update(uint8_t *otp_buffer,uint8_t *cfg_data)
 {
 GF_LOGD(LOG_TAG "[%s] enter ", __func__);

 if (otp_buffer[VBG_OTP_OFFSET] & 0x10) {
 GF_LOGD(LOG_TAG "[%s] vbg  0x%x ", __func__, otp_buffer[VBG_OTP_OFFSET]);
 cfg_data[VBG_CFG_OFFSET] = (otp_buffer[VBG_OTP_OFFSET] & 0x0F) << 1;
 }
 }
 */

/*
 * @brief
 *  update dac & R
 */
void milan_a_series_dac_otp_update(uint8_t *otp_buffer, uint8_t *cfg_data, uint8_t type) {
    int index;
    int lub_otp_value;
    int negtive;
    int high_flag;
    switch (type) {
        case 0: {
            for (index = 0; index < DAC_OTP_SIZE; index++) {
                cfg_data[dac_cfg_offset_table[index]] = otp_buffer[DAC_OTP_BASE + index];
            }
        }
            break;
        case 1: {
            if ((0 == ((otp_buffer[VERSION_OTP_ADD] & 0x70) >> 4)) && (otp_buffer[0x3F])) {
                negtive = otp_buffer[0x17];
                for (index = 0; index < DAC_OTP_SIZE; index++) {
                    if (index == 14) { //pixel key dac
                        cfg_data[dac_cfg_offset_table[index]] = otp_buffer[0x35];
                    }
                    if (index < 4) { //\u7b2c\u4e00\u7ec4DAC
                        cfg_data[dac_cfg_offset_table[index]] = otp_buffer[0x36 + index];
                    } else if (index > 4 && index < 13) { //\u7b2c2\u30013\u7ec4DAC

                        lub_otp_value = otp_buffer[0x3A + (index - 5) / 2];
                        high_flag = (index - 1) % 2;
                        if (negtive & (1 << (index - 5))) {
                            cfg_data[dac_cfg_offset_table[index]] =
                                    cfg_data[dac_cfg_offset_table[(index - 1) % 4]]
                                            - ((lub_otp_value >> (4 * high_flag)) & 0xF);
                        } else {
                            cfg_data[dac_cfg_offset_table[index]] =
                                    cfg_data[dac_cfg_offset_table[(index - 1) % 4]]
                                            + ((lub_otp_value >> (4 * high_flag)) & 0xF);
                        }
                    }
                }

                cfg_data[dac_cfg_offset_table[15]] = 0x09 | ((otp_buffer[0x3E] & 0x0F) << 4);
                cfg_data[dac_cfg_offset_table[16]] = ((otp_buffer[0x3E] & 0x0F) << 4)
                        | (otp_buffer[0x3E] & 0x0F);
                cfg_data[dac_cfg_offset_table[17]] = ((otp_buffer[0x3F] & 0x0F) << 4)
                        | (otp_buffer[0x3E] & 0x0F);
                cfg_data[dac_cfg_offset_table[18]] = ((otp_buffer[0x3F] & 0x0F) << 4)
                        | (otp_buffer[0x3F] & 0x0F);
                if (otp_buffer[0x3E] & 0x10) {
                    cfg_data[dac_cfg_offset_table[19]] = (0xF << 4) | (otp_buffer[0x3F] & 0x0F);
                } else {
                    cfg_data[dac_cfg_offset_table[19]] = (otp_buffer[0x3F] & 0x0F);
                }
                if (otp_buffer[0x3F] & 0x10) {
                    cfg_data[dac_cfg_offset_table[20]] = 0x0F;
                } else {
                    cfg_data[dac_cfg_offset_table[20]] = 0;
                }
            } else {
                for (index = 0; index < DAC_OTP_SIZE; index++) {
                    cfg_data[dac_cfg_offset_table[index]] = otp_buffer[DAC_OTP_BASE + index];
                }
            }
        }
            break;
        case 2: {
            unsigned short PkVcmCal_L = 0;
            unsigned short PkVcmCal_H = 0;
            // get otp dac and image R
            for (index = 0; index < DAC_OTP_SIZE; index++) {
                if (index == DAC_OTP_SIZE - 6) { // vcm cal 0 bit 0-3
                    cfg_data[dac_cfg_offset_table[index]] = (otp_buffer[DAC_OTP_BASE + index] & 0x0F)
                            | (otp_buffer[DAC_OTP_BASE + index + 1] << 4);
                } else if (index == DAC_OTP_SIZE - 2) { // vcm cal 0 bit 4
                    cfg_data[dac_cfg_offset_table[index]] = (otp_buffer[DAC_OTP_BASE + index] & 0xEF)
                            | ((otp_buffer[DAC_OTP_BASE + index] & 0x20) >> 1);
                } else
                    cfg_data[dac_cfg_offset_table[index]] = otp_buffer[DAC_OTP_BASE + index];
            }
            // get otp pixel R
            PkVcmCal_L = (otp_buffer[0x2F] & 0xF0) >> 4;
            if ((otp_buffer[0x33] & 0x10)) {
                PkVcmCal_H = 0xF;
            }
            cfg_data[PIXEL_KEY_VCMCAL_ADDR_START + 4] = otp_buffer[0x2F];
            for (index = PIXEL_KEY_VCMCAL_ADDR_START + 5; index < PIXEL_KEY_VCMCAL_ADDR_END - 2;
                    index++) {
                cfg_data[index] = (PkVcmCal_L << 4) | PkVcmCal_L;
            }
            cfg_data[PIXEL_KEY_VCMCAL_ADDR_END - 2] = (PkVcmCal_H << 4) | PkVcmCal_L;
            cfg_data[PIXEL_KEY_VCMCAL_ADDR_END - 1] = PkVcmCal_H;
        }
            break;
        default:
            GF_LOGE(LOG_TAG "[%s] update datc fail", __func__);
            break;
    }
}

/*
 * @brief
 *  update hbd
 */
void milan_a_series_hbd_otp_update(uint8_t *otp_buffer, uint8_t *cfg_data,
        gf_milan_a_series_fw_cfg_t *fw_cfg) {
    uint32_t data;

    //for gsc led0
    data = otp_buffer[GSC_LED0_OTP_ADD] | 0x3000;
    cfg_data[GSC_LED0_CONFIG_ADD] = (uint8_t) data;
    cfg_data[GSC_LED0_CONFIG_ADD + 1] = data >> 8;

    //for gsc base
    data = (otp_buffer[GSC_BASE_H_OTP_ADD] << 8) | otp_buffer[GSC_BASE_L_OTP_ADD];
    cfg_data[GSC_BASE_CONFIG_ADD] = (uint8_t) data;
    cfg_data[GSC_BASE_CONFIG_ADD + 1] = data >> 8;

    //for hbd cancel step
    data = 0x2400 | ((otp_buffer[HBD_CANCEL_OTP_ADD] & 0x60) >> 1);
    cfg_data[HBD_STEP_CONFIG_ADD] = (uint8_t) data;
    cfg_data[HBD_STEP_CONFIG_ADD + 1] = data >> 8;

    //for hbd pixel cancel
    data = 0xFC60 | ((otp_buffer[HBD_CANCEL_OTP_ADD] & 0x1f));
    cfg_data[HBD_CANCEL_CONFIG_ADD] = (uint8_t) data;
    cfg_data[HBD_CANCEL_CONFIG_ADD + 1] = data >> 8;

    if (fw_cfg->otp_version > 1) {
        data = 2248 - otp_buffer[HBD_LED0_OTP_ADD] - otp_buffer[HBD_LED0_OTP_ADD];
        /** use in hbd buf **/
        fw_cfg->hbd_version = 0xAA00;
        fw_cfg->hbd_otp_base = data;

    } else {
        //for hbd led0
        data = otp_buffer[HBD_LED0_OTP_ADD];

        fw_cfg->hbd_version = 0;
        fw_cfg->hbd_otp_base = 0;
    }

}

/**
 *@brief
 *  update Temper base
 */
void milan_a_series_temperature_otp_update(uint8_t *otp_buffer, uint8_t *config_buffer) {
    config_buffer[TEMPER_CONFIG_ADD] = otp_buffer[TEMPER_OTP_ADD] >> 4;
}

/**
 * @brief
 *  get sensor type
 */
gf_error_t gf_milan_a_series_init_sensor_type(uint8_t* otp_buffer) {
    gf_error_t ret = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (NULL == otp_buffer) {
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        GF_LOGD(LOG_TAG "[%s] chip_type:0x%x,0x%x", __func__,otp_buffer[GF_MILAN_A_SERIES_OTP_CHIP_TYPE_HIGH],otp_buffer[GF_MILAN_A_SERIES_OTP_CHIP_TYPE_LOW]);
        if (0x80 != otp_buffer[GF_MILAN_A_SERIES_OTP_CHIP_TYPE_HIGH]) {
            ret = GF_ERROR_UNSUPPORT_SENSOR;
            break;
        }

        switch (otp_buffer[GF_MILAN_A_SERIES_OTP_CHIP_TYPE_LOW]) {
            case 0x10:
                g_sensor_type = GF_SENSOR_5206;
                break;
            case 0x20:
                g_sensor_type = GF_SENSOR_5216;
                break;
            case 0x30:
                g_sensor_type = GF_SENSOR_5208;
                break;
            case 0x40:
                g_sensor_type = GF_SENSOR_5218;
                break;
            case 0x22:
                g_sensor_type = GF_SENSOR_GX556;
                break;
            default:
                //consider IC_TYPE 0x802X as GF5216
                if ((otp_buffer[GF_MILAN_A_SERIES_OTP_CHIP_TYPE_LOW] & 0xF0) == 0x20) {
                    g_sensor_type = GF_SENSOR_5216;
                    break;
                }

                ret = GF_ERROR_UNSUPPORT_SENSOR;
                break;
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit,ret = %d", __func__,ret);
    return ret;
}

/*
 * using user mode to read otp info
 */
static gf_error_t milan_a_series_get_otp_info(uint8_t *buf) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t otp_info_buffer[GF_MILAN_A_SERIES_OTP_INFO_LENGTH] = {0};
    uint8_t *otp_info = otp_info_buffer;
    uint16_t addr_offset = 0x00;
    uint16_t offset = 0x00;
    uint16_t control_value_1 = 0x0;
    uint16_t control_value_2 = 0x0;
    uint16_t read_value = 0x0;
    uint16_t control_reg = 0;

    GF_LOGD(LOG_TAG "[%s] enter ", __func__);

    do {
        /* read data*/
        for (offset = 0; offset <  GF_MILAN_A_SERIES_OTP_INFO_LENGTH; offset++) {

            if (offset < 0x20) {
                //OTP0
                addr_offset = offset;
                control_value_1 = 0x0;
                control_value_2 = 0x2;
                control_reg = GF_MILAN_A_SERIES_OTP_INFO_OTP0_ADDR;
            } else {
                //OTP1
                addr_offset = offset - 0x20;
                control_value_1 = 0x0;
                control_value_2 = 0x200;
                control_reg = GF_MILAN_A_SERIES_OTP_INFO_OTP1_ADDR;
            }

            if (offset < 0x20) {
                ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_OTP_INFO_CONTROL, control_value_1);
                if (GF_SUCCESS != ret) {
                    GF_LOGE(LOG_TAG "[%s] :write 0x%x failed, ret = %d.", __func__, GF_MILAN_A_SERIES_OTP_INFO_CONTROL,ret);
                    break;
                }
            }

           // ret = gf_milan_a_series_secspi_read_word(control_reg, &read_value);
           // if (GF_SUCCESS != ret) {
           //     GF_LOGE(LOG_TAG "[%s] :read 0x%x failed.", __func__,control_reg);
          //      break;
          //  }

            read_value = (addr_offset & 0x1F) << 8;

            //addr -->> (0x4302/0x4308) high
            ret = gf_milan_a_series_secspi_write_word(control_reg, read_value);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :write 0x%x failed, offset:%d,ret = %d.", __func__, control_reg, read_value, ret);
                break;
            }

            ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_OTP_INFO_CONTROL, control_value_2);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :write 0x%x failed, ret = %d.", __func__, GF_MILAN_A_SERIES_OTP_INFO_CONTROL ,ret);
                break;
            }

            ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_OTP_INFO_CONTROL, control_value_1);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :write 0x%x failed, ret = %d.", __func__, GF_MILAN_A_SERIES_OTP_INFO_CONTROL,ret);
                break;
            }

            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_OTP_INFO_DATA, &read_value);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :read otp info failed.", __func__);
                break;
            }
            if(offset < 0x20) {
                otp_info[offset] = (uint8_t)(read_value & 0x00FF);
                otp_crc[offset] = otp_info[offset];
            } else {
                otp_info[offset] = (uint8_t)((read_value & 0xFF00) >> 8);
                otp_crc[offset - 1] = otp_info[offset];
            }
            GF_LOGI(LOG_TAG "[%s]org 0x%x, otp_info[%d]:%x.", __func__,read_value,offset,otp_info[offset]);
        }

        if (GF_SUCCESS == ret) {
            cpl_memcpy(buf, otp_info_buffer, GF_MILAN_A_SERIES_OTP_INFO_LENGTH);
        }
    }while(0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d.", __func__, ret);

    return ret;
}

gf_sensor_type_t gf_milan_a_series_get_sensor_type(void) {
    return g_sensor_type;
}


gf_error_t gf_milan_a_series_otp_check_crc(uint8_t *otp_buf, uint8_t* crc_buf) {
    gf_error_t err = GF_SUCCESS;
    uint8_t crc = 0;
    uint8_t sensor_otp_buf[GF_MILAN_A_SERIES_OTP_INFO_LENGTH] = { 0 };
    uint32_t i = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if (otp_buf[SENSOR_OTP_TYPE_OFFSET] == SENSOR_OTP_TYPE_STANDARD) {
            GF_LOGD(LOG_TAG "[%s] standar sensor otp[0x39] 0x%x.", __func__, otp_buf[SENSOR_OTP_TYPE_OFFSET]);
            break;
        }

        crc = milan_a_series_otp_get_crc8(crc_buf, GF_MILAN_A_SERIES_OTP_INFO_LENGTH - 1);

        //HV & LV compatible
        if(otp_buf[HV_LV_SWITCH_OFFSET] == 0xFF){
            GF_LOGD(LOG_TAG "[%s] not check otp crc for special LV sensor HV_LV_SWITCH_OFFSET[%d]", __func__, otp_buf[HV_LV_SWITCH_OFFSET]);
            break;
        }

        if (otp_buf[CRC_OFFSET] == crc) {
            GF_LOGD(LOG_TAG "[%s] otp crc check passed, read crc 0x%x calculator 0x%x.", __func__, otp_buf[0x1F], crc);
            break;
        } else {
            GF_LOGE(LOG_TAG "[%s] otp crc check failed, read crc 0x%x calculator 0x%x.",
                    __func__, otp_buf[0x1F], crc);
        }

        //add sensor_type 2 crc check
        cpl_memcpy(sensor_otp_buf, crc_buf, GF_MILAN_A_SERIES_OTP_INFO_LENGTH);

        sensor_otp_buf[0x17] = 0;
        for (i = 0x35; i <= 0x3F; i++) {
            sensor_otp_buf[i - 1] = 0;
        }

        crc = milan_a_series_otp_get_crc8(sensor_otp_buf, GF_MILAN_A_SERIES_OTP_INFO_LENGTH - 1);
        if (otp_buf[CRC_OFFSET] == crc) {
            GF_LOGI(LOG_TAG "[%s] sensor type 2 otp crc check passed, read crc 0x%x calculator 0x%x.",
                    __func__, otp_buf[0x1F], crc);
            break;
        }

        //add workaround for Milan C 0x16 otp error
        sensor_otp_buf[0x16] = 0;
        crc = milan_a_series_otp_get_crc8(sensor_otp_buf,
                                          GF_MILAN_A_SERIES_OTP_INFO_LENGTH - 1);
        if (otp_buf[CRC_OFFSET] == crc) {
            GF_LOGI(LOG_TAG "[%s] sensor type 2(0x16 to 0) otp crc check passed, read crc 0x%x calculator 0x%x.",
                    __func__, otp_buf[0x1F], crc);
            break;
        }

        GF_LOGE(LOG_TAG "[%s] otp crc check failed, read crc 0x%x calculator 0x%x.", __func__,
                otp_buf[0x1F], crc);
        err = GF_ERROR_GET_OTP_INFO_FAILED;

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d.", __func__, err);
    return err;
}

/*
 * @brief
 *  update config from otp
 */
//uint16_t temp_config[SIZE_CONFIG] = {0};
/*uint8_t otp_buffer[MILAN_A_SERIES_OTP_BUF_LEN] = {0};*/
gf_error_t gf_milan_a_series_read_chip_otp(uint8_t *otp_buf) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        err = milan_a_series_get_otp_info(otp_buf);
        if (GF_SUCCESS != err) {
            break;
        }

        err = gf_milan_a_series_otp_check_crc(otp_buf, otp_crc);
        if (err == GF_SUCCESS) {
            break;
        }

        GF_LOGE(LOG_TAG "[%s] failed to get otp info err:%d.", __func__, err);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d.", __func__, err);
    return err;
}

gf_error_t gf_milan_a_series_otp_get_sensor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (NULL == buf || buf_len < GF_SENSOR_ID_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        cpl_memcpy(buf, otp_crc, GF_SENSOR_ID_LEN);

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err: %d", __func__, err);
    return err;
}

gf_error_t gf_milan_a_series_otp_get_vendor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (NULL == buf || buf_len < GF_VENDOR_ID_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        cpl_memset(buf, 0x0, buf_len);

        cpl_memcpy(buf, &otp_crc[VENDOR_ID_ADDR], 3);

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err: %d", __func__, err);
    return err;
}

