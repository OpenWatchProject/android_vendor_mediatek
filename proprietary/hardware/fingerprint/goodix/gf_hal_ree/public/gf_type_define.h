#ifndef __GF_TYPE_DEFINE_H__
#define __GF_TYPE_DEFINE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GF_OPERATION_ID 1
#define GF_USER_OPERATION_ID 2

#define MAX_FINGERS_PER_USER 10

typedef enum {
    GF_SENSOR_FACING_BACK = 0, //
    GF_SENSOR_FACING_FRONT,
} gf_sensor_facing_t;

typedef enum {
    GF_SAFE_CLASS_HIGHEST = 0,
    GF_SAFE_CLASS_HIGH, //
    GF_SAFE_CLASS_MEDIUM,
    GF_SAFE_CLASS_LOW,
    GF_SAFE_CLASS_LOWEST,
    GF_SAFE_CLASS_MAX, ///< The number of safe class. can't set this value.
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
    GF_NAV_DOUBLE_CLICK_MIN_INTERVAL_IN_MS = 80,
    GF_NAV_LONG_PRESS_MIN_INTERVAL_IN_MS = 600,

    GF_NAV_DOUBLE_CLICK_DEFAULT_INTERVAL_IN_MS = 300,
    GF_NAV_LONG_PRESS_DEFAULT_INTERVAL_IN_MS = 700,

    GF_NAV_DOUBLE_CLICK_MAX_INTERVAL_IN_MS = 500,
    GF_NAV_LONG_PRESS_MAX_INTERVAL_IN_MS = 1000,
} gf_nav_interval_config_time_t;

typedef enum {
    GF_SENSOR_316M = 0, //
    GF_SENSOR_318M,
    GF_SENSOR_3118M,
    GF_SENSOR_516M,
    GF_SENSOR_518M,
    GF_SENSOR_5118M,
    GF_SENSOR_816M,
    GF_SENSOR_316,
    GF_SENSOR_318,
    GF_SENSOR_516,
    GF_SENSOR_518,
    GF_SENSOR_3266, /* Milan-E */
    GF_SENSOR_3208, /* Milan-F */
    GF_SENSOR_3208FN, /* Milan-FN */
    GF_SENSOR_3206, /* Milan-G */
    GF_SENSOR_3288, /* Milan-L */
    GF_SENSOR_5206, /* Milan-A */
    GF_SENSOR_5216, /* Milan-B */
    GF_SENSOR_5208, /* Milan-C */
    GF_SENSOR_5218, /* Milan-D */
    GF_SENSOR_GX556,/* Milan-B-GX556 */
} gf_sensor_type_t;

typedef enum {
    CMD_TEST_ENUMERATE = 0,
    CMD_TEST_DRIVER,
    CMD_TEST_SENSOR,
    CMD_TEST_BAD_POINT,
    CMD_TEST_PERFORMANCE,
    CMD_TEST_SPI_PERFORMANCE,
    CMD_TEST_SPI_TRANSFER,
    CMD_TEST_SPI,
    CMD_TEST_GET_VERSION,
    CMD_TEST_DUMP_DATA,
    CMD_TEST_CANCEL_DUMP_DATA,
    CMD_TEST_DUMP_TEMPLATES,
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
    CMD_TEST_FRR_FAR_SAVE_FINGER,
    CMD_TEST_FRR_FAR_DEL_FINGER,
    CMD_TEST_CANCEL_FRR_FAR,
    CMD_TEST_RESET_PIN,
    CMD_TEST_CANCEL,
    CMD_TEST_GET_CONFIG,
    CMD_TEST_SET_CONFIG,
    CMD_TEST_DOWNLOAD_FW,
    CMD_TEST_DOWNLOAD_CFG,
    CMD_TEST_SENSOR_CHECK,
    CMD_TEST_RESET_CHIP, // test tools, just reset chip, don't do anything.
    CMD_TEST_UNTRUSTED_ENROLL,
    CMD_TEST_UNTRUSTED_AUTHENTICATE,
    CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER,
    CMD_TEST_CHECK_FINGER_EVENT,
    CMD_TEST_BIO_CALIBRATION,
    CMD_TEST_HBD_CALIBRATION,
    CMD_TEST_FPC_KEY,
    CMD_TEST_FRR_FAR_RESET_CALIBRATION,
    CMD_TEST_ENABLE_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE,
    CMD_TEST_ENABLE_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE,
    CMD_TEST_MAX = 0xFF,
} gf_cmd_test_id_t;

