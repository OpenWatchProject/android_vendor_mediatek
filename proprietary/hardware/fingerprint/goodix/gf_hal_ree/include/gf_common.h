#ifndef __GF_COMMON_H__
#define __GF_COMMON_H__

#include <stdint.h>
#include "gf_type_define.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALGO_VERSION_INFO_LEN          64
#define FW_VERSION_INFO_LEN            64
#define TEE_VERSION_INFO_LEN           72
#define TA_VERSION_INFO_LEN            64
#define PRODUCTION_DATE_LEN            32
#define GF_CHIP_ID_LEN                 16
#define GF_VENDOR_ID_LEN               16
#define GF_SENSOR_ID_LEN               16

#ifdef GF_CHIP_SERIES_MILAN_A
#define GF_SENSOR_OTP_INFO_LEN         64
#define GF_OTP_INFO_FILEPATH           "/data/vendor/fingerprint/gf_otp_info"
#else
#define GF_SENSOR_OTP_INFO_LEN         32
#endif

#define GF_HEART_BEAT_ALGO_VERSION_LEN 64

#define MAX_CALLBACK_RESULT_LEN 1024

// MAX(96 * 96, 68 * 118, 88 * 108, 54 * 176, 64 * 176, 132 * 112)
#define IMAGE_BUFFER_LEN 14784
// MAX(MILAN F:88 * 108, OSWEGO M:15 * 96 * 3)
#define NAVIGATION_BUFFER_LEN (9504 * 5)   //for save many frames data

#define DUMP_TEMPLATES_MAX_BUF_LEN (400 * 1024)

#define GSC_BUFFER_LEN 256
#define GSC_UNTOUCH_BUFFER_LEN 10
#define GSC_TOUCH_BUFFER_LEN 5
#define HBD_DISPLAY_DATA_LEN 5
#define HBD_RAW_DATA_LEN 12
#define HBD_BUFFER_LEN 24
/* fw and cfg binary length for oswego_m and oswego_s */
#define GF_OSWEGO_M_FW_LENGTH (4+8+2+(32+4+2+4)*1024)   /* 43022 bytes */
#define GF_OSWEGO_M_CFG_LENGTH 249

#define GF_MILAN_A_SERIES_FW_LENGTH   5120
#define GF_MILAN_A_SERIES_CFG_LENGTH   256
#define MAX_FINGER_PATH_NAME_SIZE        (128)

#define ORIGIN_DATA_LEN                  (30000)

typedef enum {
    MODE_IMAGE = 0, //
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
    GF_CMD_RESET_PIN_POLLING_E4,
    GF_CMD_TEST_BAD_POINT,
    GF_CMD_TEST_SENSOR,
    GF_CMD_TEST_SENSOR_STEP1,
    GF_CMD_TEST_SENSOR_FINISH,
    GF_CMD_TEST_PERFORMANCE,
    GF_CMD_TEST_SPI_PERFORMANCE,
    GF_CMD_TEST_SPI_TRANSFER,
    GF_CMD_TEST_PRE_SPI,
    GF_CMD_TEST_SPI,
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
    GF_CMD_TEST_FRR_FAR_SAVE_FINGER,
    GF_CMD_TEST_FRR_FAR_DEL_FINGER,
    GF_CMD_TEST_FRR_FAR_CANCEL,
    GF_CMD_TEST_RESET_PIN,
    GF_CMD_TEST_DOWNLOAD_FW,
    GF_CMD_TEST_DOWNLOAD_CFG,
    GF_CMD_TEST_DOWNLOAD_FW_CFG,
    GF_CMD_TEST_PRE_DOWNLOAD_FW_CFG,
    GF_CMD_TEST_SENSOR_CHECK,
    GF_CMD_TEST_SET_CONFIG,
    GF_CMD_TEST_DRIVER_CMD,
    GF_CMD_TEST_UNTRUSTED_ENROLL,
    GF_CMD_TEST_UNTRUSTED_AUTHENTICATE,
    GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER,
    GF_CMD_TEST_BIO_CHECK_FINGER_EVENT,
    GF_CMD_TEST_BIO_CALIBRATION,
    GF_CMD_TEST_HBD_CALIBRATION,
    GF_CMD_TEST_FPC_KEY_DETECT,
    GF_CMD_TEST_FRR_FAR_RESET_CALIBRATION,
    GF_CMD_TEST_CANCEL,
    GF_CMD_NAVIGATE,
    GF_CMD_NAVIGATE_COMPLETE, ///< means that has get the result for the current navigation operation
    GF_CMD_GET_NAVIGATION_DATA,
    GF_CMD_SET_SESSION_ID,
    GF_CMD_GET_SESSION_ID,
    GF_CMD_CHECK_FINGER_LONG_PRESS,
    GF_CMD_FDT_DOWN_TIMEOUT,
    GF_CMD_START_HBD,
    GF_CMD_GET_IRQ_STATUS_FOR_SPI_CLK,
    GF_CMD_RESET_MODE,
    GF_CMD_POLLING_NAV_DATA,
    GF_CMD_ENABLE_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE,
    GF_CMD_ENABLE_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE,
    GF_CMD_STUDY_FEATURE,
    GF_CMD_MAX,
} gf_cmd_id_t;

