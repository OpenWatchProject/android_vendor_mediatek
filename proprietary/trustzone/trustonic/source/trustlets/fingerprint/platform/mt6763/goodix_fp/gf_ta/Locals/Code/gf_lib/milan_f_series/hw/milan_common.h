/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __MILAN_COMMON_H__
#define __MILAN_COMMON_H__

#include "gf_common.h"
#include "gf_error.h"

#define RG_EXTRA_DATA_EN_MASK      1 << 0
#define RG_DOUBLE_RATE_MASK        1 << 0
#define RG_SCAN_MODE_MASK          (0x3)

#define OTP_LEN_IN_BYTES  32
#define CHIP_CONFIG_LEN_IN_BYTES 272    // start register address(one word) + content(0x87 in words)

#define MILAN_MAX_SPI_RATE 10     // Mbps

// For Milan-E/F/FN/G/L, only one group DAC, Milan-K/J/H/S, 4 groups DAC
#define MILAN_DAC_GROUPS_MAX_NUM    4

/**
 * Declare enumeration type for Milan-E/G/L's SPI CMDs.
 */
typedef enum {
    SPI_WRITE = 0xF0,  // spi write cmd
    SPI_READ = 0xF1,  // spi read cmd
    CMD_IDLE = 0xC0,  // jump to idle state cmd
    CMD_FDT = 0xC1,  // jump to finger detect state cmd
    CMD_IMG = 0xC2,  // jump to image state cmd
    CMD_FF = 0xC4,  // jump to fingerflash state cmd
    CMD_SLEEP = 0xC8,  // jump to sleep state cmd
    CMD_NAV = 0xCA,  // jump to navigation state cmd
    CMD_TMR_TRG = 0xA1,  // timer manual trigger cmd
    CMD_ADC_TST = 0xA2,  // adc test mode cmd
} milan_cmd_t;

/**
 * Declare enumeration type for Milan-F series's ECO version.
 */
typedef enum {
    SENSOR_ECO_TA0 = 0xA0,  // ECO TA0 version
    SENSOR_ECO_TA1 = 0xA1,  // ECO TA1 version
    SENSOR_ECO_TA2 = 0xA2,  // ECO TA2 version
    SENSOR_ECO_TA3 = 0xA3,  // ECO TA3 version
    SENSOR_ECO_TA4 = 0xA4,  // ECO TA4 version
} milan_eco_version_t;

typedef enum {
    SCAN_MODE0 = 0,  // Mode0(Reg(0x66[2:1]=0))
    SCAN_MODE1 = 1,  // Mode1(Reg(0x66[2:1]=1))
    SCAN_MODE2 = 2,  // Mode2(Reg(0x66[2:1]=2))
    SCAN_MODE3 = 3,  // TODO(goodix): just for Milan-E TA2, but Milan-E TA2 won't production.
} milan_scan_mode_t;

/**
 * Declare enumeration type for Milan-E/G/L's work mode.
 */
typedef enum {
    MILAN_STATE_SLEEP = 0,  // sleep state:low power state
    MILAN_STATE_FF,  // finger flash(finger detect) state:low power state
    MILAN_STATE_IMAGE,  // image capture(pixel scan) state
    MILAN_STATE_FDT_DOWN,  // finger press detect state:low power state
    MILAN_STATE_FDT_UP,  // finger up detect state:low power state
    MILAN_STATE_IDLE,  // idle state:low power state
    MILAN_STATE_NAV,  // navigation state
    MILAN_STATE_NAV_BASE,
} milan_state_t;

#define MILAN_TMR_MNT_IRQ                                                               (1 << 0)
#define MILAN_FDT_IRQ                                                                       (1 << 1)
#define MILAN_ONE_FRAME_DONE_IRQ                                              (1 << 2)
#define MILAN_DATA_INT_IRQ                                                           (1 << 3)
#define MILAN_ESD_INT_IRQ                                                              (1 << 4)
#define MILAN_FIFO_EMPTY_OR_FULL_IN_ADC_TEST_MODE_IRQ    (1 << 5)
#define MILAN_FIFO_DATA_HALF_FULL_IN_ADC_TEST_MODE_IRQ  (1 << 6)
#define MILAN_REVERSE_IRQ                                                               (1 << 7)
#define MILAN_FDT_MANUAL_IRQ                                                        (1 << 8)
#define MILAN_FDT_UP_IRQ                                                                (1 << 9)
#define MILAN_RESET_IRQ                                                                   (1 << 10)
#define MILAN_TEMPERATURET_IRQ                                                   (1 << 11)


/**
 * Declare enumeration type for interrupt status[0126] for Milan-E/G/L
 */
