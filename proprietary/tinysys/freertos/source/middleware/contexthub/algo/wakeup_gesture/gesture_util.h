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

#ifndef __LIFT_UTIL_H__
#define __LIFT_UTIL_H__

#include "gesture_setting.h"

gesture_acc_buf_t *get_gesture_buf_ptr();
void write_normal_log(char*);
bool out_of_range(int32_t, int32_t, int32_t);
void clone_arr(int32_t*, int32_t*);
void clear_arr(int32_t*);
bool not_initialized(int32_t*);
void gesture_acc_buffer_init(uint32_t, uint32_t, bool*);
void reinit_orientation(gesture_stored_t*);
void saturate_raw_acc(gesture_input_t*, int);

int32_t gesture_divide32(int32_t, int32_t);
int64_t gesture_divide64(int64_t, int64_t);
int32_t angle_cos_value(int32_t*, int32_t*);
int32_t x_neglected_angle_cos_value(int32_t*, int32_t*);
void count_consecutive_cond(bool, int32_t*);
int above_rise_time(int32_t*, int32_t, int32_t);

int* get_avg(gesture_acc_buf_t*, int);
int* get_var(gesture_acc_buf_t*, int);
int32_t* get_rise(gesture_acc_buf_t*, int, int32_t*);
int32_t* get_drop(gesture_acc_buf_t*, int, int32_t*);
uint64_t get_norm_sq(int32_t*);
uint32_t get_max_norm(gesture_acc_buf_t*, int);
void get_avg_and_max_norm(int32_t*, uint32_t*, int, gesture_acc_buf_t*);
uint32_t acc_norm_sqr_shift_15(int32_t, int32_t, int32_t);

bool flip_abandon_ref(int32_t*);
bool flip_match_visable_range(int32_t, int32_t, int32_t);
bool put_static_cond();
bool lift_static(int32_t*, int32_t*);
bool lift_abandon_ref(int32_t*);
bool lift_freefall_detected();

bool match_lift_head_angle(int32_t, int32_t, int32_t);
bool match_lift_desk_pitch(int32_t, int32_t, int32_t);
bool match_lift_dang_pitch(int32_t, int32_t, int32_t);
bool match_azimuth_lift(int32_t*, int32_t*);
bool match_pure_horizontal_motion(int32_t);

#endif