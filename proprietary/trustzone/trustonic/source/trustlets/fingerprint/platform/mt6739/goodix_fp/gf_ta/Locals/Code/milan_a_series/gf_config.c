/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <stdlib.h>
#include "cpl_string.h"
#include "gf_config.h"
#include "gf_type_define.h"

#define LOG_TAG "[gf_config]"

gf_config_t g_config;

void gf_config_init(void) {
    cpl_memset((void*) &g_config, 0, sizeof(gf_config_t));

    /***************** chip type *****************/
#if defined(GF5206)
    g_config.chip_type = GF_CHIP_5206;
#elif defined(GF5216)
    g_config.chip_type = GF_CHIP_5216;
#elif defined(GF5208)
    g_config.chip_type = GF_CHIP_5208;
#elif defined(GF5218)
    g_config.chip_type = GF_CHIP_5218;
#endif
    /*********************************************/
}

void gf_config_init_by_chip_type(gf_chip_type_t chip_type) {
    g_config.max_fingers = 32;
    g_config.max_fingers_per_user = 5;

    /****************** feature ******************/
    g_config.support_key_mode = 0;
    g_config.support_ff_mode = 1;
    g_config.support_power_key_feature = 0;
    g_config.support_bio_assay = 0;
    g_config.support_hbd = 0;
    g_config.support_merged_gsc = 0;
    if ((GF_CHIP_5206 == chip_type) || (GF_CHIP_5208 == chip_type)) {
        g_config.support_hbd = 1;
        g_config.support_bio_assay = 1;
        g_config.support_merged_gsc = 1;
    }
    /*********************************************/

    /****************** navigate *****************/
    g_config.support_nav_mode = GF_NAV_MODE_NONE;

    g_config.nav_config.coverType = 0;
    g_config.nav_config.inertiaX = 2;
    g_config.nav_config.inertiaY = 1;
    g_config.nav_config.staticX = 2;
    g_config.nav_config.staticY = 1;
    g_config.nav_config.sadXoffThr = 0;
    g_config.nav_config.sadYoffThr = 0;

    g_config.nav_double_click_interval_in_ms = 0;
    g_config.nav_long_press_interval_in_ms = 0;

    g_config.support_nav_get_data_method = GF_NAV_EVENT_DETECT_METHOD_POLLING;

    /*********************************************/

    /****************** enroll *******************/
#ifdef __ANDROID_51
    g_config.forbidden_untrusted_enroll = 0;
#else
    g_config.forbidden_untrusted_enroll = 1;
#endif
#ifdef __TEE_NUTLET
    g_config.forbidden_untrusted_enroll = 0;
#endif
#ifdef __TEE_QSEE
    g_config.forbidden_untrusted_enroll = 0;
#endif

    g_config.forbidden_enroll_duplicate_fingers = 0;

    if (GF_CHIP_5208 == chip_type) {
        g_config.enrolling_min_templates = 8;
        g_config.valid_image_quality_threshold = 15;
        g_config.valid_image_area_threshold = 65;
    } else if ((GF_CHIP_5206 == chip_type) || (GF_CHIP_5216 == chip_type)) {
        g_config.enrolling_min_templates = 10;
        g_config.valid_image_quality_threshold = 15;
        g_config.valid_image_area_threshold = 65;
    }
    g_config.duplicate_finger_overlay_score = 70;
    g_config.increase_rate_between_stitch_info = 15;

    /*********************************************/

    /*************** authenticate ****************/
    g_config.screen_on_authenticate_fail_retry_count = 1;
    g_config.screen_off_authenticate_fail_retry_count = 1;

    g_config.screen_on_valid_touch_frame_threshold = 1;
    g_config.screen_off_valid_touch_frame_threshold = 1;

    g_config.image_quality_threshold_for_mistake_touch = 10;

    g_config.authenticate_order = GF_AUTHENTICATE_BY_USE_RECENTLY;

    g_config.max_authenticate_failed_attempts = 0;

    /***************** template update save threshold *****************/
    /**configure fingerprint template study update save threshold**/
    /**template_update_save_threshold can not be less than 5,and can not be greater than 20**/
    g_config.template_update_save_threshold = 20;

    /*********************************************/

    /********** reissue key down event ***********/
    g_config.reissue_key_down_when_entry_ff_mode = 1;
    g_config.reissue_key_down_when_entry_image_mode = 1;
    g_config.reissue_key_down_when_entry_nav_mode = 0;
    g_config.reissue_key_down_when_entry_key_mode = 0;

    /*********************************************/

    /************ sensor broken check ************/
    g_config.support_sensor_broken_check = 0;

    /*********************************************/

    /************** bad point test ***************/
    g_config.bad_point_test_max_frame_number = 5;

    /*********************************************/

    /************** sensor fine test ***************/
    g_config.average_pixel_diff_threshold = 1200;
    /*********************************************/

    /************ down & up in pairs *************/
    g_config.require_down_and_up_in_pairs_for_image_mode = 1;
    g_config.require_down_and_up_in_pairs_for_ff_mode = 0;
    g_config.require_down_and_up_in_pairs_for_nav_mode = 0;
    g_config.require_down_and_up_in_pairs_for_key_mode = 1;
    /*********************************************/

    /****************** others *******************/
    g_config.report_key_event_only_enroll_authenticate = 1;

    g_config.support_set_spi_speed_in_tee = 1;

    g_config.support_performance_dump = 1;
    g_config.support_frr_analysis = 1;
    /*********************************************/
}