typedef enum {
    // operation for image irq
    OPERATION_AUTHENTICATE = 0, //
    OPERATION_ENROLL,
    OPERATION_FINGER_BASE,
    OPERATION_TEST_SENSOR_STEP1,
    OPERATION_TEST_SENSOR_STEP2,
    OPERATION_TEST_BAD_POINT,
    OPERATION_TEST_PERFORMANCE,
    OPERATION_TEST_SPI_PERFORMANCE,
    OPERATION_TEST_SPI_TRANSFER,
    OPERATION_TEST_FRR_FAR_RECORD_BASE_FRAME,
    OPERATION_TEST_FRR_FAR_RECORD_ENROLL,
    OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE,
    OPERATION_TEST_UNTRUSTED_ENROLL,
    OPERATION_TEST_UNTRUSTED_AUTHENTICATE,
    OPERATION_TEST_CHECK_FINGER_EVENT,
    OPERATION_TEST_BIO_CALIBRATION,
    OPERATION_TEST_HBD_CALIBRATION,
    OPERATION_TEST_FPC_KEY_DETECT,
    OPERATION_NAV,
    OPERATION_NAV_BASE,
    OPERATION_CHECK_FINGER_LONG_PRESS,
    OPERATION_SENSOR_CHECK,
    // operation for reset irq
    OPERATION_TEST_RESET_PIN,
    // operation for down/up irq
    OPERATION_HOME_KEY,
    OPERATION_POWER_KEY,
    OPERATION_CAMERA_KEY,
    OPERATION_HEARTBEAT_KEY,
    OPERATION_NONE,
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
} gf_nav_code_t;

typedef enum {
    GF_KEY_NONE = 0, //
    GF_KEY_HOME,
    GF_KEY_POWER,
    GF_KEY_MENU,
    GF_KEY_BACK,
    GF_KEY_CAMERA,
} gf_key_code_t;

typedef enum {
    GF_KEY_STATUS_UP = 0, //
    GF_KEY_STATUS_DOWN,
} gf_key_status_t;

typedef enum {
    GF_TEST_DOWNLOAD_FW = 0, //
    GF_TEST_DOWNLOAD_CFG,
} gf_download_fw_cfg_status_t;

