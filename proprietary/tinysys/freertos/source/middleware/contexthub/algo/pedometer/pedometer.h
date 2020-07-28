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

#ifndef __PEDOMETER_H__
#define __PEDOMETER_H__

//#define _PEDOMETER_PC_
#ifdef _PEDOMETER_PC_
#include "../../../../src/intrinsic_pc_lib/unit_tester.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef DEVICE_IS_WATCH
#define PEDOMETER_INPUT_SAMPLE_INTERVAL  16  //ms, 62.5Hz for wearable device
#else
#define PEDOMETER_INPUT_SAMPLE_INTERVAL  20  //ms, 50.0Hz for mobile
#endif

//******************************************************************************************
#define V3_DT_ON // V3 pedometer noise filtering, if commented, it is using V4 filter merely
//******************************************************************************************

/*
 *  Whitney pedometer step counter reporting setting, packaged by Tingen Tseng
 *
 *  Our pedometer is designed to dynamically adapt to all types of user behavior (pattern/placement),
 *  it will sometimes suffer from step roll-back issue which may jeopardize user's using experience.
 *  Thus, in order to overcome this issue, an alternative reporting interval is adopted to original step counter.
 *  Our customer can change desired reporting frequency, no reporting interval implys our pedometer will report step
 *  counts frequently, but it's more likely to face step roll-back phenomenon.
 *
 *  How to adjust MTK's pedometer to highest reporting rate?
 *  A: Change PEDOMETER_REPORT_INTERVAL to 0
 */
#define PEDOMETER_REPORT_INTERVAL            60   //sec, how long shall step counter report current step

typedef enum pedometer_trans {
    ANY_TO_NO,
    NO_TO_ANY,
    DISABLE,
} pedometer_trans_t;

typedef enum pedometer_stat {
    NOT_COUNTING,
    COUNTING,
    REPORT_STEP,
    TIMEOUT_NOW,
    ADD_TO_GLOBAL,
} pedometer_stat_t;

// basic pedometer function:
// 1) set_pedometer_t_s:
//      input: current pedometer time stamp (unit: ms)
// 2) pedometer_algorithm_int():
//      initialization function
// 3) pedometer_update_acc(x, y, z):
//      after setting time stamp, feed accelerometer raw data to lib (unit: 10^-3 m/s^2)
void set_pedometer_t_s(uint32_t);
void set_pedometer_report_interval(uint32_t);
void set_pedometer_transition(pedometer_trans_t);
void pedometer_algorithm_init(void);
void pedometer_update_acc(int32_t, int32_t, int32_t);
void pedometer_forcing_time_out(void);

float get_pedometer_step_freq(void);
uint8_t get_pedometer_smd_event(void);
uint8_t get_pedometer_step_detector(void);
uint32_t get_pedometer_step_count(void);
pedometer_stat_t get_pedometer_status(void);
uint32_t step_count_for_floor_counting(void);
#endif