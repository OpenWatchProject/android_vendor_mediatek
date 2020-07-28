/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __GF_COMMON_H__
#define __GF_COMMON_H__

#include <stdint.h>
#include "gf_type_define.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALGO_VERSION_INFO_LEN    64
#define FW_VERSION_INFO_LEN      64
#define TEE_VERSION_INFO_LEN     72
#define TA_VERSION_INFO_LEN      64
#define PRODUCTION_DATE_LEN      32
#define GF_CHIP_ID_LEN           16
#define GF_VENDOR_ID_LEN         16
#define GF_SENSOR_ID_LEN         16
#define GF_HEART_BEAT_ALGO_VERSION_LEN 64
#define GF_MAX_SPI_RW_LEN        24
#define MAX_CALLBACK_RESULT_LEN  1024

#ifdef GF_CHIP_SERIES_MILAN_A
#define GF_SENSOR_OTP_INFO_LEN         64
#else
#define GF_SENSOR_OTP_INFO_LEN         32
#endif


// MAX(oswego_m: 13824 + 22, milan_f_series: 22176 + 4, milan_a_series: 30000)
#define ORIGIN_RAW_DATA_LEN      30000

// MAX(96 * 96, 68 * 118, 88 * 108, 54 * 176, 64 * 176, 132 * 112, 64 * 120)
#define IMAGE_BUFFER_LEN         14784

// MAX(MILAN F:88 * 108, OSWEGO M:15 * 96 * 3)
#define NAV_BUFFER_LEN           9504
#define NAV_MAX_FRAMES           20

#define FPC_KEY_DATA_LEN         4

#define DUMP_TEMPLATE_BUFFER_LEN 409600  // (400 * 1024)

#define GSC_UNTOUCH_LEN          10
#define GSC_TOUCH_LEN            5
#define GF_LIVE_DATA_LEN         18
#define HBD_DISPLAY_DATA_LEN     5
#define HBD_RAW_DATA_LEN         12
#define HBD_BUFFER_LEN           24
#define GSC_RAW_DATA_BUFFER_LEN  24

/* fw and cfg binary length*/
// MAX(oswego_m:43022=(4+8+2+(32+4+2+4)*1024), milan_a_series:5120)
#define GF_FW_LEN                43022
// MAX(oswego_m:249, milan_a_series:256)
#define GF_CFG_LEN               256

// below is defined for fido, UVT means User Verify Token.
#define MAX_AAID_LEN             32
#define MAX_FINAL_CHALLENGE_LEN  32
#define MAX_UVT_LEN              512  // the size which fido suggest is 8k

/*operation array length is equal priority classes*/
#define MAX_OPERATION_ARRAY_SIZE (10)

#define PRIORITY_UNDEFINE        (10)
#define PRIORITY_TEST            (0)
#define PRIORITY_AUTHENTICATE    (1)
#define PRIORITY_PRIOR_TEST      (2)

#define MAX_CONTINUE_FRAME_NUM   (2)
#define CRC32_SIZE               (4)

#define BROKEN_CHECK_MAX_FRAME_NUM  3

typedef enum {
    MODE_IMAGE = 0,
    MODE_KEY,
    MODE_SLEEP,
    MODE_FF,
    MODE_NAV,
    MODE_NAV_BASE,
    MODE_DEBUG,
    MODE_FINGER_BASE,
    MODE_IDLE,
    MODE_HBD,
    MODE_HBD_DEBUG,
    MODE_IMAGE_CONTINUE,
    MODE_BROKEN_CHECK_DEFAULT,
    MODE_BROKEN_CHECK_NEGATIVE,
    MODE_BROKEN_CHECK_POSITIVE,
    MODE_TEST_BAD_POINT,
    MODE_TEST_PIXEL_OPEN_DEFAULT,
    MODE_TEST_PIXEL_OPEN_POSITIVE,
    MODE_TEST_BAD_POINT_FINGER_BASE,
    MODE_MAX,
    MODE_NONE = 1000,
} gf_mode_t;

