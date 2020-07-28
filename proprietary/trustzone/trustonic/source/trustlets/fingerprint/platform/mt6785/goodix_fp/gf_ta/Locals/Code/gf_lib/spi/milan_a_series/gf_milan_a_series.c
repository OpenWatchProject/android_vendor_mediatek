/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <string.h>

#include "gf_tee_internal_api.h"
#include "gf_common.h"
#include "gf_error.h"
#include "gf_sensor.h"
#include "GF11_Rawdata_InvCipher.h"
#include "gf_milan_a_series_otp.h"
#include "gf_spi_common.h"
#include "gf_milan_a_series_bus.h"
#include "gf_milan_a_series.h"
#include "gf_status.h"
#include "cpl_string.h"
#include "cpl_memory.h"
#include "gf_milan_a_series_regs.h"
#include "gf_milan_a_series_common.h"
#include "gf_secure_object.h"

#if defined(GF5206)
/***************************************
* MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV
* 1: fw open fpc_key, close nav
* 0: fw open nav, close fpc_key
* default 1,if custome need nav, then set 0
* **************************************/
#define MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV 1
#if (MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV == 1)
#include "gf_milan_a_fw_cfg_sensor_type1.h"
#include "gf_milan_a_fw_cfg_sensor_type2.h"
#include "gf_milan_a_fw_cfg_sensor_type3.h"
#include "gf_milan_a_fw_cfg_sensor_type4.h"
#include "gf_milan_a_fw_cfg_sensor_type5.h"
#else
#include "gf_milan_a_fw_cfg_sensor_type6.h"
#include "gf_milan_a_fw_cfg_sensor_type7.h"
#include "gf_milan_a_fw_cfg_sensor_type8.h"
#include "gf_milan_a_fw_cfg_sensor_type9.h"
#endif

#elif defined(GF5216)
#include "gf_milan_b_fw_cfg_sensor_type1.h"
#include "gf_milan_b_fw_cfg_sensor_type2.h"
#include "gf_milan_b_fw_cfg_sensor_type3.h"
#include "gf_milan_b_fw_cfg_sensor_type4.h"
#include "gf_milan_b_fw_cfg_sensor_type5.h"
#include "gf_milan_b_fw_cfg_sensor_type6.h"
#include "gf_milan_b_fw_cfg_sensor_type7.h"
#include "gf_milan_b_fw_cfg_sensor_type8.h"
#include "gf_milan_b_fw_cfg_sensor_type9.h"
#elif defined(GF5208)
#include "gf_milan_c_fw_cfg_sensor_type1.h"
#include "gf_milan_c_fw_cfg_sensor_type2.h"
#include "gf_milan_c_fw_cfg_sensor_type3.h"
#include "gf_milan_c_fw_cfg_sensor_type4.h"
#include "gf_milan_c_fw_cfg_sensor_type5.h"
#endif

#define LOG_TAG "[gf_milan_a_series] "

#define GF_MILAN_A_SERIES_SENSOR_STATUS_LEN 12

#define GF_MILAN_A_SERIES_IRQ_NONE                0x00
#define GF_MILAN_A_SERIES_IRQ_FW_ERR              0xE0
#define GF_MILAN_A_SERIES_IRQ_RESET               0xE4
#define GF_MILAN_A_SERIES_IRQ_CONFIG_ERR          0xE2
#define GF_MILAN_A_SERIES_IRQ_ESD_ERR         0xE3
#define GF_MILAN_A_SERIES_IRQ_FW_RESET        0xE1
#define GF_MILAN_A_SERIES_IRQ_KEY             0xB0
#define GF_MILAN_A_SERIES_IRQ_NAV_LEFT        0xB1
#define GF_MILAN_A_SERIES_IRQ_NAV_RIGHT       0xB2
#define GF_MILAN_A_SERIES_IRQ_NAV_UP     0xB3
#define GF_MILAN_A_SERIES_IRQ_NAV_DOWN     0xB4
#define GF_MILAN_A_SERIES_IRQ_NAV       0xC2

#define GF_MILAN_A_SERIES_IRQ_IMAGE           0xC0
#define GF_MILAN_A_SERIES_IRQ_GSC             0xC1    // GSC irq
#define GF_MILAN_A_SERIES_IRQ_HBD             0xA0    // HBD irq
#define GF_MILAN_A_SERIES_IRQ_UPDATE_BASE     0xD0

#define GF_MILAN_A_SERIES_HBD_BUFF_SIZE               48
#define GF_MILAN_A_SERIES_HBD_BUFF1_SIZE               24
#define GF_MILAN_A_SERIES_HBD_BUFF2_SIZE               24

#define GF_MILAN_A_SERIES_HBD_BUFFER1_ADDR            0x0942
#define GF_MILAN_A_SERIES_HBD_BUFFER2_ADDR            0x095A

#define GF_MILAN_A_SERIES_HBD_BUFFER1_VALID           0x80
#define GF_MILAN_A_SERIES_HBD_BUFFER2_VALID           0x40
#define GF_MILAN_A_SERIES_HBD_BUFFERS_VALID           0xC0

#define GF_MILAN_A_SERIES_HBD_NOP_DATA                0x00
#define GF_MILAN_A_SERIES_HBD_GSC_DATA                0x01
#define GF_MILAN_A_SERIES_HBD_HBD_BUFFER1_DATA        0x02
#define GF_MILAN_A_SERIES_HBD_HBD_BUFFER2_DATA        0x03
#define GF_MILAN_A_SERIES_VCM_CAL_SIZE        0x8
#define GF_MILAN_A_SERIES_DAC_OFFSET_LEN      0x8

//fpc_key or ring_key bit0-bit1 Back/Menu Key  bit2 Ring Key
#define GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY         16
#define GF_MILAN_A_SERIES_ENABLE_SLEEP_KEY            198

//stable judge
#define GF_MILAN_A_SERIES_JUDGE_STABLE_CONFIG_VALUE           0x0080
#define GF_MILAN_A_SERIES_NOT_JUDGE_STABLE_CONFIG_VALUE           0xFFFF
#define GF_MILAN_A_SERIES_STABLE_JUDGE_CONFIG_INDEX           84

//key down reissue
#define GF_MILAN_A_SERIES_REISSUE_DOWN_CONFIG_VALUE          0x01
#define GF_MILAN_A_SERIES_NOT_REISSUE_DOWN_CONFIG_VALUE          0x00
#define GF_MILAN_A_SERIES_DOWN_REISSUE_CONFIG_INDEX           160

//pixel open test
#define GF_MILAN_A_SERIES_PIXEL_TEST_CONFIG_SIZE      (512)
#define GF_MILAN_A_SERIES_SIZE_CONFIG (256)

//key
#define GF_MILAN_A_SERIES_HOME_KEY_MASK    (1 << 1)
#define GF_MILAN_A_SERIES_HOME_KEY_STA     (1 << 0)
#define GF_MILAN_A_SERIES_MENU_KEY_MASK    (1 << 3)
#define GF_MILAN_A_SERIES_MENU_KEY_STA     (1 << 2)
#define GF_MILAN_A_SERIES_BACK_KEY_MASK    (1 << 5)
#define GF_MILAN_A_SERIES_BACK_KEY_STA     (1 << 4)

#define GF_MILAN_A_SERIES_CLEAR_REG_OFFSET  0  // g_sensor_status use
#define GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET  1
#define GF_MILAN_A_SERIES_KEY_STATUS_OFFSET  5
#define GF_MILAN_A_SERIES_HBD_STATUS_OFFSET  7

#define USE_OTP_VENDOR_ID
#define GF_MILAN_A_VENDOR_OPT1_MASK ((0x1 << 0) | (0x1 << 1) | (0x1 << 7) | (0x1 << 8) | (0x1 << 11))
#define GF_MILAN_A_VENDOR_OPT1_SEL0 ((0x1 << 0) | (0x0 << 1) | (0x0 << 7) | (0x0 << 8))
#define GF_MILAN_A_VENDOR_OPT1_SEL1 ((0x1 << 0) | (0x1 << 1) | (0x0 << 7) | (0x0 << 8))
#define GF_MILAN_A_VENDOR_OPT1_SEL2 ((0x0 << 0) | (0x0 << 1) | (0x1 << 7) | (0x0 << 8))
#define GF_MILAN_A_VENDOR_OPT1_SEL3 ((0x0 << 0) | (0x0 << 1) | (0x1 << 7) | (0x1 << 8))

#define GF_MILAN_A_VENDOR_OPT2_MASK ((0x1 << 0) | (0x1 << 1) | (0x1 << 2) | (0x1 << 3) | (0x1 << 4))
#define GF_MILAN_A_VENDOR_OPT2_SEL0 ((0x1 << 0) | (0x0 << 1) | (0x0 << 2) | (0x0 << 3))
#define GF_MILAN_A_VENDOR_OPT2_SEL1 ((0x1 << 0) | (0x1 << 1) | (0x0 << 2) | (0x0 << 3))
#define GF_MILAN_A_VENDOR_OPT2_SEL2 ((0x0 << 0) | (0x0 << 1) | (0x1 << 2) | (0x0 << 3))
#define GF_MILAN_A_VENDOR_OPT2_SEL3 ((0x0 << 0) | (0x0 << 1) | (0x1 << 2) | (0x1 << 3))

#define MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_BYTE(config_array, register_addr, value) \
{   \
uint32_t offset = register_addr - GF_MILAN_A_SERIES_CFG_ADDR;  \
{   \
config_array[offset] = (uint8_t)((value) & 0x00FF);  \
}   \
}

#define MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_WORD(config_array, register_addr, value) \
{   \
uint32_t offset = register_addr - GF_MILAN_A_SERIES_CFG_ADDR;  \
{   \
config_array[offset] = (uint8_t)((value) & 0x00FF);  \
config_array[offset + 1] = (uint8_t)((value >> 8) & 0x00FF);    \
}   \
}

static uint16_t nav_frame_count = 0;
const uint8_t g_GF52x6_navi_rawid[15] =
{
    //prev 8 rows
    1, 5, 9, 13, 17, 21, 25, 29,
    //last 7 rows
    33, 37, 41, 45, 49, 53, 57
};

const uint8_t g_GF52x8_navi_rawid[16] =
{
    //middle 8 rows
    33, 37, 41, 45, 49, 53, 57, 61,
    //last 4 rows
    65, 69, 73, 77,
    //first 4 rows
    17, 21, 25, 29
};
//end for navigation

//start for one shot dma
static const uint8_t g_milan_c_row_array[] = { 3, 7, 2, 6, 1, 5, 0, 4, 99, 10, 14, 9, 13, 8, 12, 99,
        18, 22, 17, 21, 16, 20, 99, 26, 30, 25, 29, 24, 28, 99, 11, 15, 19, 23, 27,
        31, //scan one more time
        99, 34, 38, 33, 37, 32, 36, 99, 42, 46, 41, 45, 40, 44, 111, 51, 55, 50, 54, 49, 53, 48, 52,
        99, 58, 62, 57, 61, 56, 60, 99, 35, 39, 43, 47, 59,
        63, //
        99, 66, 70, 65, 69, 64, 68, 99, 74, 78, 73, 77, 72, 76, 99, 82, 86, 81, 85, 80, 84, 99, 67,
        71, 75, 79, 83, 87 };

static const uint8_t g_milan_c_lost_row_array[] = { 48 };

static const uint8_t g_milan_a_row_array[] = { 99, 3, 7, 11, 15, 99, 2, 6, 10, 14, 99, 1, 5, 9, 13,
        99, 0, 4, 8, 12, 99, 19, 23, 27, 31, 99, 18, 22, 26, 30, 99, 17, 21, 25, 29, 99, 16, 20, 24,
        28, 99, 35, 39, 43, 47, 99, 34, 38, 42, 46, 99, 33, 37, 41, 45, 99, 32, 36, 40, 44, 99, 51,
        55, 59, 99, 50, 54, 58, 99, 49, 53, 57, 99, 48, 52, 56 };

static gf_milan_a_series_pixel_test_t gf_milan_a_series_pixel_test_chip_info = { 0 };
static uint8_t g_pixel_test_cfg_data[GF_MILAN_A_SERIES_PIXEL_TEST_CONFIG_SIZE] = { 0 };

/**  data len 24
 *        valid data len +  data area  +  reserve
 *       =  4 +             20      +       4
 *
 *       buf1 : untouch data
 *       buf2 : touch data
 * **/
//uint8_t hbd_buf1[GF_MILAN_A_SERIES_HBD_BUFF1_SIZE] = { 0 };
//uint8_t hbd_buf2[GF_MILAN_A_SERIES_HBD_BUFF2_SIZE] = { 0 };
extern uint8_t hbd_buf1[GF_MILAN_A_SERIES_HBD_BUFF1_SIZE];
extern uint8_t hbd_buf2[GF_MILAN_A_SERIES_HBD_BUFF2_SIZE];
extern uint16_t gsc_base;
extern gf_spi_function_t g_spi_function;

uint8_t g_hbd_data_type = GF_MILAN_A_SERIES_HBD_NOP_DATA;

uint8_t gf_milan_a_series_hbd_raw_buf[GF_MILAN_A_SERIES_HBD_BUFF_SIZE * 3];

static gf_sensor_status_t g_sensor_status;
uint8_t vcm_cal[GF_MILAN_A_SERIES_VCM_CAL_SIZE + 1] = { 0 }; // +1  record otp temprature

//static uint8_t g_vendor_id = 0x00;
uint16_t g_gf_chip_id = 0;
uint8_t otp_crc[MILAN_A_SERIES_OTP_BUF_LEN] = { 0 };

#ifndef USE_OTP_VENDOR_ID
static uint16_t g_vendor_opt_map[2][4] = {
    {   GF_MILAN_A_VENDOR_OPT1_SEL0, GF_MILAN_A_VENDOR_OPT1_SEL1, GF_MILAN_A_VENDOR_OPT1_SEL2, GF_MILAN_A_VENDOR_OPT1_SEL3},
    {   GF_MILAN_A_VENDOR_OPT2_SEL0, GF_MILAN_A_VENDOR_OPT2_SEL1, GF_MILAN_A_VENDOR_OPT2_SEL2, GF_MILAN_A_VENDOR_OPT2_SEL3},
};

static uint8_t g_vendor_id_map[2][25] = {
    {
        [0] = 1,
        [1] = 4,
        [2] = 2,
        [3] = 3,
        [4] = 0,
    },
    {
        [4*5 + 4] = 3,
        [0*5 + 4] = 1,
        [2*5 + 4] = 7,
        [4*5 + 0] = 2,
        [0*5 + 0] = 0,
        [2*5 + 0] = 6,
        [4*5 + 2] = 5,
        [0*5 + 2] = 4,
        [2*5 + 2] = 8,
    },
};
#endif

static uint32_t g_sensor_ignore_irq_mask = 0;

static uint8_t g_first_get_image = 1;

static uint8_t g_otp_failed_retry = 0;

static gf_error_t test_set_hbd_cfg(void);
static gf_error_t milan_a_series_download_cfg(uint8_t *cfg_data);
static gf_error_t gf_milan_a_series_load_otp_info(uint8_t *otp_buf, uint32_t *otp_buf_len);
static gf_error_t gf_milan_a_series_save_otp_info(uint8_t *buf, uint32_t buf_len);
static gf_error_t gf_milan_a_series_print_pixel_key_value(void);

static milan_a_series_chip_handle_t g_milan_a_series_chip_handle = { 0 };

#define GF_SPI_SPEED_ONE_DMA    9
#if defined(GF5208)
#define RAW_READ_LEN      (GF_RAWDATA_LEN + GF_SPI_SPEED_ONE_DMA * 2000 + 200)
#elif defined(GF5206) || defined(GF5216)
#define RAW_READ_LEN      (GF_RAWDATA_LEN + GF_SPI_SPEED_ONE_DMA * 600 + 200)
#else
#define RAW_READ_LEN      GF_RAWDATA_LEN
#endif
static uint8_t g_rawdata[RAW_READ_LEN] = { 0 };

/*
 *  as compilor forbidden zero length array,
 *  we assume '99' as invalid line, as above row_array
 */
static const uint8_t g_milan_a_lost_row_array[] = { 99 };

static gf_error_t milan_a_series_init_configuration(milan_a_series_chip_handle_t *handle,
        gf_milan_a_series_sensor_otp_type_t sensor_otp_type, uint32_t otp_version) {

    gf_error_t err = GF_SUCCESS;
    uint8_t support_light_adjust = 0;
    gf_sensor_type_t type = GF_SENSOR_5206;

    GF_LOGI(LOG_TAG "[%s] enter, sensor_otp_type: %d, otp_version: %d", __func__, sensor_otp_type,
            otp_version);
    type = gf_milan_a_series_get_sensor_type();
    GF_LOGI(LOG_TAG "[%s] sensor ic type: %d", __func__, type);

    do {
        const milan_a_series_configuration_t *chip_configuration = NULL;

        if (NULL == handle) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        handle->feature.support_fast_acquire_image = 1;
        handle->feature.support_judge_finger_stable = 1;
        handle->feature.support_reissue_key_down = 1;
        handle->feature.support_stable_acquire_image = 1;
#if defined(GF5206)
        GF_LOGD(LOG_TAG "[%s] use Milan A fw & cfg ", __func__);
#if (MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV == 1)
        if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_1 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type1;
            g_sensor.config.support_fw_navigation = 1;
            handle->feature.support_fast_acquire_image = 0;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_2 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type2;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_3 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type3;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_4 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type4;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_5 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type5;
        } else {
            GF_LOGE(LOG_TAG "[%s] unknow sensor type", __func__);
        }
#else
        if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_6 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type6;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_7 == sensor_otp_type) {
           chip_configuration = &g_milan_config_type7;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_8 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type8;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_9 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type9;
        } else {
            GF_LOGE(LOG_TAG "[%s] unknow sensor type", __func__);
        }
#endif
        if (otp_version > 1) {
            support_light_adjust = 1;
        }
#elif defined(GF5216)
        GF_LOGD(LOG_TAG "[%s] use Milan B fw & cfg ", __func__);
        if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_1 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type1;
            g_sensor.config.support_fw_navigation = 1;
            handle->feature.support_fast_acquire_image = 0;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_2 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type2;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_3 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type3;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_4 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type4;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_5 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type5;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_6 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type6;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_7 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type7;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_8 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type8;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_9 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type9;
        }else {
            GF_LOGE(LOG_TAG "[%s] unknow sensor type", __func__);
        }
        //if chip type is GX556, use GX556 cfg
        if(type == GF_SENSOR_GX556){
            GF_LOGD(LOG_TAG "[%s] use GX556 cfg in Milan B", __func__);
            support_light_adjust = 1;
        }
