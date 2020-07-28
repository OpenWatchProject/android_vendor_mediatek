/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __MILAN_HV_COMMON_H__
#define __MILAN_HV_COMMON_H__

#include "gf_common.h"
#include "gf_error.h"

#define RG_EXTRA_DATA_EN_MASK        1 << 0
#define RG_DOUBLE_RATE_MASK          1 << 0
#define RG_SCAN_MODE_MASK            (0x3)

#define MILAN_HV_MAX_SPI_RATE        10  // Mbps

// For Milan-E/FN HV, only one group DAC, Milan-J HV, 4 groups DAC
#define MILAN_HV_DAC_GROUPS_MAX_NUM  4

#define MILAN_HV_VENDOR_MAX_COUNT    4

#define SPI_WRITE      0xF0  // spi write cmd
#define SPI_READ       0xF1  // spi read cmd

#define SPI_WRITE_HV   0xD0  // spi write hv cmd
#define SPI_READ_HV    0xD1  // spi write hv cmd

#define MILAN_HV_CONFIG_ADDRESS_START  0x0020
#define MILAN_HV_CONFIG_LENGTH         268
#define MILAN_HV_FDT_BASE_LEN          12  // need to check: 5288 (12), 6226 (10), 8206 (10)

#define SET_BIT(value, bit)   ((value) |= 1 << (bit))
#define CLEAR_BIT(value, bit) ((value) &= ~(1 << (bit)))
#define CHECK_BIT(value, bit) ((value) & (1 << (bit)))

/**
 * Declare enumeration type for interrupt status[0126] for Milan-E HV
 */
#define HV_TMR_IRQ_MNT_MASK                            (1 << 0)
#define HV_FDT_DOWN_IRQ_MASK                           (1 << 1)  // irq for detecting finger press
#define HV_ONE_FRAME_DONE_MASK                         (1 << 2)
#define HV_DATA_INT_MASK                               (1 << 3)
#define HV_ESD_INT_MASK                                (1 << 4)
#define HV_FIFO_EMPTY_OR_FULL_IN_ADC_TEST_MODE_MASK    (1 << 5)
#define HV_FIFO_DATA_HALF_FULL_IN_ADC_TEST_MODE_MASK   (1 << 6)
#define HV_FDT_IRQ1_MASK                               (1 << 7)
#define HV_FDT_MANUAL_IRQ_MASK                         (1 << 8)
#define HV_FDT_IRQ_UP_MASK                             (1 << 9)  // irq for detecting finger up
#define MILAN_E_HV_RESET_IRQ_MASK                      (1 << 10)  // irq for Milan-E HV chip reset
#define MILAN_J_HV_TEMPERATURE_IRQ_MASK                (1 << 10)  // irq for Milan-J HV temperature
#define MILAN_J_HV_RESET_IRQ_MASK                      (1 << 11)  // irq for Milan-J HV chip reset

/**
 * Milan HV OTP data.
 */
#define MILAN_HV_OTP_DATA_BANK_NUM   (8)
#define MILAN_HV_OTP_DATA_BANK_LEN   (4)
#define MILAN_HV_OTP_DATA_LEN        (32)

#define BROKEN_CHECK_TCODE           (32)
#define BROKEN_CHECK_DAC_DELTA       (40)
#define MILAN_HV_PIXEL_TEST_DAC_DELTA (26)

#define GF5266_PRODUCT_ID      0X6A
#define GF5266ZN2_PRODUCT_ID   0X64
#define GF5266ZN1_PRODUCT_ID   0X66
#define GF5266BN1_PRODUCT_ID   0X68

#define HV_TCODE_MAX           255

/**
 * Declare enumeration type for Milan HV SPI CMDs.
 */
typedef enum {
    CMD_IDLE    = 0xC0,  // jump to idle state cmd
    CMD_FDT     = 0xC1,  // jump to finger detect state cmd
    CMD_IMG     = 0xC2,  // jump to image state cmd
    CMD_FF      = 0xC4,  // jump to fingerflash state cmd
    CMD_SLEEP   = 0xC8,  // jump to sleep state cmd
    CMD_NAV     = 0xCA,  // jump to navigation state cmd
    CMD_TMR_TRG = 0xA1,  // timer manual trigger cmd
    CMD_ADC_TST = 0xA2,  // adc test mode cmd
} milan_hv_cmd_t;

/**
 * Declare enumeration type for Milan HV FDT enable/disable TX strategy
 */