typedef enum {
    GF_CMD_DETECT_SENSOR = 1000,
    GF_CMD_INIT,
    GF_CMD_EXIT,
    GF_CMD_DOWNLOAD_FW,
    GF_CMD_DOWNLOAD_CFG,
    GF_CMD_INIT_FINISHED,
    GF_CMD_PRE_ENROLL,
    GF_CMD_ENROLL,
    GF_CMD_POST_ENROLL,
    GF_CMD_CANCEL,
    GF_CMD_AUTHENTICATE,
    GF_CMD_GET_AUTH_ID,
    GF_CMD_SAVE,
    GF_CMD_REMOVE,
    GF_CMD_SET_ACTIVE_GROUP,
    GF_CMD_ENUMERATE,
    GF_CMD_IRQ,
    GF_CMD_SCREEN_ON,
    GF_CMD_SCREEN_OFF,
    GF_CMD_ESD_CHECK,
    GF_CMD_SET_SAFE_CLASS,
    GF_CMD_CAMERA_CAPTURE,
    GF_CMD_ENABLE_FINGERPRINT_MODULE,
    GF_CMD_ENABLE_FF_FEATURE,
    GF_CMD_TEST_BAD_POINT,
    GF_CMD_TEST_SENSOR_FINE,
    GF_CMD_TEST_SENSOR_FINE_FINISH,
    GF_CMD_TEST_PIXEL_OPEN,
    GF_CMD_TEST_PIXEL_OPEN_STEP1,
    GF_CMD_TEST_PIXEL_OPEN_FINISH,
    GF_CMD_TEST_PERFORMANCE,
    GF_CMD_TEST_SPI_PERFORMANCE,
    GF_CMD_TEST_SPI_TRANSFER,
    GF_CMD_TEST_PRE_SPI,
    GF_CMD_TEST_SPI,
    GF_CMD_TEST_SPI_RW,
    GF_CMD_TEST_PRE_GET_VERSION,
    GF_CMD_TEST_GET_VERSION,
    GF_CMD_TEST_FRR_FAR_INIT,
    GF_CMD_TEST_FRR_FAR_RECORD_BASE_FRAME,
    GF_CMD_TEST_FRR_FAR_RECORD_ENROLL,
    GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE,
    GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH,
    GF_CMD_TEST_FRR_FAR_PLAY_BASE_FRAME,
    GF_CMD_TEST_FRR_FAR_PLAY_ENROLL,
    GF_CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE,
    GF_CMD_TEST_FRR_FAR_ENROLL_FINISH,
    GF_CMD_TEST_FRR_FAR_CANCEL,
    GF_CMD_TEST_RESET_PIN1,
    GF_CMD_TEST_RESET_PIN2,
    GF_CMD_TEST_INTERRUPT_PIN,
    GF_CMD_TEST_DOWNLOAD_FW,
    GF_CMD_TEST_DOWNLOAD_CFG,
    GF_CMD_TEST_DOWNLOAD_FWCFG,
    GF_CMD_TEST_RESET_FWCFG,
    GF_CMD_TEST_SENSOR_VALIDITY,
    GF_CMD_TEST_SET_CONFIG,
    GF_CMD_TEST_DRIVER_CMD,
    GF_CMD_TEST_UNTRUSTED_ENROLL,
    GF_CMD_TEST_UNTRUSTED_AUTHENTICATE,
    GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER,
    GF_CMD_TEST_CHECK_FINGER_EVENT,
    GF_CMD_TEST_BIO_CALIBRATION,
    GF_CMD_TEST_HBD_CALIBRATION,
    GF_CMD_TEST_CANCEL,
    GF_CMD_TEST_REAL_TIME_DATA,
    GF_CMD_TEST_BMP_DATA,
    GF_CMD_TEST_READ_CFG,
    GF_CMD_TEST_READ_FW,
    GF_CMD_NAVIGATE,
    GF_CMD_DETECT_NAV_EVENT,
    GF_CMD_NAVIGATE_COMPLETE,
    GF_CMD_DUMP_NAV_DATA,
    GF_CMD_CHECK_FINGER_LONG_PRESS,
    GF_CMD_FDT_DOWN_TIMEOUT,
    GF_CMD_START_HBD,
    GF_CMD_AUTHENTICATE_FIDO,
    GF_CMD_DUMP_TEMPLATE,
    GF_CMD_DUMP_DATA,
    GF_CMD_DUMP_ORIGIN_DATA,
    GF_CMD_TEST_PRIOR_CANCEL,
    GF_CMD_TEST_NOISE,
    GF_CMD_TEST_RAWDATA_SATURATED,
    GF_CMD_UPDATE_STITCH,
    GF_CMD_AUTHENTICATE_STUDY,
    GF_CMD_MAX,
} gf_cmd_id_t;