#define TMR_IRQ_MNT_MASK                            (1 << 0)
#define FDT_IRQ_MASK                                (1 << 1)  // irq for detecting finger press
#define ONE_FRAME_DONE_MASK                         (1 << 2)
#define DATA_INT_MASK                               (1 << 3)
#define ESD_INT_MASK                                (1 << 4)
#define FIFO_EMPTY_OR_FULL_IN_ADC_TEST_MODE_MASK    (1 << 5)
#define FIFO_DATA_HALF_FULL_IN_ADC_TEST_MODE_MASK   (1 << 6)
#define FDT_IRQ1_MASK                               (1 << 7)
#define FDT_MANUAL_IRQ_MASK                         (1 << 8)
#define FDT_IRQ_UP_MASK                             (1 << 9)   // irq for detecting finger up
#define MILAN_EGL_RESET_IRQ_MASK                    (1 << 10)  // irq for Milan-E/G/L chip reset
#define MILAN_F_RESET_IRQ_MASK                      (1 << 8)   // irq for Milan-F chip reset
#define MILAN_KJH_TEMPERATURE_IRQ_MASK              (1 << 10)  // irq for Milan-K/J/H temperature
#define MILAN_KJH_RESET_IRQ_MASK                    (1 << 11)  // irq for Milan-K/J/H chip reset

#define FDT_DETECT_AREA_IS_1X8(reg_data)            ((0x0100 & (reg_data)) == 0)
/**
 * Declare enumeration type for chip state[000E] for Milan-E/G/L
 */
typedef enum {
    MODE_SLEEP_STATE_MASK = 1 << 0,  // Sleep Mode
    MODE_FF_STATE_MASK = 1 << 1,  // FingerFlash Mode
    MODE_IMAGE_STATE_MASK = 1 << 2,  // Image Mode
    MODE_FDT_STATE_MASK = 1 << 3,  // FDT Mode
    MODE_IDLE_STATE_MASK = 1 << 4,  // Idle Mode
    MODE_NAV_STATE_MASK = 1 << 5,  // Navigation Mode
} milan_chip_state_t;

/**
 * Declare enumeration type for chip id [0000], 4 bytes, 0x0002 save the higher 16 bits, and
 * 0x0000 save the lower 16 bits. When check the chip series, just need check the higher 24 bits,
 * because the lowest byte is used to distinguish between different batches.
 */
typedef enum {
    MILAN_E_CHIP_ID = 0x002207,
    MILAN_F_CHIP_ID = 0x002202,
    MILAN_FN_CHIP_ID = 0x00220C,
    MILAN_G_CHIP_ID = 0x002208,
    MILAN_L_CHIP_ID = 0x002205,
    MILAN_K_CHIP_ID = 0x00220A,
    MILAN_J_CHIP_ID = 0x00220E,
    MILAN_H_CHIP_ID = 0x00220D
} milan_chip_id_t;

/**
 * Define one macro to compare chip id. #src_chip_id is the real chip id that read 4 bytes
 * from register 0x0002. #dst_chip_id is #milan_chip_id_t type.
 */
#define MILAN_CHIP_ID_IS_EQUAL(src_chip_id, dst_chip_id)    ((src_chip_id >> 8) == dst_chip_id)

#define CHIP_ID_IS_MILAN_F_SERIES(chip_id)    \
    ((chip_id >> 8) == MILAN_E_CHIP_ID        \
    || (chip_id >> 8) == MILAN_F_CHIP_ID      \
    || (chip_id >> 8) == MILAN_FN_CHIP_ID     \
    || (chip_id >> 8) == MILAN_G_CHIP_ID      \
    || (chip_id >> 8) == MILAN_L_CHIP_ID      \
    || (chip_id >> 8) == MILAN_K_CHIP_ID      \
    || (chip_id >> 8) == MILAN_J_CHIP_ID      \
    || (chip_id >> 8) == MILAN_H_CHIP_ID)

typedef enum {
    VENDOR_ID_A = 0x0000,  //
    VENDOR_ID_B = 0x0010,  //
    VENDOR_ID_C = 0x0001,  //
    VENDOR_ID_D = 0x0011,  //
} milan_vendor_id_t;

#define MILAN_VENDOR_MAX_COUNT  4

#define MILAN_CONFIG_CONTENT_MAX_LEN    524

typedef struct {
    uint16_t start_address;  // the start register address of this mode's configuration
    uint16_t data_len;  // the length in bytes of this configuration byte array.
    uint8_t data[MILAN_CONFIG_CONTENT_MAX_LEN];  // the pointer to the detail configuration content.
} milan_config_unit_t;

typedef struct {
    milan_vendor_id_t vendor_id;
    uint16_t orientation;
    uint16_t facing;
} milan_vendor_params_t;

/**
 * Define the structure for different mode' configuration array.
 */
typedef struct {
    milan_config_unit_t ff;
    milan_config_unit_t fdt_manual_down;
    milan_config_unit_t fdt_down;
    milan_config_unit_t fdt_up;
    milan_config_unit_t image;
    milan_config_unit_t nav_fdt_down;
    milan_config_unit_t nav_fdt_up;
    milan_config_unit_t nav;
    milan_config_unit_t nav_base;
} milan_config_t;