#elif defined(GF5208)
        GF_LOGD(LOG_TAG "[%s] use Milan C fw & cfg", __func__);
        if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_1 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type1;
            g_sensor.config.support_fw_navigation = 1;
            g_sensor.config.support_merged_gsc = 0;
            handle->feature.support_judge_finger_stable = 0;
            handle->feature.support_fast_acquire_image = 0;
            handle->feature.support_stable_acquire_image = 0;
            handle->feature.support_reissue_key_down = 0;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_2 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type2;

            g_sensor.config.support_fw_navigation = 1;
            g_sensor.config.support_merged_gsc = 0;
            handle->feature.support_judge_finger_stable = 0;
            handle->feature.support_fast_acquire_image = 0;
            handle->feature.support_stable_acquire_image = 0;
            handle->feature.support_reissue_key_down = 0;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_3 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type3;
            if (otp_version > 1) {
                support_light_adjust = 1;
            }

            g_sensor.config.support_fw_navigation = 1;
            g_sensor.config.support_merged_gsc = 0;
            handle->feature.support_judge_finger_stable = 0;
            handle->feature.support_fast_acquire_image = 0;
            handle->feature.support_stable_acquire_image = 0;
            handle->feature.support_reissue_key_down = 0;
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_4 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type4;
            if (otp_version > 1) {
                support_light_adjust = 1;
            }
        } else if (GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_5 == sensor_otp_type) {
            chip_configuration = &g_milan_config_type5;
            if (otp_version > 1) {
                support_light_adjust = 1;
            }
        } else {
            GF_LOGE(LOG_TAG "[%s] unknow sensor type", __func__);
        }
#endif
        if (NULL == chip_configuration) {
            GF_LOGE(LOG_TAG "[%s] do not match fw & cfg for this sensor!", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        cpl_memcpy(&(handle->configuration), chip_configuration, sizeof(milan_a_series_configuration_t));

        g_sensor.fw_cfg.milan_a_series.cfg_data_len = MILAN_A_SERIES_CONFIG_MAX_LEN;
        g_sensor.fw_cfg.milan_a_series.fw_data_len = MILAN_A_SERIES_FW_MAX_LEN;
        if (1 == support_light_adjust) {
            GF_LOGD(LOG_TAG "[%s] sensor support light adjust.", __func__);
            g_sensor.fw_cfg.milan_a_series.cfg_data = handle->configuration.config.cfg_light_adjust;
        } else {
            g_sensor.fw_cfg.milan_a_series.cfg_data = handle->configuration.config.cfg;
        }
        g_sensor.fw_cfg.milan_a_series.fw_data = handle->configuration.config.fw;
        g_sensor.fw_cfg.milan_a_series.sensor_otp_type = sensor_otp_type;
    } while (0);
    GF_LOGD(LOG_TAG "[%s] support fw navigation = %d.", __func__, g_sensor.config.support_fw_navigation);
    GF_LOGD(LOG_TAG "[%s] exit", __func__);

    return err;
}

gf_error_t gf_milan_a_series_notify_fw_adc_reading_end(void) {

    gf_error_t ret = GF_SUCCESS;
    uint16_t data = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    //clear 0x8036 to 0.
    g_sensor_status.status[GF_MILAN_A_SERIES_CLEAR_REG_OFFSET] = 0;
    data = (g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET] << 8)
            | (g_sensor_status.status[GF_MILAN_A_SERIES_CLEAR_REG_OFFSET]);
    ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, data);

    return ret;
}

gf_error_t gf_milan_a_series_notify_fw_adc_reading_begin(void) {

    gf_error_t ret = GF_SUCCESS;
    uint16_t data = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    //clear 0x8037 to 0.
    g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET] = 0;
    data = (g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET] << 8)
            | (g_sensor_status.status[GF_MILAN_A_SERIES_CLEAR_REG_OFFSET]);
    ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, data);

    return ret;
}

static void gf_milan_a_series_revise_hbd_buf(uint8_t* buf, uint32_t length) {
    uint16_t tmp_reverse[GF_MILAN_A_SERIES_HBD_BUFF1_SIZE / 2] = { 0 };
    uint16_t data_len = 0;
    uint32_t i = 0;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    if ( NULL == buf || length < GF_MILAN_A_SERIES_HBD_BUFF1_SIZE) {
        GF_LOGE(LOG_TAG "[%s] :invalid input params!", __func__);
        return;
    }

    data_len = *((uint16_t*) ((void*) buf));
    if (data_len <= 0) {
        return;
    }

    for (i = 0; i < data_len; i++) {
        tmp_reverse[i] = *((uint16_t*) ((void*) buf) + i + 1);
    }

    //reverse origin buf
    for (i = 0; i < data_len; i++) {
        *((uint16_t*) ((void*) buf) + i + 1) = tmp_reverse[data_len - 1 - i];
    }

    GF_LOGD(LOG_TAG "[%s] exit.", __func__);
}

static gf_error_t gf_milan_a_series_get_hbd_data(void) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t mode = 0;
    uint8_t irq_status = 0;
    uint8_t hbd_status = 0;

    mode = g_sensor_status.mode;
    irq_status = g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET];
    hbd_status = g_sensor_status.status[GF_MILAN_A_SERIES_HBD_STATUS_OFFSET];

    GF_LOGI(LOG_TAG "[%s] :mode=0x%x, irq_status=0x%x, hbd_status=0x%x.", __func__, mode,
            irq_status, hbd_status);

    if (hbd_status == GF_MILAN_A_SERIES_HBD_BUFFER1_VALID) {
        GF_LOGD(LOG_TAG "[%s] :hbd data ready in buffer1.", __func__);
        cpl_memset(hbd_buf1, 0x00, sizeof(hbd_buf1));
        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_HBD_BUFFER1_ADDR, hbd_buf1,
                GF_MILAN_A_SERIES_HBD_BUFF1_SIZE);

        gf_milan_a_series_revise_hbd_buf(hbd_buf1, GF_MILAN_A_SERIES_HBD_BUFF1_SIZE);
        g_hbd_data_type = GF_MILAN_A_SERIES_HBD_HBD_BUFFER1_DATA;
    } else if (hbd_status == GF_MILAN_A_SERIES_HBD_BUFFER2_VALID) {
        GF_LOGD(LOG_TAG "[%s] :hbd data ready in buffer2.", __func__);
        cpl_memset(hbd_buf2, 0x00, sizeof(hbd_buf2));
        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_HBD_BUFFER2_ADDR, hbd_buf2,
                GF_MILAN_A_SERIES_HBD_BUFF2_SIZE);

        gf_milan_a_series_revise_hbd_buf(hbd_buf2, GF_MILAN_A_SERIES_HBD_BUFF2_SIZE);
        g_hbd_data_type = GF_MILAN_A_SERIES_HBD_HBD_BUFFER2_DATA;
    } else if (hbd_status == GF_MILAN_A_SERIES_HBD_BUFFERS_VALID) {
        GF_LOGD(LOG_TAG "[%s] :gsc data ready in buffer1&buffer2.", __func__);
        cpl_memset(hbd_buf1, 0x00, sizeof(hbd_buf1));
        cpl_memset(hbd_buf2, 0x00, sizeof(hbd_buf2));
        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_HBD_BUFFER1_ADDR, hbd_buf1,
                GF_MILAN_A_SERIES_HBD_BUFF1_SIZE);
        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_HBD_BUFFER2_ADDR, hbd_buf2,
                GF_MILAN_A_SERIES_HBD_BUFF2_SIZE);
        ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_GSC_BASE_ADDR, &gsc_base);

        gf_milan_a_series_revise_hbd_buf(hbd_buf1, GF_MILAN_A_SERIES_HBD_BUFF1_SIZE);
        gf_milan_a_series_revise_hbd_buf(hbd_buf2, GF_MILAN_A_SERIES_HBD_BUFF2_SIZE);
        g_hbd_data_type = GF_MILAN_A_SERIES_HBD_GSC_DATA;

    } else {
        GF_LOGE(LOG_TAG "[%s] :invalid hbd_status=0x%x.", __func__, hbd_status);
        return GF_ERROR_INVALID_DATA;
    }

    GF_LOGD(LOG_TAG "[%s] :g_hbd_data_type=0x%x.", __func__, g_hbd_data_type);

    return ret;
}

#define CRC_16_POLYNOMIALS       0x8005

static uint16_t get_crc_value_multi_dma(uint8_t *pchMsg, uint16_t wDataLen) {
    uint8_t i, chChar;
    uint16_t wCRC = 0xFFFF; //g_CRC_value;

    while (wDataLen--) {
        chChar = *pchMsg++;
        wCRC ^= (((uint16_t) chChar) << 8);

        for (i = 0; i < 8; i++) {
            if (wCRC & 0x8000) {
                wCRC = (wCRC << 1) ^ CRC_16_POLYNOMIALS;
            } else {
                wCRC <<= 1;
            }
        }
    }

    return wCRC;
}

static gf_error_t gf_milan_a_series_frame_decrypt_multi_dma(uint8_t *rawdata, uint16_t *out) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t line_offset = 0;
    uint32_t i = 0;
    uint16_t row_id = 0;
    uint16_t row_crc = 0;
    uint16_t cal_row_crc = 0;
    uint16_t rescan = 0;
    uint32_t size = 0;

    GF_LOGD(LOG_TAG "[%s] :ready to crc check and split raw data.", __func__);

    if ((NULL == rawdata) || (NULL == out)) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    // decrypt the Raw Data row by row
    for (i = 0; i < g_sensor.row; i++) {

        row_id = (rawdata[line_offset] << 8) + rawdata[line_offset + 1];
        row_id = 4 * (row_id / 4) + (4 - row_id % 4) - 1;

        // debug assert
        if (row_id >= g_sensor.row) {
            GF_LOGE(LOG_TAG "[%s] :invalid row id, usRowId=0x%x.", __func__, row_id);
            return GF_ERROR_INVALID_DATA;
        }

        // CRC check
        row_crc = (rawdata[line_offset + (g_sensor.col + 1) * 2] << 8)
                + rawdata[line_offset + (g_sensor.col + 1) * 2 + 1];

        cal_row_crc = get_crc_value_multi_dma(&rawdata[line_offset + 2], g_sensor.col * 2);

        if (row_crc != cal_row_crc) {
            GF_LOGE(LOG_TAG "[%s] :crc failed, row_id=0x%x, sensor send[0x%x], calculate[0x%x].",
                    __func__, row_id, row_crc, cal_row_crc);
            return GF_ERROR_INVALID_DATA;
        }

        cpl_memcpy(&out[row_id * g_sensor.col], &rawdata[line_offset + 2], g_sensor.col * 2);
        line_offset += ((g_sensor.col + 2) * 2);
    }

    //recombine data by rescan.
    rescan = g_sensor.fw_cfg.milan_a_series.cfg_data[78] + 1;
    GF_LOGD(LOG_TAG "[%s] :cfg rescan value: %d", __func__, rescan);
    size = g_sensor.row * g_sensor.col;
    endian_exchange((uint8_t *) out, size * sizeof(uint16_t));
    for (i = 0; i < size; i++) {
        out[i] = out[i] / rescan;
    }

    GF_LOGD(LOG_TAG "[%s] :raw data crc ok and split success.", __func__);
    return ret;
}

static uint16_t get_crc_value_one_dma(uint8_t *pchMsg, uint16_t wDataLen) {
    uint8_t i, chChar;
    uint16_t j;
    uint16_t wCRC = 0xFFFF;

    for (j = 0; j < wDataLen; j = j + 2) {
        chChar = pchMsg[j + 1];
        wCRC ^= (((uint16_t) chChar) << 8);

        for (i = 0; i < 8; i++) {
            if (wCRC & 0x8000) {
                wCRC = (wCRC << 1) ^ CRC_16_POLYNOMIALS;
            } else {
                wCRC <<= 1;
            }
        }

        chChar = pchMsg[j];
        wCRC ^= (((uint16_t) chChar) << 8);

        for (i = 0; i < 8; i++) {
            if (wCRC & 0x8000) {
                wCRC = (wCRC << 1) ^ CRC_16_POLYNOMIALS;
            } else {
                wCRC <<= 1;
            }
        }
    }

    return wCRC;
}

static inline uint16_t get_row_number(uint8_t *header) {
    return ((uint16_t) header[1] << 8) + header[0];
}

static void lost_row_interpolation(uint16_t* rawdata, const uint8_t * lost_row_table,
        uint8_t lost_row_table_size) {
    uint8_t i = 0;
    uint16_t j = 0;
    uint8_t lost_row = 0;
    uint32_t row_size = g_sensor.col;

    GF_LOGD(LOG_TAG "[%s] : entry.", __func__);

    for (i = 0; i < lost_row_table_size; i++) {
        lost_row = lost_row_table[i];
        if ((lost_row >= g_sensor.row) || (lost_row == 0)) {
            GF_LOGD(LOG_TAG "[%s] empty or invalid lost row", __func__);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] interpolate row %d", __func__, lost_row);
        for (j = 0; j < 25; j++) {
            rawdata[lost_row * row_size + j] = (rawdata[(lost_row - 1) * row_size + j]
                    + rawdata[(lost_row + 1) * row_size + j]) / 2;
        }
    }

    GF_LOGD(LOG_TAG "[%s] : exit.", __func__);
}

static gf_error_t gf_milan_a_series_frame_decrypt_one_dma(uint8_t *rawdata, uint16_t *out) {
    gf_error_t ret = GF_SUCCESS;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t usPtr = 0;
    uint16_t rescan = 0;
    uint16_t row_crc = 0;
    uint16_t cal_row_crc = 0;
    uint32_t SIZE_ROW = (g_sensor.col + 2) * 2;
    int cur_row = 0;
    int cur_row_cracked = 0;

    const uint8_t * row_array = NULL;
    uint8_t row_array_size = 0;

    const uint8_t * lost_array = NULL;
    uint8_t lost_array_size = 0;

    uint32_t size = g_sensor.row * g_sensor.col;

    GF_LOGD(LOG_TAG "[%s] : entry. ", __func__);

    /* array selector */
    if (GF_SENSOR_5206 == g_sensor.config.sensor_type
            || GF_SENSOR_5216 == g_sensor.config.sensor_type) {
        row_array = g_milan_a_row_array;
        row_array_size = sizeof(g_milan_a_row_array);
        lost_array = g_milan_a_lost_row_array;
        lost_array_size = sizeof(g_milan_a_lost_row_array);
    } else if (GF_SENSOR_5208 == g_sensor.config.sensor_type) {
        row_array = g_milan_c_row_array;
        row_array_size = sizeof(g_milan_c_row_array);
        lost_array = g_milan_c_lost_row_array;
        lost_array_size = sizeof(g_milan_c_lost_row_array);
    } else {
        GF_LOGD(LOG_TAG "[%s] wrong sensor type or otp type", __func__);
        return GF_ERROR_INVALID_DATA;
    }

    /* data re-order */
    for (i = 0; i < row_array_size; i++, usPtr += SIZE_ROW) {
        cur_row = get_row_number(&rawdata[usPtr]);
        if ((cur_row != row_array[i]) || (99 == row_array[i]) || (111 == row_array[i])) {
            if (111 == row_array[i]) {
                i++;
            }

            for (j = usPtr; j < RAW_READ_LEN; j++) {
                if (get_row_number(&rawdata[j]) == row_array[i + 1]
                        && get_row_number(&rawdata[j + SIZE_ROW]) == row_array[i + 2]
                        && get_row_number(&rawdata[j + 2 * SIZE_ROW]) == row_array[i + 3]) {
                    GF_LOGD(LOG_TAG "[%s] remap row %u(i = %u) @ offset %u", __func__, row_array[i],
                            i, j - SIZE_ROW);

                    break;
                }
            }

            if (j >= RAW_READ_LEN) {
                GF_LOGE(LOG_TAG "[%s] remap searching over boundary", __func__);
                return GF_ERROR_INVALID_DATA;
            }
            if (j > 0) {
                usPtr += (j - usPtr - SIZE_ROW);
            }
        }
        if (99 == row_array[i]) {
            continue;
        }
        cur_row = get_row_number(&rawdata[usPtr]);
        if (cur_row == row_array[i]) {

            // CRC check
            row_crc = (rawdata[usPtr + SIZE_ROW - 1] << 8) + rawdata[usPtr + SIZE_ROW - 2];
            cal_row_crc = get_crc_value_one_dma(&rawdata[usPtr + 2], SIZE_ROW - 4);
            if (row_crc != cal_row_crc) {
                GF_LOGE(LOG_TAG "[%s]crc failed @ row %u, sensor send[0x%x], calculate[0x%x].",
                        __func__, row_array[i], row_crc, cal_row_crc);
                return GF_ERROR_INVALID_DATA;
            }
        } else {
            cur_row = row_array[i];
        }
        cur_row_cracked = 4 * (row_array[i] / 4) + (4 - row_array[i] % 4) - 1;
        cpl_memcpy((uint8_t *) &out[cur_row_cracked * g_sensor.col], &rawdata[usPtr + 2],
                g_sensor.col * 2);
    }

    GF_LOGD(LOG_TAG "[%s] data total count %u", __func__, usPtr);

    if (GF_SENSOR_5208 == g_sensor.config.sensor_type) {
        /* interpolate lost row */
        lost_row_interpolation(out, lost_array, lost_array_size);
    }

    /* get rescan parameter */
    rescan = g_sensor.fw_cfg.milan_a_series.cfg_data[78] + 1;
    GF_LOGD(LOG_TAG "[%s] rescan times = %u", __func__, rescan);

    for (i = 0; i < size; i++) {
        out[i] = out[i] / rescan;
    }

    GF_LOGD(LOG_TAG "[%s] exit, err = %u", __func__, ret);
    return ret;
}

/* check chip version, MP/ECO... */
static gf_error_t gf_milan_a_series_get_chip_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t chip_id = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);
    if (NULL == buf || buf_len < sizeof(chip_id)) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_HOLD_CHIP_ID_ADDR, &chip_id);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :read chip id failed.", __func__);
        return GF_ERROR_SPI_COMMUNICATION;
    }

    cpl_memcpy(buf, &chip_id, sizeof(chip_id));

    return ret;
}

static gf_error_t gf_milan_a_series_get_fw_version(uint8_t *buf, uint32_t buf_len) {
    gf_error_t ret = GF_SUCCESS;

    if (NULL == buf || buf_len < 10) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_FW_VERSION_ADDR, buf, 10);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :read pid_vid failed.", __func__);
        return GF_ERROR_SPI_COMMUNICATION;
    }
    return ret;
}

