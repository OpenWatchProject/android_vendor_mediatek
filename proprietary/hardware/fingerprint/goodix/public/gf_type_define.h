/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_TYPE_DEFINE_H__
#define __GF_TYPE_DEFINE_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GF_OPERATION_ID 1
#define GF_USER_OPERATION_ID 2
#define GF_TEST_OPERATION_ID 3
#define GF_DUMP_OPERATION_ID 4

#define MAX_FINGERS_PER_USER 10

#define TEST_SPI_READ 0x0
#define TEST_SPI_WRITE 0x1

#ifndef UNUSED_VAR
#define UNUSED_VAR(X)   (void)(X)
#endif

#ifndef NULL
#define NULL  0
#endif

typedef enum {
    GF_SENSOR_FACING_BACK = 0,  //
    GF_SENSOR_FACING_FRONT,
} gf_sensor_facing_t;

typedef enum {
    GF_SAFE_CLASS_HIGHEST = 0,
    GF_SAFE_CLASS_HIGH,  //
    GF_SAFE_CLASS_MEDIUM,
    GF_SAFE_CLASS_LOW,
    GF_SAFE_CLASS_LOWEST,
    GF_SAFE_CLASS_MAX,  // The number of safe class. can't set this value.
} gf_safe_class_t;

typedef enum {
    GF_AUTHENTICATE_BY_USE_RECENTLY = 0,
    GF_AUTHENTICATE_BY_ENROLL_ORDER,
    GF_AUTHENTICATE_BY_REVERSE_ENROLL_ORDER,
} gf_authenticate_order_t;

typedef enum {
    GF_NAV_MODE_NONE = 0,
    GF_NAV_MODE_X    = 0x01,
    GF_NAV_MODE_Y    = 0x02,
    GF_NAV_MODE_Z    = 0x04,
    GF_NAV_MODE_XY   = GF_NAV_MODE_X | GF_NAV_MODE_Y,
    GF_NAV_MODE_XZ   = GF_NAV_MODE_X | GF_NAV_MODE_Z,
    GF_NAV_MODE_YZ   = GF_NAV_MODE_Y | GF_NAV_MODE_Z,
    GF_NAV_MODE_XYZ  = GF_NAV_MODE_XY | GF_NAV_MODE_Z,
    GF_NAV_MODE_MAX
} gf_nav_mode_t;

typedef enum {
    GF_NAV_EVENT_DETECT_METHOD_IRQ = 0,
    GF_NAV_EVENT_DETECT_METHOD_POLLING,
} gf_nav_get_data_method_t;

typedef enum {
    GF_NAV_DOUBLE_CLICK_MIN_INTERVAL_IN_MS = 80,
    GF_NAV_LONG_PRESS_MIN_INTERVAL_IN_MS = 600,

    GF_NAV_DOUBLE_CLICK_DEFAULT_INTERVAL_IN_MS = 300,
    GF_NAV_LONG_PRESS_DEFAULT_INTERVAL_IN_MS = 700,

    GF_NAV_DOUBLE_CLICK_MAX_INTERVAL_IN_MS = 1000,
    GF_NAV_LONG_PRESS_MAX_INTERVAL_IN_MS = 2000,
} gf_nav_interval_config_time_t;

typedef enum {
    GF_CHIP_316M = 0,
    GF_CHIP_318M,
    GF_CHIP_3118M,
    GF_CHIP_516M,
    GF_CHIP_518M,
    GF_CHIP_5118M,
    GF_CHIP_816M,
    GF_CHIP_3266, /* Milan-E */
    GF_CHIP_3208, /* Milan-F */
    GF_CHIP_3268, /* Milan-FN */
    GF_CHIP_3228, /* Milan-K */
    GF_CHIP_3288, /* Milan-L */
    GF_CHIP_3206, /* Milan-G */
    GF_CHIP_3226, /* Milan-J */
    GF_CHIP_3258, /* Milan-H */
    GF_CHIP_5206, /* Milan-A */
    GF_CHIP_5216, /* Milan-B */
    GF_CHIP_5208, /* Milan-C */
    GF_CHIP_5218, /* Milan-D */
    GF_CHIP_8206, /* Milan-E-HV */
    GF_CHIP_5266, /* Milan-E HV */
    GF_CHIP_5288, /* Milan-FN-HV */
    GF_CHIP_6226, /* Milan-J-HV */
    GF_CHIP_GX556,/* Milan-B-GX556 */
    GF_CHIP_UNKNOWN,
} gf_chip_type_t;