/* TODO: use bitmask, since several IRQs would occur the same time */
#define GF_IRQ_HOMEKEY_DOWN_MASK    (1 << 1)
#define GF_IRQ_HOMEKEY_UP_MASK      (1 << 2)
#define GF_IRQ_MENUKEY_DOWN_MASK    (1 << 3)
#define GF_IRQ_MENUKEY_UP_MASK      (1 << 4)
#define GF_IRQ_BACKKEY_DOWN_MASK    (1 << 5)
#define GF_IRQ_BACKKEY_UP_MASK      (1 << 6)
#define GF_IRQ_IMAGE_MASK           (1 << 7)
#define GF_IRQ_RESET_MASK           (1 << 8)
#define GF_IRQ_TMR_IRQ_MNT_MASK     (1 << 9) //idle timeout int
#define GF_IRQ_ONE_FRAME_DONE_MASK  (1 << 10)
#define GF_IRQ_ESD_IRQ_MASK         (1 << 11) //esd irq
#define GF_IRQ_ADC_FIFO_FULL_MASK   (1 << 12) //ADC test irq
#define GF_IRQ_ADC_FIFO_HALF_MASK   (1 << 13) //ADC test irq
#define GF_IRQ_FDT_REVERSE_MASK     (1 << 14) ///< fdt_irq1 for milan f
#define GF_IRQ_NAV_MASK             (1 << 15)
#define GF_IRQ_GSC_MASK             (1 << 16)//For MiLan A
#define GF_IRQ_HBD_MASK             (1 << 17)//For MiLan A
#define GF_IRQ_FW_ERR_MASK              (1 << 18)//For MiLan A
#define GF_IRQ_CFG_ERR_MASK             (1 << 19)//For MiLan A
#define GF_IRQ_ESD_ERR_MASK             (1 << 20)//For MiLan A
#define GF_IRQ_NAV_LEFT_MASK            (1 << 21)//For MiLan A
#define GF_IRQ_NAV_RIGHT_MASK         (1 << 22)//For MiLan A
#define GF_IRQ_NAV_UP_MASK            (1 << 23)//For MiLan A
#define GF_IRQ_NAV_DOWN_MASK         (1 << 24)//For MiLan A
#define GF_IRQ_PRESS_LIGHT_MASK     (1 << 25)//For MiLan A
#define GF_IRQ_PRESS_HEAVY_MASK     (1 << 26)//For MiLan A
#define GF_IRQ_UPDATE_BASE_MASK     (1 << 27)//For MiLan A

typedef enum {
    CONFIG_NORMAL = 0, //
    CONFIG_TEST_SENSOR_A,
    CONFIG_TEST_SENSOR_B,
    CONFIG_TEST_HBD,
    CONFIG_TEST_FROM_FILE,
} gf_config_type_t;

typedef struct {
    uint8_t reset_flag;

    gf_mode_t mode;
    gf_operation_type_t operation;

    /*compatible with different TEE*/
    gf_error_t result;
} gf_cmd_header_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_config_t config;
    uint8_t otp_buf_sd[64];
    int32_t otp_buf_len_sd;
    int32_t choose_otp_source;
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
    uint32_t nav_gap;
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
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

    uint16_t nav_base[IMAGE_BUFFER_LEN];
    uint32_t nav_base_len; // the count of nav base in uint16_t, not length in bytes.

    uint16_t finger_base[IMAGE_BUFFER_LEN];
    uint32_t finger_base_len; // the count of nav base in uint16_t, not length in bytes.
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
    uint64_t auth_id;
} gf_get_auth_id_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_operation_type_t operation;
} gf_cancel_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
    uint64_t authenticator_id;
} gf_save_t;

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
    uint32_t preprocess_time;
    uint32_t algo_start_time;
    uint32_t get_feature_time;
    uint32_t enroll_time;
    uint32_t authenticate_time;
    uint32_t authenticate_id;
    uint32_t authenticate_update_flag;
    uint32_t authenticate_finger_count;
    uint32_t authenticate_finger_time;
    uint32_t total_time;
    uint32_t get_gsc_data_time;
    uint32_t bio_assay_time;
    int32_t bio_assay_ret;
    uint32_t cover_broken_check_time;
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
    int32_t base_frame_len; //0: invalid, >0:valid
    int16_t base_frame_data[IMAGE_BUFFER_LEN];
} gf_test_bad_point_t;

typedef struct {
    gf_cmd_header_t cmd_header;

    // it is used to test heard beat
    uint32_t hbd_switch_flag; // 0 : disable, 1: enable
    uint32_t electricity_value; // LED0

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
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint8_t sensor_otp_type;
} gf_test_spi_t;

typedef struct {
    gf_cmd_header_t cmd_header;

    //if greater than zero, don't check enroll failed condition
    uint8_t check_flag;

    int32_t algo_index;
    uint16_t raw_data[IMAGE_BUFFER_LEN];
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    // TEST_TOKEN_RAW_DATA or TEST_TOKEN_BMP_DATA, using what kind of data to run test
    uint32_t data_type;
    uint8_t gsc_data[GSC_BUFFER_LEN];
    uint32_t image_quality;
    uint32_t image_valid_area;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t authenticate_time;
} gf_test_frr_far_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;  //user num
    uint32_t finger_id;
    uint8_t finger_name[MAX_FINGER_PATH_NAME_SIZE];
} gf_test_frr_far_save_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t group_id; //user num
    uint32_t finger_id;
} gf_test_frr_far_remove_t;