static gf_error_t gf_milan_a_series_get_vendor_id(uint8_t *buf, uint32_t buf_len) {

#ifndef USE_OTP_VENDOR_ID
    int i;
    uint16_t opt1, opt2;
    uint16_t res, id1 = 0, id2 = 0;
    uint8_t id;
    gf_error_t ret = GF_SUCCESS;
    uint8_t year = 0, month = 0, date = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    if (NULL == buf || buf_len < GF_VENDOR_ID_LEN ) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    cpl_memset(buf, 0x0, buf_len);

    if (g_sensor.config.sensor_type == GF_SENSOR_5206) {
        for(i = 0; i < 4; i++) {
            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_VENDOR_OPT1, &opt1);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5206 read opt1 failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            opt1 &= ~GF_MILAN_A_VENDOR_OPT1_MASK;
            opt1 |= g_vendor_opt_map[0][i];

            ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_VENDOR_OPT1, opt1);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5206 write opt1 failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_GPIO_IN, &res);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5206 read gpio in failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            id1 += !!(res & (0x1<< 15));
        }

        id = g_vendor_id_map[0][id1];
    } else if (g_sensor.config.sensor_type == GF_SENSOR_5208) {
        for(i = 0; i < 4; i++) {
            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_VENDOR_OPT1, &opt1);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5208 read opt1 failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            opt1 &= ~GF_MILAN_A_VENDOR_OPT1_MASK;
            opt1 |= g_vendor_opt_map[0][i];

            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_VENDOR_OPT2, &opt2);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5208 read opt2 failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            opt2 &= ~GF_MILAN_A_VENDOR_OPT2_MASK;
            opt2 |= g_vendor_opt_map[1][i];

            ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_VENDOR_OPT1, opt1);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5208 write opt1 failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_VENDOR_OPT2, opt2);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5208 write opt2 failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_GPIO_IN, &res);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :5208 read gpio in failed.", __func__);
                return GF_ERROR_SPI_COMMUNICATION;
            }

            id1 += !!(res & (0x1<< 15));
            id2 += !!(res & (0x1<< 14));
        }

        id = g_vendor_id_map[1][((id1 << 0) + (id2 * 5))];
    }

    ret = gf_milan_a_series_otp_get_vendor_id(buf, buf_len);
    year = buf[1] & 0x07; //0  -->2016
    date = (buf[1] & 0xf8) >> 3;
    month= buf[0];
    buf[0] = id;
    buf[1] = year;
    buf[2] = month;
    buf[3] = date;
    cpl_memcpy(buf, &id, sizeof(id));
#else
    uint8_t year = 0, month = 0, date = 0;
    uint8_t vendor_id = 0;
    gf_error_t ret = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    if (NULL == buf || buf_len < GF_VENDOR_ID_LEN) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    ret = gf_milan_a_series_otp_get_vendor_id(buf, buf_len);
    year = buf[1] & 0x07; //0  -->2016
    date = (buf[1] & 0xf8) >> 3;
    month = buf[0];
    vendor_id = buf[2];
    buf[0] = vendor_id;
    buf[1] = year;
    buf[2] = month;
    buf[3] = date;
#endif
    GF_LOGD(LOG_TAG "[%s] vendor_id 0x%x, data %d--%d--%d.", __func__, buf[0], year, month, date);

    return ret;
}

static gf_error_t gf_milan_a_series_get_production_date(uint8_t *buf, uint32_t buf_len) {
    gf_error_t ret = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    cpl_memset(buf, 0, buf_len);

    return ret;
}

static gf_error_t gf_milan_a_series_get_sensor_id(uint8_t *buf, uint32_t buf_len) {
    gf_error_t ret = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    if (NULL == buf || buf_len < GF_SENSOR_ID_LEN) {
        return GF_ERROR_BAD_PARAMS;
    }

    ret = gf_milan_a_series_otp_get_sensor_id(buf, buf_len);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :get sensor id failed.", __func__);
        return GF_ERROR_GENERIC;
    }

    return ret;
}

//static gf_error_t gf_milan_a_series_get_product_info(void) {
//    return GF_SUCCESS;
//}

//static gf_error_t gf_milan_a_series_write_product_info(uint8_t product_cfg_idx) {
//    UNUSED_VAR(product_cfg_idx);
//    return GF_SUCCESS;
//}

static gf_error_t gf_milan_a_series_update_cfg_from_otp(uint8_t *otp_buf) {

    gf_error_t err = GF_SUCCESS;
    uint8_t *cfg_data = NULL;
    uint32_t cfg_data_len = 0;
    uint32_t otp_version = 0;
    gf_milan_a_series_sensor_otp_type_t sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_UNKNOWN;
    uint16_t chksum = 0;
    uint8_t dac_update_type = 0; //0:normal;1:compatible with error otp;2:otp version >= 4 milan a or b
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        otp_version = ((otp_buf[45] & 0x70) >> 4);
        g_sensor.fw_cfg.milan_a_series.otp_version = otp_version;

        if (otp_version <= 2) {
            g_sensor.flag_chip_info |= 0x02;  // ispixelcancel = 1
        } else {
            g_sensor.flag_chip_info &= 0xFFFFFFFD;  // ispixelcancel = 0
        }

        if (MILAN_C_CHIP_ID_V0 == g_gf_chip_id) {
            sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_1;
        } else if (MILAN_C_CHIP_ID_V1 == g_gf_chip_id || MILAN_C_CHIP_ID_V2 == g_gf_chip_id) {
            if (0 == otp_version) {
                if (otp_buf[63] == 0) {
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_2;
                } else {
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_3;
                    dac_update_type = 1;
                }
            } else if (otp_version >= 3) {
                sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_5;
            } else {
                sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_4;
            }
        } else if (MILAN_A_CHIP_ID == g_gf_chip_id) {
            if (otp_version > 2) {
                if (otp_version > 3) {
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_3;
#if (defined(GF5206) && (MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV == 0))
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_7;
#endif
#if defined(GF5216)
                    if(otp_buf[63] == 0xAA || otp_buf[63] == 0xFF) {
                        sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_9;
                    }
#endif
                } else {
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_2;
#if (defined(GF5206) && (MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV == 0))
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_6;
#endif
#if defined(GF5216)
                    if(otp_buf[63] == 0xAA || otp_buf[63] == 0xFF) {
                        sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_8;
                    }
#endif
                }
            } else {
                sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_1;
            }
            if (otp_version >= 4) {
                dac_update_type = 2;
            }
        } else if (MILAN_A_CHIP_ID_C2 == g_gf_chip_id) {
            if (otp_version > 3) {
                sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_5;
#if (defined(GF5206) && (MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV == 0))
                sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_9;
#endif
#if defined(GF5216)
                if(otp_buf[63] == 0xAA || otp_buf[63] == 0xFF) {
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_7;
                }
#endif
            } else {
                sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_4;
#if (defined(GF5206) && (MILAN_A_FW_OPEN_FPCKEY_CLOSE_NAV == 0))
                sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_8;
#endif
#if defined(GF5216)
                if(otp_buf[63] == 0xAA || otp_buf[63] == 0xFF) {
                    sensor_otp_type = GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_6;
                }
#endif
            }
            if (otp_version >= 4) {
                dac_update_type = 2;
            }
        } else {
            GF_LOGE(LOG_TAG "[%s] un-support chip type , gf_chip_id :%d", __func__, g_gf_chip_id);
            break;
        }
        GF_LOGI(LOG_TAG "[%s] sensor otp type = %d, otp_version = %u, dac_update_type = %u", __func__, sensor_otp_type, otp_version, dac_update_type);
        err = milan_a_series_init_configuration(&g_milan_a_series_chip_handle,
                sensor_otp_type, otp_version);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] init configuration fail, err:%d", __func__, err);
            break;
        }
        cfg_data = g_sensor.fw_cfg.milan_a_series.cfg_data;
        milan_a_series_clock_otp_update(otp_buf, cfg_data);
        milan_a_series_dac_otp_update(otp_buf, cfg_data, dac_update_type);
        milan_a_series_hbd_otp_update(otp_buf, cfg_data,
                &(g_sensor.fw_cfg.milan_a_series));
        milan_a_series_temperature_otp_update(otp_buf, cfg_data);

    } while (0);

    if (GF_SUCCESS == err) {

        /*close stable judgement for acquiring image*/
        cfg_data[GF_MILAN_A_SERIES_STABLE_JUDGE_CONFIG_INDEX] =
                GF_MILAN_A_SERIES_NOT_JUDGE_STABLE_CONFIG_VALUE & 0xFF;

        cfg_data[GF_MILAN_A_SERIES_STABLE_JUDGE_CONFIG_INDEX + 1] =
                GF_MILAN_A_SERIES_NOT_JUDGE_STABLE_CONFIG_VALUE >> 8;

        /* according gf_config.c to update fw_config*/
        if (1 == g_sensor.config.support_fpc_key) {
            cfg_data[GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY] = cfg_data[GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY] | 0x03;
            GF_LOGD(LOG_TAG "[%s] GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY=%d", __func__, cfg_data[GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY]);
        }
        if (1 == g_sensor.config.support_ring_key) {
            cfg_data[GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY] = cfg_data[GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY] | 0x04;
            GF_LOGD(LOG_TAG "[%s] GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY=%d", __func__, cfg_data[GF_MILAN_A_SERIES_ENABLE_FPC_RING_KEY]);
        }
        if (1 == g_sensor.config.support_sleep_key) {
            cfg_data[GF_MILAN_A_SERIES_ENABLE_SLEEP_KEY] = cfg_data[GF_MILAN_A_SERIES_ENABLE_SLEEP_KEY] | 0x01;
            GF_LOGD(LOG_TAG "[%s] GF_MILAN_A_SERIES_ENABLE_SLEEP_KEY=%d", __func__, cfg_data[GF_MILAN_A_SERIES_ENABLE_SLEEP_KEY]);
        }
        /* update config checksum */
        chksum = milan_a_series_otp_get_checksum16(g_sensor.fw_cfg.milan_a_series.cfg_data,
                (GF_MILAN_A_SERIES_SIZE_CONFIG - 4) / 2);
        GF_LOGD(LOG_TAG "[%s] chksum %d ", __func__, chksum);

        cfg_data[GF_MILAN_A_SERIES_SIZE_CONFIG - 4] = chksum & 0xFF;
        cfg_data[GF_MILAN_A_SERIES_SIZE_CONFIG - 3] = chksum >> 8;
        cfg_data[GF_MILAN_A_SERIES_SIZE_CONFIG - 1] = 0x01;

        gf_milan_a_series_vcm_cal(otp_buf);

        cfg_data_len = g_sensor.fw_cfg.milan_a_series.cfg_data_len;
        cpl_memcpy(g_pixel_test_cfg_data, g_sensor.fw_cfg.milan_a_series.cfg_data, cfg_data_len);
    }

    GF_LOGD(LOG_TAG "[%s] exit err: %d.", __func__, err);
    return err;
}

static gf_error_t gf_milan_a_series_select_chip_otp(gf_detect_sensor_t* cmd) {
    gf_error_t err = GF_SUCCESS;
    gf_error_t load_result = GF_SUCCESS;
    uint8_t* otp_buf = NULL;
    uint8_t otp_from_so[MILAN_A_SERIES_OTP_BUF_LEN] = { 0 };
    uint8_t crc_buf_temp[MILAN_A_SERIES_OTP_BUF_LEN] = { 0 };
    int32_t is_otp_load_so = 0;
    int32_t crc_ok = 1;
    uint32_t otp_so_len = 0;
    uint8_t checked_otp = 0;
    int32_t i = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = gf_milan_a_series_read_chip_otp(g_milan_a_series_chip_handle.otp_buf);
        if (err != GF_SUCCESS) {
            if (g_otp_failed_retry < 5) {
                g_otp_failed_retry++;
                err = GF_ERROR_GET_OTP_INFO_FAILED;
            }
            GF_LOGE(LOG_TAG "[%s] get OTP fail", __func__);
            crc_ok = 0;
            cmd->choose_otp_source = -1;
        } else {
            cmd->choose_otp_source = 0;
            checked_otp = 1;
            GF_LOGD(LOG_TAG "[%s] Got sensor otp successfully", __func__);
        }

        load_result = gf_milan_a_series_load_otp_info(otp_from_so, &otp_so_len);
        if (load_result != GF_SUCCESS) {
            is_otp_load_so = 0;
        } else {
            is_otp_load_so = 1;
        }

        if (is_otp_load_so == 1) {
            otp_buf = otp_from_so;
            if (memcmp((const int8_t*)g_milan_a_series_chip_handle.otp_buf, (const int8_t*)otp_buf, 16) == 0) {
                //use otp from tee so
                for (i = 0; i < 0x20; i++){
                    crc_buf_temp[i] = otp_buf[i];
                }
                for (i = 0x20; i < MILAN_A_SERIES_OTP_BUF_LEN; i++){
                    crc_buf_temp[i-1] = otp_buf[i];
                }
                err = gf_milan_a_series_otp_check_crc(otp_buf, crc_buf_temp);
                if (err == GF_SUCCESS) {
                    cmd->choose_otp_source = 1;
                    checked_otp = 1;
                    cpl_memcpy(g_milan_a_series_chip_handle.otp_buf, otp_buf, MILAN_A_SERIES_OTP_BUF_LEN);
                    cpl_memcpy(otp_crc, crc_buf_temp, MILAN_A_SERIES_OTP_BUF_LEN);
                    GF_LOGD(LOG_TAG "[%s] using so otp", __func__);
                } else {
                    GF_LOGD(LOG_TAG "[%s] tee so otp, UUID is ok, but crc check failed", __func__);
                    for (i = 0; i < 64; ) {
                        GF_LOGD(LOG_TAG "Otp data:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,",
                                otp_buf[i],otp_buf[i+1],otp_buf[i+2],otp_buf[i+3],
                                otp_buf[i+4],otp_buf[i+5],otp_buf[i+6],otp_buf[i+7]);
                        i += 8;
                    }
                    gf_so_delete_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID);
                }
            } else {
                gf_so_delete_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID);
            }
        }

        if (cmd->otp_buf_len_sd != 0) {
            otp_buf = cmd->otp_buf_sd;
            if (memcmp((const int8_t*) g_milan_a_series_chip_handle.otp_buf, (const int8_t*) otp_buf, 16) == 0) {
                //use otp from sdcard
                for (i = 0; i < 0x20; i++){
                    crc_buf_temp[i] = otp_buf[i];
                }
                for (i = 0x20; i < MILAN_A_SERIES_OTP_BUF_LEN; i++){
                    crc_buf_temp[i-1] = otp_buf[i];
                }
                err = gf_milan_a_series_otp_check_crc(otp_buf, crc_buf_temp);
                if (err == GF_SUCCESS) {
                    cmd->choose_otp_source = 2;
                    checked_otp = 1;
                    cpl_memcpy(g_milan_a_series_chip_handle.otp_buf, otp_buf, MILAN_A_SERIES_OTP_BUF_LEN);
                    cpl_memcpy(otp_crc, crc_buf_temp, MILAN_A_SERIES_OTP_BUF_LEN);
                    GF_LOGD(LOG_TAG "[%s] using sd otp", __func__);
                } else {
                    GF_LOGD(LOG_TAG "[%s] sdcard otp, UUID is ok, but crc check failed", __func__);
                    for (i = 0; i < 64; ) {
                        GF_LOGD(LOG_TAG "Otp data:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,",
                                otp_buf[i],otp_buf[i+1],otp_buf[i+2],otp_buf[i+3],
                                otp_buf[i+4],otp_buf[i+5],otp_buf[i+6],otp_buf[i+7]);
                        i += 8;
                    }
                }
                break;
            }
        }

    } while (0);

    switch (cmd->choose_otp_source) {
    case 0: //chip otp
        if (crc_ok == 1) {
            GF_LOGE(LOG_TAG "[%s] UUID not equal, replace OTP in tee/sdcard ", __func__);
            gf_milan_a_series_save_otp_info(g_milan_a_series_chip_handle.otp_buf, MILAN_A_SERIES_OTP_BUF_LEN);
            cpl_memcpy(cmd->otp_buf_sd, g_milan_a_series_chip_handle.otp_buf, MILAN_A_SERIES_OTP_BUF_LEN);
        }
        break;
    case 1: //tee saved otp
        GF_LOGE(LOG_TAG "[%s] replace OTP in sdcard ", __func__);
        cpl_memcpy(cmd->otp_buf_sd, g_milan_a_series_chip_handle.otp_buf, MILAN_A_SERIES_OTP_BUF_LEN);
        break;
    case 2: //sdcard otp
        if (is_otp_load_so == 0) {
            GF_LOGE(LOG_TAG "[%s] replace OTP in tee", __func__);
            gf_milan_a_series_save_otp_info(g_milan_a_series_chip_handle.otp_buf, MILAN_A_SERIES_OTP_BUF_LEN);
        }
        break;
    default:
        break;
    }

    if (checked_otp > 0) {
        err = GF_SUCCESS;
        GF_LOGD(LOG_TAG "[%s] get write otp from sensor or tee_file or sdcard", __func__);
    }
    GF_LOGD(LOG_TAG "[%s] exit err = %u", __func__, err);
    return err;
}

static gf_error_t gf_milan_a_series_get_sensor_otp_info(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s], enter", __func__);

    do {
        if (NULL == buf || buf_len < MILAN_A_SERIES_OTP_BUF_LEN) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] bad parameter(buf_len:%d)", __func__, buf_len);
            break;
        }

        cpl_memcpy(buf, g_milan_a_series_chip_handle.otp_buf, MILAN_A_SERIES_OTP_BUF_LEN);
    } while (0);

    GF_LOGD(LOG_TAG "[%s], exit", __func__);

    return err;
}

static gf_error_t gf_milan_a_series_detect_sensor(void) {
    gf_error_t err = GF_SUCCESS;
    gf_sensor_type_t type = GF_SENSOR_5206;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_HOLD_CHIP_ID_ADDR, &g_gf_chip_id);
        GF_LOGI(LOG_TAG "[%s] sensor chip id: 0x%x", __func__, g_gf_chip_id);

        if (!CHIP_ID_IS_MILAN_A_SERIES(g_gf_chip_id)) {
            GF_LOGD(LOG_TAG "[%s] sensor is not milan a series type!", __func__);
            err = GF_ERROR_SENSOR_NOT_AVAILABLE;
            break;
        }
        g_milan_a_series_chip_handle.chip_id = g_gf_chip_id;

        err = gf_milan_a_series_init_sensor_type(g_milan_a_series_chip_handle.otp_buf);
        if(err!= GF_SUCCESS){
            GF_LOGE(LOG_TAG "[%s] fail to get sensor type", __func__);
        }
        type = gf_milan_a_series_get_sensor_type();
        GF_LOGI(LOG_TAG "[%s] sensor type: %d", __func__, type);

        err = gf_milan_a_series_update_cfg_from_otp(g_milan_a_series_chip_handle.otp_buf);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] update cfg  fail", __func__);
            break;
        }

        //TODO judgment whether release package match with sensor
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit", __func__);
    return err;
}

static gf_error_t gf_milan_a_series_judge_finger_stable(uint32_t sleep_time) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t finger_up_flag = 1;
    uint32_t sub_time = sleep_time / 1000;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if (0 == g_milan_a_series_chip_handle.feature.support_judge_finger_stable) {
            GF_LOGE(LOG_TAG "[%s] not support judge finger stable", __func__);
            break;
        }

        if (sub_time < 35) {
            gf_sleep(35 - sub_time);
        }

        ret = gf_milan_a_series_secspi_read_byte(GF_MILAN_A_SERIES_FINGERON_STATUS_ADDR,
                &finger_up_flag);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] read FingerOn failed", __func__);
            break;
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] enter time = %d sub time = %d finger_up_flag = %d .",
            __func__, sleep_time, sub_time, finger_up_flag);

    if (finger_up_flag == 0) {
        ret = GF_ERROR_GENERIC;
    } else {
        ret = GF_SUCCESS;
    }
    return ret;
}