typedef enum {
    GF_OSWEGO_M = 0,
    GF_MILAN_F_SERIES,
    GF_MILAN_A_SERIES,
    GF_MILAN_HV,
    GF_UNKNOWN_SERIES,
} gf_chip_series_t;

typedef enum {
    CMD_TEST_ENUMERATE = 0,
    CMD_TEST_DRIVER,
    CMD_TEST_PIXEL_OPEN,
    CMD_TEST_BAD_POINT,
    CMD_TEST_SENSOR_FINE,
    CMD_TEST_PERFORMANCE,
    CMD_TEST_SPI_PERFORMANCE,
    CMD_TEST_SPI_TRANSFER,
    CMD_TEST_SPI,
    CMD_TEST_GET_VERSION,
    CMD_TEST_FRR_FAR_GET_CHIP_TYPE,
    CMD_TEST_FRR_FAR_INIT,
    CMD_TEST_FRR_FAR_RECORD_BASE_FRAME,
    CMD_TEST_FRR_FAR_RECORD_ENROLL,
    CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE,
    CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH,
    CMD_TEST_FRR_FAR_PLAY_BASE_FRAME,
    CMD_TEST_FRR_FAR_PLAY_ENROLL,
    CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE,
    CMD_TEST_FRR_FAR_ENROLL_FINISH,
    CMD_TEST_CANCEL_FRR_FAR,
    CMD_TEST_RESET_PIN,
    CMD_TEST_INTERRUPT_PIN,
    CMD_TEST_CANCEL,
    CMD_TEST_GET_CONFIG,
    CMD_TEST_SET_CONFIG,
    CMD_TEST_DOWNLOAD_FW,
    CMD_TEST_DOWNLOAD_CFG,
    CMD_TEST_DOWNLOAD_FWCFG,
    CMD_TEST_RESET_FWCFG,
    CMD_TEST_SENSOR_VALIDITY,
    CMD_TEST_RESET_CHIP,  // test tools, just reset chip, don't do anything.
    CMD_TEST_UNTRUSTED_ENROLL,
    CMD_TEST_UNTRUSTED_AUTHENTICATE,
    CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER,
    CMD_TEST_CHECK_FINGER_EVENT,
    CMD_TEST_BIO_CALIBRATION,
    CMD_TEST_HBD_CALIBRATION,
    CMD_TEST_SPI_RW,
    CMD_TEST_REAL_TIME_DATA,
    CMD_TEST_READ_CFG,
    CMD_TEST_READ_FW,
    CMD_TEST_FRR_DATABASE_ACCESS,  // 42
    CMD_TEST_PRIOR_CANCEL,  // 43
    CMD_TEST_NOISE,
    CMD_TEST_RAWDATA_SATURATED,
    CMD_TEST_BMP_DATA ,
    CMD_TEST_MEMMGR_SET_CONFIG,  // 47
    CMD_TEST_MEMMGR_GET_CONFIG,  // 48
    CMD_TEST_MEMMGR_GET_INFO,  // 49
    CMD_TEST_MEMMGR_DUMP_POOL,
    CMD_TEST_UNKNOWN,
} gf_cmd_test_id_t;

typedef enum {
    CMD_DUMP_DATA = 1000,
    CMD_CANCEL_DUMP_DATA,
    CMD_DUMP_TEMPLATES,
    CMD_DUMP_PATH,
    CMD_DUMP_NAV_BASE,
    CMD_DUMP_FINGER_BASE,
} gf_cmd_dump_id_t;

typedef enum {
    DUMP_PATH_SDCARD = 0,  //
    DUMP_PATH_DATA,
} gf_dump_path_t;