typedef enum {
    OPERATION_ENROLL = 0,/*0*/
    OPERATION_AUTHENTICATE_IMAGE,/*1*/
    OPERATION_AUTHENTICATE_FF,/*2*/
    OPERATION_AUTHENTICATE_SLEEP,/*3*/
    OPERATION_AUTHENTICATE_FIDO,/*4*/
    OPERATION_FINGER_BASE,/*5*/
    OPERATION_NAV,/*6*/
    OPERATION_NAV_BASE,/*7*/
    OPERATION_CHECK_FINGER_LONG_PRESS,/*8*/
    OPERATION_HOME_KEY,/*9*/
    OPERATION_POWER_KEY,/*10*/
    OPERATION_CAMERA_KEY,/*11*/
    OPERATION_HEARTBEAT_KEY,/*12*/
    OPERATION_TEST_IMAGE_MODE,/*13*/
    OPERATION_TEST_DEBUG_MODE,/*14*/
    OPERATION_TEST_FF_MODE,/*15*/
    OPERATION_TEST_KEY_MODE,/*16*/
    OPERATION_TEST_NAV_MODE,/*17*/
    OPERATION_TEST_NAV_BASE_MODE,/*18*/
    OPERATION_TEST_FINGER_BASE_MODE,/*19*/
    OPERATION_TEST_IDLE_MODE,/*20*/
    OPERATION_TEST_SLEEP_MODE,/*21*/
    OPERATION_TEST_HBD_DEBUG_MODE,/*22*/
    OPERATION_TEST_HBD_MODE,/*23*/
    OPERATION_TEST_PIXEL_OPEN_STEP1,/*24*/
    OPERATION_TEST_PIXEL_OPEN_STEP2,/*25*/
    OPERATION_TEST_BAD_POINT_RECODE_BASE,/*26*/
    OPERATION_TEST_BAD_POINT,/*27*/
    OPERATION_TEST_PERFORMANCE,/*28*/
    OPERATION_TEST_SPI_PERFORMANCE,/*29*/
    OPERATION_TEST_SPI_TRANSFER,/*30*/
    OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME,/*31*/
    OPERATION_TEST_FRR_FAR_RECORD_ENROLL,/*32*/
    OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE,/*33*/
    OPERATION_TEST_UNTRUSTED_ENROLL,/*34*/
    OPERATION_TEST_UNTRUSTED_AUTHENTICATE,/*35*/
    OPERATION_TEST_CHECK_FINGER_EVENT,/*36*/
    OPERATION_TEST_BIO_CALIBRATION,/*37*/
    OPERATION_TEST_HBD_CALIBRATION,/*38*/
    OPERATION_TEST_REAL_TIME_DATA,/*39*/
    OPERATION_TEST_BMP_DATA,/*40*/
    OPERATION_TEST_SENSOR_VALIDITY,/*41*/
    OPERATION_TEST_RESET_PIN,/*42*/
    OPERATION_TEST_INTERRUPT_PIN,/*43*/
    OPERATION_TEST_PRE_SPI,/*44*/
    OPERATION_SCREEN_OFF_SLEEP,/*45*/
    OPERATION_TEST_DATA_NOISE, /*46*/
    OPERATION_TEST_RAWDATA_SATURATED, /*47*/
    OPERATION_TEST_SENSOR_FINE_STEP1,/*48*/
    OPERATION_TEST_SENSOR_FINE_STEP2,/*49*/
    OPERATION_BROKEN_CHECK_DEFAULT,/*50*/
    OPERATION_BROKEN_CHECK_NEGATIVE,/*51*/
    OPERATION_BROKEN_CHECK_POSITIVE,/*52*/
    OPERATION_NONE,/*53*/
    OPERATION_INVAILD,/*54*/
    OPERATION_MAX,
} gf_operation_type_t;

