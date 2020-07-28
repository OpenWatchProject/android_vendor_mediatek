/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER\'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER\'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM. RECEIVER\'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK\'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE, AT MEDIATEK\'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE
 * AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
 * RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software") have been modified by MediaTek Inc. All revisions are
 * subject to any receiver\'s applicable license agreements with MediaTek Inc.
 */

#ifndef __PEDOMETER_UTIL_H__
#define __PEDOMETER_UTIL_H__

#include "pedometer_private.h"

uint32_t max(uint32_t a, uint32_t b);
uint32_t min(uint32_t a, uint32_t b);
void cut_to_4g(int32_t *);
int32_t te_divide32(int32_t, int32_t);

pedo_ax_info_t* best_walk_ax();
pedo_ax_info_t* best_run_ax();
pedo_ax_info_t* best_best_ax();
int8_t best_run_ax_num();
uint8_t best_ax_num(int);
int8_t most_stable_ax(int);
int8_t two_ax_consistency(uint32_t, uint32_t, float);
bool four_ax_consistency(pedo_ax_info_t *, float);
uint32_t smallest_of_8_axes();

uint32_t get_current_count();
uint32_t get_step_count_without_buffer();
void clear_one_axis_count(pedo_ax_info_t*);
void clear_all_axes_counts(erasing_reason_t);
void reinit_counters();

int32_t* s_ptr(int, int, int);
int32_t moving_avg(int32_t);
void update_smoothing_idx();
void input_signal_smoothing(int32_t, uint8_t);
void update_interval_buffer(uint16_t, int);
void update_feature_vector(uint32_t, int32_t, int32_t*, int32_t*);

#ifdef _PEDOMETER_PC_
uint32_t ax_count(int, int);
void feature_to_arr(pedo_feature_t*, int*, int*);
void pedometer_update_record();   // if step changes, lib will call this function to write instant information of algorithm
#endif

#endif