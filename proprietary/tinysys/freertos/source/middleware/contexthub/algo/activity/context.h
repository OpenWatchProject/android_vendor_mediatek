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
#ifndef __CONTEXT_H
#define __CONTEXT_H

#include "activity_algorithm.h"
#include "context_setting.h"

#define CONTEXT_SMOOTH_TIMELENGTH  10000 //ms
#define DOMINATE_CONFIDENCE        100
#define ACT_TIME_SEC_TO_NANO           1000000000ull
#define LOCK_VEHICLE_THRESHOLD 15 * ACT_TIME_SEC_TO_NANO
#define SUPPRESS_STILL_COUNT 60 * ACT_TIME_SEC_TO_NANO
#define SUPPRESS_STILL_CONFIDENCE 90
#define SUPPRESS_STILL_VEHICLE_CONFIDENCE_DIFF 40
#define SUPPRESS_LIMIT 90 * ACT_TIME_SEC_TO_NANO
#define CONFIDENT_ACTIVITY_THRESHOLD 10
#define RUNNING_FAVOR_RATIO 0.2
#define SIGNIFICANT_COMFIDENCE_IN_VEHICLE 20
#define SIGNIFICANT_COMFIDENCE_DRIVING 80

typedef enum {
    AR_STILL,
    AR_STANDING,
    AR_SITTING,
    AR_LYING,
    AR_ON_FOOT,
    AR_WALKING,
    AR_RUNNING,
    AR_CLIMBING,
    AR_ON_BICYCLE,
    AR_IN_VEHICLE,
    AR_TILTING,
    AR_UNKNOWN,
    AR_ACTIVITY_MAX
} _activity_type_t_;

typedef enum {
    TILT_INIT_STATE,
    TILT_TRIGGERED,
    TILT_HOLD,
    UNKNOWN_HOLD,
} _activity_tilt_state_;

typedef struct {
    uint8_t probability[AR_ACTIVITY_MAX];    // 0~100
} _activity_t_;

/* Context Awareness functions */
void activity_pararmeter_init();
void context_alg_enter_point();
void context_alg_predict();
void context_get_alg_result(Activity_algorithm_output_t* output, uint64_t min_update_interval);
int max3(int a, int b, int c);
void act_prob_to_state(Activity_algorithm_output_t *input_prob, _activity_type_t_ *output_act_state);

#endif

