#include <stdlib.h>
#include "gf_config.h"
#include "gf_type_define.h"

#if defined(GF316M)
#include "gf316m_fw_cfg.h"
#elif defined(GF318M)
#include "gf318m_fw_cfg.h"
#elif defined(GF3118M)
#include "gf3118m_fw_cfg.h"
#elif defined(GF516M)
#include "gf516m_fw_cfg.h"
#elif defined(GF518M)
#include "gf518m_fw_cfg.h"
#elif defined(GF5118M)
#include "gf5118m_fw_cfg.h"
#elif defined(GF816M)
#include "gf816m_fw_cfg.h"
#elif defined(GF316)
#include "gf316_fw_cfg.h"
#elif defined(GF318)
#include "gf318_fw_cfg.h"
#elif defined(GF516)
#include "gf516_fw_cfg.h"
#elif defined(GF518)
#include "gf518_fw_cfg.h"
#elif defined(GF5216)
#include "gf5216_fw_cfg.h"
#endif

#define LOG_TAG "[gf_config] "

gf_error_t gf_config_init(gf_config_t *config) {
    gf_error_t err = GF_SUCCESS;
    if (NULL == config) {
        return GF_ERROR_BAD_PARAMS;
    }

    config->max_fingers = 32;
    config->max_fingers_per_user = 5;

    /**
     * 0: entry sleep mode when screen off.
     * 1: entry Finger Flash mode when screen off
     */
    config->support_ff_mode = 1;

    /*
     *  if opened, sensor will try to aquire another image if the current one is dirty or partial.
     *  0: disable image rescan feature
     *  x>0: enable image rescan feature
     */
    config->support_image_rescan = 1; // enable by default

    /**
     * 0: disable feature(key mode), 1: enable.
     */
    config->support_key_mode = 0;
    config->support_power_key_feature = 0;

    config->support_nav_mode = GF_NAV_MODE_NONE;

    config->forbidden_enroll_duplicate_fingers = 0;

    config->bad_point_test_max_frame_number = 2;

#if defined(GF316M) || defined(GF516M) || defined(GF316) || defined(GF516) || defined(GF816M)
    config->enrolling_min_templates = 10;
#elif defined(GF318M) || defined(GF3118M) || defined(GF518M) || defined(GF5118M) \
        || defined(GF318) || defined(GF518)
    config->enrolling_min_templates = 8;
#elif defined(GF3208)
    /**
     * 0: disable broken check function, 1: enable. for Milan_F
     */
    config->support_sensor_broken_check = 1;
    config->broken_pixel_threshold_for_disable_sensor = 600;
    config->broken_pixel_threshold_for_disable_study = 300;

    config->enrolling_min_templates = 8;
#elif defined(GF5216)
    config->enrolling_min_templates = 10;
#endif

    config->valid_image_quality_threshold = 15;
    config->valid_image_area_threshold = 65;
    config->duplicate_finger_overlay_score = 70;
    config->increase_rate_between_stitch_info = 15;
    config->finger_up_threshold = 0;
    config->finger_down_threshold = 6;

    config->rescan_image_quality_threshold = 10;
    config->rescan_image_area_threshold = 60;

    config->forbidden_untrusted_enroll = 1;
    /*
     * This parameter is set for image rescan feature.
     * 0: rescan until the image quality&area is enough.
     * n(>0): rescan for at most n times.
     */
    config->rescan_retry_count = 1; // When set to 1, the sensor will aquire 2 image at most.

#if defined(GF316M)
    config->sensor_type = GF_SENSOR_316M;
#elif defined(GF318M)
    config->sensor_type = GF_SENSOR_318M;
#elif defined(GF3118M)
    config->sensor_type = GF_SENSOR_3118M;
#elif defined(GF516M)
    config->sensor_type = GF_SENSOR_516M;
#elif defined(GF518M)
    config->sensor_type = GF_SENSOR_518M;
#elif defined(GF5118M)
    config->sensor_type = GF_SENSOR_5118M;
#elif defined(GF816M)
    config->sensor_type = GF_SENSOR_816M;
#elif defined(GF316)
    config->sensor_type = GF_SENSOR_316;
#elif defined(GF318)
    config->sensor_type = GF_SENSOR_318;
#elif defined(GF516)
    config->sensor_type = GF_SENSOR_516;
#elif defined(GF518)
    config->sensor_type = GF_SENSOR_518;
#elif defined(GF3208)
    config->sensor_type = GF_SENSOR_3208;
#elif defined(GF5216)
    config->sensor_type = GF_SENSOR_5216;
#endif

#if defined(GF316M) || defined(GF318M) || defined(GF3118M) || defined(GF516M) || defined(GF518M) \
        || defined(GF5118M) || defined(GF816M)
    config->fw_cfg.oswego_m.fw_data = g_fw_data;
    config->fw_cfg.oswego_m.fw_data_len = sizeof(g_fw_data);

    for (uint32_t i = 0; i < GF_VENDOR_COUNT_MAX; i++) {
        config->fw_cfg.oswego_m.cfg_data[i] = &g_cfg_data[i];
    }

    config->fw_cfg.oswego_m.cfg_data_pixel_test = g_pixel_test_cfg_data;
    config->fw_cfg.oswego_m.cfg_data_len = GF_OSWEGO_M_CFG_LENGTH;

#elif defined(GF316) || defined(GF318) || defined(GF516) || defined(GF518)
    config->fw_cfg.oswego_s.fw_data = g_gf_fw_data;
    config->fw_cfg.oswego_s.fw_data_len = sizeof(g_gf_fw_data);

    config->fw_cfg.oswego_s.cfg_data_eco_normal = g_gf_eco_normal_cfg_data;
    config->fw_cfg.oswego_s.cfg_data_eco_ff = g_gf_eco_ff_cfg_data;
    config->fw_cfg.oswego_s.cfg_data_mp_normal = g_gf_mp_normal_cfg_data;
    config->fw_cfg.oswego_s.cfg_data_mp_ff = g_gf_mp_ff_cfg_data;
    config->fw_cfg.oswego_s.cfg_data_len = sizeof(g_gf_eco_normal_cfg_data);
#elif defined(GF5216)
    config->fw_cfg.milan_ab.fw_data = g_fw_data;
    config->fw_cfg.milan_ab.fw_data_len = sizeof(g_fw_data);

    config->fw_cfg.milan_ab.cfg_data = g_cfg_data;
    config->fw_cfg.milan_ab.cfg_data_len = sizeof(g_cfg_data);

    config->support_key_mode = 1;
#endif
    return err;
}