static gf_error_t gf_milan_a_series_chip_init(uint8_t *download_fw_flag, uint8_t *download_cfg_flag) {
    gf_error_t ret = GF_SUCCESS;

    do {
        *download_fw_flag = 1;
        *download_cfg_flag = 1;

    } while (0);

    GF_LOGD(LOG_TAG "[%s] enter, err:%d.", __func__, ret);

    return ret;
}

//static gf_error_t gf_milan_a_series_erase_fw(void) {
//    gf_error_t ret = GF_SUCCESS;
//    GF_LOGI(LOG_TAG "[%s] enter.", __func__);
//
//    return ret;
//}

gf_error_t gf_milan_a_series_vcm_cal(uint8_t *otp) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t i = 0;
    uint32_t otp_version = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    vcm_cal[0] = (otp[0x2F] >> 4) | (((otp[0x33] >> 4) & 0x01) << 4);
    vcm_cal[1] = (otp[0x30] & 0x0f) | (((otp[0x33] >> 5) & 0x01) << 4);
    vcm_cal[2] = (otp[0x30] >> 4) | (((otp[0x33] >> 6) & 0x01) << 4);
    vcm_cal[3] = (otp[0x31] & 0x0f) | (((otp[0x33] >> 7) & 0x01) << 4);

    vcm_cal[4] = (otp[0x31] >> 4) | ((otp[0x34] & 0x01) << 4);
    vcm_cal[5] = (otp[0x32] & 0x0f) | (((otp[0x34] >> 1) & 0x01) << 4);
    vcm_cal[6] = (otp[0x32] >> 4) | (((otp[0x34] >> 2) & 0x01) << 4);
    vcm_cal[7] = (otp[0x33] & 0x0f) | (((otp[0x34] >> 3) & 0x01) << 4);

    vcm_cal[8] = otp[0x34] >> 4;

    //for otp version > 4 sensor, vcm_cal[0] need to replace by [1]
    otp_version = ((otp[45] & 0x70) >> 4);
    if(otp_version >=4){
        vcm_cal[0] = vcm_cal[1];
    }

    //log out vcm_cal
    for (i = 0; i < 8; i++) {
        GF_LOGD(LOG_TAG "[%s] vcm_cal[%d]=%d.", __func__, i, vcm_cal[i]);
    }

    return ret;
}

static int16_t DAC_OFFSET[GF_MILAN_A_SERIES_DAC_OFFSET_LEN] = { 0 };
static gf_error_t gf_milan_a_series_update_temprature_dac(void) {
    gf_error_t ret = GF_SUCCESS;
    int i = 0;
    uint16_t data = 0;
    int temp_diff = 0;
    uint16_t DATA_PER_DAC = 0;
    uint8_t temp_status = 0;

    GF_LOGI(LOG_TAG "[%s] enter.", __func__);

    ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TEMP_STATUS_ADDR, &data);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :read temperature status failed, ret=%d.", __func__, ret);
        return GF_ERROR_SPI_COMMUNICATION;
    }

    temp_status = (uint8_t) (data & 0x00FF);

    //val_cal value  [0 - 31]
    for (i = 0; i < GF_MILAN_A_SERIES_DAC_OFFSET_LEN; i++) {
        DAC_OFFSET[i] = 0;
        temp_diff = 0;
        if (g_gf_chip_id == MILAN_A_CHIP_ID) {
            DATA_PER_DAC = 120;

            if (vcm_cal[i] < 11) {
                temp_diff = -65 * vcm_cal[i] + 1128;
            } else if (vcm_cal[i] < 31) {
                temp_diff = -15 * vcm_cal[i] + 719;
            } else if (vcm_cal[i] == 31) {
                temp_diff = 126;
            }
        } else if (g_gf_chip_id == MILAN_A_CHIP_ID_C2) {
            DATA_PER_DAC = 150;

            if (vcm_cal[i] < 31) {
                temp_diff = -9 * vcm_cal[i] + 350;
            } else if (vcm_cal[i] == 31) {
                temp_diff = 25;
            }
        }
        if (g_gf_chip_id == MILAN_C_CHIP_ID_V0) {
            DATA_PER_DAC = 157;

            if (vcm_cal[i] < 31) {
                temp_diff = -18 * vcm_cal[i] + 960;
            } else if (vcm_cal[i] == 31) {
                temp_diff = 240;
            }
        }
        if (g_gf_chip_id == MILAN_C_CHIP_ID_V1) {
            DATA_PER_DAC = 157;

            if (vcm_cal[i] < 16) {
                temp_diff = -3 * vcm_cal[i] + 527;
            } else if (vcm_cal[i] < 31) {
                temp_diff = -21 * vcm_cal[i] + 800;
            } else if (vcm_cal[i] == 31) {
                temp_diff = 50;
            }
        } else if (g_gf_chip_id == MILAN_C_CHIP_ID_V2) {
            DATA_PER_DAC = 157;

            if (vcm_cal[i] < 31) {
                temp_diff = -8 * vcm_cal[i] + 376;
            } else if (vcm_cal[i] == 31) {
                temp_diff = 55;
            }
        }
        DAC_OFFSET[i] = (vcm_cal[8] - temp_status) * (temp_diff) / (2 * DATA_PER_DAC);
        GF_LOGD(LOG_TAG "[%s] DAC_OFFSET[%d] =  %d.", __func__, i, DAC_OFFSET[i]);
    }

    ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_TEMPRATURE_DAC_BASE_ADDR,
            (uint8_t*) DAC_OFFSET, (GF_MILAN_A_SERIES_DAC_OFFSET_LEN * 2));
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :wirte temprature dac fail!", __func__);
    }

    GF_LOGI(LOG_TAG "[%s] exit ret: %d.", __func__, ret);

    return ret;
}

static gf_error_t gf_milan_a_series_download_fw(uint8_t *reset_flag) {

    gf_error_t ret = GF_SUCCESS;
    uint32_t loop_time = 0;
    uint32_t i = 0;
    uint32_t fw_len = 0;
    uint8_t *fw_data = NULL;
    uint32_t cfg_len = 0;
    uint8_t *cfg_data = g_sensor.fw_cfg.milan_a_series.cfg_data;
    uint8_t tmp_buf[1024] = { 0 };
    uint16_t data = 0;
    uint8_t need_to_release_mcu = 0;
    uint8_t need_to_disable_spi_ram = 0;
    uint8_t enable_flag = 1;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    cfg_len = g_sensor.fw_cfg.milan_a_series.cfg_data_len;
    fw_len = g_sensor.fw_cfg.milan_a_series.fw_data_len;
    fw_data = g_sensor.fw_cfg.milan_a_series.fw_data;
    *reset_flag = 0;

    GF_LOGD(LOG_TAG "[%s] :ready to load fw. cfg len=%d, fw len= %d.", __func__, cfg_len, fw_len);
    do {
        //hold mcu.
        GF_LOGD(LOG_TAG "[%s] :do hold mcu.", __func__);
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0200);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :hold mcu failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_release_mcu = 1;

        //enable spi  ram.
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_ENABLE_SPI_RAM_ADDR, 0x0003);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :enable spi ram failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_disable_spi_ram = 1;

        //download cfg.
        ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_CFG_ADDR, cfg_data, cfg_len);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :update cfg failed.", __func__);
            break;
        }

        //read back cfg data to check.
        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_CFG_ADDR, tmp_buf, cfg_len);
        GF_LOGD(LOG_TAG "[%s] : cfg len=%d, fw len= %d.", __func__, cfg_len, fw_len);
        if (memcmp(cfg_data, tmp_buf, cfg_len)) {
            GF_LOGE(LOG_TAG "[%s] :cfg read data is different from write.", __func__);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] :cfg download success.", __func__);

        loop_time = (fw_len / 1000);
        for (i = 0; i < loop_time; i++) {
            ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_FW_ADDR + i * 1000, (fw_data + i * 1000),  1000);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :update fw failed.", __func__);
                break;
           }
        }

        if ((fw_len % 1000) > 0)  {
            ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_FW_ADDR + loop_time * 1000, (fw_data + loop_time * 1000),  (fw_len % 1000));
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :update fw failed.", __func__);
                break;
           }
        }

        //read back fw data to check.
        loop_time = (fw_len / 512);
        for (i = 0; i < loop_time; i++) {
            ret = gf_milan_a_series_secspi_read_bytes((GF_MILAN_A_SERIES_FW_ADDR + i * 512),
                    tmp_buf, 512);
            if (memcmp((fw_data + i * 512), tmp_buf, 512)) {
                GF_LOGE(
                        LOG_TAG "[%s] :fw read data is different write,address=0x%p,i=%d,loop_time=%d.",
                        __func__, (void*) (fw_data + i * 512), i, loop_time);
                ret = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
                break;
            }
        }

        if (GF_SUCCESS != ret) {
            break;
        }

        //diable spi  ram.
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_ENABLE_SPI_RAM_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :diable spi ram failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_disable_spi_ram = 0;

        //release mcu.
        GF_LOGD(LOG_TAG "[%s] :do release mcu.", __func__);
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :release mcu failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_release_mcu = 0;

        //delay for fw running and clear reset irq.
        gf_sleep(20);
        for (i = 0; i < 5; i++) {
            gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, &data);
            if ((data & 0xff00) == 0xE400) {
                i = 0;
                gf_milan_a_series_update_temprature_dac();
                break;
            }
            gf_sleep(20);
            GF_LOGD(LOG_TAG "[%s] :read irq status %x. i %d", __func__, data, i);
        }

        if (5 == i) {
            GF_LOGE(LOG_TAG "[%s] :not detected E4 interrupt!", __func__);
            gf_milan_a_series_update_temprature_dac();
        }

        gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, 0x0000);
        GF_LOGD(LOG_TAG "[%s] :cfg download success.", __func__);

    } while (0);

    if (need_to_disable_spi_ram == 1) {
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_ENABLE_SPI_RAM_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :diable spi ram failed, ret = %d.", __func__, ret);
        }
    }

    if (need_to_release_mcu == 1) {
        GF_LOGD(LOG_TAG "[%s] :do release mcu.", __func__);
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :release mcu failed, ret = %d.", __func__, ret);
        }
    }

    GF_LOGD(LOG_TAG "[%s] :fw download success.", __func__);

    if (g_milan_a_series_chip_handle.feature.support_reissue_key_down > 0) {
        if (1 == g_sensor.config.reissue_key_down_when_entry_ff_mode) {
            enable_flag = GF_MILAN_A_SERIES_REISSUE_DOWN_CONFIG_VALUE;
        } else {
            enable_flag = GF_MILAN_A_SERIES_NOT_REISSUE_DOWN_CONFIG_VALUE;
        }
        gf_milan_a_series_secspi_write_byte(
            GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE_ADDR, enable_flag);
        GF_LOGD(LOG_TAG "[%s] :set ff reissue key down enable = %u.", __func__, enable_flag);

        if (1 == g_sensor.config.reissue_key_down_when_entry_image_mode) {
            enable_flag = GF_MILAN_A_SERIES_REISSUE_DOWN_CONFIG_VALUE;
        } else {
            enable_flag = GF_MILAN_A_SERIES_NOT_REISSUE_DOWN_CONFIG_VALUE;
        }
        gf_milan_a_series_secspi_write_byte(
            GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE_ADDR, enable_flag);
        GF_LOGD(LOG_TAG "[%s] :set image reissue key down enable = %u.", __func__, enable_flag);
    }


    return ret;
}

static gf_error_t gf_milan_a_series_download_cfg(gf_config_type_t config_type) {

    gf_error_t ret = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);
    switch (config_type) {
        case CONFIG_NORMAL: {
            ret = milan_a_series_download_cfg(g_sensor.fw_cfg.milan_a_series.cfg_data);
            break;
        }
        case CONFIG_TEST_HBD: {
            ret = test_set_hbd_cfg();
            break;
        }
        default:
            break;
    }

    GF_LOGD(LOG_TAG "[%s] :exit, config_type: %d, err: %d", __func__, config_type, ret);

    return ret;
}

static gf_error_t gf_milan_a_series_download_pixel_test_cfg(uint8_t *cfg_data) {
    gf_error_t ret = GF_SUCCESS;
    uint32_t cfg_len = 0;
    uint8_t tmp_buf[512] = { 0 };

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if (NULL == cfg_data) {
            ret = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "%s, bad input params", __func__);
            break;
        }

        cfg_len = g_sensor.fw_cfg.milan_a_series.cfg_data_len;
        GF_LOGD(LOG_TAG "[%s] :ready to load cfg len=%d", __func__, cfg_len);

//hold mcu.
//        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0200);
//        if (GF_SUCCESS != ret) {
//            GF_LOGE(LOG_TAG "[%s] :hold mcu failed, ret = %d.", __func__, ret);
//            break;
//        }

//download cfg.
        ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_CFG_ADDR, cfg_data, cfg_len);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :update cfg failed.", __func__);
            break;
        }

//read back cfg data to check.
        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_CFG_ADDR, tmp_buf, cfg_len);

        GF_LOGD(LOG_TAG "[%s] : cfg len=%d.", __func__, cfg_len);
        if (memcmp(cfg_data, tmp_buf, cfg_len)) {
            GF_LOGE(LOG_TAG "[%s] :cfg read data is different from write.", __func__);
            break;
        }

//release mcu.
//        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0000);
//        if (GF_SUCCESS != ret) {
//            GF_LOGE(LOG_TAG "[%s] :release mcu failed, ret = %d.", __func__,
//                    ret);
//            break;
//        }

        //delay for fw running and clear reset irq.
        gf_sleep(20);
        gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, 0x0000);
        GF_LOGD(LOG_TAG "[%s] :cfg download success.", __func__);

    } while (0);

    GF_LOGD(LOG_TAG "[%s], exit, err: %d", __func__, ret);

    return ret;
}

/*Suspend to sample finger-print data*/
static gf_error_t gf_milan_a_series_data_sample_suspend(void) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t data = 0;
    data = 0x0001;
    ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_SAMPLING_PAUSE_ADDR, data);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :sample suspend failed.", __func__);
        return GF_ERROR_SPI_COMMUNICATION;
    }

    return ret;
}

/*Resume to sample finger-print data*/
static gf_error_t gf_milan_a_series_data_sample_resume(void) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t data = 0;
    data = 0x0000;
    ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_SAMPLING_PAUSE_ADDR, data);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :sample resume failed.", __func__);
        return GF_ERROR_SPI_COMMUNICATION;
    }
    GF_LOGD(LOG_TAG "[%s] :sample resume.", __func__);

    return ret;
}

static gf_error_t gf_milan_a_series_set_mode(gf_mode_t mode) {
    gf_error_t ret = GF_SUCCESS;
    static uint16_t data = 0;
    uint8_t val = 0xFF;
    static uint32_t sleep_mode_flag = 0;

    if (mode >= MODE_MAX) {
        GF_LOGE(LOG_TAG "[%s] :invalid mode,mode=0x%x.", __func__, mode);
        return GF_ERROR_BAD_PARAMS;
    }

    switch (mode) {
        case MODE_IMAGE:
            val = GF_MILAN_A_SERIES_MODE_IMAGE;
            break;
        case MODE_KEY:
            val = GF_MILAN_A_SERIES_MODE_KEY;
            break;
        case MODE_SLEEP:
            if (1 == sleep_mode_flag) {
                return ret;
            }
            val = GF_MILAN_A_SERIES_MODE_SLEEP;
            sleep_mode_flag = 1;
            break;
        case MODE_FF:
            val = GF_MILAN_A_SERIES_MODE_FF;
            break;
        case MODE_NAV:
            val = GF_MILAN_A_SERIES_MODE_NAV;
            break;
        case MODE_HBD:
            val = GF_MILAN_A_SERIES_MODE_HBD;
            break;
        case MODE_HBD_DEBUG:
            val = GF_MILAN_A_SERIES_MODE_HBD_BASE;
            break;
        case MODE_IDLE:
            val = GF_MILAN_A_SERIES_MODE_IDLE;
            break;

        case MODE_DEBUG:
        case MODE_FINGER_BASE:
            val = GF_MILAN_A_SERIES_MODE_FINGER_BASE;
            break;
        case MODE_NAV_BASE:
            val = GF_MILAN_A_SERIES_MODE_NAV_BASE;
            break;
        default:
            val = GF_MILAN_A_SERIES_MODE_UNKNOWN;
            break;
    }

    if (GF_MILAN_A_SERIES_MODE_UNKNOWN == val) {
        GF_LOGE(LOG_TAG "[%s] :unsupport mode,mode=0x%x.", __func__, mode);
        return GF_ERROR_BAD_PARAMS;
    }

    // write 0x06 to 0x0834 and sleep 5ms if set mode from sleep to other mode;
    if ((1 == sleep_mode_flag) && (MODE_SLEEP != mode)) {
        sleep_mode_flag = 0;
        data = (data & 0xFF00) | 0x06;
        gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_MODE_ADDR, data);
        gf_sleep(6);
        gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_MODE_ADDR, data);
        gf_sleep(6);
    }
    data = (data & 0xFF00) | (uint16_t) val;
    ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_MODE_ADDR, data);
    GF_LOGI(LOG_TAG "[%s] : set mode to 0x%x.", __func__, val);

    return ret;
}

static gf_error_t gf_milan_a_series_get_mode(gf_mode_t *value) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t mode = 0;
    uint16_t data = 0;

    if (NULL == value) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_MODE_ADDR, &data);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :get mode failed.", __func__);
        return GF_ERROR_SPI_COMMUNICATION;
    }

    mode = (uint8_t) (data & 0x00FF);

    switch (mode) {
        case GF_MILAN_A_SERIES_MODE_IMAGE:
            *value = MODE_IMAGE;
            break;
        case GF_MILAN_A_SERIES_MODE_KEY:
            *value = MODE_KEY;
            break;
        case GF_MILAN_A_SERIES_MODE_SLEEP:
            *value = MODE_SLEEP;
            break;
        case GF_MILAN_A_SERIES_MODE_FF:
            *value = MODE_FF;
            break;
        case GF_MILAN_A_SERIES_MODE_NAV:
            *value = MODE_NAV;
            break;
        case GF_MILAN_A_SERIES_MODE_HBD:
            *value = MODE_HBD;
            break;
        case GF_MILAN_A_SERIES_MODE_IDLE:
            *value = MODE_IDLE;
            break;
        case GF_MILAN_A_SERIES_MODE_FINGER_BASE:
            *value = MODE_FINGER_BASE;
            break;
        default:
            ret = GF_ERROR_INVALID_DATA;
            GF_LOGE(LOG_TAG "[%s] :unknown mode=0x%x.", __func__, mode);
            break;
    }

    GF_LOGD(LOG_TAG "[%s] : read mode is 0x%x, *value=0x%x.", __func__, mode, *value);

    return ret;
}