typedef enum {
    HV_FDT_NONE_TX_STRATEGY,
    HV_FDT_ENABLE_TX_STRATEGY,
    HV_FDT_DISABLE_TX_STRATEGY
} milan_hv_fdt_tx_strategy_t;

/**
 * Declare enumeration type for Milan HV NAV enable/disable TX strategy
 */
typedef enum {
    HV_NAV_NONE_TX_STRATEGY,
    HV_NAV_ENABLE_TX_STRATEGY,
    HV_NAV_DISABLE_TX_STRATEGY
} milan_hv_nav_tx_strategy_t;

/**
 * Declare enumeration type for Milan HV NAV enable/disable TX strategy
 */
typedef enum {
    HV_TX_DISABLED = 0,
    HV_TX_ENABLED = 1,
    HV_TX_NONE = 2
} milan_hv_tx_status_t;

/**
 * Declare enumeration type for Milan HV OTP Version
 */
typedef enum {
    HV_NONE_OTP_VERSION = 0x00,
    HV_OLD_OTP_VERSION  = 0xC0,
    HV_NEW_OTP_VERSION  = 0xE0
} milan_hv_otp_version_e;

/**
 * Declare enumeration type for chip state[000E] for Milan-E/G/L
 */
typedef enum {
    HV_MODE_SLEEP_STATE_MASK = 1 << 0,  // Sleep Mode
    HV_MODE_FF_STATE_MASK    = 1 << 1,  // FingerFlash Mode
    HV_MODE_IMAGE_STATE_MASK = 1 << 2,  // Image Mode
    HV_MODE_FDT_STATE_MASK   = 1 << 3,  // FDT Mode
    HV_MODE_IDLE_STATE_MASK  = 1 << 4,  // Idle Mode
    HV_MODE_NAV_STATE_MASK   = 1 << 5,  // Navigation Mode
} milan_hv_chip_state_mask_t;

/**
 * Declare enumeration type for Milan-HV series's ECO version.
 */
typedef enum {
    SENSOR_ECO_TA0 = 0xA0,  // ECO TA0 version
    SENSOR_ECO_TA1 = 0xA1,  // ECO TA1 version
    SENSOR_ECO_TA2 = 0xA2,  // ECO TA2 version
    SENSOR_ECO_TA3 = 0xA3,  // ECO TA3 version
    SENSOR_ECO_TA4 = 0xA4,  // ECO TA4 version
} milan_hv_eco_version_t;

typedef enum {
    SCAN_MODE0 = 0,  // Mode0(Reg(0x66[2:1]=0))
    SCAN_MODE1 = 1,  // Mode1(Reg(0x66[2:1]=1))
    SCAN_MODE2 = 2,  // Mode2(Reg(0x66[2:1]=2))
    SCAN_MODE3 = 3,  // TODO(goodix): just for Milan-E TA2, but Milan-E HV TA2 won't production.
} milan_hv_scan_mode_t;

/**
 * Declare enumeration type for Milan-E HV work mode.
 */

typedef enum {
    // chip mode
    CHIP_MODE_IDLE  = 1 << 1,
    CHIP_MODE_SLEEP = 1 << 2,
    CHIP_MODE_FF    = 1 << 3,
    CHIP_MODE_FDT   = 1 << 4,
    CHIP_MODE_IMAGE = 1 << 5,
    CHIP_MODE_NAV   = 1 << 6,

    CHIP_MODE_MASK = CHIP_MODE_IDLE | CHIP_MODE_SLEEP | CHIP_MODE_FF | CHIP_MODE_FDT
            | CHIP_MODE_IMAGE | CHIP_MODE_NAV,

    // MODE_IDLE state
    STATE_IDLE = CHIP_MODE_IDLE,

    // MODE_IDLE state
    STATE_SLEEP = CHIP_MODE_SLEEP,

    // MODE_FDT state
    STATE_FDT_DOWN        = CHIP_MODE_FDT | (1 << 7),
    STATE_FDT_UP          = CHIP_MODE_FDT | (1 << 8),
    STATE_FDT_NAV_DOWN    = CHIP_MODE_FDT | (1 << 9),
    STATE_FDT_NAV_UP      = CHIP_MODE_FDT | (1 << 10),
    STATE_FDT_MANUAL_DOWN = CHIP_MODE_FDT | (1 << 11),

    // MODE_NV state
    STATE_NAV      = CHIP_MODE_NAV,
    STATE_NAV_BASE = CHIP_MODE_NAV | (1 << 12),

    // MODE_IMAGE state
    STATE_IMAGE                       = CHIP_MODE_IMAGE,
    STATE_IMAGE_FINGER_BASE           = CHIP_MODE_IMAGE | (1 << 13),
    STATE_IMAGE_CONTINUE              = CHIP_MODE_IMAGE | (1 << 14),
    STATE_IMAGE_BROKEN_CHECK_DEFAULT  = CHIP_MODE_IMAGE | (1 << 15),
    STATE_IMAGE_BROKEN_CHECK_NEGATIVE = CHIP_MODE_IMAGE | (1 << 16),
    STATE_IMAGE_BROKEN_CHECK_POSITIVE = CHIP_MODE_IMAGE | (1 << 17),

    STATE_IMAGE_TEST_BAD_POINT             = CHIP_MODE_IMAGE | (1 << 18),
    STATE_IMAGE_TEST_PIXEL_OPEN_DEFAULT    = CHIP_MODE_IMAGE | (1 << 19),
    STATE_IMAGE_TEST_PIXEL_OPEN_POSITIVE   = CHIP_MODE_IMAGE | (1 << 20),
    STATE_IMAGE_TEST_BAD_POINT_FINGER_BASE = CHIP_MODE_IMAGE | (1 << 21),
} milan_hv_chip_state_t;

