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

//#define SWITCH_OFF_NORMAL_CASE
//#define SWITCH_OFF_PROJECTED_CASE
//#define SWITCH_OFF_BED_CASE

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

#define MTK_LIFT_CONFIG_NUM 49

// #define LIFT_GRAVITY        980664
#define GRAV_UP_MARGIN_SQ   1640100280896 // 1280664 * 1280664
#define GRAV_DN_MARGIN_SQ   463303480896  //  680664 *  680664

typedef struct {
    uint32_t time_stamp;
    int32_t x;
    int32_t y;
    int32_t z;
} gesture_input_t;

void feed_acc_into_lift(gesture_input_t);

// Pitch represents the rotation around X axis
// Roll represents the rotation around Y axis
// Azimuth represents the rotation around Z axis
//
//
// LG lift gesture positive examples
// 1. Stand(hold the device)
// 2. Stand(inside pocket)
// 3. The device on the desk(screen up)
// 4. The device on the desk(screen down)
// 5. Sit on the sofa(screen up)
// 6. Sit on the sofa(screen down)
// 7. Walk
// 8. Lie on the bed(screen up)
// 9. Lie on the bed(screen down)
// 10. Go up the stairs
// 11. Go down the stairs

#define MAX_TILT_ACC_NORM_SQ 122070312

typedef struct customer_gesture_config {
    int32_t LIFT_MAX_COSINE;
    int32_t LIFT_MIN_COSINE;
    int32_t LIFT_FLAT_Y_VAL;
    int32_t LIFT_CUR_MIN_X;
    int32_t LIFT_CUR_MAX_X;
    int32_t LIFT_CUR_MIN_Y;
    int32_t LIFT_CUR_MAX_Y;
    int32_t LIFT_CUR_MIN_Z;
    int32_t LIFT_CUR_MAX_Z;
    int64_t LIFT_RANGE_AVG_INTERVAL;
    int32_t LIFT_RANGE_MIN_X;
    int32_t LIFT_RANGE_MAX_X;
    int32_t LIFT_RANGE_MIN_Y;
    int32_t LIFT_RANGE_MAX_Y;
    int32_t LIFT_RANGE_MIN_Z;
    int32_t LIFT_RANGE_MAX_Z;
    int32_t LIFT_Y_SENSITIVITY;
    int32_t LIFT_Z_SENSITIVITY;
    int64_t LIFT_WIDE_VAR_SUM;
    int64_t LIFT_WIDE_VAR_SUM_INTERVAL;
    int64_t LIFT_X_WIDE_VAR;
    int64_t LIFT_Y_WIDE_VAR;
    int64_t LIFT_Z_WIDE_VAR;
    int64_t LIFT_WIDE_VAR_INTERVAL;
    int32_t BED_CUR_MIN_X;
    int32_t BED_CUR_MAX_X;
    int32_t BED_CUR_MIN_Y;
    int32_t BED_CUR_MAX_Y;
    int32_t BED_CUR_MIN_Z;
    int32_t BED_CUR_MAX_Z;
    int64_t BED_RANGE_AVG_INTERVAL;
    int32_t BED_RANGE_MIN_X;
    int32_t BED_RANGE_MAX_X;
    int32_t BED_RANGE_MIN_Y;
    int32_t BED_RANGE_MAX_Y;
    int32_t BED_RANGE_MIN_Z;
    int32_t BED_RANGE_MAX_Z;
    int32_t BED_REFRESH_MIN_Z_VAL;
    int64_t BED_SMALL_MOTION_INTERVAL;
    int64_t BED_SMALL_MOTION_VAR;
    int32_t BED_MAX_COSINE;
    int32_t BED_FLAT_X_VAL;
    int32_t BED_FLAT_Y_VAL;
    int32_t LIFT_REF_MIN_Y;
    int64_t LIFT_REF_INTERVAL;
    int64_t LIFT_CUR_INTERVAL;
    int32_t SATURATE_ACC_VAL;
    uint8_t ENABLE_NORMAL_LOG;
    uint8_t ENABLE_BED_LOG;
} customer_gesture_config_t;

// 3 gestures related to tilt, recognized in tilt_option in structure tilt_stored_t
#define FLIP_FLAG                  0
#define TILT_FLAG                  1
#define LIFT_TO_WAKE_FLAG          2        // lift-to-wake and put-to-sleep belong to this category

#define IDLE                       0
#define UPDATE_REF                 1
#define LIFT_TO_WAKE_TRIGGER       2

#define INT32_MAX_VAL              2147483647

// current value of customized parameters are defined in lift_common_sep.h