typedef enum {
    GF_NAV_NONE = 0,
    GF_NAV_FINGER_UP,
    GF_NAV_FINGER_DOWN,
    GF_NAV_UP,
    GF_NAV_DOWN,
    GF_NAV_LEFT,
    GF_NAV_RIGHT,
    GF_NAV_CLICK,
    GF_NAV_HEAVY,
    GF_NAV_LONG_PRESS,
    GF_NAV_DOUBLE_CLICK,
    GF_NAV_MAX,
} gf_nav_code_t;

typedef enum {
    GF_NAV_CLICK_STATUS_NONE = 0,
    GF_NAV_CLICK_STATUS_DOWN,
    GF_NAV_CLICK_STATUS_DOWN_UP,
} gf_nav_click_status_t;

typedef enum {
    GF_KEY_NONE = 0,  //
    GF_KEY_HOME,
    GF_KEY_POWER,
    GF_KEY_MENU,
    GF_KEY_BACK,
    GF_KEY_CAMERA,
    GF_KEY_MAX,
} gf_key_code_t;

typedef enum {
    GF_KEY_STATUS_UP = 0,  //
    GF_KEY_STATUS_DOWN,
} gf_key_status_t;

/* TODO: use bitmask, since several IRQs would occur the same time */
#define GF_IRQ_FINGER_DOWN_MASK     (1 << 1)
#define GF_IRQ_FINGER_UP_MASK       (1 << 2)
#define GF_IRQ_MENUKEY_DOWN_MASK    (1 << 3)
#define GF_IRQ_MENUKEY_UP_MASK      (1 << 4)
#define GF_IRQ_BACKKEY_DOWN_MASK    (1 << 5)
#define GF_IRQ_BACKKEY_UP_MASK      (1 << 6)
#define GF_IRQ_IMAGE_MASK           (1 << 7)
#define GF_IRQ_RESET_MASK           (1 << 8)
#define GF_IRQ_TMR_IRQ_MNT_MASK     (1 << 9)  // idle timeout int
#define GF_IRQ_ONE_FRAME_DONE_MASK  (1 << 10)
#define GF_IRQ_ESD_IRQ_MASK         (1 << 11)  // esd irq
#define GF_IRQ_ADC_FIFO_FULL_MASK   (1 << 12)  // ADC test irq
#define GF_IRQ_ADC_FIFO_HALF_MASK   (1 << 13)  // ADC test irq
#define GF_IRQ_FDT_REVERSE_MASK     (1 << 14)  // fdt_irq1 for milan f
#define GF_IRQ_NAV_MASK             (1 << 15)
#define GF_IRQ_GSC_MASK             (1 << 16)  // For MiLan A
#define GF_IRQ_HBD_MASK             (1 << 17)  // For MiLan A
#define GF_IRQ_FW_ERR_MASK          (1 << 18)  // For MiLan A
#define GF_IRQ_CFG_ERR_MASK         (1 << 19)  // For MiLan A
#define GF_IRQ_ESD_ERR_MASK         (1 << 20)  // For MiLan A
#define GF_IRQ_NAV_LEFT_MASK        (1 << 21)  // For MiLan A
#define GF_IRQ_NAV_RIGHT_MASK       (1 << 22)  // For MiLan A
#define GF_IRQ_NAV_UP_MASK          (1 << 23)  // For MiLan A
#define GF_IRQ_NAV_DOWN_MASK        (1 << 24)  // For MiLan A
#define GF_IRQ_PRESS_LIGHT_MASK     (1 << 25)  // For MiLan A
#define GF_IRQ_PRESS_HEAVY_MASK     (1 << 26)  // For MiLan A
#define GF_IRQ_UPDATE_BASE_MASK     (1 << 27)  // For MiLan A
#define GF_IRQ_TEMPERATURE_CHANGE_MASK  (1 << 28)  // For Milan HV