static gf_error_t gf_milan_a_series_get_image_multi_dma(uint16_t **buf, uint32_t *length, uint8_t **origin_buf, uint32_t *origin_length) {

    gf_error_t ret = GF_SUCCESS;
    uint32_t total_len = 0;
    uint32_t time_out = 200; //will be change later.
    uint32_t i = 0;
    uint16_t data;
    uint32_t fifo_len = 0;
    uint32_t data_len = 0;
    UNUSED_VAR(origin_buf);
    UNUSED_VAR(origin_length);

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    if ((NULL == buf) || (NULL == length)) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    GF_LOGD(LOG_TAG "[%s] :receive image irq. mode=0x%x,irq_status=0x%x.", __func__,
            g_sensor_status.mode, g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET]);

    data_len = g_sensor.row * (g_sensor.col + 2) * 2;

    gf_milan_a_series_notify_fw_adc_reading_begin();

    do {
        for (i = 0; i < 1000; i++) {
            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_DATA_READY_STATUS_ADDR,
                    &data);
            if (2 == (data & 0x00FF)) {
                //clear 0x5018 to 0x0;
                ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_DATA_READY_STATUS_ADDR,
                        (data & 0xFF00));
                break;
            }
        }

        if (i >= 1000) {
            GF_LOGD(LOG_TAG "[%s] :wait 0x5018 time out. i=%d, time_out=%d.", __func__, i,
                    time_out);
            ret = GF_ERROR_TIMEOUT;
            break;
        }

        ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_FIFO_STATUS_ADDR, &data);
        if ((data & 0x8000) != 0) {
            GF_LOGD(LOG_TAG "[%s] :data is not ready in fifo.", __func__);
            break;
        }

        fifo_len = (data & 0x07FF) * 2;
//        GF_LOGD(LOG_TAG "[%s] :data is ready in fifo, fifo_len=%d.", __func__, fifo_len);
        if (fifo_len > 0) {
            ret = gf_milan_a_series_secspi_read_bytes_high_speed(GF_MILAN_A_SERIES_FIFO_DATA_ADDR,
                    g_rawdata + total_len, fifo_len);
            total_len += fifo_len;

            if (total_len >= data_len) {
                GF_LOGI(LOG_TAG "[%s] raw data read sucess,total_len=%d.", __func__, total_len);

                //resume to big endian.
                endian_exchange(g_rawdata, data_len);
                ret = gf_milan_a_series_frame_decrypt_multi_dma(g_rawdata, g_gf_raw_data);

                if (GF_SUCCESS != ret) {
                    GF_LOGI(LOG_TAG "[%s] raw data decrypt failed.", __func__);
                }
                break;
            }
        } else {

            GF_LOGE(LOG_TAG "[%s] :error,fifo_len=%d, data=%d.", __func__, fifo_len, data);
        }
    } while (time_out--);

    *buf = g_gf_raw_data;
    *length = g_sensor.row * g_sensor.col;

    GF_LOGD(LOG_TAG "[%s] : raw data word len=%d.", __func__, *length);

    return ret;
}

static gf_error_t gf_milan_a_series_get_image_one_dma(uint16_t **buf, uint32_t *length, uint8_t **origin_buf, uint32_t *origin_length) {
    gf_error_t ret = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] enter(one-shot dma)", __func__);

    if ((NULL == buf) || (NULL == length) || (NULL == origin_buf) || (NULL == origin_length)) {
        GF_LOGE(LOG_TAG "[%s] :invalid param buf=%p, length=%u, origin_buf=%p, origin_length=%u.",
                __func__, buf, *length, origin_buf, *origin_length);
        return GF_ERROR_BAD_PARAMS;
    }

    GF_LOGD(LOG_TAG "[%s] receive image irq. mode=0x%x,irq_status=0x%x.", __func__,
            g_sensor_status.mode, g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET]);

    gf_milan_a_series_notify_fw_adc_reading_begin();

    ret = gf_milan_a_series_secspi_read_bytes_with_speed(GF_MILAN_A_SERIES_FIFO_DATA_ADDR,
                g_rawdata, RAW_READ_LEN, GF_SPI_SPEED_ONE_DMA);
    GF_LOGI(LOG_TAG "[%s] raw data read success,total_len=%u.", __func__, RAW_READ_LEN);
    ret = gf_milan_a_series_frame_decrypt_one_dma(g_rawdata, g_gf_raw_data);
    if (GF_SUCCESS != ret) {
        GF_LOGI(LOG_TAG "[%s] raw data decrypt failed.", __func__);
    }

    if (g_first_get_image > 0) {
        gf_sleep(200);
        g_first_get_image = 0;
        GF_LOGD(LOG_TAG "[%s] bootstart sleep 200ms to wait for fw base", __func__);
    }

    *origin_buf = g_rawdata;
    *origin_length = RAW_READ_LEN;


    *buf = g_gf_raw_data;
    *length = g_sensor.row * g_sensor.col;

    GF_LOGD(LOG_TAG "[%s] : raw data word len=%u, origin_length=%u", __func__, *length, *origin_length);
    return ret;
}

static gf_error_t gf_milan_a_series_get_image(uint16_t **buf, uint32_t *length, uint8_t **origin_buf, uint32_t *origin_length) {
    if (g_milan_a_series_chip_handle.feature.support_fast_acquire_image > 0) {
        return gf_milan_a_series_get_image_one_dma(buf, length, origin_buf, origin_length);
    } else {
        return gf_milan_a_series_get_image_multi_dma(buf, length, origin_buf, origin_length);
    }
}

static gf_error_t milan_ab_navi_partial_decrypt(uint16_t *usPtr, uint8_t *rawdata, uint16_t *out,
    uint8_t start_row, uint8_t line_num) {

    gf_error_t ret = GF_SUCCESS;
    uint16_t rowid;
    uint16_t usCrcCalResult;
    uint32_t i;
    uint32_t naviRowSize = (g_sensor.col+2)*2;

    i = *usPtr;
    do {
        usCrcCalResult = (rawdata[i] << 8) + rawdata[i + 1];
        *usPtr = (rawdata[i+naviRowSize] << 8) + rawdata[i + naviRowSize + 1];
        i++;
    } while(!((g_GF52x6_navi_rawid[start_row] == usCrcCalResult) &&
        (g_GF52x6_navi_rawid[start_row + 1] == *usPtr)) && (i < (NAVI_FRAME_SIZE-naviRowSize)));

    if (i >= (NAVI_FRAME_SIZE-naviRowSize))
    {
        GF_LOGE(LOG_TAG "[%s] :BO1 invalid row id& i= %d.", __func__, i);
        return GF_ERROR_INVALID_DATA;
    }

    *usPtr = i-1;
    for (i = 0; i < line_num; i++)
    {
        rowid = (rawdata[*usPtr] << 8) + rawdata[*usPtr + 1];
        if (rowid >= g_sensor.row)
        {
            GF_LOGE(LOG_TAG "[%s] :BO2 invalid row id= %d.", __func__, rowid);
            return GF_ERROR_INVALID_DATA;
        }

        memcpy(((uint8_t*)out + ((rowid-GF52X6_NAVI_START_ROWID)/4) * (naviRowSize-4)),
                    &rawdata [*usPtr + 2], naviRowSize-4);

        *usPtr += naviRowSize;
    }
    return ret;
}

static gf_error_t milan_ab_nav_frame_decrypt(uint8_t *rawdata, uint16_t *out) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t rowid;
    uint16_t usPtr;
    uint32_t i;
    uint32_t size = 0;
    uint32_t naviRowSize = (g_sensor.col+2)*2;

    //first 4 rows
    usPtr = 0;
    for (i = 0; i < 4; i ++)
    {
        rowid = (rawdata[usPtr] << 8) + rawdata[usPtr + 1];
        // debug assert
        if (rowid >= g_sensor.row)
        {
            GF_LOGE(LOG_TAG "[%s] :AO0 invalid row id=0x%d.", __func__, rowid);
            return GF_ERROR_INVALID_DATA;
        }
        memcpy(((uint8_t*)out + ((rowid-GF52X6_NAVI_START_ROWID)/4) * (naviRowSize-4)),
                    &rawdata [usPtr + 2], naviRowSize-4);
        usPtr += naviRowSize;
    }

    //second 4 rows
    ret = milan_ab_navi_partial_decrypt(&usPtr, rawdata, out, 4, 4);

    if (ret != GF_SUCCESS)
    {
        GF_LOGE(LOG_TAG "[%s] :AO1 invalid row id& i= %d.", __func__, i);
        return GF_ERROR_INVALID_DATA;
    }

    //third 4 rows
    ret = milan_ab_navi_partial_decrypt(&usPtr, rawdata, out, 8, 4);

    if (ret != GF_SUCCESS)
    {
        GF_LOGE(LOG_TAG "[%s] :AO2 invalid row id& i= %d.", __func__, i);
        return GF_ERROR_INVALID_DATA;
    }

    //last 3 rows
    ret = milan_ab_navi_partial_decrypt(&usPtr, rawdata, out, 12, 3);

    if (ret != GF_SUCCESS)
    {
        GF_LOGE(LOG_TAG "[%s] :AO3 invalid row id& i= %d.", __func__, i);
        return GF_ERROR_INVALID_DATA;
    }

    //change endian & divide 2
    size = g_sensor.nav_row * g_sensor.nav_col;
    endian_exchange((uint8_t *) out, size * sizeof(uint16_t));

    for (i = 0; i < size; i++) {
        out[i] = out[i] >> 1;
    }

    GF_LOGD(LOG_TAG "[%s] :raw data crc ok and split success.", __func__);

    ret = GF_SUCCESS;
    return ret;
}

static gf_error_t milan_c_navi_partial_decrypt(uint16_t *usPtr, uint8_t *rawdata, uint16_t *out,
    uint8_t start_row, uint8_t line_num) {

    gf_error_t ret = GF_SUCCESS;
    uint16_t rowid;
    uint16_t usCrcCalResult;
    uint32_t i;
    uint32_t naviRowSize = (g_sensor.col+2)*2;

    i = *usPtr;
    usCrcCalResult = (rawdata[i] << 8) + rawdata[i + 1];
    *usPtr = (rawdata[i+naviRowSize] << 8) + rawdata[i + naviRowSize + 1];
    while (!((g_GF52x8_navi_rawid[start_row] == usCrcCalResult) && (g_GF52x8_navi_rawid[start_row +1 ] == *usPtr))
            && (i < (NAVI_FRAME_SIZE-naviRowSize)))
    {
        i ++;
        usCrcCalResult = (rawdata[i] << 8) + rawdata[i + 1];
        *usPtr = (rawdata[i+naviRowSize] << 8) + rawdata[i + naviRowSize + 1];
    }

    if (i >= (NAVI_FRAME_SIZE-naviRowSize))
    {
        GF_LOGE(LOG_TAG "[%s] :DO1 invalid row id& i= %d.", __func__, i);
        return GF_ERROR_INVALID_DATA;
    }

    *usPtr = i-naviRowSize;
    rawdata[*usPtr] = g_GF52x8_navi_rawid[start_row - 1]>>8;
    rawdata[*usPtr + 1] = g_GF52x8_navi_rawid[start_row - 1]&0xFF;
    for (i = 0; i < line_num; i++)
    {
        rowid = (rawdata[*usPtr] << 8) + rawdata[*usPtr + 1];
        if (rowid >= g_sensor.row)
        {
            GF_LOGE(LOG_TAG "[%s] :DO2 invalid row id= %d.", __func__, rowid);
            return GF_ERROR_INVALID_DATA;
        }

        memcpy(((uint8_t*)out + ((rowid-GF52X8_NAVI_START_ROWID)/4) * (naviRowSize-4)),
                    &rawdata [*usPtr + 2], naviRowSize-4);

        *usPtr += naviRowSize;
    }

    return ret;
}

static gf_error_t milan_c_nav_frame_decrypt(uint8_t *rawdata, uint16_t *out) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t rowid;
    uint16_t usPtr;
    uint32_t i;
    uint32_t size = 0;
    uint32_t naviRowSize = (g_sensor.col+2)*2;
    //normal 8 rows
    for (i = 0; i < 8; i ++)
    {
        usPtr = i * naviRowSize;
        rowid = (rawdata[usPtr] << 8) + rawdata[usPtr + 1];
        // debug assert
        if (rowid >= g_sensor.row)
        {
            GF_LOGE(LOG_TAG "[%s] :cO0 invalid row id= %d.", __func__, rowid);
            return GF_ERROR_INVALID_DATA;
        }
        memcpy(((uint8_t*)out + ((rowid-GF52X8_NAVI_START_ROWID)/4) * (naviRowSize-4)),
                    &rawdata [usPtr + 2], naviRowSize-4);
    }
    usPtr = 8 * naviRowSize;
    //last 4 rows
    ret = milan_c_navi_partial_decrypt(&usPtr, rawdata, out, 9, 4);
    if (ret != GF_SUCCESS)
    {
        GF_LOGE(LOG_TAG "[%s] :CO1 invalid row id &i = %d.", __func__, rowid);
        return GF_ERROR_INVALID_DATA;
    }

    //prev 4 rows
    ret = milan_c_navi_partial_decrypt(&usPtr, rawdata, out, 13, 4);
    if (ret != GF_SUCCESS)
    {
        GF_LOGE(LOG_TAG "[%s] :CO2 invalid row id &i = %d.", __func__, rowid);
        return GF_ERROR_INVALID_DATA;
    }

    size = g_sensor.nav_row * g_sensor.nav_col;
    endian_exchange((uint8_t *) out, size * sizeof(uint16_t));

    for (i = 0; i < size; i++) {
        out[i] = out[i] >> 1;
    }

    GF_LOGD(LOG_TAG "[%s] :raw data crc ok and split success.", __func__);

    ret = GF_SUCCESS;
    return ret;

}

static gf_error_t milan_a_series_get_nav_data(uint8_t *over_flag) {
    gf_error_t ret = GF_SUCCESS;
    uint32_t i = 0;
    uint8_t irq_status = 0;
    uint8_t key_status = 0;

    GF_LOGD(LOG_TAG "[%s] :receive nav irq. mode=0x%x,irq_status=0x%x.", __func__,
            g_sensor_status.mode, g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET]);
    do {
        for (i = 0; i < 50; i++) {
            ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_MODE_ADDR,
                (uint8_t *) &g_sensor_status, GF_MILAN_A_SERIES_SENSOR_STATUS_LEN);
            irq_status = g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET];
            key_status = g_sensor_status.status[GF_MILAN_A_SERIES_KEY_STATUS_OFFSET];

            if (GF_MILAN_A_SERIES_IRQ_NAV == irq_status) {
                ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_CLEAR_ADDR, 0x0002);
                GF_LOGD(LOG_TAG "[%s] :data is ready in fifo.", __func__);
                break;
            }else if (!(key_status & GF_MILAN_A_SERIES_HOME_KEY_STA)){
                GF_LOGD(LOG_TAG "[%s] :Finger Up., keystate=0x%x.", __func__,key_status);
                *over_flag = 2;
                break;
            }
        }

        if (i >= 50 || *over_flag == 2) {
            GF_LOGD(LOG_TAG "[%s] :wait 0x5204 time out. i=%d, key=0x%x.", __func__, i, key_status);
            *over_flag = 2;
            nav_frame_count = 0;
            break;
        }

        ret = gf_milan_a_series_secspi_read_bytes_high_speed(GF_MILAN_A_SERIES_FIFO_DATA_ADDR, g_rawdata, NAVI_FRAME_SIZE);
        gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, 0x0000);  //clear irq;

        *over_flag = 0;

    } while (0);

    return ret;
}

static gf_error_t gf_milan_a_series_get_nav_image(uint16_t **buf, uint32_t *length,
        uint8_t *frame_num, uint8_t *over_flag) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t poll_times = 0;
    uint32_t data_len = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    if ((NULL == buf) || (NULL == length)) {
        GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    data_len = g_sensor.nav_row * g_sensor.nav_col;
    do {
        ret = milan_a_series_get_nav_data(over_flag);
        if (*over_flag == 2) {
            GF_LOGD(LOG_TAG "[%s] : polling nav finger up return %d", __func__, poll_times);
            break;
        }

        if( GF_SUCCESS != ret ) {
            GF_LOGE(LOG_TAG "[%s] : polling nav ERROR process stoped.%d.", __func__, poll_times);
            break;
        }
        endian_exchange(g_rawdata, NAVI_FRAME_SIZE);

        if (g_sensor.config.sensor_type == GF_SENSOR_5206 ||
            g_sensor.config.sensor_type == GF_SENSOR_5216) {
            ret = milan_ab_nav_frame_decrypt(g_rawdata, g_gf_raw_data + poll_times * data_len);
        } else if (g_sensor.config.sensor_type == GF_SENSOR_5208) {
            ret = milan_c_nav_frame_decrypt(g_rawdata, g_gf_raw_data + poll_times * data_len);
        }

        if (GF_SUCCESS != ret) {
            GF_LOGI(LOG_TAG "[%s] raw data decrypt failed.", __func__);
            break;
        }
        if (++nav_frame_count >= g_sensor.config.nav_config.maxNvgFrameNum){
            nav_frame_count = 0;
            *over_flag = 3;
            GF_LOGD(LOG_TAG "[%s] : nav reach nav_max_frame_num.", __func__);
        }
    }while(++poll_times < g_sensor.config.nav_frame_num);

    *buf = g_gf_raw_data;
    *length = poll_times * data_len;
    *frame_num = poll_times;
    GF_LOGD(LOG_TAG "[%s] : raw data word len=%d.", __func__, *length);

    return ret;

}

static gf_error_t gf_milan_a_series_get_nav_base(uint16_t **buf, uint32_t *length) {

    gf_error_t ret = GF_SUCCESS;
    uint32_t data_len = 0;
    uint8_t over_flag = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    data_len = g_sensor.nav_row * g_sensor.nav_col;
    ret = milan_a_series_get_nav_data(&over_flag);
    if( GF_SUCCESS != ret ) {
        GF_LOGD(LOG_TAG "[%s] : get nav base FAILED.", __func__);
    }

    endian_exchange(g_rawdata, NAVI_FRAME_SIZE);

    if (g_sensor.config.sensor_type == GF_SENSOR_5206 ||
        g_sensor.config.sensor_type == GF_SENSOR_5216) {
        ret = milan_ab_nav_frame_decrypt(g_rawdata, g_gf_raw_data);
    } else if (g_sensor.config.sensor_type == GF_SENSOR_5208) {
        ret = milan_c_nav_frame_decrypt(g_rawdata, g_gf_raw_data);
    }

    if (GF_SUCCESS != ret) {
        GF_LOGI(LOG_TAG "[%s] raw data decrypt failed.", __func__);
    }
    *buf = g_gf_raw_data;
    *length = data_len;

    GF_LOGD(LOG_TAG "[%s] exit.", __func__);
    return ret;
}

