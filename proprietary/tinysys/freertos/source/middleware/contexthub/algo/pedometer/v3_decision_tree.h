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

#ifndef __V3_DECISION_TREE_H__
#define __V3_DECISION_TREE_H__

#include <stdint.h>

// #define DEVICE_IS_WATCH      //HC: If this is commented, it's Phone
#define THRESHOLD_NOISE_COUNTER    32  // 16*N, N=1,2,3..., the sample count for noise state
#define STATE_MACH_BUF_L           11  // HC state machine total vote, must be an odd number
#define V3_ACC_BUF_L               32  // 50Hz * 2s
#define V3_ACC_AXIS_N              4
#define V3_FEATURE_NUM             6
#define V3_FEAT_VEC_L              V3_ACC_AXIS_N * V3_FEATURE_NUM  // user definition (temporary)
#define MIN_SAMPLES                64  // queue data for first time v3_predict_by_tree

// 2016/9/21: data size: 0.10kB (TE)
typedef struct {
    int32_t mean;
    uint8_t divide_bits;
    uint8_t reader_idx;
    uint8_t writer_idx;
    int32_t data[V3_FEAT_VEC_L];
} v3_feature_t;

// 2016/9/21: data size: 0.50kB (TE), could be decrease to 0.25kB
typedef struct {
    uint16_t writer_idx;
    uint16_t reader_idx;
    int32_t data[V3_ACC_AXIS_N][V3_ACC_BUF_L];
} v3_acc_buf_t;

typedef struct {
    const uint8_t *node_dist;
    const uint8_t *node_feature;
    const int16_t *node_theshold;
} v3_tree_t;

typedef struct {
    uint8_t noise_flag;
    int8_t noise_flag_counter;
    int8_t valid_step_prediction;
    uint8_t state_mach_idx;
    uint8_t predict_counter;
    uint8_t result_label;
    int8_t step_detection_queue[STATE_MACH_BUF_L];
    v3_tree_t tree;
    v3_acc_buf_t buffer;
    v3_feature_t feature;
} v3_noise_detector_t;

int8_t v3_predict_noise(int32_t*);
uint8_t v3_noise_detected(int32_t*);
uint8_t v3_noise_majority_vote(int8_t*);
uint8_t v3_predict_by_tree(v3_feature_t*, v3_tree_t*);
void v3_noise_detection_init();
void v3_update_buffer(int32_t*);
void v3_feature_extraction(v3_acc_buf_t*, v3_feature_t*);

// util functions
void v3_sort(int32_t*, uint8_t);
uint8_t next_idx(uint8_t, uint8_t);
int32_t pedometer_dsp_norm(int32_t, int32_t, int32_t);
int32_t pedometer_divide32(int32_t, int32_t);

#endif