typedef struct gf_test_frr_far_finger {
    uint32_t group_id;
    uint32_t finger_id;
    struct gf_test_frr_far_finger* next;
} gf_test_frr_far_finger_t;

typedef struct {
    uint32_t count;
    struct gf_test_frr_far_finger* finger;
} gf_test_frr_far_finger_list_t;

typedef struct {
    gf_cmd_header_t cmd_header;

    uint32_t safe_class;
    uint32_t template_count;
    //uint32_t support_bio_assay;
    uint32_t forbidden_duplicate_finger;
    uint32_t finger_group_id;
    uint32_t test_type; //specify frr or far
} gf_test_frr_far_init_t;

typedef struct {
    uint8_t finish_flag;
    uint8_t frame_num;
    uint16_t raw_data[NAVIGATION_BUFFER_LEN];
    uint32_t raw_data_len;
} gf_navigation_t;

// keep the definition consistent to the algorithm implementation in gf_heart_beat.c
typedef enum {
    GF_HEART_BEAT_UNSTABLE = 2, //
    GF_HEART_BEAT_STABLE = 0
} gf_heart_beat_status_t;

typedef struct {
    uint8_t heart_beat_rate;
    uint8_t status;
    uint8_t index;
    uint8_t raw_data[HBD_BUFFER_LEN];
    int32_t display_data[HBD_DISPLAY_DATA_LEN];
} gf_heart_beat_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    int32_t is_pass_sensor_test; // < 0:fail, 1:pass>
} gf_test_check_sensor_test_info_t;

typedef struct {
    uint16_t untouch_data_len;
    uint16_t untouch_data[GSC_UNTOUCH_BUFFER_LEN];
    uint16_t touch_data_len;
    uint16_t touch_data[GSC_TOUCH_BUFFER_LEN];
    uint8_t gsc_flag;
} gf_gsc_t;

typedef struct gf_dump_data {

    int32_t is_base_frame_valid; ///< 0: invalid, 1:valid

    uint8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];

    uint32_t frame_num;
    int16_t kr[IMAGE_BUFFER_LEN];
    int16_t b[IMAGE_BUFFER_LEN];

    /*
     * navigation raw data is stored in #gf_navigation_t,
     * so should dump from #gf_navigation_t for navigation data
     */
    uint16_t raw_data[IMAGE_BUFFER_LEN];
    uint8_t origin_data[ORIGIN_DATA_LEN];

    uint16_t raw_data_for_sensor_broken_check[IMAGE_BUFFER_LEN * 2];

    uint32_t sensor_broken_check_frame_num;

    uint32_t cover_broken_flag;
    uint8_t broken_mask[IMAGE_BUFFER_LEN];

    uint16_t cali_res[IMAGE_BUFFER_LEN];
    uint8_t data_bmp[IMAGE_BUFFER_LEN];
    uint8_t sito_bmp[IMAGE_BUFFER_LEN];

    uint32_t select_index; ///< 0: no sito; 1: sito

    uint32_t image_quality;
    uint32_t image_valid_area;

    uint32_t overlap_rate_to_big_temp;
    uint32_t overlap_rate_to_last_temp;
    uint32_t enroll_finger_id;
    uint32_t duplicated_finger_id;

    uint32_t match_score;
    uint32_t match_finger_id;
    uint32_t study_flag;

    gf_heart_beat_t heart_beat;
} gf_dump_data_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t irq_type;
    gf_operation_type_t operation;
    gf_mode_t mode;
    uint8_t too_fast_flag;
    uint8_t dump_data_flag;
    uint8_t dump_finger_base_flag;
    uint8_t dump_nav_base_flag;
    uint32_t dump_selete_index;
    uint32_t bad_pixel_num;
    uint32_t image_count;
    int32_t cover_broken_num;
    gf_navigation_t navigation;
    gf_test_performance_t test_performance;
    gf_test_spi_performance_t test_spi_performance;
    gf_test_frr_far_t test_frr_far;
    gf_test_bad_point_t test_bad_point;
    gf_test_hbd_feature_t test_hdb_feature;
    gf_sensor_broken_check_t broken_check;
    gf_heart_beat_t heart_beat;
    gf_gsc_t gsc;
    gf_dump_data_t dump_data;

    gf_test_check_sensor_test_info_t test_check_sensor_test_info;

    uint32_t group_id;
    uint32_t finger_id;
    uint16_t samples_remaining;
    uint32_t duplicate_finger_id;
    uint8_t update_flag;
    uint8_t preprocess_success_flag;
    gf_hw_auth_token_t auth_token;
} gf_irq_t;

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
    uint8_t sensor_otp_type;
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
    uint64_t session_id;
} gf_set_session_id_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint64_t session_id;
} gf_get_session_id_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_download_fw_cfg_status_t fw_cfg_status;
    uint8_t fw_cfg_data[GF_OSWEGO_M_FW_LENGTH];
    uint32_t fw_cfg_data_len;
} gf_test_download_fw_cfg_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_config_t config;
    uint32_t token;
} gf_test_set_config_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t irq_type;
} gf_irq_stautus_t;

