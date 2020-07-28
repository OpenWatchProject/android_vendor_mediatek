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

#ifndef __PEDOMETER_PRIVATE_H__
#define __PEDOMETER_PRIVATE_H__

#include "pedometer_constant.h"
#include "v3_decision_tree.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef enum {
    WALK,
    RUN,
    SET_N,
} para_set_t;

typedef enum {
    INITIALIZED,
    SHORT_PERIOD,
    GOOD_DURATION,
} erasing_reason_t;

typedef struct pedo_smoother {
    int8_t n;                        // Represent amount of acc instance. If acc instance is enough, n = -1
    uint8_t idx;                     // indicate the current idx of acc
    int32_t acc[BUF_L][BUF_N][AX_N]; // (5 buffer),(LP/MA/HP/original),(x/y/z/n)
} pedo_smoother_t;

typedef struct step {
    uint32_t time;
    int32_t magnitude;
} step_t;

typedef struct pedo_parameter {
    int8_t finding_range;
    int32_t min_interval;
    int32_t min_peak_mag;
    const para_set_t num;
} pedo_parameter_t;

typedef struct pedo_feature {
    uint16_t interval;       // step interval
    int32_t mmean, mvar;     // peak magnitude
    int32_t tmean, tvar;     // peak-to-peak interval
    int32_t dtmean, dtvar;   // delta-t
} pedo_feature_t;

typedef struct pedo_ax_info {
    bool local_max_existed;
    int8_t finding_count;
    uint16_t duration;
    uint32_t count;
    uint32_t stored_count;
    step_t peak, last_step;
    pedo_feature_t f;
} pedo_ax_info_t;

pedo_smoother_t sm;
step_t local_max[SET_N][AX_N];
pedo_ax_info_t worker[SET_N][AX_N];
uint16_t interval_buf[SET_N][INTVL_L];
erasing_reason_t count_erasing_reason;

bool v3_nonfoot_flag;           // nonfoot is detected by v3 DCT
bool force_timeout_flag;        // the abnormal acc input interval leads re-init to algorithm
bool write_log_flag;
bool acc_smooth_ready;          // input acc instance num is enough
bool run_detected;              // used for parameter selection
bool smd_detected;              // smd detected, new walking/running duration
bool smd_reported;              // ready to report smd event, this parameter will be reset if smd event is triggered and delivered

uint32_t ts;                    // current time stamp of pedometer, use set_pedometer_t_s() to set
uint32_t last_step_ts;          // last time step count added, record for detect time-out
uint32_t processing_count;      // temporary count counter is counting
uint32_t global_count;          // reliable step counts that has been accumulated.
uint32_t step_detected_count;   // how many steps aren't reported by step detector event
uint32_t latest_report_count;   // last time pedometer report step counts, see PEDOMETER_REPORT_INTERVAL and latest_report_ts
uint32_t max_historical_count;  // to alleviate traceback steps caused by axis change, add this parameter to assure strictly ascending
uint32_t reporting_interval;    // how long pedometer will change step result
uint32_t latest_report_ts;      // last time pedometer report step counts change, see PEDOMETER_REPORT_INTERVAL for detail

#ifdef _PEDOMETER_PC_
uint16_t ut_file_idx;
int count_arr[8], feature[18], res_feature[4];
void set_main_file_idx(uint32_t);
void pedometer_record_file_result(
    int);  // accumulate final results of different axes of pedometer and export this to unit test
#endif

#endif