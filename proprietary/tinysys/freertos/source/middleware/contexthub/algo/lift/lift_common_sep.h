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
#include "lift_setting.h"

#define AX_N                      3
enum Directions { X = 0, Y = 1, Z = 2 };

#define ACC_BUF_L                   50        // length of accel buffer
#define TILT_INPUT_SAMPLE_DELAY     20        // unit: ms
#define TILT_INPUT_ACCUMULATE       2000      // unit: ms
#define TILT_REF_INTERVAL           1000      // Minimum time length of acc to count reference gravity (unit:ms)
#define TILT_CUR_INTERVAL           2000      // Minimum time length of acc to count current gravity (unit:ms)
#define TILT_ANGLE_SENSITIVITY      2748      // Fixed, 2748 for 35 degree (original tilt angle defined by google)

#define LIFT_INPUT_SAMPLE_DELAY     20        // unit: ms
#define FLIP_INPUT_SAMPLE_DELAY     20        // unit: ms
#define FLIP_ANGLE_SENSITIVITY      274       // Flip, 274 for 75 degree, adjust to a smaller value for larger angle

#define TILT_FEATURE_UPDATE_TIME    20
#define LIFT_FEATURE_UPDATE_TIME    20
#define EXCEP_TIME_BACK_JUMP        2000 // unit:ms
#define EXCEP_INPUT_MAX_INTERVAL    10   // unit:ms

extern customer_gesture_config_t lift_config;

typedef struct tilt_stored {
    uint32_t current_ts;

    int32_t ref[AX_N], cur[AX_N];
    uint32_t ref_max_norm, cur_max_norm; // maximum norm in the ref/max gravity window
    int ref_is_ready;
    int tilt_option;

    // show how many number of acc Ref/Cur data is ready
    unsigned long available_ref_interval, available_cur_interval;
    unsigned int available_ref_num, available_cur_num;
    unsigned long starting_ref_ts, starting_cur_ts;
} lift_stored_t;

typedef struct {
    uint32_t last_ts;
    int32_t writer_idx;
    int32_t reader_idx;
    int32_t x[ACC_BUF_L];
    int32_t y[ACC_BUF_L];
    int32_t z[ACC_BUF_L];
    double pitch[ACC_BUF_L];
    double roll[ACC_BUF_L];
} gesture_acc_buf_t;

gesture_acc_buf_t *get_acc_buf_ptr();

void tilt_alg_init(uint32_t init_time);
void lift_alg_init(uint32_t init_time);
void tilt_reinit_cur_grav(lift_stored_t*);

void saturate_acc_4g(gesture_input_t*);
void lift_common_set_ts(lift_stored_t*, uint32_t);
uint32_t lift_common_get_ts(lift_stored_t*);
uint32_t ref_grav_ready(lift_stored_t*);
uint32_t cur_grav_ready(lift_stored_t*);
void lift_fill_all_buffer(gesture_input_t, lift_stored_t*);
void update_ref_cur(lift_stored_t *tilt_ptr, gesture_acc_buf_t *acc_ptr);
void derive_avg_n_max_norm(int32_t*, uint32_t*, int , gesture_acc_buf_t*);

int normal_lift_detected(int32_t*, int32_t*);
int proj_lift_detected(int32_t*, int32_t*, int8_t, int8_t);
int bed_lift_detected(int32_t*, int32_t*);

#endif