typedef struct {
    gf_key_code_t key;
    gf_key_status_t status;
} gf_key_event_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t key_code;
    uint8_t fpc_key_en;
    uint8_t rawdata[6]; //reserve
    uint8_t fpc_key_cancel[6]; //reserve
} gf_test_fpc_key_data_t;

enum gf_netlink_cmd {
    GF_NETLINK_TEST = 0, //
    GF_NETLINK_IRQ = 1,
    GF_NETLINK_SCREEN_OFF,
    GF_NETLINK_SCREEN_ON
};

typedef enum {
    GF_OSWEGO_S = 1, //
    GF_OSWEGO_M,
    GF_MILAN_E,
    GF_MILAN_F,
    GF_MILAN_FN,
    GF_MILAN_G,
    GF_MILAN_L,
    GF_MILAN_A,
    GF_MILAN_B,
    GF_MILAN_C,
    GF_MILAN_D,
    GF_OTHERS,
} gf_chip_type_t;

#define IS_MILAN_F_SERIES_BY_CHIP_TYPE(chip_type)         \
                ((GF_MILAN_E == chip_type)      \
                || (GF_MILAN_F == chip_type)    \
                || (GF_MILAN_FN == chip_type)    \
                || (GF_MILAN_G == chip_type)    \
                || (GF_MILAN_L == chip_type))

#define IS_MILAN_F_SERIES_BY_SENSOR_TYPE(sensor_type)         \
                ((GF_SENSOR_3206 == sensor_type)      \
                || (GF_SENSOR_3266 == sensor_type)    \
                || (GF_SENSOR_3288 == sensor_type)    \
                || (GF_SENSOR_3208FN == sensor_type)    \
                || (GF_SENSOR_3208 == sensor_type))

#define IS_MILAN_A_SERIES_BY_CHIP_TYPE(chip_type)         \
                ((GF_MILAN_A == chip_type)    \
                || (GF_MILAN_B == chip_type)  \
                || (GF_MILAN_C == chip_type)  \
                || (GF_MILAN_D == chip_type))

#define IS_MILAN_A_SERIES_BY_SENSOR_TYPE(sensor_type)         \
                ((GF_SENSOR_5206 == sensor_type)       \
                || (GF_SENSOR_5216 == sensor_type)     \
                || (GF_SENSOR_5208 == sensor_type)     \
                || (GF_SENSOR_5218 == sensor_type))

#define IS_OSWEGO_M_BY_SENSOR_TYPE(sensor_type)         \
                ((GF_SENSOR_316M == sensor_type)       \
                || (GF_SENSOR_318M == sensor_type)     \
                || (GF_SENSOR_3118M == sensor_type)     \
                || (GF_SENSOR_516M == sensor_type)     \
                || (GF_SENSOR_518M == sensor_type)     \
                || (GF_SENSOR_5118M == sensor_type)     \
                || (GF_SENSOR_816M == sensor_type))

/* other sensor type need to be added*/

#ifdef __cplusplus
}
#endif

#endif // __GF_COMMON_H__
