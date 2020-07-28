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

#ifndef __TILT_COMMON_H__
#define __TILT_COMMON_H__

#include <stdint.h>

// 3 gestures related to tilt, recognized in tilt_option in structure tilt_stored_t
#define FLIP_FLAG                 0
#define TILT_FLAG                 1
#define LIFT_TO_WAKE_FLAG         2        // lift-to-wake and put-to-sleep belong to this category

#define IDLE                      0
#define UPDATE_REF                1
#define LIFT_TO_WAKE_TRIGGER      2

#define TILT_INPUT_SAMPLE_DELAY   20        // unit: ms
#define TILT_INPUT_ACCUMULATE     2000      // unit: ms
#define TILT_REF_INTERVAL         1000      // Minimum time length of acc to count reference gravity (unit:ms)
#define TILT_CUR_INTERVAL         2000      // Minimum time length of acc to count current gravity (unit:ms)
#define MAX_TILT_ACC_NORM_SQ      122070312 // If there's an epoch of acc exceed 20 m/s^2, get rid of collected data
// (20 * 1000 * 100) ^ 2 >> 15
#define TILT_ANGLE_SENSITIVITY    2748      // Fixed, 2748 for 35 degree (original tilt angle defined by google)

// Answercall parameter
#define ANSWERCALL_INPUT_SAMPLE_DELAY   20

// Put-to-sleep parameters
#define PUT_INPUT_SAMPLE_DELAY    20        // unit: ms
#define PUT_ANGLE_SENSITIVITY     4068      // 4068 for 30 degree, adjust to a larger value for smaller angle
#define PUT_Y_VAL_SENSITIVITY     250000    // set as 150000 for more sensitive, but cause more false alarm
#define PUT_FLAT_Y_VAL            171800    // if absolute value of y is less than this, recognized as flat
#define PUT_INCLINED_Y_MAX        692900    // if y value exceed this value, put will not be triggered.
#define PUT_FLAT_STATIC           1000      // if mobile is flat and static, trigger put-to-sleep, smaller for more strict condition

// Lift-to-wake parameters
#define LIFT_INPUT_SAMPLE_DELAY   20        // unit: ms
#define LIFT_ANGLE_SENSITIVITY    4068      // 4068 for 30 degree, adjust to a larger value for larger angle
#define LIFT_Y_VAL_SENSITIVITY    100000    // set as 100000 for optimal, set lower to be more sensitive but cause more false alarm
#define LIFT_FLAT_Y_VAL           171800    // if absolute value of y is less than this, recognized as flat.
#define LIFT_MINIMUM_MOTION       150000    // Acc variance of 1s should exceed this value to trigger lift-to-wake, larger for more strict condition

// Flip parameter
#define FLIP_INPUT_SAMPLE_DELAY   20        // unit: ms
#define FLIP_ANGLE_SENSITIVITY    274       // Flip, 274 for 75 degree, adjust to a smaller value for larger angle

#define NUM_DIRECTIONS            3
enum Directions { X = 0, Y = 1, Z = 2 };

#define TILT_FEATURE_UPDATE_TIME        20
#define LIFT_FEATURE_UPDATE_TIME        20
#define ANSWERCALL_FEATURE_UPDATE_TIME        20
#define DIRECTIONAL_DATA_PROTECT_JUMP 2000 // unit:ms
#define DIRECTIONAL_DATA_PROTECT_INTERVAL 10 // unit:ms

typedef struct tilt_stored {
    uint32_t current_ts;

    // show how many number of acc Ref/Cur data is ready
    unsigned long available_ref_interval, available_cur_interval;
    unsigned int available_ref_num, available_cur_num;
    unsigned long starting_ref_ts, starting_cur_ts;

    // avg of acc measurements over the first sec. after activation or the estimated gravity when the last tilt event was generated
    int32_t ref_estimated_grav[NUM_DIRECTIONS];
    // avg of acc measurements over the last 2 seconds
    int32_t cur_estimated_grav[NUM_DIRECTIONS];

    // collect the maximum norm in the ref/max gravity window
    uint32_t ref_max_grav_norm, cur_max_grav_norm;
    int ref_is_calculated;

    // Customized tilt angle, 0/1/2/3: Flip/tilt/lift-to-wake/put-to-sleep
    int tilt_option;
} tilt_stored_t;

typedef struct flip_stored {
    tilt_stored_t tilt_in_flip;
    int tilt_trigger_then_waiting_now;
    int data_count_from_tilt_trigger;
} flip_stored_t;

///* Data Structure for Directional Sensors */
#define XYZ_DIR 3
#define ACC_BUFFER_DIR_LENGTH   50
typedef struct {
    uint32_t last_time;
    int32_t wrt_accbuffer_ptr;
    int32_t rd_accbuffer_ptr;
    int32_t acceleration_mean[XYZ_DIR];
    int32_t acceleration_norm[ACC_BUFFER_DIR_LENGTH];
    int32_t acceleration_x[ACC_BUFFER_DIR_LENGTH];
    int32_t acceleration_y[ACC_BUFFER_DIR_LENGTH];
    int32_t acceleration_z[ACC_BUFFER_DIR_LENGTH];
} AccSignals_DIR_t;

/* m/s^2 */
typedef struct {
    uint32_t time_stamp;
    int32_t x;
    int32_t y;
    int32_t z;
} Directional_Sensor_Ms2_Data_t;

// basic directional functions
AccSignals_DIR_t *get_acc_buffer_dir_ptr();
void get_signal_dir_acc(Directional_Sensor_Ms2_Data_t signal_xyz);
void check_dir_signal_range(Directional_Sensor_Ms2_Data_t* signal_xyz);
void accsignal_dir_init(uint32_t current_time, uint32_t sample_delay);
void answercall_algorithm_init(uint32_t init_time);
void tilt_algorithm_init(uint32_t init_time);
void lift_algorithm_init(uint32_t init_time);
void flip_algorithm_init(uint32_t init_time);
void put_algorithm_init(uint32_t init_time);
int32_t divide32_dir(int32_t numerator, int32_t denominator);
int64_t divide64_dir(int64_t numerator, int64_t denominator);
void tilt_init_cur_grav(tilt_stored_t*);
void tilt_common_set_ts(tilt_stored_t*, uint32_t);
void accumulate_ref_cur(tilt_stored_t*, AccSignals_DIR_t*);
uint32_t ref_grav_is_ready(tilt_stored_t*);
uint32_t cur_grav_is_ready(tilt_stored_t*);

int* cal_mean(AccSignals_DIR_t*, int);
int* cal_std(AccSignals_DIR_t*, int);
void cal_mean_max_acc(int32_t*, uint32_t*, int , AccSignals_DIR_t*);

int meizu_put(int32_t*, int8_t, int8_t);
int meizu_lift(int32_t*, int32_t*, int8_t, int8_t);
int identify_tilt_angle(int32_t*, int32_t*, int);

#endif