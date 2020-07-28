/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __LIFT_SETTING_H__
#define __LIFT_SETTING_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define SWITCH_OFF_DEBUG_LOG
#ifdef SWITCH_OFF_DEBUG_LOG
#define TE_LOG(...) (void)(0)
#else
#define TE_LOG(fmt, ...) fprintf(fmt, ##__VA_ARGS__)
#endif

enum Directions { X = 0, Y = 1, Z = 2, AX_N = 3, };

#define ACC_BUF_L                   50
#define SHAKE_BUFFER_L              100
#define GESTURE_FEATURE_UPDATE_TIME 20
#define EXCEP_TIME_BACK_JUMP        2000
#define EXCEP_INPUT_MAX_INTERVAL    10
#define MTK_GESTURE_CONFIG_NUM      98

typedef enum {
    ANY_TO_NO,
    NO_TO_ANY,
    DISABLE,
} gesture_trans_t;

typedef struct {
    uint32_t time_stamp;
    int32_t x;
    int32_t y;
    int32_t z;
} gesture_input_t;

typedef struct {
    uint32_t last_ts;
    int32_t writer_idx;
    int32_t reader_idx;
    int32_t x[ACC_BUF_L];
    int32_t y[ACC_BUF_L];
    int32_t z[ACC_BUF_L];
} gesture_acc_buf_t;

typedef struct {
    uint8_t input_num;
    uint32_t input_interval;
    uint32_t first_ts;
    uint32_t max_norm;
    int32_t vec[AX_N];
} lift_orient_t;

typedef struct {
    bool invisible_before;
    bool bed_moved_before;
    int32_t visible_count;
    int32_t semivisible_count;
    int32_t invisible_count;
    uint32_t bigmotion_ts;
    uint32_t last_glance_ts;
    uint32_t y_change_ts;
    int32_t dang_ref[AX_N];
} glance_indicator_t;

typedef struct {
    bool mobile_in_put_range;
    uint32_t last_put_ts;
    int32_t put_ref[AX_N];
} put_indicator_t;

typedef struct {
    bool enough_rotation;
    uint32_t enough_rotation_ts;
    int32_t ref[AX_N];
} flip_indicator_t;

typedef struct {
    bool match_desk_pitch;
    bool match_dang_pitch;
    bool match_head_angle;
    uint32_t current_ts;
    int32_t prev_avg_y;
    int ref_is_ready;
    uint32_t freefall_start_ts;
    bool detect_freefall;
    lift_orient_t ref, cur;
} lift_indicator_t;

typedef struct {
    bool shake_buf_set;
    bool shake_detected;
    int32_t z_buffer[SHAKE_BUFFER_L];
} gesture_shake_t;

typedef struct {
    uint8_t input_num;
    uint32_t input_interval;
    uint32_t current_ts;
    uint32_t first_input_ts;

    int32_t orient[AX_N];
    uint32_t max_norm;
} gesture_stored_t;