enum {
    TEST_TOKEN_ERROR_CODE = 100,
    TEST_TOKEN_CHIP_TYPE,
    TEST_TOKEN_CHIP_SERIES,
    TEST_TOKEN_ALGO_VERSION = 200,
    TEST_TOKEN_PREPROCESS_VERSION,
    TEST_TOKEN_FW_VERSION,
    TEST_TOKEN_TEE_VERSION,
    TEST_TOKEN_TA_VERSION,
    TEST_TOKEN_CHIP_ID,
    TEST_TOKEN_VENDOR_ID,
    TEST_TOKEN_SENSOR_ID,
    TEST_TOKEN_PRODUCTION_DATE,
    TEST_TOKEN_SENSOR_OTP_TYPE,
    TEST_TOKEN_CODE_FW_VERSION,
    TEST_TOKEN_AVG_DIFF_VAL = 300,
    TEST_TOKEN_NOISE,
    TEST_TOKEN_BAD_PIXEL_NUM,
    TEST_TOKEN_FDT_BAD_AREA_NUM,
    TEST_TOKEN_LOCAL_BAD_PIXEL_NUM,
    TEST_TOKEN_FRAME_NUM,
    TEST_TOKEN_MAX_FRAME_NUM,
    TEST_TOKEN_DATA_DEVIATION_DIFF,
    TEST_TOKEN_ALL_TILT_ANGLE,
    TEST_TOKEN_BLOCK_TILT_ANGLE_MAX,
    TEST_TOKEN_LOCAL_WORST,
    TEST_TOKEN_SINGULAR,
    TEST_TOKEN_IN_CIRCLE,
    TEST_TOKEN_BIG_BUBBLE,
    TEST_TOKEN_LINE,
    TEST_TOKEN_LOCAL_SMALL_BAD_PIXEL_NUM,
    TEST_TOKEN_LOCAL_BIG_BAD_PIXEL_NUM,
    TEST_TOKEN_FLATNESS_BAD_PIXEL_NUM,
    TEST_TOKEN_IS_BAD_LINE,
    TEST_TOKEN_GET_DR_TIMESTAMP_TIME = 400,
    TEST_TOKEN_GET_MODE_TIME,
    TEST_TOKEN_GET_CHIP_ID_TIME,
    TEST_TOKEN_GET_VENDOR_ID_TIME,
    TEST_TOKEN_GET_SENSOR_ID_TIME,
    TEST_TOKEN_GET_FW_VERSION_TIME,
    TEST_TOKEN_GET_IMAGE_TIME,
    TEST_TOKEN_RAW_DATA_LEN,
    TEST_TOKEN_CFG_DATA,
    TEST_TOKEN_CFG_DATA_LEN,
    TEST_TOKEN_FW_DATA,
    TEST_TOKEN_FW_DATA_LEN,
    TEST_TOKEN_IMAGE_QUALITY = 500,
    TEST_TOKEN_VALID_AREA,
    TEST_TOKEN_KEY_POINT_NUM,
    TEST_TOKEN_INCREATE_RATE,
    TEST_TOKEN_OVERLAY,
    TEST_TOKEN_GET_RAW_DATA_TIME,
    TEST_TOKEN_PREPROCESS_TIME,
    TEST_TOKEN_ALGO_START_TIME,
    TEST_TOKEN_GET_FEATURE_TIME,
    TEST_TOKEN_ENROLL_TIME,
    TEST_TOKEN_AUTHENTICATE_TIME,
    TEST_TOKEN_AUTHENTICATE_ID,
    TEST_TOKEN_AUTHENTICATE_UPDATE_FLAG,
    TEST_TOKEN_AUTHENTICATE_FINGER_COUNT,
    TEST_TOKEN_AUTHENTICATE_FINGER_ITME,
    TEST_TOKEN_TOTAL_TIME,
    TEST_TOKEN_GET_GSC_DATA_TIME,
    TEST_TOKEN_BIO_ASSAY_TIME,
    TEST_TOKEN_RESET_FLAG = 600,
    TEST_TOKEN_RAW_DATA = 700,
    TEST_TOKEN_BMP_DATA = 701,
    TEST_TOKEN_ALGO_INDEX = 702,
    TEST_TOKEN_SAFE_CLASS = 703,
    TEST_TOKEN_TEMPLATE_COUNT = 704,
    TEST_TOKEN_GSC_DATA = 705,
    TEST_TOKEN_HBD_BASE = 706,
    TEST_TOKEN_HBD_AVG = 707,
    TEST_TOKEN_HBD_RAW_DATA = 708,
    TEST_TOKEN_ELECTRICITY_VALUE = 709,
    TEST_TOKEN_FINGER_EVENT = 710,
    TEST_TOKEN_GSC_FLAG = 711,
    TEST_TOKEN_BASE_DATA = 712,
    TEST_TOKEN_KR_DATA = 713,
    TEST_TOKEN_B_DATA = 714,
    TEST_TOKEN_FPC_KEY_DATA = 715,
    TEST_TOKEN_MAX_FINGERS = 800,
    TEST_TOKEN_MAX_FINGERS_PER_USER,
    TEST_TOKEN_SUPPORT_KEY_MODE,
    TEST_TOKEN_SUPPORT_FF_MODE,
    TEST_TOKEN_SUPPORT_POWER_KEY_FEATURE,
    TEST_TOKEN_FORBIDDEN_UNTRUSTED_ENROLL,
    TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS,
    TEST_TOKEN_SUPPORT_BIO_ASSAY,
    TEST_TOKEN_SUPPORT_PERFORMANCE_DUMP,
    TEST_TOKEN_SUPPORT_NAV_MODE,
    TEST_TOKEN_NAV_DOUBLE_CLICK_TIME,
    TEST_TOKEN_NAV_LONG_PRESS_TIME,
    TEST_TOKEN_ENROLLING_MIN_TEMPLATES,
    TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD,
    TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD,
    TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE,
    TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO,
    TEST_TOKEN_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT,
    TEST_TOKEN_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT,
    TEST_TOKEN_SCREEN_ON_VALID_TOUCH_FRAME_THRESHOLD,
    TEST_TOKEN_SCREEN_OFF_VALID_TOUCH_FRAME_THRESHOLD,
    TEST_TOKEN_IMAGE_QUALITY_THRESHOLD_FOR_MISTAKE_TOUCH,
    TEST_TOKEN_AUTHENTICATE_ORDER,
    TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE,
    TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE,
    TEST_TOKEN_SUPPORT_SENSOR_BROKEN_CHECK,
    TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_SENSOR,
    TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_STUDY,
    TEST_TOKEN_BAD_POINT_TEST_MAX_FRAME_NUMBER,
    TEST_TOKEN_REPORT_KEY_EVENT_ONLY_ENROLL_AUTHENTICATE,
    TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_IMAGE_MODE,
    TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_FF_MODE,
    TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_KEY_MODE,
    TEST_TOKEN_REQUIRE_DOWN_AND_UP_IN_PAIRS_FOR_NAV_MODE,
    TEST_TOKEN_SUPPORT_SET_SPI_SPEED_IN_TEE,
    TEST_TOKEN_SUPPORT_FRR_ANALYSIS,
    TEST_TOKEN_SENSOR_VALIDITY = 900,
    TEST_TOKEN_SPI_TRANSFER_RESULT,
    TEST_TOKEN_SPI_TRANSFER_REMAININGS,
    TEST_TOKEN_AVERAGE_PIXEL_DIFF,
    TEST_TOKEN_DUMP_IS_ENCRYPTED = 1000,
    TEST_TOKEN_DUMP_ENCRYPTED_DATA,
    TEST_TOKEN_DUMP_OPERATION,
    TEST_TOKEN_DUMP_TIMESTAMP,
    TEST_TOKEN_DUMP_YEAR,
    TEST_TOKEN_DUMP_MONTH,
    TEST_TOKEN_DUMP_DAY,
    TEST_TOKEN_DUMP_HOUR,
    TEST_TOKEN_DUMP_MINUTE,
    TEST_TOKEN_DUMP_SECOND,
    TEST_TOKEN_DUMP_MICROSECOND,
    TEST_TOKEN_DUMP_VERSION_CODE,  // the token of version code must be 1011 and can't be modified
    TEST_TOKEN_DUMP_WIDTH,
    TEST_TOKEN_DUMP_HEIGHT,
    TEST_TOKEN_DUMP_PREPROCESS_VERSION,
    TEST_TOKEN_DUMP_CHIP_ID,
    TEST_TOKEN_DUMP_VENDOR_ID,
    TEST_TOKEN_DUMP_SENSOR_ID,
    TEST_TOKEN_DUMP_FRAME_NUM,
    TEST_TOKEN_DUMP_KR,
    TEST_TOKEN_DUMP_B,
    TEST_TOKEN_DUMP_RAW_DATA,
    TEST_TOKEN_DUMP_BROKEN_CHECK_RAW_DATA,
    TEST_TOKEN_DUMP_BROKEN_CHECK_FRAME_NUM,
    TEST_TOKEN_DUMP_CALI_RES,
    TEST_TOKEN_DUMP_DATA_BMP,
    TEST_TOKEN_DUMP_SITO_BMP,
    TEST_TOKEN_DUMP_SELECT_INDEX,
    TEST_TOKEN_DUMP_IMAGE_QUALITY,
    TEST_TOKEN_DUMP_VALID_AREA,
    TEST_TOKEN_DUMP_INCREASE_RATE_BETWEEN_STITCH_INFO,
    TEST_TOKEN_DUMP_OVERLAP_RATE_BETWEEN_LAST_TEMPLATE,
    TEST_TOKEN_DUMP_ENROLLING_FINGER_ID,
    TEST_TOKEN_DUMP_DUMPLICATED_FINGER_ID,
    TEST_TOKEN_DUMP_MATCH_SCORE,
    TEST_TOKEN_DUMP_MATCH_FINGER_ID,
    TEST_TOKEN_DUMP_STUDY_FLAG,
    TEST_TOKEN_DUMP_NAV_TIMES,
    TEST_TOKEN_DUMP_NAV_FRAME_INDEX,
    TEST_TOKEN_DUMP_NAV_FRAME_NUM,
    TEST_TOKEN_DUMP_NAV_FRAME_COUNT,
    TEST_TOKEN_DUMP_FINGER_ID,
    TEST_TOKEN_DUMP_GROUP_ID,
    TEST_TOKEN_DUMP_TEMPLATE,
    TEST_TOKEN_DUMP_REMAINING_TEMPLATES,
    TEST_TOKEN_DUMP_STROPERATION,
    TEST_TOKEN_SPI_RW_CMD = 1100,
    TEST_TOKEN_SPI_RW_START_ADDR,
    TEST_TOKEN_SPI_RW_LENGTH,
    TEST_TOKEN_SPI_RW_CONTENT,
    TEST_TOKEN_PACKAGE_VERSION = 1200,
    TEST_TOKEN_PROTOCOL_VERSION,
    TEST_TOKEN_CHIP_SUPPORT_BIO,
    TEST_TOKEN_IS_BIO_ENABLE,
    TEST_TOKEN_AUTHENTICATED_WITH_BIO_SUCCESS_COUNT,
    TEST_TOKEN_AUTHENTICATED_WITH_BIO_FAILED_COUNT,
    TEST_TOKEN_AUTHENTICATED_SUCCESS_COUNT,
    TEST_TOKEN_AUTHENTICATED_FAILED_COUNT,
    TEST_TOKEN_BUF_FULL,
    TEST_TOKEN_UPDATE_POS,
    TEST_TOKEN_METADATA,
    TEST_TOKEN_UNDER_SATURATED_PIXEL_COUNT = 1300,
    TEST_TOKEN_OVER_SATURATED_PIXEL_COUNT,
    TEST_TOKEN_SATURATED_PIXEL_THRESHOLD,
    TEST_TOKEN_MEMMGR_ENABLE = 1400,
    TEST_TOKEN_MEMMGR_DEBUG_ENABLE,
    TEST_TOKEN_MEMMGR_BEST_MATCH_ENABLE,
    TEST_TOKEN_MEMMGR_FREE_ERASE_ENABLE,
    TEST_TOKEN_MEMMGR_DUMP_TIME_ENABLE,
    TEST_TOKEN_MEMMGR_NEXT_REBOOT_ENABLE,
    TEST_TOKEN_MEMMGR_POOL_SIZE,
    TEST_TOKEN_MEMMGR_USED_INFO,
    TEST_TOKEN_MEMMGR_POOL_START_ADDR,
    TEST_TOKEN_MEMMGR_POOL_END_ADDR,
    TEST_TOKEN_MEMMGR_CUR_USED_BLOCK_COUNT,
    TEST_TOKEN_MEMMGR_MAX_USED_BLOCK_COUNT,
    TEST_TOKEN_MEMMGR_CUR_USED_MEM_SIZE,
    TEST_TOKEN_MEMMGR_MAX_USED_MEM_SIZE,
    TEST_TOKEN_MEMMGR_TOTAL_NODE_COUNT,
    TEST_TOKEN_MEMMGR_NODE_INFO,
    TEST_TOKEN_MEMMGR_DUMP_TIME,
    TEST_TOKEN_MEMMGR_DUMP_OFFSET,
    TEST_TOKEN_MEMMGR_DUMP_FINISHED,
    TEST_TOKEN_MEMMGR_DUMP_POOL,
    TEST_PARAM_TOKEN_FW_DATA = 5000,
    TEST_PARAM_TOKEN_CFG_DATA,
    TEST_PARAM_TOKEN_DUMP_PATH = 5100,
    TEST_TOKEN_TEMPLATE_UPDATE_SAVE_THRESHOLD = 5200,
};