typedef enum {
    CONFIG_NORMAL = 0,  //
    CONFIG_TEST_PIXEL_OPEN_A,
    CONFIG_TEST_PIXEL_OPEN_B,
    CONFIG_TEST_HBD,
    CONFIG_TEST_FROM_FILE,
} gf_config_type_t;

typedef struct {
    uint8_t reset_flag;

    gf_mode_t mode;
    gf_operation_type_t operation;

    /*compatible with different TEE*/
    gf_error_t result;

    gf_operation_type_t operation_array[MAX_OPERATION_ARRAY_SIZE];

    uint32_t operation_id;
    uint32_t cmd_id;
    uint8_t rsv_data[32];
} gf_cmd_header_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_config_t config;
} gf_detect_sensor_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t download_fw_flag;
    uint8_t download_cfg_flag;
    uint8_t esd_check_flag;
    uint32_t row;
    uint32_t col;
    uint32_t nav_row;
    uint32_t nav_col;
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t otp_info[GF_SENSOR_OTP_INFO_LEN];
    uint32_t otp_info_len;
} gf_init_t;

typedef struct {
    uint8_t vendor_id;
    uint8_t mode;
    uint8_t operation;
    uint8_t reserved[5];
} gf_ioc_chip_info;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint16_t orientation;
    uint16_t facing;
} gf_init_finished_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint64_t challenge;
} gf_pre_enroll_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
    uint8_t system_auth_token_version;
    gf_hw_auth_token_t hat;
} gf_enroll_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint64_t operation_id;
    int32_t screen_on_flag;
} gf_authenticate_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t aaid_len;
    uint8_t aaid[MAX_AAID_LEN];
    uint32_t final_challenge_len;
    uint8_t final_challenge[MAX_FINAL_CHALLENGE_LEN];
} gf_authenticate_fido_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint64_t auth_id;
} gf_get_auth_id_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_operation_type_t operation;
} gf_cancel_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t nav_mode;
} gf_nav_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
} gf_save_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
} gf_update_stitch_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
    uint32_t removing_templates;
    uint32_t deleted_fids[MAX_FINGERS_PER_USER];
    uint32_t deleted_gids[MAX_FINGERS_PER_USER];
} gf_remove_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
} gf_set_active_group_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t safe_class;
} gf_set_safe_class_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t size;
    uint32_t group_ids[MAX_FINGERS_PER_USER];
    uint32_t finger_ids[MAX_FINGERS_PER_USER];
} gf_enumerate_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t enable_flag;
} gf_enable_fingerprint_module_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t enable_flag;
} gf_enable_ff_feature_t;

typedef struct {
    int32_t image_quality;
    int32_t valid_area;
    uint32_t match_score;
    int32_t key_point_num;
    uint32_t increase_rate;
    uint32_t overlay;
    uint32_t get_raw_data_time;
    uint32_t broken_check_time;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t enroll_time;
    uint32_t authenticate_time;
    uint32_t authenticate_update_flag;
    uint32_t authenticate_finger_count;
    uint32_t authenticate_finger_time;
    uint32_t total_time;
    uint32_t get_gsc_data_time;
    uint32_t bio_assay_time;
    int32_t bio_assay_ret;
} gf_test_performance_t;

