/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_MILAN_A_SERIES_COMMON_H__
#define __GF_MILAN_A_SERIES_COMMON_H__

#include "gf_common.h"
#include "gf_error.h"

/**
 * Declare enumeration type for chip id [0000], 4 bytes, 0x0002 save the higher 16 bits, and
 * 0x0000 save the lower 16 bits. When check the chip series, just need check the higher 24 bits,
 * because the lowest byte is used to distinguish between different batches.
 */
typedef enum {
    MILAN_A_CHIP_ID = 0x12A1,
    MILAN_A_CHIP_ID_C2 = 0x12A4,
    MILAN_C_CHIP_ID_V0 = 0x12B0,
    MILAN_C_CHIP_ID_V1 = 0x12B1,
    MILAN_C_CHIP_ID_V2 = 0x12B2,
} milan_a_series_chip_id_t;

typedef enum {
    VENDOR_ID_A = 0x0000, //
    VENDOR_ID_B = 0x0010, //
    VENDOR_ID_C = 0x0001, //
    VENDOR_ID_D = 0x0011, //
} milan_a_series_vendor_id_t;

#define CHIP_ID_IS_MILAN_A_SERIES(chip_id)        \
    (chip_id == MILAN_A_CHIP_ID            \
    || chip_id == MILAN_A_CHIP_ID_C2       \
    || chip_id == MILAN_C_CHIP_ID_V0       \
    || chip_id == MILAN_C_CHIP_ID_V1       \
    || chip_id == MILAN_C_CHIP_ID_V2)

#define MILAN_A_SERIES_FW_MAX_LEN    5120
#define MILAN_A_SERIES_CONFIG_MAX_LEN 256
#define MILAN_A_SERIES_OTP_BUF_LEN 64

// for navigation
#define NAVI_FRAME_SIZE     (1024*5-1)
#define GF52X6_NAVI_START_ROWID    (1)
#define GF52X8_NAVI_START_ROWID    (17)

typedef enum {
    GF_MILAN_A_SERIES_MODE_IMAGE = 0x00,
    GF_MILAN_A_SERIES_MODE_KEY = 0x01,
    GF_MILAN_A_SERIES_MODE_SLEEP = 0x02,
    GF_MILAN_A_SERIES_MODE_FF = 0x03,
    GF_MILAN_A_SERIES_MODE_NAV = 0x04,
    GF_MILAN_A_SERIES_MODE_HBD = 0x05,
    GF_MILAN_A_SERIES_MODE_IDLE = 0x06,
    GF_MILAN_A_SERIES_MODE_HBD_BASE = 0x07,
    GF_MILAN_A_SERIES_MODE_FINGER_BASE = 0x56,
    GF_MILAN_A_SERIES_MODE_NAV_BASE = 0x14,
    GF_MILAN_A_SERIES_MODE_UNKNOWN = 0xFF,
} gf_milan_a_series_mode_t;

typedef enum {

    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_1 = 0x01,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_2 = 0x02,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_3 = 0x03,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_4 = 0x04,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_5 = 0x05,
    //for LV
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_6 = 0x06,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_7 = 0x07,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_8 = 0x08,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_9 = 0x09,
    GF_MILAN_A_SERIES_SENSOR_OTP_TYPE_UNKNOWN = 0xFF,
} gf_milan_a_series_sensor_otp_type_t;

typedef struct {
    uint8_t res;
    uint8_t IrqStatus; // indicate it is key event, ADC event or ...
    uint8_t FrameIndex;
    uint8_t PkIndex;
    uint8_t FrameLenH;
    uint8_t FrameLenL;
    uint8_t KeyStatus;
    uint8_t ImageStatus;
    uint8_t HbdStatus;
//uint8_t PkLenH;
//uint8_t PkLenL;
    uint8_t CheckSum; // check sum: all the byte adding together will be zero
}__attribute__ ((packed)) gf_milan_a_series_frame_header_t;

typedef struct {
    uint16_t len;
    uint8_t data[20];
    uint16_t res;
}__attribute__ ((packed)) gf_hbd_buf_t;

typedef struct {
    uint8_t mode;
    uint8_t reserve;
    uint8_t status[10];
}__attribute__ ((packed)) gf_sensor_status_t;

typedef struct {
    uint16_t tcode;
    uint8_t dac1[4];
    uint8_t dac2[4];
    uint8_t dac3[4];
    uint8_t dac12_bit9;
    uint8_t dac3_bit9;
    uint16_t checksum;
}__attribute__ ((packed)) gf_milan_a_series_pixel_test_t;

typedef struct {
    uint32_t version;
    uint8_t fw[MILAN_A_SERIES_FW_MAX_LEN];
    uint8_t cfg[MILAN_A_SERIES_CONFIG_MAX_LEN];
    uint8_t cfg_light_adjust[MILAN_A_SERIES_CONFIG_MAX_LEN];
} milan_a_series_fw_cfg_t;

/**
 * Define the structure for different mode' configuration array.
 */
typedef struct {
    milan_a_series_vendor_id_t vendor_id;
    uint16_t orientation;
    uint16_t facing;

    milan_a_series_fw_cfg_t config;

} milan_a_series_configuration_t;

typedef struct {
    uint8_t support_fast_acquire_image;
    uint8_t support_judge_finger_stable;
    uint8_t support_stable_acquire_image;
    uint8_t support_reissue_key_down;
}milan_a_series_feature_t;

typedef struct {

    uint32_t chip_id; // record the current chip's chip id that contains the batches information.
    uint16_t vendor_id; // register[0006], that contain two bytes which can distinguish four different vendors.

    uint8_t otp_buf[MILAN_A_SERIES_OTP_BUF_LEN];

    milan_a_series_configuration_t configuration;
    milan_a_series_feature_t feature;

} milan_a_series_chip_handle_t;

#endif //__GF_MILAN_A_SERIES_COMMON_H__
