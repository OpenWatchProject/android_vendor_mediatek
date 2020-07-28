/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

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

#endif

#define LOG_TAG "[gf_config] "

gf_error_t gf_config_init(gf_config_t *config, gf_fw_cfg_t *fw_cfg) {
    gf_error_t err = GF_SUCCESS;

    if (NULL == config || NULL == fw_cfg) {
        return GF_ERROR_BAD_PARAMS;
    }

    config->max_fingers = 32;
    config->max_fingers_per_user = 5;

    /**
     * 0: entry sleep mode when screen off.
     * 1: entry Finger Flash mode when screen off
     */
    config->support_ff_mode = 1;

    /**
     * 0: disable feature(key mode), 1: enable.
     */
    config->support_key_mode = 0;
    config->support_power_key_feature = 0;
    config->support_nav_mode = GF_NAV_MODE_NONE; //GF_NAV_MODE_XY,GF_NAV_MODE_NONE;

    /*
     * 0:coating
     * 1:glass
     * 2:ceramic
     */
    config->cover_type = 1;

    config->nav_config.coverType = 0;
    config->nav_config.inertiaX = 2;
    config->nav_config.inertiaY = 1;
    config->nav_config.staticX = 2;
    config->nav_config.staticY = 1;
    config->nav_config.sadXoffThr = 0;
    config->nav_config.sadYoffThr = 0;
    config->nav_config.maxNvgFrameNum = 20;

    config->forbidden_enroll_duplicate_fingers = 0;

    config->report_key_event_only_enroll_authenticate = 1;

    config->support_performance_dump = 1;

    config->require_down_and_up_in_pairs_for_image_mode = 1;
    config->require_down_and_up_in_pairs_for_ff_mode = 0;

    /**
     * 0: do not print the key rawdata
     * 1: print the key rawdata in the following two cases:
     *     a) ESD check
     *     b) when receive B0-interrupt
     */
    config->support_print_key_value = 0;

    //for milan a series
    config->support_fw_navigation = 0;
    config->nav_double_click_interval_in_ms = 300;
    config->nav_long_press_interval_in_ms = 2000;
    config->nav_frame_num = 10;

    config->support_double_click_event = 0;
    config->double_click_time_ms = 300;

    config->support_long_pressed_event = 0;
    config->long_pressed_time_ms = 2000;

#if defined(GF316M) || defined(GF516M) || defined(GF316) || defined(GF516) || defined(GF816M)
    config->enrolling_min_templates = 10;
    config->bad_point_test_max_frame_number = 2;
    config->valid_image_quality_threshold = 15;
    config->valid_image_area_threshold = 65;
    config->duplicate_finger_overlay_score = 70;
    config->increase_rate_between_stitch_info = 15;
    config->reissue_key_down_when_entry_ff_mode = 0;
    config->reissue_key_down_when_entry_image_mode = 1;

    /* broken test config */
    config->support_sensor_broken_check = 1;
    config->broken_pixel_threshold_for_disable_sensor = 400;
    config->broken_pixel_threshold_for_disable_study = 150;

#elif defined(GF318M) || defined(GF3118M) || defined(GF518M) || defined(GF5118M) \
        || defined(GF318) || defined(GF518)
    config->enrolling_min_templates = 8;
    config->bad_point_test_max_frame_number = 2;
    config->valid_image_quality_threshold = 15;
    config->valid_image_area_threshold = 65;
    config->duplicate_finger_overlay_score = 70;
    config->increase_rate_between_stitch_info = 15;
    config->reissue_key_down_when_entry_ff_mode = 0;
    config->reissue_key_down_when_entry_image_mode = 1;

    /* broken test config */
    config->support_sensor_broken_check = 0;
    config->broken_pixel_threshold_for_disable_sensor = 600;
    config->broken_pixel_threshold_for_disable_study = 300;

#elif defined(GF3208) || defined (GF3208FN) || defined(GF3206) \
      || defined(GF3266) || defined(GF3288)
    /**
     * 0: disable broken check function, 1: enable. for Milan_E/F/FN/G/L
     */
    config->support_sensor_broken_check = 0;
    config->broken_pixel_threshold_for_disable_sensor = 600;
    config->broken_pixel_threshold_for_disable_study = 300;
    config->bad_point_test_max_frame_number = 5;

    config->enrolling_min_templates = 8;
    config->valid_image_quality_threshold = 25;
    config->valid_image_area_threshold = 65;
    config->duplicate_finger_overlay_score = 70;
    config->increase_rate_between_stitch_info = 15;
    config->reissue_key_down_when_entry_ff_mode = 0;
    config->reissue_key_down_when_entry_image_mode = 1;
#elif defined(GF5208)

    config->enrolling_min_templates = 8;
    config->bad_point_test_max_frame_number = 5;
    config->valid_image_quality_threshold = 15;
    config->valid_image_area_threshold = 65;
    config->duplicate_finger_overlay_score = 70;
    config->increase_rate_between_stitch_info = 15;

#endif



#ifdef __ANDROID_51
    config->forbidden_untrusted_enroll = 0;
    config->max_authenticate_failed_attempts = 0;
#else
    config->max_authenticate_failed_attempts = 5;
    config->forbidden_untrusted_enroll = 1;
#endif

    config->screen_on_authenticate_fail_retry_count = 1;
    config->screen_off_authenticate_fail_retry_count = 1;

    /**
     * GF_AUTHENTICATE_BY_USE_RECENTLY
     * GF_AUTHENTICATE_BY_ENROLL_ORDER
     * GF_AUTHENTICATE_BY_REVERSE_ENROLL_ORDER
     */
    config->authenticate_order = GF_AUTHENTICATE_BY_USE_RECENTLY;

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
#elif defined(GF5206)
    config->sensor_type = GF_SENSOR_5206;
#elif defined(GF5216)
    config->sensor_type = GF_SENSOR_5216;
#elif defined(GF5208)
    config->sensor_type = GF_SENSOR_5208;
#elif defined(GF3208FN)
    config->sensor_type = GF_SENSOR_3208FN;
#endif

#if defined(GF316M) || defined(GF318M) || defined(GF3118M) || defined(GF516M) || defined(GF518M) \
        || defined(GF5118M) || defined(GF816M)
    fw_cfg->oswego_m.fw_data = (uint8_t *)g_fw_data;
    fw_cfg->oswego_m.fw_data_len = sizeof(g_fw_data);

    {
        uint32_t i = 0;
        for (i = 0; i < OSWEGO_M_VENDOR_CFG_COUNT_MAX; i++) {
            fw_cfg->oswego_m.cfg_data_by_vendor_id[i] = (gf_oswego_m_cfg_by_vendor_id_t *)&g_cfg_data_by_vendor_id[i];
        }

        for (i = 0; i < OSWEGO_M_PRODUCT_CFG_COUNT_MAX; i++) {
            fw_cfg->oswego_m.cfg_data_by_product_idx[i] = (gf_oswego_m_cfg_by_product_idx_t *)&g_cfg_data_by_product_idx[i];
        }
    }

    fw_cfg->oswego_m.cfg_data_pixel_test = (uint8_t *)g_pixel_test_cfg_data;
    fw_cfg->oswego_m.cfg_data_len = GF_OSWEGO_M_CFG_LENGTH;

#elif defined(GF316) || defined(GF318) || defined(GF516) || defined(GF518)
    fw_cfg->oswego_s.fw_data = g_gf_fw_data;
    fw_cfg->oswego_s.fw_data_len = sizeof(g_gf_fw_data);

    fw_cfg->oswego_s.cfg_data_eco_normal = g_gf_eco_normal_cfg_data;
    fw_cfg->oswego_s.cfg_data_eco_ff = g_gf_eco_ff_cfg_data;
    fw_cfg->oswego_s.cfg_data_mp_normal = g_gf_mp_normal_cfg_data;
    fw_cfg->oswego_s.cfg_data_mp_ff = g_gf_mp_ff_cfg_data;
    fw_cfg->oswego_s.cfg_data_len = sizeof(g_gf_eco_normal_cfg_data);
#elif defined(GF5206) || defined(GF5216) || defined(GF5208)
    config->support_key_mode = 0;
    config->support_fpc_key = 0;
    config->support_ring_key = 0;
    config->support_sleep_key = 0;
    config->screen_on_authenticate_fail_retry_count = 0;
    config->screen_off_authenticate_fail_retry_count = 0;
    config->max_authenticate_failed_attempts = 0;
    config->support_bio_assay = 1;
    config->support_hbd = 1;
    config->support_merged_gsc = 1;
    config->support_fw_navigation = 0;
    config->reissue_key_down_when_entry_ff_mode = 1;
    config->reissue_key_down_when_entry_image_mode = 1;
#endif

#if defined(GF5206) || defined(GF5216)
    config->enrolling_min_templates = 20;
    config->bad_point_test_max_frame_number = 5;
    config->valid_image_quality_threshold = 15;
    config->valid_image_area_threshold = 65;
    config->duplicate_finger_overlay_score = 70;
    config->increase_rate_between_stitch_info = 15;
    /************** sensor broken check ***************/
    config->support_sensor_broken_check = 0;
    config->cover_broken_pixel_threshold_for_disable_sensor = 600;
    config->cover_broken_pixel_threshold_for_disable_study = 400;
    /************** cover broken check ***************/
    config->support_cover_broken_check = 1;
    config->cover_broken_pixel_threshold_for_disable_sensor = 2304;
    /************** dynamic enroll ******************/
    config->support_dynamic_enroll = 0;
#endif
#if defined(GF5216)
    config->support_bio_assay = 0;
    config->support_hbd = 0;
#endif
    return err;
}