#define GF_HW_AUTH_TOKEN_VERSION 0

typedef struct __attribute__((packed)) {
    uint8_t version;  // Current version is 0
    uint64_t challenge;
    uint64_t user_id;  // secure user ID, not Android user ID
    uint64_t authenticator_id;  // secure authenticator ID
    uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
    uint64_t timestamp;  // in network order
    uint8_t hmac[32];
} gf_hw_auth_token_t;

typedef enum {
    GF_HW_AUTH_NONE = 0,
    GF_HW_AUTH_PASSWORD = (int)(1 << 0),
    GF_HW_AUTH_FINGERPRINT = (int)(1 << 1),
    // Additional entries should be powers of 2.
    GF_HW_AUTH_ANY = (int)UINT32_MAX,
} gf_hw_authenticator_type_t;

typedef struct {
    int32_t coverType;
    int32_t inertiaX; /* 左右方向的迟钝程度，默认为1，值越大越迟钝 */
    int32_t inertiaY; /* 上下方向的迟钝程度，默认为1，值越大越迟钝 */
    /* 判断两帧为静止的SAD移动X方向阈值，此阈值应小于等于inertiaX，阈值越大，越容易判断为静止 */
    int32_t staticX;
    /* 判断两帧为静止的SAD移动Y方向阈值，此阈值应小于等于inertiaY，阈值越大，越容易判断为静止*/
    int32_t staticY;
    int32_t sadXoffThr;
    int32_t sadYoffThr; /*总移动判为移动的Y方向阈值*/
    int32_t maxNvgFrameNum; /* 导航输出结果前的最大帧数，默认为20 */
} gf_nav_config_t;