typedef struct {
    uint32_t get_dr_timestamp_time;
    uint32_t get_mode_time;
    uint8_t fw_version[FW_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint32_t get_chip_id_time;
    uint32_t get_vendor_id_time;
    uint32_t get_sensor_id_time;
    uint32_t get_fw_version_time;
    uint32_t get_image_time;
    uint32_t raw_data_len;
} gf_test_spi_performance_t;

typedef struct {
    unsigned short m_avgDiffVal;  // NOLINT
    double m_noise;
    unsigned int m_badPixelNum;
    unsigned int m_localSmallBadPixelNum;
    unsigned int m_localBigBadPixelNum;
    unsigned int m_flatnessBadPixelNum;
    unsigned int m_isBadLine;
    float m_allTiltAngle;
    float m_blockTiltAngleMax;
} gf_bad_point_test_result_oswego_t;

// è¿”å??‚æ•°
typedef struct {
    uint16_t total;
    uint16_t local;
    uint16_t localWorst;
    uint32_t singular;
    uint8_t *pBadPixels;
} gf_bad_point_test_result_milan_t;

typedef union {
    gf_bad_point_test_result_oswego_t oswego;
    gf_bad_point_test_result_milan_t milan;
} gf_bad_point_test_result_t;

typedef struct {
    gf_bad_point_test_result_t result;
    uint8_t algo_processed_flag;
} gf_test_bad_point_t;

typedef struct {
    gf_cmd_header_t cmd_header;

    // it is used to test heard beat
    uint32_t hbd_switch_flag;  // 0 : disable, 1: enable
    uint32_t electricity_value;  // LED0

    // virable below used to test both gsc and hear beat
    uint16_t hbd_base;
    uint16_t hbd_avg;

    uint8_t hdb_data[HBD_BUFFER_LEN];
    uint32_t hbd_data_len;
} gf_test_hbd_feature_t;

typedef struct {
    uint8_t broken_checked;
    uint8_t disable_sensor;
    uint8_t disable_study;
} gf_sensor_broken_check_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t fw_version[FW_VERSION_INFO_LEN];
    uint8_t code_fw_version[FW_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint8_t sensor_otp_type;
} gf_test_spi_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t spi_cmd;
    uint32_t start_addr;
    uint32_t rw_len;
    uint8_t rw_content[GF_MAX_SPI_RW_LEN];
} gf_test_spi_rw_t;

typedef struct {
    gf_cmd_header_t cmd_header;

    // if greater than zero, don't check enroll failed condition
    uint8_t check_flag;

    int32_t algo_index;
    uint32_t safe_class;
    uint32_t template_count;
    uint32_t support_bio_assay;

    uint16_t raw_data[IMAGE_BUFFER_LEN];
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    // TEST_TOKEN_RAW_DATA or TEST_TOKEN_BMP_DATA, using what kind of data to run test
    uint32_t data_type;
    int32_t gsc_data[GF_LIVE_DATA_LEN];
    uint32_t gsc_flag;
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t authenticate_time;
} gf_test_frr_far_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    int16_t kr_data[IMAGE_BUFFER_LEN];
    int16_t b_data[IMAGE_BUFFER_LEN];
    uint16_t base_data[IMAGE_BUFFER_LEN];
    uint16_t raw_data[IMAGE_BUFFER_LEN];
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    uint8_t fpc_key_data[FPC_KEY_DATA_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t preprocess_time;
} gf_test_real_time_data_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t preprocess_time;
} gf_test_bmp_data_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t frame_num; /* n */
    uint32_t max_frame_num; /* N */
    uint16_t raw_data[IMAGE_BUFFER_LEN];
} gf_test_data_noise_t;

typedef struct {
    gf_nav_code_t nav_code;
    uint8_t finger_up_flag;
} gf_nav_result_t;

typedef struct {
    uint32_t average_pixel_diff;
} gf_test_sensor_fine_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_nav_result_t nav_result;
} gf_detect_nav_event_t;

// keep the definition consistent to the algorithm implementation in gf_heart_beat.c
typedef enum {
    GF_HEART_BEAT_UNSTABLE = 2,  //
    GF_HEART_BEAT_STABLE = 0
} gf_heart_beat_status_t;

typedef struct {
    uint8_t heart_beat_rate;
    uint8_t status;
    uint8_t index;
    uint16_t raw_data[HBD_BUFFER_LEN];
    int32_t display_data[HBD_DISPLAY_DATA_LEN];
} gf_heart_beat_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    int32_t is_passed;  // < 0:fail, 1:pass>
} gf_test_sensor_validity_t;

typedef struct {
    uint16_t untouch_data_len;
    uint16_t untouch_data[GSC_UNTOUCH_LEN];
    uint16_t touch_data_len;
    uint16_t touch_data[GSC_TOUCH_LEN];
    uint8_t gsc_flag;
} gf_gsc_t;

