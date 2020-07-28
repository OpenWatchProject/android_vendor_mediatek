/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __TILT_COMMON_H
#define __TILT_COMMON_H

#include "feature_struct.h"
#include "sensor_manager.h"

#define NUM_DIRECTIONS           3
#define TILT_REF_INTERVAL     1000 // collect enough data number than count the avg
#define TILT_CUR_INTERVAL     2000 // collect enough data number than count the avg
#define TILT_CLASS_NUM           2
#define TILT_INPUT_SAMPLE_DELAY (ACC_DELAY_PER_FIFO_LOOP / ACC_EVENT_COUNT_PER_FIFO_LOOP) // unit: ms
#define TILT_INPUT_ACCUMULATE 2000 // unit: ms
#define MAX_TILT_ACC_NORM_SQ     122070312 // (20 * 1000 * 100) ^ 2 >> 15

enum Directions { X = 0, Y = 1, Z = 2 };
#define COS_0_DEGREES            1         // cos(0 degree) = ((int)(1*SHIFT_VALUE)) = 1 * SHIFT_VALUE
#define COS_35_DEGREES     81915        // cos(35 degree) = 839= 0.8192 * SHIFT_VALUE
#define COS_75_DEGREES     25882        // cos(75 degree)
#define COS_35_DEGREE_SQ   2748   // ask Tingen for this value
#define COS_75_DEGREE_SQ    274   // ask Tingen for this value

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

    // customized tilt angle
    int use_default_angle; // 1: 35 degree, 0: 90 degree (flip)
} tilt_stored_t;

#define WAIT_TIME_SINCE_TILTED 50
#define GRAVITY 980000
#define FACEDOWN_CRITERION_PROB 80
#define FACEDOWN_DURATION 50
#define FACEDOWN_MARGIN 150000

typedef struct flip_stored {
    tilt_stored_t tilt_in_flip;
    int tilt_trigger_then_waiting_now;
    int data_count_from_tilt_trigger;
} flip_stored_t;

uint32_t ref_grav_is_ready(tilt_stored_t *tilt_ptr);
uint32_t cur_grav_is_ready(tilt_stored_t *tilt_ptr);
void tilt_init_cur_grav(tilt_stored_t*);

void tilt_common_set_ts(tilt_stored_t *tilt_ptr, uint32_t input_ts);
void accumulate_ref_cur(tilt_stored_t*, AccSignals_t*);
uint32_t get_acc_norm_sqr_shift_15(int32_t x, int32_t y, int32_t z);

void cal_mean_max_acc(int32_t*, uint32_t*, int , AccSignals_t*);
int cal_facedown_prob(AccSignals_t*, int);
int* cal_mean(AccSignals_t* ac_ptr, int hist_data_leng);
int* cal_std(AccSignals_t*, int);

int facedown_criterion(int acc_x, int acc_y, int acc_z);
int identify_tilt_angle(int32_t* acc_vec1, int32_t* acc_vec2, int default_angle);

#endif