static gf_error_t gf_milan_a_series_clear_irq(uint32_t irq_type) {
    gf_error_t ret = GF_SUCCESS;
    gf_sensor_status_t sensor_status;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    if ((0 == irq_type) || (irq_type & g_sensor_ignore_irq_mask)) {
        GF_LOGD(LOG_TAG "[%s] ignore irq type mask[0x%x]! return without clear.", __func__,
                g_sensor_ignore_irq_mask);
        return ret;
    }
    cpl_memcpy(&sensor_status, &g_sensor_status, sizeof(gf_sensor_status_t));

    ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_MODE_ADDR,
            (uint8_t *) &g_sensor_status, GF_MILAN_A_SERIES_SENSOR_STATUS_LEN);
    if (sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET]
            != g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET]) {
        GF_LOGD(LOG_TAG "[%s] irq status is changed, do not clear irq!", __func__);
        return ret;
    } else {
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, 0x0000);
    }

    return ret;

}

static gf_error_t gf_milan_a_series_common_get_irq_type(uint32_t *irq_type, uint8_t read_flag,  gf_mode_t current_mode) {

    gf_error_t ret = GF_SUCCESS;
    uint8_t irq_status = 0;
    uint8_t key_status = 0;
    static uint8_t s_read_flag = 0;

    if ((s_read_flag == read_flag) || (read_flag == 1)) {
        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_MODE_ADDR,
                (uint8_t *) &g_sensor_status, GF_MILAN_A_SERIES_SENSOR_STATUS_LEN);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :read irq status failed, ret=%d.", __func__, ret);
            return GF_ERROR_SPI_COMMUNICATION;
        }
    }
    s_read_flag = read_flag;

    irq_status = g_sensor_status.status[GF_MILAN_A_SERIES_IRQ_STATUS_OFFSET];
    key_status = g_sensor_status.status[GF_MILAN_A_SERIES_KEY_STATUS_OFFSET];

    GF_LOGI(LOG_TAG "[%s] :mode=0x%x, irq_status=0x%x, key_status=0x%x.", __func__,
            g_sensor_status.mode, irq_status, key_status);

    if (GF_MILAN_A_SERIES_IRQ_IMAGE == irq_status) {
        gf_milan_a_series_data_sample_suspend();
        *irq_type |= GF_IRQ_IMAGE_MASK;
        GF_LOGD(LOG_TAG "[%s] :received image irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_GSC == irq_status) {
        *irq_type |= GF_IRQ_GSC_MASK;
        GF_LOGD(LOG_TAG "[%s] :received image gsc irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_HBD == irq_status) {
        *irq_type |= GF_IRQ_HBD_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied hbd irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_KEY == irq_status) {
        if (1 == g_sensor.config.support_print_key_value) {
            gf_milan_a_series_print_pixel_key_value();
        }

        if (key_status & GF_MILAN_A_SERIES_HOME_KEY_MASK) {

            if (key_status & GF_MILAN_A_SERIES_HOME_KEY_STA) {
                *irq_type |= GF_IRQ_HOMEKEY_DOWN_MASK;
                GF_LOGD(LOG_TAG "[%s] :home key down, irq_type=0x%x, key_status=0x%x.", __func__,
                        *irq_type, key_status);
            } else {
                *irq_type |= GF_IRQ_HOMEKEY_UP_MASK;
                GF_LOGD(LOG_TAG "[%s] :home key up, irq_type=0x%x, key_status=0x%x.", __func__,
                        *irq_type, key_status);
            }

        }
        if (key_status & GF_MILAN_A_SERIES_MENU_KEY_MASK) {
            if (key_status & GF_MILAN_A_SERIES_MENU_KEY_STA) {
                *irq_type |= GF_IRQ_MENUKEY_DOWN_MASK;
                GF_LOGD(LOG_TAG "[%s] :menu key down, irq_type=0x%x, key_status=0x%x.", __func__,
                        *irq_type, key_status);
            } else {
                *irq_type |= GF_IRQ_MENUKEY_UP_MASK;
                GF_LOGD(LOG_TAG "[%s] :menu key up, irq_type=0x%x, key_status=0x%x.", __func__,
                        *irq_type, key_status);
            }
        }
        if (key_status & GF_MILAN_A_SERIES_BACK_KEY_MASK) {
            if (key_status & GF_MILAN_A_SERIES_BACK_KEY_STA) {
                *irq_type |= GF_IRQ_BACKKEY_DOWN_MASK;
                GF_LOGD(LOG_TAG "[%s] :back key down, irq_type=0x%x, key_status=0x%x.", __func__,
                        *irq_type, key_status);
            } else {
                *irq_type |= GF_IRQ_BACKKEY_UP_MASK;
                GF_LOGD(LOG_TAG "[%s] :back key up, irq_type=0x%x, key_status=0x%x.", __func__,
                        *irq_type, key_status);
            }
        }
    } else if (GF_MILAN_A_SERIES_IRQ_NAV_LEFT == irq_status) {
        *irq_type |= GF_IRQ_NAV_LEFT_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied nav left irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_NAV_RIGHT == irq_status) {
        *irq_type |= GF_IRQ_NAV_RIGHT_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied nav right irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_NAV_UP == irq_status) {
        *irq_type |= GF_IRQ_NAV_UP_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied up irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_NAV_DOWN == irq_status) {
        *irq_type |= GF_IRQ_NAV_DOWN_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied down irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_FW_ERR == irq_status) {
        *irq_type |= GF_IRQ_FW_ERR_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied fw error irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_CONFIG_ERR == irq_status) {
        *irq_type |= GF_IRQ_CFG_ERR_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied cfg error irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_ESD_ERR == irq_status) {
        *irq_type |= GF_IRQ_ESD_ERR_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied esd error irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_FW_RESET == irq_status) {
        *irq_type |= GF_IRQ_FW_ERR_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied fw reset irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_RESET == irq_status) {
        *irq_type |= GF_IRQ_RESET_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied reset irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_UPDATE_BASE == irq_status) {
        *irq_type |= GF_IRQ_UPDATE_BASE_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied update base irq, irq_type=0x%x.", __func__, *irq_type);
    } else if (GF_MILAN_A_SERIES_IRQ_NAV == irq_status) {
        *irq_type |= GF_IRQ_NAV_MASK;
        GF_LOGD(LOG_TAG "[%s] :receied nav irq, irq_type=0x%x.", __func__, *irq_type);
    } else {
        GF_LOGD(LOG_TAG "[%s] :receied invalid irq.", __func__);
    }

    if (MODE_SLEEP == current_mode) {
        uint16_t data = 0;
        gf_milan_a_series_clear_irq((0 | *irq_type));
        *irq_type = 0;
        data = (data & 0xFF00) | (uint16_t) GF_MILAN_A_SERIES_MODE_SLEEP;
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_MODE_ADDR, data);
        GF_LOGI(LOG_TAG "[%s] get the irq on sleep mode", __func__);
    }


    return ret;
}

static gf_error_t gf_milan_a_series_get_irq_type(uint32_t *irq_type, gf_mode_t current_mode) {
    GF_LOGD(LOG_TAG "[%s] :enter.", __func__);

    return gf_milan_a_series_common_get_irq_type(irq_type, 0, current_mode);
}

static gf_error_t gf_milan_a_series_get_irq_status_for_spi_clk(uint32_t *irq_type, gf_mode_t current_mode) {
    gf_error_t ret = GF_SUCCESS;

    GF_LOGD(LOG_TAG "[%s] :enter.", __func__);
    ret = gf_milan_a_series_common_get_irq_type(irq_type, 1, current_mode);

    return ret;
}

static gf_error_t gf_milan_a_series_print_pixel_key_value(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t touch_key_rawdata[68] = {0};
    uint8_t touch_key_base[68] = {0};
    uint8_t touch_key_status[14] = {0};
    uint32_t i = 0;
    uint16_t back_base = 0;
    uint16_t back_rawdata = 0;
    uint16_t menu_base = 0;
    uint16_t menu_rawdata = 0;

    do {
        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_TOUCH_KEY_RAWDATA, touch_key_rawdata, 68);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read touch_key_rawdata failed,err=%d" , __func__, err);
            break;
        }
        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_TOUCH_KEY_BASE, touch_key_base, 68);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read touch_key_base failed,err=%d" , __func__, err);
            break;
        }
        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_TOUCH_KEY_STATUS, touch_key_status, 14);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] read touch_key_status failed,err=%d" , __func__, err);
            break;
        }

        for (i = 0; i < 4; i++) {
            GF_LOGE(LOG_TAG " [%s] touch_key_rawdata :%d, %d, %d, %d, %d, %d, %d, %d, %d,", __func__, i,
                touch_key_rawdata[0 + i*16] + ((uint16_t) touch_key_rawdata[1 + i*16] << 8),
                touch_key_rawdata[2 + i*16] + ((uint16_t) touch_key_rawdata[3 + i*16] << 8),
                touch_key_rawdata[4 + i*16] + ((uint16_t) touch_key_rawdata[5 + i*16] << 8),
                touch_key_rawdata[6 + i*16] + ((uint16_t) touch_key_rawdata[7 + i*16] << 8),
                touch_key_rawdata[8 + i*16] + ((uint16_t) touch_key_rawdata[9 + i*16] << 8),
                touch_key_rawdata[10 + i*16] + ((uint16_t) touch_key_rawdata[11 + i*16] << 8),
                touch_key_rawdata[12 + i*16] + ((uint16_t) touch_key_rawdata[13 + i*16] << 8),
                touch_key_rawdata[14 + i*16] + ((uint16_t) touch_key_rawdata[15 + i*16] << 8));
        }
        for (i = 0; i < 4; i++) {
            GF_LOGI(LOG_TAG " [%s] touch_key_base :%d, %d, %d, %d, %d, %d, %d, %d, %d,", __func__, i,
                touch_key_base[0 + i*16] + ((uint16_t) touch_key_base[1 + i*16] << 8),
                touch_key_base[2 + i*16] + ((uint16_t) touch_key_base[3 + i*16] << 8),
                touch_key_base[4 + i*16] + ((uint16_t) touch_key_base[5 + i*16] << 8),
                touch_key_base[6 + i*16] + ((uint16_t) touch_key_base[7 + i*16] << 8),
                touch_key_base[8 + i*16] + ((uint16_t) touch_key_base[9 + i*16] << 8),
                touch_key_base[10 + i*16] + ((uint16_t) touch_key_base[11 + i*16] << 8),
                touch_key_base[12 + i*16] + ((uint16_t) touch_key_base[13 + i*16] << 8),
                touch_key_base[14 + i*16] + ((uint16_t) touch_key_base[15 + i*16] << 8));
        }
        for (i = 0; i < 1; i++) {
            GF_LOGI(LOG_TAG " [%s] touch_key_status:%d, %d, %d, %d, %d, %d, %d, %d,", __func__, i,
                touch_key_status[ 0+ i*16], touch_key_status[1 + i*16], touch_key_status[2 + i*16], touch_key_status[3 + i*16],
                touch_key_status[4 + i*16], touch_key_status[5 + i*16], touch_key_status[6 + i*16]);
            GF_LOGI(LOG_TAG " [%s] touch_key_status:%d, %d, %d, %d, %d, %d, %d, %d,", __func__, i,
                touch_key_status[7 + i*16], touch_key_status[8 + i*16], touch_key_status[9 + i*16], touch_key_status[10 + i*16],
                touch_key_status[11 + i*16], touch_key_status[12 + i*16], touch_key_status[13 + i*16]);
        }
        menu_base = ((uint16_t) touch_key_base[67] << 8) + touch_key_base[66];
        menu_rawdata = ((uint16_t) touch_key_rawdata[67] << 8) + touch_key_rawdata[66];
        back_base = ((uint16_t) touch_key_base[65] << 8) + touch_key_base[64];
        back_rawdata = ((uint16_t) touch_key_rawdata[65] << 8) + touch_key_rawdata[64];
        GF_LOGI(LOG_TAG " [%s] Menu_Base:%d, Menu_Rawdata:%d, Back_Base:%d, Back_rawdata:%d", __func__,
            menu_base, menu_rawdata, back_base, back_rawdata);

    } while (0);

    return err;
}

static gf_error_t gf_milan_a_series_esd_check(uint8_t *result) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t val = 0;
    uint8_t reg = 0;
    uint16_t esd_chip_id = 0;
    // GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    /*write dummy data to wake up chip*/
    gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_DUMMY_ADDR, 0x0000);
    gf_sleep(1);

    ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_HOLD_CHIP_ID_ADDR, &esd_chip_id);
    if (esd_chip_id != g_gf_chip_id) {
        GF_LOGE(LOG_TAG "[%s] read sensor chip id failed: 0x%x", __func__, esd_chip_id);
    }

    ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_ESD_ADDR, &val);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :read esd failed.", __func__);
        return GF_ERROR_SPI_COMMUNICATION;
    }
    reg = (uint8_t) (val & 0x00FF);
    if (0xC6 == reg) {
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_ESD_ADDR,
                ((val & 0xFF00) | 0x00AA));
        *result = 0;
        // GF_LOGD(LOG_TAG "[%s] esd ok.\n", __func__);
    } else {
        GF_LOGE(LOG_TAG "[%s] :esd check failed.reg=0x%x.", __func__, reg);
        *result = 1;
    }

    if (1 == g_sensor.config.support_print_key_value) {
        gf_milan_a_series_print_pixel_key_value();
    }

    return ret;
}

static gf_error_t gf_milan_a_series_get_key_status(uint8_t *touch) {
    gf_error_t err = GF_SUCCESS;
    uint16_t data = 0;
    uint8_t key_status = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if (NULL == touch) {
            GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_KEY_STATUS_ADDR - 1, &data);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] :read key status failed, ret=%d.", __func__, err);
            err = GF_ERROR_SPI_COMMUNICATION;
            break;
        }

        key_status = (uint8_t) ((data >> 8) & 0x00FF);
        if (key_status & GF_MILAN_A_SERIES_HOME_KEY_STA) {
            *touch = 1;
        } else {
            *touch = 0;
        }

        GF_LOGD(LOG_TAG "[%s] :key_status=0x%x, touch=0x%x", __func__, key_status,
                *touch);
    } while (0);

    return err;
}

static gf_error_t gf_milan_a_series_get_sensor_status(uint8_t *touch, uint8_t *temperature) {

    gf_error_t ret = GF_SUCCESS;
    uint16_t data = 0;
    uint8_t key_status = 0;
    uint8_t temp_status = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if ((NULL == touch) || (NULL == temperature)) {
            GF_LOGE(LOG_TAG "[%s] :invalid param.", __func__);
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_KEY_STATUS_ADDR - 1, &data);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :read key status failed, ret=%d.", __func__, ret);
            ret = GF_ERROR_SPI_COMMUNICATION;
            break;
        }

        key_status = (uint8_t) ((data >> 8) & 0x00FF);
        if (key_status & GF_MILAN_A_SERIES_HOME_KEY_STA) {
            *touch = 1;
        } else {
            *touch = 0;
        }

        ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TEMP_STATUS_ADDR, &data);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :read temperature status failed, ret=%d.", __func__, ret);
            ret = GF_ERROR_SPI_COMMUNICATION;
            break;
        }

        temp_status = (uint8_t) (data & 0x00FF);
        *temperature = temp_status;

        GF_LOGD(LOG_TAG "[%s] :key_status=0x%x, touch=0x%x, temp_status=0x%x.", __func__, key_status,
                *touch, temp_status);
    } while (0);

    return ret;
}

static gf_error_t gf_milan_a_series_get_hbd_base(uint16_t *base) {
    gf_error_t ret = GF_SUCCESS;

    ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_GSC_BASE_ADDR, base);
    if (GF_SUCCESS != ret) {
        GF_LOGE(LOG_TAG "[%s] :read hbd base failed, ret=%d.", __func__, ret);
        return GF_ERROR_SPI_COMMUNICATION;
    }

    return ret;
}

static gf_error_t gf_milan_a_series_get_gsc(uint8_t **buf, uint32_t *length) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {

        err = gf_milan_a_series_get_hbd_data();

        if (NULL == buf || NULL == length) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        *buf = gf_milan_a_series_hbd_raw_buf;

        *length = GF_MILAN_A_SERIES_HBD_BUFF1_SIZE + GF_MILAN_A_SERIES_HBD_BUFF2_SIZE;

        if (g_hbd_data_type == GF_MILAN_A_SERIES_HBD_GSC_DATA) {
            cpl_memcpy(*buf, hbd_buf1, GF_MILAN_A_SERIES_HBD_BUFF1_SIZE);
            cpl_memcpy(*buf + GF_MILAN_A_SERIES_HBD_BUFF1_SIZE, hbd_buf2,
                    GF_MILAN_A_SERIES_HBD_BUFF2_SIZE);
        } else {
            GF_LOGE(LOG_TAG "[%s] fail to get gsc data.", __func__);
            err = GF_ERROR_GENERIC;
            break;
        }
    } while (0);

    //reset data status
    g_hbd_data_type = GF_MILAN_A_SERIES_HBD_NOP_DATA;
    return err;
}

static gf_error_t gf_milan_a_series_get_hbd(uint8_t **buf, uint32_t *length) {
    gf_error_t err = GF_SUCCESS;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        gf_milan_a_series_get_hbd_data();

        if (NULL == buf || NULL == length) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        *buf = gf_milan_a_series_hbd_raw_buf;

        *length = GF_MILAN_A_SERIES_HBD_BUFF1_SIZE;

        if (g_hbd_data_type == GF_MILAN_A_SERIES_HBD_HBD_BUFFER1_DATA) {
            cpl_memcpy(*buf, hbd_buf1, *length);
        } else if (g_hbd_data_type == GF_MILAN_A_SERIES_HBD_HBD_BUFFER2_DATA) {
            cpl_memcpy(*buf, hbd_buf2, *length);
        } else {
            GF_LOGE(LOG_TAG "[%s] fail to get hbd data.", __func__);
            err = GF_ERROR_GENERIC;
            break;
        }

    } while (0);
    //reset data status
    g_hbd_data_type = GF_MILAN_A_SERIES_HBD_NOP_DATA;

    return err;
}

