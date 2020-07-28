/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef ANDROID_PERFSERVICE_TYPES_H
#define ANDROID_PERFSERVICE_TYPES_H

#define PACK_NAME_MAX   128
#define CLASS_NAME_MAX  128
#define CLUSTER_MAX     8
#define COMM_NAME_SIZE  64
#define FIELD_SIZE      64


/* Scenarios, MUST align with PerfService.java */
enum {
    STATE_PAUSED    = 0,
    STATE_RESUMED   = 1,
    STATE_DESTORYED = 2,
    STATE_DEAD      = 3,
    STATE_STOPPED   = 4,
    STATE_NUM       = 5,
};

typedef struct tScnNode{
    int  handle_idx;
    int  scn_type;
    int  scn_state;
    char pack_name[PACK_NAME_MAX];
    char act_name[CLASS_NAME_MAX];
    int  scn_core_total;
    int  scn_core_min[CLUSTER_MAX];
    int  scn_core_max[CLUSTER_MAX];
    int  scn_freq_min[CLUSTER_MAX];
    int  scn_freq_max[CLUSTER_MAX];
    int  scn_gpu_freq;
    int  scn_gpu_freq_max;    // upper bound
    int  hint_hold_time;
    int  ext_hint;
    int  ext_hint_hold_time;
    int  launch_time_cold;
    int  launch_time_warm;
    int  screen_off_action;
    int  scn_valid;  // valid of pre-defined scenario
    int  scn_app_crash;  // valid of pre-defined scenario
    int  pid;
    int  tid;
    int  scn_rsc[FIELD_SIZE];
    int  scn_prev_rsc[FIELD_SIZE];
    int  scn_param[FIELD_SIZE];
    int  scn_freq_hard_min[CLUSTER_MAX];
    int  scn_freq_hard_max[CLUSTER_MAX];
    char comm[COMM_NAME_SIZE];
    int  lock_duration;
    int  lock_rsc_size;
    int  *lock_rsc_list;
}tScnNode;

enum {
    SCREEN_OFF_DISABLE      = 0,
    SCREEN_OFF_ENABLE       = 1,
    SCREEN_OFF_WAIT_RESTORE = 2,
};

enum {
    DISPLAY_TYPE_GAME   = 0,
    DISPLAY_TYPE_OTHERS = 1,
    DISPLAY_TYPE_NO_TOUCH_BOOST = 2,
};

enum {
    NOTIFY_USER_TYPE_PID = 0,
    NOTIFY_USER_TYPE_FRAME_UPDATE = 1,
    NOTIFY_USER_TYPE_DISPLAY_TYPE = 2,
    NOTIFY_USER_TYPE_SCENARIO_ON  = 3,
    NOTIFY_USER_TYPE_SCENARIO_OFF = 4,
    NOTIFY_USER_TYPE_CORE_ONLINE  = 5,
    NOTIFY_USER_TYPE_PERF_MODE    = 6,
    NOTIFY_USER_TYPE_OTHERS       = 7,
    NOTIFY_USER_TYPE_DETECT       = 8,
};

enum {
    PERF_MODE_IGNORE = -1,
    PERF_MODE_NORMAL = 0,
    PERF_MODE_SPORTS = 1,
    PERF_MODE_USER_SETTING = 2,
};

enum {
    PPM_MODE_LOW_POWER   = 0,
    PPM_MODE_JUST_MAKE   = 1,
    PPM_MODE_PERFORMANCE = 2,
    PPM_MODE_NUM         = 3,
};

enum {
    DCM_MODE_NORMAL   = 0,
    DCM_MODE_SPORTS   = 1,
    DCM_MODE_NUM      = 2,
};

enum {
    DCS_2_CHANNEL = 0,
    DCS_4_CHANNEL = 1,
    DCS_MODE_NUM  = 2,
};

enum {
    PPM_HICA_VAR_SPORTS = 0,
    PPM_HICA_VAR_NORMAL = 1,
    PPM_HICA_VAR_NUM    = 2,
};

enum {
    DFPS_MODE_DEFAULT = 0,
    DFPS_MODE_FRR,
    DFPS_MODE_ARR,
    DFPS_MODE_INTERNAL_SW,
    DFPS_MODE_MAXIMUM,
};

#endif // ANDROID_PERFSERVICE_H

