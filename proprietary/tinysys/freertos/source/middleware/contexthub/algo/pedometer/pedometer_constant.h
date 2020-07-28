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
#ifndef __PEDOMETER_CONSTANT_H__
#define __PEDOMETER_CONSTANT_H__

#include "pedometer.h"
#ifndef M_PI
#define M_PI               3.14159265358979323846
#endif

//#define SET_ADAPTOR_103    1    //(1)HC debug 097:   File*0.97 and then Adaptor*1.03 in pedometer_adaptor.c line 47
#define DIVIDE5_NUMTOR     410    // 410 / 2048 ~ 0.2
#define DIVIDE_2048        11     // 410 / 2048 ~ 0.2

#define AX_N               4      // total parameter set number
#define BUF_N              4      // total number of acc smoothing buffer
#define BUF_L              5      // length of acc buffer
#define INTVL_L            8      // step interval buffer length (use for calculating freq)
#define ENOUGH_COUNT       10     // minimum steps that represents meaningful duartion
#define GOOD_STEP_LAG      20     // when sensor encounter time lag exceed than 400 ms, if temp step exceed this value, add temp to global
#define STEP_DETECTOR_T    2000   // if last step is 2 second close than current ts, report detector = 1
#define STEP_DETECTOR_C    1      // minimum steps that can trigger step detector
#define SMD_MIN_C          8      // minimum steps that can trigger smd

#define ACC_LAG_T          200    // no acc larger than 0.2 sec, need re-init
#define MIN_INTERVAL       2000   // if new step exceed this time, counter will see as a new duration
#define TIMEOUT_T          2000   // HC May tune this (?) to Solve WalkSlow issue, but cause side effects, leave it 2016/5/31
#define MIN_DURATION       8000   // the required minimum duration

#define FIRST_MIN_MAG      5000
#define ACC_CUTOFF_V       38259  // saturate acc to 4g. TE 2016/9/21 test int16_t possibility
#define RUN_MUL            1.15f  // utilized in exception 5 if run_norm doesn't count larger than other axes
#define RUN_CONS_PORTION   0.02f  // running axes consistency, if this parameter equal to 0.02 => 198, 250, 200, 100 is consistent
#define WALK_CONS_PORTION  0.03f  // walking axes consistency, if this parameter equal to 0.03 => 198, 202, 200, 100 is consistent

#endif