static gf_error_t gf_milan_a_series_set_sensor_dac_value(uint16_t delta, uint32_t is_trigger_image) {
    gf_error_t err = GF_SUCCESS;
    uint16_t i = 0;
    uint16_t tmp_sum = 0;

    uint16_t tcode = 0;
    uint16_t tmp_dac1[4];
    uint16_t tmp_dac2[4];
    uint16_t tmp_dac3[4];

    uint8_t read_tmp_dac1[4];
    uint8_t read_tmp_dac2[4];
    uint8_t read_tmp_dac3[4];

    uint8_t tmp_dac12_bit9 = 0;
    uint8_t tmp_dac3_bit9 = (gf_milan_a_series_pixel_test_chip_info.dac3_bit9 & 0xF0);

    uint8_t tmp_dac[2] = { 0 };
    uint16_t checksum = 0;

    uint16_t release_mcu_buf = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {

        if (delta > 0x1FF || NULL == g_pixel_test_cfg_data) {
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        for (i = 0; i < 4; i++) {
            tmp_dac1[i] = ((uint16_t) ((gf_milan_a_series_pixel_test_chip_info.dac12_bit9 >> i)
                    & 0x01) << 8) + gf_milan_a_series_pixel_test_chip_info.dac1[i] + delta;

            tmp_dac2[i] =
                    ((uint16_t) ((gf_milan_a_series_pixel_test_chip_info.dac12_bit9 >> (i + 4))
                            & 0x01) << 8) + gf_milan_a_series_pixel_test_chip_info.dac2[i] + delta;

            tmp_dac12_bit9 |= ((uint8_t) ((tmp_dac1[i] >> 8) & 0x01) << i)
                    + ((uint8_t) ((tmp_dac2[i] >> 8) & 0x01) << (i + 4));

            tmp_dac3[i] = ((uint16_t) ((gf_milan_a_series_pixel_test_chip_info.dac3_bit9 >> i)
                    & 0x01) << 8) + gf_milan_a_series_pixel_test_chip_info.dac3[i] + delta;

            tmp_dac3_bit9 |= ((uint8_t) ((tmp_dac3[i] >> 8) & 0x01) << i);
        }

        tmp_sum = gf_milan_a_series_pixel_test_chip_info.checksum + delta * 12;

        GF_LOGD(LOG_TAG "[%s]  tcode = %d, tmp_sum = %d, delta = %d", __func__,
                gf_milan_a_series_pixel_test_chip_info.tcode, tmp_sum, delta);

        for (i = 0; i < 4; i++) {
            GF_LOGD(LOG_TAG "[%s] tmp_dac1[%d] = %d", __func__, i, (uint8_t) tmp_dac1[i]);
            GF_LOGD(LOG_TAG "[%s] tmp_dac2[%d] = %d", __func__, i, (uint8_t) tmp_dac2[i]);
            GF_LOGD(LOG_TAG "[%s] tmp_dac3[%d] = %d", __func__, i, (uint8_t) tmp_dac3[i]);
        }

        GF_LOGD(LOG_TAG "[%s] tmp_dac12_bit9 = %d", __func__, tmp_dac12_bit9);
        GF_LOGD(LOG_TAG "[%s] tmp_dac3_bit9 = %d", __func__, tmp_dac3_bit9);

//update config array
        for (i = 0; i < 4; i++) {
            MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_BYTE(g_pixel_test_cfg_data,
                    (GF_MILAN_A_SERIES_DAC1_START_ADDR + i), (uint8_t )(tmp_dac1[i] & 0x00FF));

            MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_BYTE(g_pixel_test_cfg_data,
                    (GF_MILAN_A_SERIES_DAC2_START_ADDR + i), (uint8_t )(tmp_dac2[i] & 0x00FF));

            MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_BYTE(g_pixel_test_cfg_data,
                    (GF_MILAN_A_SERIES_DAC3_START_ADDR + i), (uint8_t )(tmp_dac3[i] & 0x00FF));
        }

        MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_BYTE(g_pixel_test_cfg_data,
                GF_MILAN_A_SERIES_DAC12_BIT9_ADDR, (uint8_t )tmp_dac12_bit9);

        MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_BYTE(g_pixel_test_cfg_data,
                GF_MILAN_A_SERIES_DAC3_BIT9_ADDR, (uint8_t )tmp_dac3_bit9);

        MILAN_A_SERIES_UPDATE_CONFIG_ARRAY_BY_WORD(g_pixel_test_cfg_data,
                GF_MILAN_A_SERIES_CHECKSUM_ADDR, tmp_sum);

        //download cfg
//        err = gf_milan_a_series_set_mode(MODE_IDLE);

        if (delta > 0) {
            err = gf_milan_a_series_download_pixel_test_cfg(g_pixel_test_cfg_data);
        } else {
            err = gf_milan_a_series_download_pixel_test_cfg(g_sensor.fw_cfg.milan_a_series.cfg_data);
        }

        err = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR,
                &release_mcu_buf);
        if (GF_SUCCESS != err) {
            GF_LOGE(LOG_TAG "[%s] :read release mcu failed, ret = %d.", __func__, err);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] release_mcu_buf  = %d", __func__, release_mcu_buf);

//test read
        err = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TCODE_ADDR, &tcode);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read tcode failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC1_START_ADDR,
                (uint8_t *) read_tmp_dac1, 4);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac1 failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC2_START_ADDR,
                (uint8_t *) read_tmp_dac2, 4);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac2 failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC3_START_ADDR,
                (uint8_t *) read_tmp_dac3, 4);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac2 failed %d", __func__, err);
            break;
        }
        err = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_CHECKSUM_ADDR, &checksum);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read checksum failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes((GF_MILAN_A_SERIES_DAC12_BIT9_ADDR - 1), tmp_dac,
                2);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac12_bit9 failed %d", __func__, err);
            break;
        }
        tmp_dac12_bit9 = tmp_dac[1];

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC3_BIT9_ADDR, tmp_dac, 2);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac12_bit9 failed %d", __func__, err);
            break;
        }
        tmp_dac3_bit9 = tmp_dac[0];

//test print
        for (i = 0; i < 4; i++) {
            GF_LOGD(LOG_TAG "[%s] read_tmp_dac1[%d] = %d", __func__, i, read_tmp_dac1[i]);
            GF_LOGD(LOG_TAG "[%s] read_tmp_dac2[%d] = %d", __func__, i, read_tmp_dac2[i]);
            GF_LOGD(LOG_TAG "[%s] read_tmp_dac3[%d] = %d", __func__, i, read_tmp_dac3[i]);
        }

        GF_LOGD(LOG_TAG "[%s] tmp_dac12_bit9 = %d", __func__, tmp_dac12_bit9);
        GF_LOGD(LOG_TAG "[%s] tmp_dac3_bit9 = %d", __func__, tmp_dac3_bit9);

        GF_LOGD(LOG_TAG "[%s] tcode = %d", __func__, tcode);
        GF_LOGD(LOG_TAG "[%s] checksum = %d", __func__, checksum);

        if (is_trigger_image) {

            err = gf_milan_a_series_set_mode(MODE_DEBUG);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] set milan_a_series debug mode failed %d", __func__, err);
                break;
            }

            err = gf_milan_a_series_data_sample_resume();
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] data sample resume failed %d", __func__, err);
                break;
            }
        }

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_milan_a_series_read_tx_and_dac(void) {
    gf_error_t err = GF_SUCCESS;
    uint8_t i = 0;

    uint8_t dac12_bit9_temp[2] = { 0 };
    uint8_t dac3_bit9_temp[2] = { 0 };

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TCODE_ADDR,
                &(gf_milan_a_series_pixel_test_chip_info.tcode));
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read tcode failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC1_START_ADDR,
                gf_milan_a_series_pixel_test_chip_info.dac1, 4);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac1 failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC2_START_ADDR,
                gf_milan_a_series_pixel_test_chip_info.dac2, 4);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac2 failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC3_START_ADDR,
                gf_milan_a_series_pixel_test_chip_info.dac3, 4);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac3 failed %d", __func__, err);
            break;
        }

        err = gf_milan_a_series_secspi_read_bytes((GF_MILAN_A_SERIES_DAC12_BIT9_ADDR - 1),
                dac12_bit9_temp, 2);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac12_bit9 failed %d", __func__, err);
            break;
        }

        gf_milan_a_series_pixel_test_chip_info.dac12_bit9 = dac12_bit9_temp[1];

        err = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_DAC3_BIT9_ADDR, dac3_bit9_temp,
                2);
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read dac3_bit9 failed %d", __func__, err);
            break;
        }
        gf_milan_a_series_pixel_test_chip_info.dac3_bit9 = dac3_bit9_temp[0];

        err = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_CHECKSUM_ADDR,
                &(gf_milan_a_series_pixel_test_chip_info.checksum));
        if (err != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read checksum failed %d", __func__, err);
            break;
        }

//debug print
        for (i = 0; i < 4; i++) {
            GF_LOGD(LOG_TAG "[%s] tmp_dac1[%d] = %d", __func__, i,
                    gf_milan_a_series_pixel_test_chip_info.dac1[i]);
            GF_LOGD(LOG_TAG "[%s] tmp_dac2[%d] = %d", __func__, i,
                    gf_milan_a_series_pixel_test_chip_info.dac2[i]);
            GF_LOGD(LOG_TAG "[%s] tmp_dac3[%d] = %d", __func__, i,
                    gf_milan_a_series_pixel_test_chip_info.dac3[i]);
        }

        GF_LOGD(LOG_TAG "[%s] dac12_bit9 = 0x%x", __func__,
                gf_milan_a_series_pixel_test_chip_info.dac12_bit9);
        GF_LOGD(LOG_TAG "[%s] dac3_bit9 = 0x%x", __func__,
                gf_milan_a_series_pixel_test_chip_info.dac3_bit9);

        GF_LOGD(LOG_TAG "[%s] tcode = %d", __func__, gf_milan_a_series_pixel_test_chip_info.tcode);
        GF_LOGD(LOG_TAG "[%s] checksum = %d", __func__,
                gf_milan_a_series_pixel_test_chip_info.checksum);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_milan_a_series_update_dac_for_sensor_pixel_open_test(
        uint32_t is_set_default_dac, uint32_t is_trigger_image) {

    gf_error_t err = GF_SUCCESS;
    uint16_t offset = 0;

    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        if (is_set_default_dac) {
            err = gf_milan_a_series_set_sensor_dac_value(offset, is_trigger_image);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] read set_sensor_dac failed %d", __func__, err);
                break;
            }
        } else {
            err = gf_milan_a_series_read_tx_and_dac();
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] read tcode and dac failed %d", __func__, err);
                break;
            }

            if (0 == gf_milan_a_series_pixel_test_chip_info.tcode) {
                GF_LOGE(LOG_TAG "[%s] read tcode = %d cannot be used to update dac value", __func__,
                        gf_milan_a_series_pixel_test_chip_info.tcode);
                break;
            }

            offset = 128 * 16 / (gf_milan_a_series_pixel_test_chip_info.tcode);
            err = gf_milan_a_series_set_sensor_dac_value(offset, is_trigger_image);
            if (err != GF_SUCCESS) {
                GF_LOGE(LOG_TAG "[%s] read set_sensor_dac failed, offset = %d, err= %d", __func__,
                        offset, err);
                break;
            }
        }
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, err);
    return err;
}

static gf_error_t gf_milan_a_series_test_download_fw_cfg(uint8_t *fw_cfg_data_test, uint32_t fw_cfg_data_len,
        gf_download_fw_cfg_status_t fw_cfg_status) {

    gf_error_t ret = GF_SUCCESS;
    uint32_t loop_time = 0;
    uint32_t i = 0;
    uint32_t fw_len = 0;
    uint8_t *fw_data = NULL;
    uint32_t cfg_len = 0;
    uint8_t *cfg_data = NULL;
    uint8_t tmp_buf[1024] = { 0 };
    uint16_t data = 0;
    uint8_t need_to_release_mcu = 0;
    uint8_t need_to_disable_spi_ram = 0;
    uint8_t enable_flag = 1;

    GF_LOGD(LOG_TAG "[%s] enter, fw_cfg_status=%u.", __func__, fw_cfg_status);

    if (GF_TEST_DOWNLOAD_FW == fw_cfg_status) {
        fw_data = fw_cfg_data_test;
        fw_len = fw_cfg_data_len;
    } else {
        cfg_data = g_sensor.fw_cfg.milan_a_series.cfg_data;
        cfg_len = g_sensor.fw_cfg.milan_a_series.cfg_data_len;
        fw_len = g_sensor.fw_cfg.milan_a_series.fw_data_len;
        fw_data = g_sensor.fw_cfg.milan_a_series.fw_data;
        UNUSED_VAR(fw_cfg_data_test);
        UNUSED_VAR(fw_cfg_data_len);
    }

    GF_LOGD(LOG_TAG "[%s] :ready to load fw. cfg len=%d, fw len= %d.", __func__, cfg_len, fw_len);
    do {
        //hold mcu.
        GF_LOGD(LOG_TAG "[%s] :do hold mcu.", __func__);
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0200);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :hold mcu failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_release_mcu = 1;

        //enable spi  ram.
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_ENABLE_SPI_RAM_ADDR, 0x0003);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :enable spi ram failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_disable_spi_ram = 1;

        if (GF_TEST_DOWNLOAD_CFG == fw_cfg_status) {
            //download cfg.
            ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_CFG_ADDR, cfg_data, cfg_len);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :update cfg failed.", __func__);
                break;
            }

            //read back cfg data to check.
            ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_CFG_ADDR, tmp_buf, cfg_len);
            GF_LOGD(LOG_TAG "[%s] : cfg len=%d, fw len= %d.", __func__, cfg_len, fw_len);
            if (memcmp(cfg_data, tmp_buf, cfg_len)) {
                GF_LOGE(LOG_TAG "[%s] :cfg read data is different from write.", __func__);
                break;
            }

            GF_LOGD(LOG_TAG "[%s] :cfg download success.", __func__);
        }

        loop_time = (fw_len / 1000);
        for (i = 0; i < loop_time; i++) {
            ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_FW_ADDR + i * 1000, (fw_data + i * 1000),  1000);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :update fw failed.", __func__);
                break;
           }
        }

        if ((fw_len % 1000) > 0)  {
            ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_FW_ADDR + loop_time * 1000, (fw_data + loop_time * 1000),  (fw_len % 1000));
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :update fw failed.", __func__);
                break;
           }
        }

        //read back fw data to check.
        loop_time = (fw_len / 512);
        for (i = 0; i < loop_time; i++) {
            ret = gf_milan_a_series_secspi_read_bytes((GF_MILAN_A_SERIES_FW_ADDR + i * 512),
                    tmp_buf, 512);
            if (memcmp((fw_data + i * 512), tmp_buf, 512)) {
                GF_LOGE(
                        LOG_TAG "[%s] :fw read data is different write,address=0x%p,i=%d,loop_time=%d.",
                        __func__, (void*) (fw_data + i * 512), i, loop_time);
                ret = GF_ERROR_SPI_FW_DOWNLOAD_FAILED;
                break;
            }
        }

        if (GF_SUCCESS != ret) {
            break;
        }

        //diable spi  ram.
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_ENABLE_SPI_RAM_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :diable spi ram failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_disable_spi_ram = 0;

        //release mcu.
        GF_LOGD(LOG_TAG "[%s] :do release mcu.", __func__);
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :release mcu failed, ret = %d.", __func__, ret);
            break;
        }
        need_to_release_mcu = 0;

        //delay for fw running and clear reset irq.
        gf_sleep(20);
        for (i = 0; i < 5; i++) {
            gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, &data);
            if ((data & 0xff00) == 0xE400) {
                i = 0;
                gf_milan_a_series_update_temprature_dac();
                break;
            }
            gf_sleep(20);
            GF_LOGD(LOG_TAG "[%s] :read irq status %x. i %d", __func__, data, i);
        }

        if (5 == i) {
            GF_LOGE(LOG_TAG "[%s] :not detected E4 interrupt!", __func__);
            gf_milan_a_series_update_temprature_dac();
        }

        gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_IRQ_RW_ADDR, 0x0000);
        GF_LOGD(LOG_TAG "[%s] :fw download success.", __func__);

    } while (0);

    if (need_to_disable_spi_ram == 1) {
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_ENABLE_SPI_RAM_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :diable spi ram failed, ret = %d.", __func__, ret);
        }
    }

    if (need_to_release_mcu == 1) {
        GF_LOGD(LOG_TAG "[%s] :do release mcu.", __func__);
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_HOLD_RELEASE_MCU_ADDR, 0x0000);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :release mcu failed, ret = %d.", __func__, ret);
        }
    }

    if (g_milan_a_series_chip_handle.feature.support_reissue_key_down > 0) {
        if (1 == g_sensor.config.reissue_key_down_when_entry_ff_mode) {
            enable_flag = GF_MILAN_A_SERIES_REISSUE_DOWN_CONFIG_VALUE;
        } else {
            enable_flag = GF_MILAN_A_SERIES_NOT_REISSUE_DOWN_CONFIG_VALUE;
        }
        gf_milan_a_series_secspi_write_byte(
            GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE_ADDR, enable_flag);
        GF_LOGD(LOG_TAG "[%s] :set ff reissue key down enable = %u.", __func__, enable_flag);

        if (1 == g_sensor.config.reissue_key_down_when_entry_image_mode) {
            enable_flag = GF_MILAN_A_SERIES_REISSUE_DOWN_CONFIG_VALUE;
        } else {
            enable_flag = GF_MILAN_A_SERIES_NOT_REISSUE_DOWN_CONFIG_VALUE;
        }
        gf_milan_a_series_secspi_write_byte(
            GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE_ADDR, enable_flag);
        GF_LOGD(LOG_TAG "[%s] :set image reissue key down enable = %u.", __func__, enable_flag);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err:%d", __func__, ret);
    return ret;
}

#if 0
static gf_error_t gf_milan_a_series_get_temperature(int32_t *temperature_value) {
    gf_error_t ret = GF_SUCCESS;
    uint16_t read_value = 0;
    uint32_t n = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if (NULL == temperature_value) {
            ret = GF_ERROR_BAD_PARAMS;
            break;
        }

        ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TEMPERATURE_CONTROL, &read_value);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :read failed.", __func__);
            break;
        }

        read_value&=0xFF;
        read_value|=0x0010;
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_TEMPERATURE_CONTROL, read_value);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :write 0x%x failed, ret = %d.", __func__, GF_MILAN_A_SERIES_TEMPERATURE_CONTROL,ret);
            break;
        }

        while(n <= 15 ) {
            ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_TEMPERATURE_CONTROL, n);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :write 0x%x failed, ret = %d.", __func__, GF_MILAN_A_SERIES_TEMPERATURE_CONTROL,ret);
                break;
            }

            ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TEMPERATURE_DA_TM_CODE, &read_value);
            if (GF_SUCCESS != ret) {
                GF_LOGE(LOG_TAG "[%s] :read failed.", __func__);
                break;
            }

            if (1 == read_value) {
                break;
            }

            n++;
        }

        ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TEMPERATURE_CONTROL, &read_value);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :read failed.", __func__);
            break;
        }

        read_value&= ~0x0010;
        ret = gf_milan_a_series_secspi_write_word(GF_MILAN_A_SERIES_TEMPERATURE_CONTROL, read_value);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :write 0x%x failed, ret = %d.", __func__, GF_MILAN_A_SERIES_TEMPERATURE_CONTROL,ret);
            break;
        }

        if (n >= 15) {
            ret = GF_ERROR_GET_TEMPERATURE_FAILED;
        }
    }while(0);

    if(GF_SUCCESS == ret) {
        *temperature_value = n;
    }

    GF_LOGD(LOG_TAG "[%s] exit:%d,%d.", __func__, n, ret);
    return ret;
}
#endif

