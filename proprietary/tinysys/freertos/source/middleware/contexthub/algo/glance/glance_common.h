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

// 1. readability
// 2. How to optimize the parameters?
// 3. TE_LOG and log_ptr debug
//
// Algorithm point of view
// 1. More indicator? (ex: Grav range)
// 2. Face down 15 degree
// 3. Track the pitch angle (difference, glance/putting down)
// 4. Bed range should divided by norm
//
// Current CR list:
// ALPS03614915: [CV5A][G.R]It will trigger event when tilt up the device slowly with screen up.(5/5)
//   Status: expected to solved in 1117 FW update, solved by add LAST_BIG_MOTION_TS_DIFF = 2s
// ALPS03614951: [CV5A][G.R]It will keep triggering event when device keep shift moving 45 degree pitch angle lien with screen up.(5/5)
//   Status: expected to solved in 1117 FW update by adding BIG_MOTION_Z_VAR
// ALPS03615834: [CV5A][G.R]It will keep triggering event when put the device in 3~5 pitch degree angle on the desk with screen down.(5/5)
//   Status: expected to solved in 1117 FW update

#ifndef __GLACNE_COMMON_SEP_H__
#define __GLANCE_COMMON_SEP_H__

#include <stdint.h>
#include "glance_setting.h"
#include "glance_util.h"

extern customer_gesture_config_t glance_config;

void glance_common_set_ts(glance_stored_t*, uint32_t);
uint32_t glance_common_get_ts(glance_stored_t*);

gesture_acc_buf_t *get_glance_buf_ptr();
void feed_acc_into_glance(gesture_input_t);
void fill_all_glance_buffer(gesture_input_t, glance_stored_t*);
bool normal_glance_detected(int32_t*);
bool bed_glance_detected(int32_t*);

bool match_glance_visable_range(int32_t, int32_t, int32_t);
bool match_semi_glance_visable_range(int32_t, int32_t, int32_t);
bool glance_orient_is_ready(glance_stored_t*);
void update_glance_orient(glance_stored_t *tilt_ptr, gesture_acc_buf_t *acc_ptr);

#endif