/*
// Customized parameters
// 1) Min/Max for angle threshold of X/Y/Z-axis
#define LIFT_MAX_COSINE     3821      // 3821 for 15 degree, adjust to a larger value for larger angle
                                            // Fixed-point computation: cos^2(30 degree) << 12
                                            // Example: 3072 for 30 degree
                                            //          2748 for 35 degree
                                            //          274  for 75 degree
#define LIFT_MIN_COSINE     0

// 2) Min and Max: The algorithm reports the event if it is between Min and Max; otherwise, it’s not permitted
#define LIFT_FLAT_X_VAL            0         // if absolute value of x is less than this, recognized as flat.
#define LIFT_FLAT_Y_VAL            171800    // if absolute value of y is less than this, recognized as flat.
#define LIFT_CUR_MIN_X   -490332
#define LIFT_CUR_MAX_X    490332
#define LIFT_CUR_MIN_Y    253659    // if mobile is heading down, don't trigger lift-to-wake
#define LIFT_CUR_MAX_Y   INT32_MAX_VAL
#define LIFT_CUR_MIN_Z    0         // Block facing DOWN criterion, aim to improve facedown->inhand criterion
#define LIFT_CUR_MAX_Z    INT32_MAX_VAL

#define LIFT_RANGE_AVG_INTERVAL    0         // if this value equal to zero means that we don't consider this range (1: 20ms interval, one sample)
#define LIFT_RANGE_MIN_X          -INT32_MAX_VAL
#define LIFT_RANGE_MAX_X           INT32_MAX_VAL
#define LIFT_RANGE_MIN_Y          -INT32_MAX_VAL
#define LIFT_RANGE_MAX_Y           INT32_MAX_VAL
#define LIFT_RANGE_MIN_Z          -INT32_MAX_VAL
#define LIFT_RANGE_MAX_Z           INT32_MAX_VAL

// 3) Speed of the pick up gesture for Pitch and Roll, it can be monitored by the value of comparing the delta of each frame over time.
// MTK internal note: be care of we need to write a detailed description to LG
//                    What is the unit of each parameters?

#define LIFT_Y_SENSITIVITY     100000    // set as 100000 for optimal, set lower to be more sensitive but cause more false alarm
                                            // physical meaning of trigger timing:
                                            //     current y-axis gravity should be larger than stored y-axis reference gravity + this value
#define LIFT_Z_SENSITIVITY     0         // set as 0 for optimal, set lower to be more sensitive but cause more false alarm
                                            // physical meaning of trigger timing:
                                            //     current z-axis gravity + this value should be less than stored z-axis reference gravity

// 4) Wide variation: lift can regard the gesture as the intended pick-up when it has wide variation.
//                    Otherwise, it should ignore it
#define LIFT_WIDE_VAR_SUM          150000    // Acc variance sum of 3 axis of 1s should exceed this value to trigger lift gesture, larger for more strict condition
#define LIFT_WIDE_VAR_SUM_INTERVAL 20        // Indicate how many instances will calculate minimum motion
#define LIFT_X_WIDE_VAR            0         // minimum theshold of x axis variance
#define LIFT_Y_WIDE_VAR            0         // minimum theshold of y axis variance
#define LIFT_Z_WIDE_VAR            0         // minimum theshold of z axis variance
#define LIFT_WIDE_VAR_INTERVAL     0         // Indicate how many instances will calculate minimum motion of each axis, set 0 to disable

// 5) The algorithm should make use of Azimuth; it should distinguish the direction of SET (display side based) : skyward or toward the floor

#define MAX_TILT_ACC_NORM_SQ       122070312 // If there's an epoch of acc input exceed 20 m/s^2, get rid of collected data
                                            // (20 * 1000 * 100) ^ 2 >> 15

// LGE lie on bed scenario
#define BED_CUR_MIN_X    -INT32_MAX_VAL
#define BED_CUR_MAX_X     INT32_MAX_VAL
#define BED_CUR_MIN_Y    -693434
#define BED_CUR_MAX_Y     INT32_MAX_VAL
#define BED_CUR_MIN_Z    -INT32_MAX_VAL
#define BED_CUR_MAX_Z    -490332         // control only facedown criterion to trigger bed lift

#define BED_RANGE_AVG_INTERVAL     0         // if this value equal to zero means that we don't consider this range (1: 20ms interval, one sample)
#define BED_RANGE_MIN_X           -INT32_MAX_VAL
#define BED_RANGE_MAX_X            INT32_MAX_VAL
#define BED_RANGE_MIN_Y           -INT32_MAX_VAL
#define BED_RANGE_MAX_Y            INT32_MAX_VAL
#define BED_RANGE_MIN_Z           -INT32_MAX_VAL
#define BED_RANGE_MAX_Z            INT32_MAX_VAL

#define BED_REFRESH_MIN_Z_VAL      BED_CUR_MAX_Z   // indicate if z value larger than this threshold, we need to refresh ref gravity
#define BED_SMALL_MOTION_INTERVAL  10        // use 10 samples to determine small motion (20ms * 10 = 200)
#define BED_SMALL_MOTION_VAR       30000     // if the sum of acc 3-axis variance lower than this value, recognized as small motion
#define BED_MAX_COSINE      3821      // set 3821 for 15 degree

#define BED_FLAT_X_VAL             0         // if absolute value of x is less than this, recognized as flat.
#define BED_FLAT_Y_VAL             102400    // if absolute value of y is less than this, recognized as flat.

#define LIFT_REF_MIN_Y             750000   // reference gravity should be lower than +45 degree; otherwise, lift gesture won't be triggered

#define LIFT_REF_INTERVAL          250       // Time interval of acc to count avg lift reference gravity (unit:ms)
#define LIFT_CUR_INTERVAL          500       // Time interval of acc to count avg lift current gravity (unit:ms)

#define MIN_INTERVAL_OF_2_LIFT     1000      // Minimum interval between two different lifts, prevent 2 consecutive lift events

#define SATURATE_ACC_VAL           38220     // 9.8*1000*3.9
*/

#endif