typedef struct {
    gf_chip_type_t chip_type;
    gf_chip_series_t chip_series;

    uint32_t max_fingers;
    uint32_t max_fingers_per_user;

    /**
     * 0: disable key mode feature; 1: enable.
     */
    uint8_t support_key_mode;
    /**
     * 0: entry sleep mode when screen off.
     * 1: entry Finger Flash mode when screen off
     */
    uint8_t support_ff_mode;
    uint8_t support_power_key_feature;

    gf_nav_mode_t support_nav_mode;
    gf_nav_config_t nav_config;
    gf_nav_get_data_method_t support_nav_get_data_method;

    /**
     * 0: disable navigation double click feature, otherwise enable
     */
    uint32_t nav_double_click_interval_in_ms;

    /**
     * 0: disable navigation long press feature, otherwise enable
     */
    uint32_t nav_long_press_interval_in_ms;

     /**
      * min_value <= enrolling_min_templates <= max_value
      *           CHIP_TYPE                 min_value   max_value
      *  GF316M/GF516M/GF816M                  10          40
      *  GF318M/GF3118M/GF518M/GF5118M         8           30
      *  GF3206/GF3208/GF3288/GF3268           8           30
      *  GF3266                                8           25
      *  GF3288                                8           20
      *  GF5206/GF5216                         10          40
      *  GF5208/GF5218                         8           40
      */
    uint32_t enrolling_min_templates;

    uint32_t valid_image_quality_threshold;
    uint32_t valid_image_area_threshold;
    uint32_t duplicate_finger_overlay_score;
    uint32_t increase_rate_between_stitch_info;

    uint8_t forbidden_untrusted_enroll;
    uint8_t forbidden_enroll_duplicate_fingers;

    /**
     * Reference Android M com.android.server.fingerprint.FingerprintService.java
     * private static final int MAX_FAILED_ATTEMPTS = 5;
     * authenticate failed too many attempts. Try again later.
     * value 0, don't check authenticate failed attempts.
     */
    uint32_t max_authenticate_failed_attempts;

    /**
     * Define authenticate failure retry strategy.
     * These configuration only works when safe class is set to be equal or greater than #GF_SAFE_CLASS_MEDIUM.
     * 0: don't retry.
     */
    uint32_t screen_on_authenticate_fail_retry_count;
    uint32_t screen_off_authenticate_fail_retry_count;

    /**
     * screen_on_valid_touch_frame_threshold <= screen_on_authenticate_fail_retry_count;
     * screen_off_valid_touch_frame_threshold <= screen_off_authenticate_fail_retry_count;
     */
    uint32_t screen_on_valid_touch_frame_threshold;
    uint32_t screen_off_valid_touch_frame_threshold;

    uint32_t image_quality_threshold_for_mistake_touch;

    gf_authenticate_order_t authenticate_order;

    /**
     * configure fingerprint template study update save threshold
     */
    uint32_t template_update_save_threshold;

    /**
     * Configuration to reissue key down event when entry FF or IMAGE mode.
     * 0:disable, 1: enable. If disable this feature, when entry FF or IMAGE mode,
     * will first check if the finger has leaved from the sensor, if not, won't report key down
     * event, until finger up and finger down.
     */
    uint32_t reissue_key_down_when_entry_ff_mode;
    uint32_t reissue_key_down_when_entry_image_mode;
    uint32_t reissue_key_down_when_entry_nav_mode;
    uint32_t reissue_key_down_when_entry_key_mode;


    uint8_t support_sensor_broken_check;
    // broken pixel number to disable sensor
    uint16_t broken_pixel_threshold_for_disable_sensor;
    // broken pixel number to disable study function
    uint16_t broken_pixel_threshold_for_disable_study;

    uint32_t bad_point_test_max_frame_number;

    uint32_t average_pixel_diff_threshold;

    uint32_t require_down_and_up_in_pairs_for_image_mode;
    uint32_t require_down_and_up_in_pairs_for_ff_mode;
    uint32_t require_down_and_up_in_pairs_for_nav_mode;
    uint32_t require_down_and_up_in_pairs_for_key_mode;

    uint8_t report_key_event_only_enroll_authenticate;

    uint32_t support_set_spi_speed_in_tee;

    uint8_t support_performance_dump;
    uint8_t support_frr_analysis;

    /**
     * Milan A series configuration
     */
    uint8_t support_bio_assay;
    uint8_t support_hbd;
    uint8_t support_merged_gsc;

    /* Milan HV */
    uint8_t support_hv_dac_adjust;
    uint8_t enable_nav_tx_strategy;
    uint8_t enable_fdt_tx_strategy;

    uint8_t support_hw_encrypt;
    // uint16_t pixel_open_test_delta;
} gf_config_t;

const char* gf_strnavmode(gf_nav_mode_t navmode);
const char* gf_strchiptype(gf_chip_type_t chip);
const char* gf_strtestcmd(gf_cmd_test_id_t test_cmd_id);

#ifdef __cplusplus
}
#endif

#endif  // __GF_TYPE_DEFINE_H__