typedef enum {
    GF_IRQ_STEP_IDLE = 0,  //
    GF_IRQ_STEP_GET_IRQ_TYPE,
    GF_IRQ_STEP_POLLING,
    GF_IRQ_STEP_PRE_GET_IMAGE,  // spi speed: 1M
    GF_IRQ_STEP_GET_IMAGE,  // spi speed: high
    GF_IRQ_STEP_POST_GET_IMAGE,  // spi speed: 1M
    GF_IRQ_STEP_CLEAR_IRQ,
    GF_IRQ_STEP_PROCESS,
} gf_irq_step_t;

typedef struct {
    uint32_t uvt_len;
    uint8_t uvt_buf[MAX_UVT_LEN];
} gf_uvt_t;

typedef struct {
    uint32_t under_saturated_pixel_count;
    uint32_t over_saturated_pixel_count;
    uint32_t saturated_pixel_threshold;
} gf_test_rawdata_saturated_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t irq_type;
    gf_operation_type_t operation;
    gf_mode_t mode;
    uint8_t too_fast_flag;
    uint8_t mistake_touch_flag;
    uint8_t report_authenticate_fail_flag;
    uint32_t dump_selete_index;
    uint32_t bad_pixel_num;
    uint32_t image_count;
    gf_nav_result_t nav_result;
    gf_test_performance_t test_performance;
    gf_test_spi_performance_t test_spi_performance;
    gf_test_frr_far_t test_frr_far;
    gf_test_bad_point_t test_bad_point;
    gf_test_hbd_feature_t test_hdb_feature;
    gf_test_rawdata_saturated_t test_rawdata_saturated;
    gf_sensor_broken_check_t broken_check;
    gf_heart_beat_t heart_beat;
    gf_gsc_t gsc;
    gf_test_sensor_validity_t test_sensor_validity;
    gf_test_real_time_data_t test_real_time_data;
    gf_test_bmp_data_t test_bmp_data;
    gf_test_data_noise_t test_data_noise;
    gf_test_sensor_fine_t test_sensor_fine;

    uint32_t group_id;
    uint32_t finger_id;
    uint16_t samples_remaining;
    uint32_t duplicate_finger_id;
    uint8_t save_flag;
    uint8_t update_stitch_flag;
    gf_test_performance_t dump_performance;

    gf_hw_auth_token_t auth_token;
    gf_uvt_t uvt;

    uint32_t speed;
    gf_irq_step_t step;
} gf_irq_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_sensor_broken_check_t broken_check;
    gf_operation_type_t operation;
    uint32_t group_id;
    uint32_t finger_id;
    uint8_t save_flag;
    uint8_t update_stitch_flag;
} gf_authenticate_study_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t download_fw_flag;
    uint8_t download_cfg_flag;
} gf_esd_check_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    int8_t algo_version[ALGO_VERSION_INFO_LEN];
    int8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    int8_t fw_version[FW_VERSION_INFO_LEN];
    int8_t tee_version[TEE_VERSION_INFO_LEN];
    int8_t ta_version[TA_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    int8_t heart_beat_algo_version[GF_HEART_BEAT_ALGO_VERSION_LEN];
} gf_test_get_version_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t cmd_buf[32];
    uint8_t download_fw_flag;
    gf_mode_t mode;
    uint8_t product_cfg_idx;
    uint32_t secure_share_memory_count;
    uint32_t secure_share_memory_size[10];
    uint64_t secure_share_memory_time[10];
    uint32_t esd_exception_count;
    uint16_t address;
    uint8_t value;
    uint8_t ignore_irq_type;
} gf_test_driver_cmd_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t fw_data[GF_FW_LEN];
    uint32_t fw_data_len;
    uint8_t cfg_data[GF_CFG_LEN];
    uint32_t cfg_data_len;
} gf_test_download_fw_cfg_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_config_t config;
    uint32_t token;
} gf_test_set_config_t;

typedef struct {
    uint8_t data[ORIGIN_RAW_DATA_LEN * MAX_CONTINUE_FRAME_NUM];
    uint32_t data_len;
} gf_image_origin_data_t;