/**
 * Declare enumeration type for chip id [0000], 4 bytes, 0x0002 save the higher 16 bits, and
 * 0x0000 save the lower 16 bits. When check the chip series, just need check the higher 24 bits,
 * because the lowest byte is used to distinguish between different batches.
 */
typedef enum {
    MILAN_E_HV_CHIP_ID  = 0x002207,
    MILAN_FN_HV_CHIP_ID = 0x00220C,
    MILAN_J_HV_CHIP_ID  = 0x00220E,
} milan_hv_chip_id_t;

typedef enum {
    VENDOR_ID_A = 0x0000,
    VENDOR_ID_B = 0x0010,
    VENDOR_ID_C = 0x0001,
    VENDOR_ID_D = 0x0011,
} milan_hv_vendor_id_t;

typedef enum {
    FDT_UP_DETECT_MODE_OLD_VERSION = 0,
    FDT_UP_DETECT_MODE_NEW_VERSION
} milan_hv_fdt_up_detect_mode_t;

typedef struct {
    milan_hv_vendor_id_t vendor_id;
    uint16_t orientation;
    uint16_t facing;
} milan_hv_vendor_params_t;

/**
 * Define the structure for different mode' configuration array.
 */
typedef struct {
    uint8_t ff[MILAN_HV_CONFIG_LENGTH];
    uint8_t fdt_manual_down[MILAN_HV_CONFIG_LENGTH];
    uint8_t fdt_down[MILAN_HV_CONFIG_LENGTH];
    uint8_t fdt_up[MILAN_HV_CONFIG_LENGTH];
    uint8_t image[MILAN_HV_CONFIG_LENGTH];
    uint8_t nav_fdt_down[MILAN_HV_CONFIG_LENGTH];
    uint8_t nav_fdt_up[MILAN_HV_CONFIG_LENGTH];
    uint8_t nav[MILAN_HV_CONFIG_LENGTH];
    uint8_t nav_base[MILAN_HV_CONFIG_LENGTH];
} milan_hv_config_t;

typedef struct {
    uint8_t set_flag;
    uint8_t update_direction;  // update direction(0: increase DAC; > 0 :decrease DAC)
    uint16_t positive_dac[MILAN_HV_DAC_GROUPS_MAX_NUM];  // default DAC + DAC_DELTA
    uint16_t negative_dac[MILAN_HV_DAC_GROUPS_MAX_NUM];  // default DAC - DAC_DELTA
    uint16_t dac_delta;
} milan_hv_sensor_broken_check_t;

typedef struct {
    uint16_t positive_dac[MILAN_HV_DAC_GROUPS_MAX_NUM];  // default DAC + DAC_DELTA
    uint16_t dac_delta;  // dac_delta
} milan_hv_pixel_test_t;

/**
 * Define OTP Info structure
 * These Infomation will be calculated by otp data which read from OTP
 */
typedef struct {
    uint8_t valid;  // CRC result
    uint8_t product_id;
    uint16_t dac_h;
    uint16_t dac_l;
    uint16_t otp_tcode;
    uint16_t fdt_tcode;
    uint16_t fdt_down_delta;
    uint16_t fdt_up_delta;
} milan_hv_otp_info_t;

typedef struct {
    uint8_t (*get_otp_data_crc)(uint8_t* otp_buf);
    gf_error_t (*set_general_reg)(void);
    gf_error_t (*update_dac_to_regs)(uint16_t dac_groups[]);
} milan_hv_function_t;

