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

#ifndef __LIFT_H
#define __LIFT_H

#define WAIT_DATA_COUNT_SINCE_TILTED  100      // If 75-degree tilt is detected, mobile must be static within next 2s to trigger flip
#define FLIP_STATIC_DURATION          25       // static criterion is obtained by 0.5s time window (20ms * 25)
#define FLIP_STATIC_THRESHOLD         150000   // 100000 means 10 (m/s^2)^2 variance threshold. Acc variance in 0.5s need to smaller than
#                                              // this value to satify the necessary condition of flip

/**************************************************************
   Mediatek Flip gesture, packaged by Tingen Tseng (2017/3/9)
   Flip description:
       1) Recognize if the mobile has a rotation to flip in user's hand
       2) Identify which screen the mobile is facing to user
          (Our customer can use M_orientation provided by MTK)
       3) Identify whether the mobile is heading up/down
          (Our customer can use facing gesture provided by MTK)

    The trigger condition of flip:
       1) The raising-up angle exceeds 75 degree.
       2) After flipping the mobile, it should be relatively static.

    Limitation of flip:
       1) In common using scenario, flip gesture don't support when the mobile is too stright.
          To be more precise, we only support when the angle between sea plane and rotation
          vector is smaller than 60 degree.
       2) The restriction can be alleviated if the position of the G-sensor chip on the mobile
          is obtain by our algorithm. The reason is:
          a) Each mobile has its unique layout. Without the relative position information of
             G-sensor, we cannot derive the rotation movement of entire mobile. Hence, we need
             to implement flip by adopting tilt sensor.
          b) If the mobile rotation vector is close to perpendicular to horizontal plane, the
             tilt angle sensored by mobile will be too small to trigger flip (75 degree tilt).
        3) You can only utilize MTK facing gesture to simply implement an alternative solution
           of flip. But this implementation may not sufficiently report the angle movement of
           mobile, then increase the false alarm rate of flip gesture.
 **************************************************************/

#define OVERHEAD_BOUNDARY_ANGLE    15  // Suggestion range: 0 ~ 25 (degree)
#define TILT_HISTORY_SIZE          200
#define NEAR_ZERO_MAGNITUDE        1.0f                        // m/s^2
#define ACCELERATION_TOLERANCE     4.0f
#define STANDARD_GRAVITY           9.8f
#define MIN_ACCELERATION_MAGNITUDE (STANDARD_GRAVITY - ACCELERATION_TOLERANCE)
#define MAX_ACCELERATION_MAGNITUDE (STANDARD_GRAVITY + ACCELERATION_TOLERANCE)

#define NS2US(x) (x >> 10)                  // convert nsec to approx usec
#define PROPOSAL_SETTLE_TIME                            NS2US(40000000ull)       // 40 ms
#define PROPOSAL_MIN_TIME_SINCE_ACCELERATION_ENDED      NS2US(500000000ull)      // 500 ms

#define RADIANS_TO_DEGREES                  (180.0f / M_PI)
#define SWING_AWAY_ANGLE_DELTA              20
#define SWING_TIME                          NS2US(300000000ull)         // 300 ms
#define MAX_FILTER_DELTA_TIME               NS2US(1000000000ull)        // 1 sec
#define FILTER_TIME_CONSTANT                NS2US(200000000ull)         // 200 ms

#define TILT_REFERENCE_PERIOD               NS2US(1800000000000ull)     // 30 min
#define TILT_REFERENCE_BACKOFF              NS2US(300000000000ull)      // 5 min
#define MIN_ACCEL_INTERVAL                  NS2US(20000000ull)          // 20 ms for 50 Hz

struct FacingTask {
    uint64_t tilt_reference_time;
    uint64_t accelerating_time;
    uint64_t predicted_facing_time;
    uint64_t swinging_time;

    int8_t tilt_history[TILT_HISTORY_SIZE];
    uint32_t tilt_history_time[TILT_HISTORY_SIZE];
    int tilt_history_index;

    uint64_t last_filtered_time;
    float last_filtered_sample[3];

    int8_t current_facing;
    int8_t prev_valid_facing;
    int8_t proposed_facing;
    int8_t predicted_facing;
    uint8_t swinging;
    uint8_t accelerating;
    uint8_t overhead;
};

int *get_flip_result_ptr(uint8_t);
void flip_original_reinit();
void flip_alg_enter_point();
void flip_get_alg_result(int*);
void flip_set_ts(uint32_t input_ts);

#endif