typedef struct {
    uint8_t data[5][ORIGIN_RAW_DATA_LEN];
    uint32_t data_len;
} gf_nav_origin_data_t;

typedef struct {
    gf_operation_type_t operation;
    union {
        gf_image_origin_data_t origin_image;
        gf_nav_origin_data_t origin_nav;
    } data;
} gf_dump_origin_data_t;

typedef struct {
    uint16_t raw_data[IMAGE_BUFFER_LEN * MAX_CONTINUE_FRAME_NUM];
    uint32_t raw_data_len;
    int32_t enroll_select_index;

    uint8_t broken_check_origin_data[2][ORIGIN_RAW_DATA_LEN];
    uint16_t broken_check_raw_data[BROKEN_CHECK_MAX_FRAME_NUM][IMAGE_BUFFER_LEN];
    uint32_t broken_check_frame_num;

    uint8_t gsc_untouch_data[GSC_RAW_DATA_BUFFER_LEN];
    uint32_t gsc_untouch_data_len;
    uint8_t gsc_touch_data[GSC_RAW_DATA_BUFFER_LEN];
    uint32_t gsc_touch_data_len;
    uint16_t gsc_base;

    uint8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];

    uint32_t frame_num;
    int16_t kr[IMAGE_BUFFER_LEN];
    int16_t b[IMAGE_BUFFER_LEN];
    uint16_t cali_res[IMAGE_BUFFER_LEN];
    uint8_t data_bmp[IMAGE_BUFFER_LEN];
    uint8_t sito_bmp[IMAGE_BUFFER_LEN];
    uint32_t select_index;  // 0: no sito; 1: sito

    uint32_t image_quality;
    uint32_t valid_area;

    uint32_t increase_rate_between_stitch_info;
    uint32_t overlap_rate_between_last_template;
    uint32_t enrolling_finger_id;
    uint32_t duplicated_finger_id;

    uint32_t match_score;
    uint32_t match_finger_id;
    uint32_t study_flag;
} gf_image_data_t;

typedef struct {
    uint16_t raw_data[NAV_MAX_FRAMES][NAV_BUFFER_LEN];
    uint32_t raw_data_len;
    uint8_t frame_num[NAV_MAX_FRAMES];
    uint8_t finger_up_flag[NAV_MAX_FRAMES];
    uint32_t nav_times;
    uint32_t nav_frame_index;
    uint32_t nav_frame_count;
} gf_nav_data_t;

typedef struct {
    uint8_t hbd_data[HBD_BUFFER_LEN];
    uint32_t hbd_data_len;
} gf_hbd_data_t;

typedef struct {
    uint8_t template_data[DUMP_TEMPLATE_BUFFER_LEN];
    uint32_t template_len;
    uint32_t group_id;
    uint32_t finger_id;
} gf_dump_template_t;

typedef struct {
    gf_operation_type_t operation;
    union {
        gf_image_data_t image;
        gf_nav_data_t nav;
        gf_hbd_data_t hbd;
    } data;
} gf_dump_data_t;

typedef struct {
    gf_key_code_t key;
    gf_key_status_t status;
} gf_key_event_t;

enum gf_netlink_cmd {
    GF_NETLINK_TEST = 0,  //
    GF_NETLINK_IRQ = 1,
    GF_NETLINK_SCREEN_OFF,
    GF_NETLINK_SCREEN_ON
};

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_irq_t irq;
    uint32_t speed;
    gf_irq_step_t next_step;
} gf_process_irq_t;


typedef struct {
    uint16_t finger_base_rawdata[IMAGE_BUFFER_LEN];
    uint32_t finger_base_rawdata_len;
} gf_base_data_t;

const char* gf_strmode(gf_mode_t mode);
const char* gf_strcmd(gf_cmd_id_t cmd_id);
const char* gf_stroperation(gf_operation_type_t opera);
const char* gf_strnav(gf_nav_code_t nav);
const char* gf_strkey(gf_key_code_t key);
const char* gf_strirq(uint32_t irq_type);

#ifdef __cplusplus
}
#endif

#endif  // __GF_COMMON_H__
