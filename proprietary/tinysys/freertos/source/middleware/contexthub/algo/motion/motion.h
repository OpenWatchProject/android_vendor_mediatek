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

#ifndef __MOTION_H__
#define __MOTION_H__

#define MOTION_BUF              15
#define MOTION_GRAVITY_NORM     9806 * 9806     // 9.8 * 10^3

enum motion_state {
    INIT_STATE,
    ANY_MOTION_STATE,
    NO_MOTION_STATE,
    UNCERTAIN_STATE
};

typedef struct motion_stored {
    uint32_t current_ts;                        // last set_data ts
    uint32_t prev_ts;                           // previous set_data ts
    uint32_t ref_ts;                            // last non-motion or motion ts

    int32_t acc_x[MOTION_BUF];
    int32_t acc_y[MOTION_BUF];
    int32_t acc_z[MOTION_BUF];
    uint8_t curr_index;

    uint8_t state;
    uint8_t prev_state;
    uint8_t first_state;
    uint8_t is_any_motion;
} motion_stored_t;

typedef struct motion_parameter {
    int32_t var_thre;
    uint32_t time_to_any;
    uint32_t time_to_no;
    int32_t g_sec;
    int32_t timestamp_large_gap;
} motion_parameter_t;

/* motion functions */
void motion_set_ts(uint32_t input_ts);
int get_motion_notify();
void motion_stored_init();
void motion_parameter_init(int32_t var_thre, uint32_t time_to_any, uint32_t time_to_no, int32_t g_sec,
                           int32_t timestamp_large_gap);
void motion_set_acc(int32_t ax, int32_t ay, int32_t az);
void motion_run_algo();
void motion_change_state();

#endif