// TODO: code size here can be optimized by range checking and type-casting
typedef struct {
    bool ENABLE_NORMAL_LOG;
    bool ENABLE_LIFT;
    bool ENABLE_GLANCE;
    bool ENABLE_EXT_GLANCE_GLANCE;
    int32_t LIFT_TO_PUT_HOLD_TIME;
    int64_t ORIENTATION_INTERVAL;
    int32_t SATURATE_ACC_VAL;
    int64_t ABSORB_GRAV_L_BOUND;
    int64_t ABSORB_GRAV_U_BOUND;
    int32_t PUT_ANGLE_SENSITIVITY;
    int32_t PUT_STATIC_INTERVAL;
    int32_t PUT_STATIC_TH;
    int32_t PUT_STATIC_COUNT;
    int32_t PUT_Y_SENSITIVITY;
    int32_t PUT_FLAT_Y_VAL;
    int32_t PUT_DANG_FLAT_Y_VAL;
    float PUT_FACEUP_YZ_RATIO;
    float PUT_FACEDOWN_YZ_RATIO;
    int32_t PUT_FACEDOWN_MAX_Z;
    int32_t PUT_TWO_EVENT_INTERVAL;
    int8_t PUT_RANGE_COUNT_BOUND;
    int32_t FLIP_ANGLE_SENSITIVITY;
    int32_t FLIP_ANGLE_HOLD_TIME;
    int32_t FLIP_STATIC_INTERVAL;
    int32_t FLIP_STATIC_TH;
    int32_t FLIP_STATIC_COUNT;
    int32_t FLIP_RANGE_MAX_REF_Y;
    float FLIP_ZX_RATIO;
    int32_t FLIP_SUPPORT_PUT_Y;
    uint32_t FLIP_DISABLE_TIME;
    int32_t FLIP_SMALL_Y_CHANGE;
    int64_t GLANCE_SEMIVIS_COUNT;
    uint8_t GLANCE_VIS_COUNT;
    uint8_t GLANCE_INVIS_COUNT;
    int64_t GLANCE_BIGMOT_VAR;
    int64_t GLANCE_BIGMOT_Z_VAR;
    int64_t GLANCE_BIGMOT_INTERVAL;
    uint32_t GLANCE_BIGMOT_MAX_DURATION;
    float GLANCE_VIS_YX_RATIO;
    float GLANCE_VIS_FACEUP_YZ_RATIO;
    float GLANCE_VIS_FACEDOWN_Y_Z_RATIO;
    float GLANCE_SEMIVIS_FACEUP_YZ_RATIO;
    float GLANCE_SEMIVIS_FACEDOWN_YZ_RATIO;
    float GLANCE_DESK_DANG_Y_BOUND;
    int32_t EXT_GLANCE_MIN_X;
    int32_t EXT_GLANCE_MAX_X;
    int32_t EXT_GLANCE_MIN_Y;
    int32_t EXT_GLANCE_MAX_Y;
    int32_t EXT_GLANCE_MIN_Z;
    int32_t EXT_GLANCE_MAX_Z;
    int64_t EXT_GLANCE_STATIC_INTERVAL;
    int64_t EXT_GLANCE_STATIC_COND_VAR;
    int8_t EXT_GLANCE_STATIC_COND_COUNT;
    int64_t EXT_GLANCE_X_Y_MOTION_VAR;
    int32_t EXT_GLANCE_Y_CH_VAL;
    uint32_t EXT_GLANCE_Y_CH_INTERVAL;
    int32_t EXT_GLANCE_X_OSCIL_VAL;
    int32_t EXT_GLANCE_Z_OSCIL_U_VAL;
    int32_t EXT_GLANCE_Z_OSCIL_L_VAL;
    int32_t LIFT_TWO_EVENT_INTERVAL;
    int64_t LIFT_REF_INTERVAL;
    int64_t LIFT_INTERVAL;
    int32_t LIFT_ANGLE_SENSITIVITY;
    int32_t LIFT_Y_SENSITIVITY;
    int32_t LIFT_FLAT_Y_VAL;
    int32_t LIFT_FLAT_Z_VAL;
    int32_t LIFT_MIN_X;
    int32_t LIFT_MAX_X;
    int32_t LIFT_DESK_MIN_Y;
    int32_t LIFT_DANG_MIN_Y;
    int32_t LIFT_MAX_Y;
    int32_t LIFT_MIN_Z;
    int32_t LIFT_MAX_Z;
    int32_t LIFT_ABAN_REF_Y_VAL;
    int32_t LIFT_ABAN_REF_Z_VAL;
    int64_t LIFT_ABAN_REF_U_BOUND;
    int64_t LIFT_ABAN_REF_L_BOUND;
    int64_t LIFT_SMALLMOT_VAR;
    int64_t LIFT_SMALLMOT_INTERVAL;
    int64_t LIFT_SMALLMOT_X_VAR;
    int64_t LIFT_SMALLMOT_Y_VAR;
    int64_t LIFT_SMALLMOT_Z_VAR;
    int64_t LIFT_HORIZ_MOTION_U_SQ;
    int64_t LIFT_HORIZ_MOTION_L_SQ;
    uint8_t LIFT_HORIZ_MOTION_INTERVAL;
    int32_t LIFT_HORIZ_MOTION_Z_DIFF;
    int8_t LIFT_HORIZ_MOTION_COUNT;
    float LIFT_YX_RATIO;
    float LIFT_DESK_YZ_RATIO;
    float LIFT_DANG_YZ_RATIO;
    int32_t LIFT_FOUND_BETTER_Z;
    int32_t LIFT_DESK_DANG_Y_BOUND;
    int32_t LIFT_AZIMUTH_ROTATION_VAL;
    int8_t LIFT_ABSORB_GRAV_COUNT;
    int32_t LIFT_INSTANT_Z_BOUND;
    uint64_t LIFT_FREEFALL_TH;
    int32_t LIFT_FREEFALL_SUSPEND_INTERVAL;
    int8_t LIFT_FREEFALL_COUNT;
} mtk_gesture_config_t;

extern mtk_gesture_config_t config;

void reset_original_glance_struct(gesture_stored_t*);
void reset_ext_glance_struct(gesture_stored_t*);
void reset_put_struct();
void reset_lift_struct();
void reset_flip_struct();

void set_glance_ts(gesture_stored_t*, gesture_stored_t*, uint32_t);
void set_lift_ts(uint32_t);
void set_flip_ts(uint32_t);
void set_flip_y_to_lift(int32_t);

bool lift_detected();
bool put_detected(int32_t*, uint32_t);
bool flip_detected(gesture_stored_t*);
bool glance_detected(gesture_stored_t*);
bool ext_glance_detected(gesture_stored_t*);
bool bed_static_detected(gesture_stored_t*);

void feed_acc_to_glance(gesture_input_t);
void feed_acc_in_lift(gesture_input_t);
void lift_fill_all_buffer();
bool lift_vec_ready();
void lift_update_vec(gesture_acc_buf_t *acc_ptr);
int32_t last_lift_y_val();
void disable_flip_ref(uint32_t);
void get_avg_and_max_norm(int32_t*, uint32_t*, int , gesture_acc_buf_t*);

#endif