/* open the led0 to dimming */
static gf_error_t test_get_hbd_switch(uint8_t *hdb_switch_value) {

    gf_error_t ret = GF_SUCCESS;
    uint8_t hdb_switch_value_tmp = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        ret = gf_milan_a_series_secspi_read_byte(GF_MILAN_A_SERIES_HBD_SWITCH_ADDR,
                &hdb_switch_value_tmp);
        if (GF_SUCCESS != ret) {
            GF_LOGD(LOG_TAG "[%s] read led0 switch value error.", __func__);
            break;
        }
        *hdb_switch_value = (uint8_t) ((hdb_switch_value_tmp) | 0x0004);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit:%d.", __func__, ret);
    return ret;
}

static gf_error_t test_set_config_item(uint8_t *cfg_data, uint16_t offset, uint8_t *data,
        uint16_t len) {

    gf_error_t ret = GF_SUCCESS;
    uint16_t index = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if ((NULL == cfg_data) || (NULL == data) || (0 == len)) {
            ret = GF_ERROR_BAD_PARAMS;
            GF_LOGD(LOG_TAG "[%s] cfg_data or data or len is invalid value.", __func__);
            break;
        }

        if (1 == len) {
            cfg_data[offset] = *data;
            break;
        }

        while (index < len) {
            cfg_data[offset + index] = data[index];
            index = index + 1;
        };

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit:%d.", __func__, ret);
    return ret;
}

static gf_error_t test_set_hbd_cfg(void) {

    gf_error_t ret = GF_SUCCESS;
    uint8_t hdb_cfg[10] = { 0 };
    uint8_t hdb_step[2] = { 0 };
    uint8_t hbd_drv[2] = { 0 };
    uint8_t hdb_switch_value = 0;

    uint32_t cfg_data_len = 0;
    uint8_t *cfg_data = NULL;

    uint16_t temp_config[MILAN_A_SERIES_SIZE_CONFIG] = { 0 };
    uint16_t chksum;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        cfg_data_len = g_sensor.fw_cfg.milan_a_series.cfg_data_len;
        cpl_memcpy(temp_config, g_sensor.fw_cfg.milan_a_series.cfg_data, cfg_data_len);
        cfg_data = (uint8_t*) temp_config;

        ret = test_get_hbd_switch(&hdb_switch_value);
        if (GF_SUCCESS != ret) {
            GF_LOGD(LOG_TAG "[%s] read led0 switch value error.", __func__);
            break;
        }
        /* 0x08B0 - 0x0842 = 0x6E = 110*/
        test_set_config_item(cfg_data, 110, &hdb_switch_value, 1);

        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_HBD_CONFIG_ADDR, hdb_cfg, 10);
        if (GF_SUCCESS != ret) {
            GF_LOGD(LOG_TAG "[%s] read hbd config error.", __func__);
            break;
        }
        /* 0x08B4 - 0x0842 = 0x72 =114 */
        test_set_config_item(cfg_data, 114, hdb_cfg, 10);

        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_HBD_STEP_ADDR, hdb_step, 2);
        if (GF_SUCCESS != ret) {
            GF_LOGD(LOG_TAG "[%s] read setp error.", __func__);
            break;
        }
        /* 0x08C2 - 0x0842 = 0x80 =128 */
        test_set_config_item(cfg_data, 128, hdb_step, 2);

        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_HBD_LED0_DRV_ADDR, hbd_drv, 2);
        if (GF_SUCCESS != ret) {
            GF_LOGD(LOG_TAG "[%s] read led0 driver error.", __func__);
            break;
        }

        GF_LOGD(LOG_TAG "[%s] hbd_drv[0]: 0x%x, hbd_drv[1]: 0x%x", __func__, hbd_drv[0],
                hbd_drv[1]);
        /* 0x08BE - 0x0842 = 0x7C =124 */
        hbd_drv[1] = 0x30;
        test_set_config_item(cfg_data, 124, hbd_drv, 2);

        /* update config checksum */
        chksum = milan_a_series_otp_get_checksum16(cfg_data,
                (MILAN_A_SERIES_SIZE_CONFIG - 4) / 2);
        GF_LOGD(LOG_TAG "[%s] chksum %d ", __func__, chksum);

        cfg_data[MILAN_A_SERIES_SIZE_CONFIG - 4] = chksum & 0xFF;
        cfg_data[MILAN_A_SERIES_SIZE_CONFIG - 3] = chksum >> 8;
        cfg_data[MILAN_A_SERIES_SIZE_CONFIG - 1] = 0x01;

        ret = milan_a_series_download_cfg(cfg_data);

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit:%d.", __func__, ret);
    return ret;
}

static gf_error_t milan_a_series_download_cfg(uint8_t *cfg_data) {

    gf_error_t ret = GF_SUCCESS;
    uint32_t cfg_len = 0;
    uint8_t tmp_buf[512] = { 0 };

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        cfg_len = g_sensor.fw_cfg.milan_a_series.cfg_data_len;

        ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_CFG_ADDR, cfg_data, cfg_len);
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :recover cfg failed.", __func__);
            break;
        }

        ret = gf_milan_a_series_secspi_read_bytes(GF_MILAN_A_SERIES_CFG_ADDR, tmp_buf, cfg_len);
        if (GF_SUCCESS != ret) {
            GF_LOGD(LOG_TAG "[%s] read cfg error %d.", __func__, ret);
            break;
        }

        if (memcmp(cfg_data, tmp_buf, cfg_len)) {
            GF_LOGD(LOG_TAG "[%s] download cfg error %d.", __func__, ret);
            break;
        }

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit:%d.", __func__, ret);
    return ret;
}

static gf_error_t gf_milan_a_series_set_config_for_stable_acquire_image(uint8_t flag) {

    gf_error_t ret = GF_SUCCESS;
    uint16_t tmp_cfg[GF_MILAN_A_SERIES_SIZE_CONFIG / 2] = { 0 };
    uint16_t chksum = 0;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);
    //flag = 0 for close & flag =1 for open
    do {
        if (0 == g_milan_a_series_chip_handle.feature.support_stable_acquire_image) {
            GF_LOGE(LOG_TAG "[%s] not support stable acquire image", __func__);
            break;
        }
        cpl_memcpy(tmp_cfg, g_sensor.fw_cfg.milan_a_series.cfg_data, GF_MILAN_A_SERIES_SIZE_CONFIG);
        if (0 == flag) {
            tmp_cfg[GF_MILAN_A_SERIES_STABLE_JUDGE_CONFIG_INDEX / 2] =
                    GF_MILAN_A_SERIES_NOT_JUDGE_STABLE_CONFIG_VALUE;
        }else{
            tmp_cfg[GF_MILAN_A_SERIES_STABLE_JUDGE_CONFIG_INDEX / 2] =
                    GF_MILAN_A_SERIES_JUDGE_STABLE_CONFIG_VALUE;
        }

        chksum = milan_a_series_otp_get_checksum16((uint8_t *) tmp_cfg,
                (GF_MILAN_A_SERIES_SIZE_CONFIG - 4) / 2);
        tmp_cfg[GF_MILAN_A_SERIES_SIZE_CONFIG / 2 - 2] = chksum;

        ret = gf_milan_a_series_secspi_write_word(
                GF_MILAN_A_SERIES_CFG_ADDR + GF_MILAN_A_SERIES_STABLE_JUDGE_CONFIG_INDEX,
                tmp_cfg[GF_MILAN_A_SERIES_STABLE_JUDGE_CONFIG_INDEX / 2]);

        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :write cfg failed.", __func__);
            break;
        }

        ret = gf_milan_a_series_secspi_write_word(
                GF_MILAN_A_SERIES_CFG_ADDR + GF_MILAN_A_SERIES_SIZE_CONFIG - 4,
                tmp_cfg[GF_MILAN_A_SERIES_SIZE_CONFIG / 2 - 2]);

        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :write checksum failed.", __func__);
            break;
        }

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit:%d.", __func__, ret);
    return ret;
}

static gf_error_t gf_milan_a_series_enable_reissue_key_down(gf_mode_t mode) {
    gf_error_t ret = GF_SUCCESS;
    uint8_t enable_flag = 1;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);
    do{
        if (0 == g_milan_a_series_chip_handle.feature.support_reissue_key_down) {
            GF_LOGE(LOG_TAG "[%s] not support reissue key down", __func__);
            break;
        }
        switch (mode) {
            case MODE_FF:
                ret = gf_milan_a_series_secspi_write_byte(
                GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE_ADDR, enable_flag);
                break;
            case MODE_IMAGE:
                ret = gf_milan_a_series_secspi_write_byte(
                GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE_ADDR, enable_flag);
                break;
            default:
                break;
        }
    }while(0);

    GF_LOGD(LOG_TAG "[%s] exit.", __func__);
    return ret;
}

static gf_error_t gf_milan_a_series_update_cfg_for_reissue_key_down(void) {

    gf_error_t ret = GF_SUCCESS;
    uint8_t enable_flag = 0;
    GF_LOGD(LOG_TAG "[%s] enter.", __func__);
    do {

        if (0 == g_milan_a_series_chip_handle.feature.support_reissue_key_down) {
            GF_LOGE(LOG_TAG "[%s] not support reissue key down", __func__);
            break;
        }

        if (1 == g_sensor.config.reissue_key_down_when_entry_ff_mode) {
            enable_flag = GF_MILAN_A_SERIES_REISSUE_DOWN_CONFIG_VALUE;
        } else {
            enable_flag = GF_MILAN_A_SERIES_NOT_REISSUE_DOWN_CONFIG_VALUE;
        }
        ret = gf_milan_a_series_secspi_write_byte(
            GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE_ADDR, enable_flag);
        GF_LOGD(LOG_TAG "[%s] :set ff reissue key down enable = %u.", __func__, enable_flag);

        if (1 == g_sensor.config.reissue_key_down_when_entry_image_mode) {
            enable_flag = GF_MILAN_A_SERIES_REISSUE_DOWN_CONFIG_VALUE;
        } else {
            enable_flag = GF_MILAN_A_SERIES_NOT_REISSUE_DOWN_CONFIG_VALUE;
        }
        ret = gf_milan_a_series_secspi_write_byte(
            GF_MILAN_A_SERIES_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE_ADDR, enable_flag);
        GF_LOGD(LOG_TAG "[%s] :set image reissue key down enable = %u.", __func__, enable_flag);
    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit.", __func__);
    return ret;
}

static gf_error_t gf_set_ignore_irq_type(uint8_t irq_type_mask_offset) {
    gf_error_t ret = GF_SUCCESS;

    if (irq_type_mask_offset <= 0) {
        g_sensor_ignore_irq_mask = 0;
    } else {
        g_sensor_ignore_irq_mask = 1 << irq_type_mask_offset;
    }

    return ret;
}

static gf_error_t gf_milan_a_series_update_dac_offset_by_mean(int32_t mean_value) {
    gf_error_t ret = GF_SUCCESS;
    uint32_t i = 0;
    uint16_t tcode = 0;
    uint16_t tmp_delta = 0;

    GF_LOGD(LOG_TAG "[%s] enter.", __func__);

    do {
        if (mean_value >= 2000 && mean_value <= 2900) {
            break;
        }

        ret = gf_milan_a_series_secspi_read_word(GF_MILAN_A_SERIES_TCODE_ADDR, &tcode);
        if (ret != GF_SUCCESS) {
            GF_LOGE(LOG_TAG "[%s] read tcode failed %d", __func__, ret);
            break;
        }

        if (0 == tcode) {
            tmp_delta = 152;
        } else {
            tmp_delta = 0.76 * tcode;
        }

        if (mean_value < 2000) {
            for (i = 0; i < GF_MILAN_A_SERIES_DAC_OFFSET_LEN; i++) {
                DAC_OFFSET[i] = DAC_OFFSET[i] + (mean_value - 2300) / tmp_delta;
                GF_LOGD(LOG_TAG "[%s] DAC_OFFSET[%d] =  %d.", __func__, i, DAC_OFFSET[i]);
            }

        } else if (mean_value > 2900) {
            for (i = 0; i < GF_MILAN_A_SERIES_DAC_OFFSET_LEN; i++) {
                DAC_OFFSET[i] = DAC_OFFSET[i] + (mean_value - 2600) / tmp_delta;
                GF_LOGD(LOG_TAG "[%s] DAC_OFFSET[%d] =  %d.", __func__, i, DAC_OFFSET[i]);
            }
        }

        ret = gf_milan_a_series_secspi_write_bytes(GF_MILAN_A_SERIES_TEMPRATURE_DAC_BASE_ADDR,
                (uint8_t*) DAC_OFFSET, (GF_MILAN_A_SERIES_DAC_OFFSET_LEN * 2));
        if (GF_SUCCESS != ret) {
            GF_LOGE(LOG_TAG "[%s] :wirte dac offset fail!", __func__);
        }

    } while (0);

    GF_LOGD(LOG_TAG "[%s] exit:%d.", __func__, ret);
    return ret;
}

static gf_error_t gf_milan_a_series_get_orientation(uint16_t *orientation, uint16_t *facing) {
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    *orientation = g_milan_a_series_chip_handle.configuration.orientation;
    *facing = g_milan_a_series_chip_handle.configuration.facing;

    GF_LOGD(LOG_TAG "[%s] exit orientation:%d, facing: %d", __func__, *orientation, *facing);

    return GF_SUCCESS;
}

static gf_error_t gf_milan_a_series_load_otp_info(uint8_t *otp_buf, uint32_t *otp_buf_len) {
    gf_error_t err = GF_SUCCESS;
    uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);

    do {
        err = gf_so_load_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID, &buf, &buf_len);
        if (err != GF_SUCCESS) {
            GF_LOGI(LOG_TAG "[%s] load so(%s) fail, err:%d", __func__, GF_OTP_INFO_SECURE_ID, err);
            break;
        }

        memcpy(otp_buf, (void*) buf, buf_len);
        *otp_buf_len = buf_len;

/*
        for (i = 0; i < 64; ) {
            GF_LOGD(LOG_TAG "Otp data:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,",
                    buf[i],buf[i+1],buf[i+2],buf[i+3],
                    buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
            i += 8;
        }
*/
    } while (0);

    if (buf != NULL) {
        CPL_MEM_FREE(buf);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err: %d", __func__, err);
    return err;
}

static gf_error_t gf_milan_a_series_save_otp_info(uint8_t *buf, uint32_t buf_len) {
    gf_error_t err = GF_SUCCESS;
//    int32_t i = 0;
    GF_LOGD(LOG_TAG "[%s] enter", __func__);
/*
    for (i = 0; i < 64; ) {
        GF_LOGD(LOG_TAG "Otp data:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,",
                buf[i],buf[i+1],buf[i+2],buf[i+3],
                buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
        i += 8;
    }
*/
    err = gf_so_save_persistent_object((int8_t*) GF_OTP_INFO_SECURE_ID, buf, buf_len);
    if (err != GF_SUCCESS) {
        GF_LOGI(LOG_TAG "[%s] save so(%s) fail, err:%d", __func__, GF_OTP_INFO_SECURE_ID, err);
    }

    GF_LOGD(LOG_TAG "[%s] exit, err: %d", __func__, err);
    return err;
}

gf_error_t gf_spi_function_customize(gf_spi_function_t *spi_function) {
    gf_error_t err = GF_SUCCESS;

    do {
        if (NULL == spi_function) {
            err = GF_ERROR_BAD_PARAMS;
            GF_LOGE(LOG_TAG "[%s] invalid param", __func__);
            break;
        }

        spi_function->get_chip_id = gf_milan_a_series_get_chip_id;
        spi_function->get_fw_version = gf_milan_a_series_get_fw_version;
        spi_function->get_vendor_id = gf_milan_a_series_get_vendor_id;
        spi_function->get_production_date = gf_milan_a_series_get_production_date;
        spi_function->get_sensor_id = gf_milan_a_series_get_sensor_id;

        spi_function->chip_init = gf_milan_a_series_chip_init;
        spi_function->download_fw = gf_milan_a_series_download_fw;
        spi_function->download_cfg = gf_milan_a_series_download_cfg;

        spi_function->get_image = gf_milan_a_series_get_image;
        spi_function->esd_check = gf_milan_a_series_esd_check;

        spi_function->data_sample_suspend = gf_milan_a_series_data_sample_suspend;
        spi_function->data_sample_resume = gf_milan_a_series_data_sample_resume;

        spi_function->set_mode = gf_milan_a_series_set_mode;
        spi_function->get_mode = gf_milan_a_series_get_mode;

        spi_function->update_dac_for_sensor_pixel_open_test =
                gf_milan_a_series_update_dac_for_sensor_pixel_open_test;

        spi_function->get_irq_type = gf_milan_a_series_get_irq_type;
        spi_function->clear_irq = gf_milan_a_series_clear_irq;

        spi_function->get_gsc = gf_milan_a_series_get_gsc;
        spi_function->get_hbd = gf_milan_a_series_get_hbd;
        spi_function->get_hbd_base = gf_milan_a_series_get_hbd_base;

        spi_function->test_download_fw_cfg = gf_milan_a_series_test_download_fw_cfg;

        spi_function->get_sensor_status = gf_milan_a_series_get_sensor_status;
        spi_function->get_key_status = gf_milan_a_series_get_key_status;
        spi_function->get_irq_status_for_spi_clk = gf_milan_a_series_get_irq_status_for_spi_clk;

        spi_function->spi_write_byte = gf_milan_a_series_secspi_write_byte;
        spi_function->spi_read_byte = gf_milan_a_series_secspi_read_byte;
        spi_function->set_ignore_irq_type = gf_set_ignore_irq_type;

        spi_function->update_temprature_dac = gf_milan_a_series_update_temprature_dac;
        spi_function->update_dac_offset_by_mean = gf_milan_a_series_update_dac_offset_by_mean;
        spi_function->get_sensor_otp_info = gf_milan_a_series_get_sensor_otp_info;
        spi_function->select_chip_otp = gf_milan_a_series_select_chip_otp;
        spi_function->detect_sensor = gf_milan_a_series_detect_sensor;
        spi_function->judge_finger_stable = gf_milan_a_series_judge_finger_stable;
        spi_function->get_nav_base = gf_milan_a_series_get_nav_base;
        spi_function->get_nav_image = gf_milan_a_series_get_nav_image;
        spi_function->set_config_for_stable_acquire_image = gf_milan_a_series_set_config_for_stable_acquire_image;

        spi_function->update_cfg_for_reissue_key_down = gf_milan_a_series_update_cfg_for_reissue_key_down;
        spi_function->enable_reissue_key_down = gf_milan_a_series_enable_reissue_key_down;
        spi_function->get_orientation = gf_milan_a_series_get_orientation;

        spi_function->print_pixel_key_value = gf_milan_a_series_print_pixel_key_value;
    } while (0);

    return err;
}

