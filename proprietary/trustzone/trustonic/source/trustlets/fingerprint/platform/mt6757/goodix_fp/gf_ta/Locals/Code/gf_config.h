#ifndef __GF_CONFIG_H__
#define __GF_CONFIG_H__

#include <stdint.h>
#include "gf_error.h"
#include "gf_type_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GF_VENDOR_COUNT_MAX 6

/* fw and cfg binary length for oswego_m and oswego_s */
#define GF_OSWEGO_M_FW_LENGTH (4+8+2+(32+4+2+4)*1024)   /* 43022 bytes */
#define GF_OSWEGO_M_CFG_LENGTH 249

typedef struct {
    uint8_t vendor_id;
    uint16_t orientation;
    uint16_t facing;
    uint8_t data[GF_OSWEGO_M_CFG_LENGTH];
} gf_oswego_m_cfg_data_t;

typedef struct {
    gf_oswego_m_cfg_data_t *cfg_data[GF_VENDOR_COUNT_MAX];

    uint8_t *cfg_data_pixel_test;
    uint32_t cfg_data_len;

    uint8_t *fw_data;
    uint32_t fw_data_len;
} gf_oswego_m_fw_cfg_t;

typedef struct {
    uint8_t *fw_data;
    uint32_t fw_data_len;

    uint8_t *cfg_data_eco_normal;
    uint8_t *cfg_data_eco_ff;
    uint8_t *cfg_data_mp_normal;
    uint8_t *cfg_data_mp_ff;
    uint32_t cfg_data_len;
} gf_oswego_s_fw_cfg_t;

typedef struct {
    uint8_t *fw_data;
    uint32_t fw_data_len;
    uint8_t *cfg_data;
    uint32_t cfg_data_len;
} gf_milan_ab_fw_cfg_t;

typedef struct {
    gf_sensor_type_t sensor_type;
    uint32_t max_fingers;
    uint32_t max_fingers_per_user;
    uint8_t support_key_mode;
    uint8_t support_image_rescan;
    gf_nav_mode_t support_nav_mode;
    uint8_t support_ff_mode;
    uint8_t support_sensor_broken_check;
    uint16_t broken_pixel_threshold_for_disable_sensor; // broken pixel number to disable sensor
    uint16_t broken_pixel_threshold_for_disable_study; // broken pixel number to disable study function
    uint8_t support_power_key_feature;
    uint8_t forbidden_enroll_duplicate_fingers;
    uint32_t enrolling_min_templates;

    uint32_t valid_image_quality_threshold;
    uint32_t valid_image_area_threshold;
    uint32_t duplicate_finger_overlay_score;
    uint32_t increase_rate_between_stitch_info;
    uint32_t finger_up_threshold;
    uint32_t finger_down_threshold;
    uint32_t rescan_image_quality_threshold;
    uint32_t rescan_image_area_threshold;
    uint32_t rescan_retry_count;
    uint32_t forbidden_untrusted_enroll;

    uint8_t support_bio_assay;
    uint32_t bad_point_test_max_frame_number;

    union {
        gf_oswego_m_fw_cfg_t oswego_m;
        gf_oswego_s_fw_cfg_t oswego_s;
        gf_milan_ab_fw_cfg_t milan_ab;
    } fw_cfg;

} gf_config_t;

gf_error_t gf_config_init(gf_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // __GF_CONFIG_H__