enum {
    TEST_TOKEN_ERROR_CODE = 100,
    TEST_TOKEN_SENSOR_TYPE,
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
    TEST_TOKEN_AVG_DIFF_VAL = 300,
    TEST_TOKEN_NOISE,
    TEST_TOKEN_BAD_PIXEL_NUM,
    TEST_TOKEN_LOCAL_BAD_PIXEL_NUM,
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
    TEST_TOKEN_HBD_FINGER_EVENT = 710,
    TEST_TOKEN_TEST_FRR_FAR_TYPE = 711,
    TEST_TOKEN_FPC_KEY_EVENT = 712,
    TEST_TOKEN_FPC_KEY_STATUS = 713,
    TEST_TOKEN_FPC_KEY_EN_FLAG = 714,
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
    TEST_TOKEN_ENROLLING_MIN_TEMPLATES,
    TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD,
    TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD,
    TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE,
    TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO,
    TEST_TOKEN_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT,
    TEST_TOKEN_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT,
    TEST_TOKEN_AUTHENTICATE_ORDER,
    TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE,
    TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE,
    TEST_TOKEN_SUPPORT_SENSOR_BROKEN_CHECK,
    TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_SENSOR,
    TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_STUDY,
    TEST_TOKEN_BAD_POINT_TEST_MAX_FRAME_NUMBER,
    TEST_TOKEN_REPORT_KEY_EVENT_ONLY_ENROLL_AUTHENTICATE,
    TEST_TOKEN_SENSOR_CHECK = 900,
    TEST_TOKEN_SPI_TRANSFER_RESULT,
    TEST_TOKEN_SPI_TRANSFER_REMAININGS,

    TEST_TOKEN_GROUP_ID = 950,
    TEST_TOKEN_FINGER_ID,
    TEST_TOKEN_SAMPLES_REMAINING,
    TEST_TOKEN_ACQUIRED_INFO,
    TEST_TOKEN_FRR_FAR_GROUP_ID = 1000,
    TEST_TOKEN_FRR_FAR_FINGER_ID,
    TEST_TOKEN_FRR_FAR_SAVE_FINGER_PATH
};

#define GF_HW_AUTH_TOKEN_VERSION 0

typedef struct __attribute__((packed)) {
    uint8_t version; // Current version is 0
    uint64_t challenge;
    uint64_t user_id; // secure user ID, not Android user ID
    uint64_t authenticator_id; // secure authenticator ID
    uint32_t authenticator_type; // hw_authenticator_type_t, in network order
    uint64_t timestamp; // in network order
    uint8_t hmac[32];
} gf_hw_auth_token_t;

typedef enum {
    GF_HW_AUTH_NONE = 0,
    GF_HW_AUTH_PASSWORD = (int)(1 << 0),
    GF_HW_AUTH_FINGERPRINT = (int)(1 << 1),
    // Additional entries should be powers of 2.
    GF_HW_AUTH_ANY = (int)UINT32_MAX,
} gf_hw_authenticator_type_t;

typedef enum {
    GF_COATING,
    GF_GLASS,
    GF_CERAMIC
} gf_cover_type_t;

typedef struct {
    int32_t coverType;
    int32_t inertiaX; /* 左右方向的迟钝程度，默认为1，值越大越迟钝 */
    int32_t inertiaY; /* 上下方向的迟钝程度，默认为1，值越大越迟钝 */
    int32_t staticX; /* 判断两帧为静止的SAD移动X方向阈值，此阈值应小于等于inertiaX，阈值越大，越容易判断为静止 */
    int32_t staticY; /* 判断两帧为静止的SAD移动Y方向阈值，此阈值应小于等于inertiaY，阈值越大，越容易判断为静止*/
    int32_t sadXoffThr;
    int32_t sadYoffThr; /*总移动判为移动的Y方向阈值*/
    int32_t maxNvgFrameNum; /* 导航输出结果前的最大帧数，默认为20 */
} gf_nav_config_t;

