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

#ifndef __GLANCE_SETTING_H__
#define __GLANCE_SETTING_H__

#include <stdbool.h>

//#define SWITCH_OFF_NORMAL_CASE
//#define SWITCH_OFF_BED_CASE

//#define _PC_DEBUG_
//#define READ_CONFIG_ON_PC      // remember to turn this off in device version
#define SWITCH_OFF_DEBUG_LOG
#define SWITCH_OFF_BED_LOG

#ifdef SWITCH_OFF_DEBUG_LOG
#define TE_LOG(...) (void)(0)
#else
#define TE_LOG(fmt, ...) fprintf(fmt, ##__VA_ARGS__)
#endif

#ifdef SWITCH_OFF_BED_LOG
#define BD_LOG(fmt, ...) (void)(0)
#else
#define BD_LOG(fmt, ...) fprintf(fmt, ##__VA_ARGS__)
#endif

#define IDLE                        0
#define GLANCE_TRIGGER              1
#define AX_N                        3
enum Directions { X = 0, Y = 1, Z = 2 };

#ifndef PI
#define PI                          3.14159265
#endif
#define INT32_MAX_VAL               2147483647

#define GLANCE_INPUT_SAMPLE_DELAY     20
#define GLANCE_FEATURE_UPDATE_TIME    20
#define EXCEP_TIME_BACK_JUMP        2000
#define EXCEP_INPUT_MAX_INTERVAL    10

#define ACC_BUF_L                   50

typedef struct {
    float x;
    float y;
    float z;
} float_input_t;

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

typedef struct glance_indicator {
    bool moved_before;
    bool out_of_visibility_before;
    int32_t visible_count;
    int32_t semi_visible_count;
    int32_t invisible_count;
    int32_t no_2_any_motion_count;
    uint32_t last_big_motion_ts;
} glance_indicator_t;

typedef struct glance_stored {
    uint8_t input_acc_num;
    uint32_t input_acc_interval;
    uint32_t current_ts;
    uint32_t first_input_ts;

    int32_t orient[AX_N];
    uint32_t orient_max_norm;
} glance_stored_t;

#define MTK_GLANCE_CONFIG_NUM         27
typedef struct customer_gesture_config {
    int32_t BED_CUR_MIN_X;
    int32_t BED_CUR_MAX_X;
    int32_t BED_CUR_MIN_Y;
    int32_t BED_CUR_MAX_Y;
    int32_t BED_CUR_MIN_Z;
    int32_t BED_CUR_MAX_Z;
    int64_t BIG_MOTION_VAR;
    int64_t BIG_MOTION_Z_VAR;
    int64_t BIG_MOTION_INTERVAL;
    uint32_t LAST_BIG_MOTION_TS_DIFF;
    int64_t BED_STATIC_INTERVAL;
    int64_t BED_STATIC_COND_VAR;
    int8_t  BED_STATIC_COND_COUNT;
    int64_t BED_X_Y_MOTION_VAR;
    int64_t ORIENTATION_INTERVAL;
    int32_t SATURATE_ACC_VAL;
    float VIS_Y_X_RATIO;
    float VIS_Y_Z_F_UP_RATIO;
    float VIS_Y_Z_F_DOWN_RATIO;
    float SEMI_VIS_Y_Z_F_UP_RATIO;
    float SEMI_VIS_Y_Z_F_DOWN_RATIO;
    int64_t SEMI_VISIBLE_BOUND;
    uint8_t VISIBLE_BOUND;
    uint8_t INVISIBLE_BOUND;
    uint8_t ENABLE_NORMAL_LOG;
    uint8_t ENABLE_BED_LOG;
    int32_t MIN_INTERVAL_BETWEEN_TWO_GLANCE;
} customer_gesture_config_t;

#endif