typedef struct {
    uint8_t set_flag;
    uint8_t update_direction;  // update direction(increase DAC/decrease DAC)
    uint16_t positive_dac[MILAN_DAC_GROUPS_MAX_NUM];  // default DAC + DAC_DELTA
    uint16_t negative_dac[MILAN_DAC_GROUPS_MAX_NUM];  // default DAC - DAC_DELTA
    uint16_t dac_delta;
} milan_sensor_broken_check_t;

typedef struct {
    uint16_t positive_dac[MILAN_DAC_GROUPS_MAX_NUM];  // default DAC + DAC_DELTA
    uint16_t dac_delta;  // dac_delta
} milan_pixel_test_t;

typedef enum {
    FDT_UP_DETECT_MODE_OLD_VERSION = 0,
    FDT_UP_DETECT_MODE_NEW_VERSION
} milan_fdt_up_detect_mode_t;

typedef struct {
    gf_mode_t mode;
    milan_state_t cur_chip_state;  // record the Milan-E/G/L chip's mode
    milan_state_t next_chip_state;  // record the Milan-E/G/L next chip mode
    milan_chip_state_t chip_state;  // record the Milan-E/G/L chip's current state

    uint32_t auto_switch_mode_flag;  // 0: don't auto switch mode to next_chip_mode, 1: auto switch
    uint32_t suspend_data_sampling_flag;  // 1: suspend rescanning data, 0: continue to rescan data

    gf_error_t (*milan_raw_data_split)(uint16_t *des, uint8_t *raw_data, uint32_t raw_data_len,
            uint32_t col_num, uint8_t is_double_rate, uint32_t col_gaps, uint32_t scan_mode);
    gf_error_t (*milan_update_dac_register_from_opt)(uint8_t *opt_info);
    gf_error_t (*milan_update_config_array_from_opt)(uint8_t *opt_info);

    gf_error_t (*milan_calculate_otp_crc)(uint8_t* otp_buf);
    gf_error_t (*milan_check_ft_test_info)(uint8_t* otp_buf);


    uint32_t image_double_rate_flag;
    uint32_t image_add_extra_data_rows;

    uint32_t nav_double_rate_flag;
    uint32_t nav_add_extra_data_rows;

    uint32_t chip_id;  // record the current chip's chip id that contains the batches information.
    // register[0006], that contain two bytes which can distinguish four different vendors.
    uint16_t vendor_id;
    uint8_t otp[OTP_LEN_IN_BYTES];

    milan_config_t* config;
    milan_vendor_params_t vendor_param;

    milan_sensor_broken_check_t sensor_broken;

    uint32_t otp_crc_check_result;  // 0: fail, 1: success

    uint32_t data_base_status;

    uint32_t img_fifo_spi_rate;  // unit:Mbps
    uint32_t nav_fifo_spi_rate;  // unit:Mbps

    uint32_t is_report_finger_up_for_ff_or_image_mode;
    uint32_t is_received_fdt_up_for_ff_or_image_mode;

    milan_pixel_test_t pixel_test_chip_info;

    milan_scan_mode_t scan_mode;


    uint16_t default_dac[MILAN_DAC_GROUPS_MAX_NUM];  // default DAC
    milan_fdt_up_detect_mode_t fdt_up_detect_new_mode_flag;
    uint32_t pixel_test_dac_update_value;

    uint32_t encrypt_flag;
    uint32_t encrypt_key;
} milan_chip_handle_t;

/**
 * Define macro to update word value into config structure. #config should be #milan_config_unit_t
 */
#define MILAN_UPDATE_CONFIG_ARRAY_BY_WORD(config, register_addr, value)     \
{                                                                           \
    uint16_t offset = register_addr - config.start_address;                 \
    if (offset < config.data_len - 1) {                                     \
        config.data[offset] = (uint8_t)((value) >> 8);                      \
        config.data[offset + 1] = (uint8_t)((value) & 0xFF);                \
    }                                                                       \
}

#define MILAN_UPDATE_CONFIG_ARRAY_BY_BYTE(config, register_addr, value)     \
{                                                                           \
    uint16_t offset = register_addr - config.start_address;                 \
    if (offset < config.data_len) {                                         \
        config.data[offset] = (uint8_t)((value) & 0xFF);                    \
    }                                                                       \
}

#define MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(config, register_addr, value)  \
{                                                                                   \
    uint16_t offset = register_addr - config.start_address;                         \
    if (offset < config.data_len - 1) {                                             \
        value = config.data[offset] << 8;                                           \
        value |= config.data[offset + 1];                                           \
    }                                                                               \
}

#define MILAN_GET_REGISTER_FROM_CONFIG_ARRAY_BY_BYTE(config, register_addr, value)  \
{                                                                                   \
    uint16_t offset = register_addr - config.start_address;                         \
    if (offset < config.data_len) {                                                 \
        value = config.data[offset];                                                \
    }                                                                               \
}

#endif  // __MILAN_COMMON_H__
