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

#ifndef __STATIONARY_H
#define __STATIONARY_H

#define STATIONARY_INPUT_ACCUMULATE         160     // unit: ms

#define STATIONARY_SLIDING_WINDOW           64
#define STATIONARY_SENSOR_MULTIPLE_TIMES    100000

#define STATIONARY_OVER_VARIANCE_RATIO      30      // 0.3 * 100 (percentage)

#define STATIONARY_STATE_INIT               -1
#define STATIONARY_STATE_NONSTATIONARY      0
#define STATIONARY_STATE_STATIONARY         1
#define STATIONARY_STATE_STOP               2

typedef struct stationary_stored {
    uint32_t current_ts;                        // last set_data ts
    uint32_t ref_ts;                            // last non-stationary or stationary ts
    int state;
    int previous_state;
    int last_on_change_result;                  // last non-stationary or stationary result
} stationary_stored_t;

typedef struct stationary_feature {
    int32_t acc_var_norm;
    int32_t ratio;                                // stationary_over_variance_ratio
    int32_t acc_mean_z;
} stationary_feature_t;

typedef struct stationary_paremeter {
    int32_t variance_threshold;
    int32_t time_to_stationary;
    int32_t time_to_nonstationary;
    int32_t gravity_section;
} stationary_paremeter_t;

/* Stationary functions */
void stationary_set_ts(uint32_t input_ts);
int get_stationary_result();
int get_stationary_notify();
int get_device_flat();
void stationary_original_init(int32_t variance_threshold, int32_t time_to_stationary, int32_t time_to_nonstationary,
                              int32_t gravity_section);
void stationary_original_stop();
void update_var_and_ratio(AccSignals_t* ac_ptr, int hist_data_leng, int threshold);
int cal_current_state();
void stationary_alg_enter_point();

#endif