typedef struct {
    gf_mode_t mode;
    milan_hv_chip_state_t cur_chip_state;  // record the Milan-HV chip's mode

    uint32_t image_double_rate_flag;
    uint32_t image_add_extra_data_rows;

    uint32_t nav_double_rate_flag;
    uint32_t nav_add_extra_data_rows;

    uint32_t chip_id;  // record the current chip's chip id that contains the batches information.
    // register[0006], that contain two bytes which can distinguish four different vendors.
    uint16_t vendor_id;

    uint8_t otp_data[MILAN_HV_OTP_DATA_LEN];
    milan_hv_otp_info_t otp_info;
    milan_hv_otp_version_e otp_version;

    milan_hv_config_t *config;
    milan_hv_vendor_params_t vendor_param;

    milan_hv_sensor_broken_check_t sensor_broken;

    uint32_t data_base_status;

    uint32_t img_fifo_spi_rate;  // unit:Mbps
    uint32_t nav_fifo_spi_rate;  // unit:Mbps

    uint32_t is_report_finger_up_for_ff_or_image_mode;
    uint32_t is_received_fdt_up_for_ff_or_image_mode;

    milan_hv_pixel_test_t pixel_test_chip_info;

    milan_hv_scan_mode_t scan_mode;

    uint8_t fdt_base[MILAN_HV_FDT_BASE_LEN];
    uint16_t default_dac[MILAN_HV_DAC_GROUPS_MAX_NUM];  // default DAC

    uint16_t cur_dac_h;

    milan_hv_fdt_up_detect_mode_t fdt_up_detect_new_mode_flag;
    uint32_t pixel_test_dac_update_value;

    milan_hv_function_t function;

    uint16_t pixel_open_test_delta;

    uint8_t init_complete_flag;

    uint16_t* touch_diff_table;

    uint16_t fdt_threshold;

    milan_hv_fdt_tx_strategy_t fdt_tx_strategy;
    milan_hv_nav_tx_strategy_t nav_tx_strategy;
    milan_hv_tx_status_t current_tx_status;
} milan_hv_vfw_handle_t;

/**
 * Define one macro to compare chip id. #src_chip_id is the real chip id that read 4 bytes
 * from register 0x0002. #dst_chip_id is #milan_chip_id_t type.
 */
#define MILAN_HV_CHIP_ID_IS_EQUAL(src_chip_id, dst_chip_id)    ((src_chip_id >> 8) == dst_chip_id)

#define FDT_DETECT_AREA_IS_1X8(reg_data)            ((0x0100 & (reg_data)) == 0)

/**
 * Define macro to update word value into config structure. #config should be #milan_config_unit_t
 */
#define MILAN_HV_UPDATE_CONFIG_ARRAY_BY_WORD(config, register_addr, value)  \
{                                                                           \
    uint16_t offset = register_addr - MILAN_HV_CONFIG_ADDRESS_START;        \
    if (offset < MILAN_HV_CONFIG_LENGTH - 1) {                              \
        config[offset] = (uint8_t)((value) >> 8);                           \
        config[offset + 1] = (uint8_t)((value) & 0xFF);                     \
    }                                                                       \
}

#define MILAN_HV_UPDATE_CONFIG_ARRAY_BY_BYTE(config, register_addr, value)  \
{                                                                           \
    uint16_t offset = register_addr - MILAN_HV_CONFIG_ADDRESS_START;        \
    if (offset < MILAN_HV_CONFIG_LENGTH) {                                  \
        config[offset] = (uint8_t)((value) & 0xFF);                         \
    }                                                                       \
}

#define MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_WORD(config, register_addr, value)   \
{                                                                                       \
    uint16_t offset = register_addr - MILAN_HV_CONFIG_ADDRESS_START;                    \
    if (offset < MILAN_HV_CONFIG_LENGTH - 1) {                                          \
        value = config[offset] << 8;                                                    \
        value |= config[offset + 1];                                                    \
    }                                                                                   \
}

#define MILAN_HV_GET_REGISTER_FROM_CONFIG_ARRAY_BY_BYTE(config, register_addr, value)   \
{                                                                                       \
    uint16_t offset = register_addr - MILAN_HV_CONFIG_ADDRESS_START;                    \
    if (offset < MILAN_HV_CONFIG_LENGTH) {                                              \
        value = config[offset];                                                         \
    }                                                                                   \
}

#endif  // __MILAN_HV_COMMON_H__