typedef struct {
    gf_sensor_type_t sensor_type;
    uint32_t max_fingers;
    uint32_t max_fingers_per_user;

    uint8_t support_key_mode;
    uint8_t support_ff_mode;
    uint8_t support_power_key_feature;
    uint8_t forbidden_untrusted_enroll;
    uint8_t forbidden_enroll_duplicate_fingers;
    uint8_t support_bio_assay;
    uint8_t support_performance_dump;

    gf_nav_mode_t support_nav_mode;
    gf_nav_config_t nav_config;

    uint32_t enrolling_min_templates;

    uint32_t valid_image_quality_threshold;
    uint32_t valid_image_area_threshold;
    uint32_t duplicate_finger_overlay_score;
    uint32_t increase_rate_between_stitch_info;

    /**
     * Reference Android M com.android.server.fingerprint.FingerprintService.java
     * private static final int MAX_FAILED_ATTEMPTS = 5;
     * authenticate failed too many attempts. Try again later.
     * value 0, don't check authenticate failed attempts.
     */
    uint32_t max_authenticate_failed_attempts;

    /**
     * Define authenticate failure retry strategy for low safe class.
     * These configuration only work when safe class is set to be #GF_SAFE_CLASS_LOW.
     * value 0, don't retry.
     */
    uint32_t screen_on_authenticate_fail_retry_count;
    uint32_t screen_off_authenticate_fail_retry_count;

    gf_authenticate_order_t authenticate_order;

    /**
     * Configuration to reissue key down event when entry FF or IMAGE mode.
     * 0:disable, 1: enable. If disable this feature, when entry FF or IMAGE mode,
     * will first check if the finger has leaved from the sensor, if not, won't report key down
     * event, until finger up and finger down.
     */
    uint32_t reissue_key_down_when_entry_ff_mode;
    uint32_t reissue_key_down_when_entry_image_mode;

    uint8_t support_cover_broken_check;
    uint16_t cover_broken_pixel_threshold_for_disable_sensor;// cover broken pixel number to disable sensor
    uint16_t cover_broken_pixel_threshold_for_disable_study;// cover broken pixel number to disable study function

    uint8_t support_sensor_broken_check;
    uint16_t broken_pixel_threshold_for_disable_sensor; // broken pixel number to disable sensor
    uint16_t broken_pixel_threshold_for_disable_study; // broken pixel number to disable study function

    uint32_t bad_point_test_max_frame_number;

    uint8_t report_key_event_only_enroll_authenticate;

    uint32_t require_down_and_up_in_pairs_for_image_mode;
    uint32_t require_down_and_up_in_pairs_for_ff_mode;

    /**
     * 0: disable navigation double click feature, otherwise enable
     */
    uint32_t nav_double_click_interval_in_ms;

    /**
     * 0: disable navigation long press feature, otherwise enable
     */
    uint32_t nav_long_press_interval_in_ms;

    /**
     * Milan A series fw & cfg configuration for nav scheme & acquire image func & sensor judge finger stable
     */
    uint8_t support_fw_navigation; //add for switch firmware navigation or hal navigation
    uint8_t nav_frame_num;
    uint8_t support_hbd;
    uint8_t support_merged_gsc;

    gf_cover_type_t cover_type;

    uint32_t support_double_click_event;
    uint32_t double_click_time_ms;

    uint32_t support_long_pressed_event;
    uint32_t long_pressed_time_ms;

    uint8_t support_print_key_value;

    /**
    * dynamic enroll
    **/
    uint8_t support_dynamic_enroll;

    uint8_t support_fpc_key;
    uint8_t support_ring_key;
    uint8_t support_sleep_key;

} gf_config_t;

#ifdef __cplusplus
}
#endif

#endif // __GF_TYPE_DEFINE_H__
