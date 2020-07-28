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

#ifndef ANDROID_PERFSERVICE_RSCCFGTBL_H
#define ANDROID_PERFSERVICE_RSCCFGTBL_H

#include <string>
using namespace std;

#include "utility_consys.h"
#include "utility_thermal.h"
#include "utility_fps.h"
#include "utility_vpu.h"
#include "utility_ux.h"
#include "utility_netd.h"
#include "utility_ril.h"
#include "utility_io.h"
#include "utility_sys.h"

tRscConfig RscCfgTbl[] = {
    /* test 1 */
    {
        /* cmd */
        .cmdID                = PERF_RES_THERMAL_POLICY,
        .cmdName              = "PERF_RES_THERMAL_POLICY",
        .comp                 = ONESHOT,
        .maxVal               = 19,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = load_thm_api_start,
        .unset_func           = load_thm_api_stop,
        .init_func            = reset_thermal_policy,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_NET_WIFI_LOW_LATENCY,
        .cmdName              = "PERF_RES_NET_WIFI_LOW_LATENCY",
        .comp                 = BIGGEST,
        .maxVal               = 1,
        .minVal               = 0,
        .defaultVal           = 0,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = wifi_low_latency,
        .unset_func           = wifi_low_latency,
        .init_func            = wifi_reset,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_NET_WIFI_SMART_PREDICT,
        .cmdName              = "PERF_RES_NET_WIFI_SMART_PREDICT",
        .comp                 = BIGGEST,
        .maxVal               = 1,
        .minVal               = 0,
        .defaultVal           = 0,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = wifi_smart_predict,
        .unset_func           = wifi_smart_predict,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_NET_NETD_BOOST_UID,
        .cmdName              = "PERF_RES_NET_NETD_BOOST_UID",
        .comp                 = ONESHOT,
        .maxVal               = 500000, /* max uid */
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = netd_set_priority_uid,
        .unset_func           = netd_clear_priority_uid,
        .init_func            = netd_reset,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_FPS_FSTB_FPS_UPPER,
        .cmdName              = "PERF_RES_FPS_FSTB_FPS_UPPER",
        .comp                 = BIGGEST,
        .maxVal               = 60,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setFstbFpsHigh,
        .unset_func           = setFstbFpsHigh,
        .init_func            = fstb_init,
        },

    {
        /* cmd */
        .cmdID                = PERF_RES_FPS_FSTB_FPS_LOWER,
        .cmdName              = "PERF_RES_FPS_FSTB_FPS_LOWER",
        .comp                 = BIGGEST,
        .maxVal               = 60,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setFstbFpsLow,
        .unset_func           = setFstbFpsLow,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_FPS_FSTB_SOFT_FPS_UPPER,
        .cmdName              = "PERF_RES_FPS_FSTB_SOFT_FPS_UPPER",
        .comp                 = BIGGEST,
        .maxVal               = 60,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setFstbSoftFpsHigh,
        .unset_func           = setFstbSoftFpsHigh,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_FPS_FSTB_SOFT_FPS_LOWER,
        .cmdName              = "PERF_RES_FPS_FSTB_SOFT_FPS_LOWER",
        .comp                 = BIGGEST,
        .maxVal               = 60,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setFstbSoftFpsLow,
        .unset_func           = setFstbSoftFpsLow,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_AI_VPU_FREQ_MIN_CORE_0,
        .cmdName              = "PERF_RES_AI_VPU_FREQ_MIN_CORE_0",
        .comp                 = BIGGEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setVpuFreqMin_core_0,
        .unset_func           = setVpuFreqMin_core_0,
        .init_func            = vpu_init,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_AI_VPU_FREQ_MIN_CORE_1,
        .cmdName              = "PERF_RES_AI_VPU_FREQ_MIN_CORE_1",
        .comp                 = BIGGEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setVpuFreqMin_core_1,
        .unset_func           = setVpuFreqMin_core_1,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_AI_VPU_FREQ_MAX_CORE_0,
        .cmdName              = "PERF_RES_AI_VPU_FREQ_MAX_CORE_0",
        .comp                 = SMALLEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setVpuFreqMax_core_0,
        .unset_func           = setVpuFreqMax_core_0,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_AI_VPU_FREQ_MAX_CORE_1,
        .cmdName              = "PERF_RES_AI_VPU_FREQ_MAX_CORE_1",
        .comp                 = SMALLEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setVpuFreqMax_core_1,
        .unset_func           = setVpuFreqMax_core_1,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_AI_MDLA_FREQ_MIN,
        .cmdName              = "PERF_RES_AI_MDLA_FREQ_MIN",
        .comp                 = BIGGEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setMdlaFreqMin,
        .unset_func           = setMdlaFreqMin,
        .init_func            = mdla_init,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_AI_MDLA_FREQ_MAX,
        .cmdName              = "PERF_RES_AI_MDLA_FREQ_MAX",
        .comp                 = SMALLEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setMdlaFreqMax,
        .unset_func           = setMdlaFreqMax,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_UX_PREDICT_LOW_LATENCY,
        .cmdName              = "PERF_RES_UX_PREDICT_LOW_LATENCY",
        .comp                 = BIGGEST,
        .maxVal               = 1,
        .minVal               = 0,
        .defaultVal           = 0,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = notifyLowLatencyMode,
        .unset_func           = notifyLowLatencyMode,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_NET_MD_LOW_LATENCY,
        .cmdName              = "PERF_RES_NET_MD_LOW_LATENCY",
        .comp                 = BIGGEST,
        .maxVal               = 1,
        .minVal               = 0,
        .defaultVal           = 0,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 1,
        .set_func             = notify_rild_opt_update,
        .unset_func           = notify_rild_opt_update,
        .init_func            = reset_rild_opt_info,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_NET_MD_CERT_PID,
        .cmdName              = "PERF_RES_NET_MD_CERT_PID",
        .comp                 = ONESHOT,
        .maxVal               = 500000,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = notify_rild_cert_pid_set,
        .unset_func           = notify_rild_cert_pid_reset,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_NET_MD_CRASH_PID,
        .cmdName              = "PERF_RES_NET_MD_CRASH_PID",
        .comp                 = ONESHOT,
        .maxVal               = 500000,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = notify_rild_crash_pid_set,
        .unset_func           = notify_rild_crash_pid_reset,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_CFP_UP_LOADING,
        .cmdName              = "PERF_RES_CFP_UP_LOADING",
        .comp                 = SMALLEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setCfpUpLoading,
        .unset_func           = setCfpUpLoading,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_CFP_DOWN_LOADING,
        .cmdName              = "PERF_RES_CFP_DOWN_LOADING",
        .comp                 = SMALLEST,
        .maxVal               = 100,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setCfpDownLoading,
        .unset_func           = setCfpDownLoading,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_IO_BLKDEV_READAHEAD,
        .cmdName              = "PERF_RES_IO_BLKDEV_READAHEAD",
        .comp                 = BIGGEST,
        .maxVal               = 1024,
        .minVal               = 0,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setBlkDev_readAhead,
        .unset_func           = setBlkDev_readAhead,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_NET_MD_WEAK_SIG_OPT,
        .cmdName              = "PERF_RES_NET_MD_WEAK_SIG_OPT",
        .comp                 = BIGGEST,
        .maxVal               = 1,
        .minVal               = 0,
        .defaultVal           = 0,
        .normalVal            = 0,
        .sportVal             = 0,
        .force_update         = 0,
        .set_func             = notify_rild_weak_sig_opt_set,
        .unset_func           = notify_rild_weak_sig_opt_set,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_SCHED_CPU_PREFER_TASK_1_BIG,
        .cmdName              = "PERF_RES_SCHED_CPU_PREFER_TASK_1_BIG",
        .comp                 = ONESHOT,
        .maxVal               = 536870912, // max pid
        .minVal               = 1,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setTaskPreferCpu_big,
        .unset_func           = unsetTaskPreferCpu_big,
        .init_func            = initTaskPreferCpu,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_SCHED_CPU_PREFER_TASK_1_LITTLE,
        .cmdName              = "PERF_RES_SCHED_CPU_PREFER_TASK_1_LITTLE",
        .comp                 = ONESHOT,
        .maxVal               = 536870912, // max pid
        .minVal               = 1,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setTaskPreferCpu_little,
        .unset_func           = unsetTaskPreferCpu_little,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_SCHED_CPU_PREFER_TASK_2_BIG,
        .cmdName              = "PERF_RES_SCHED_CPU_PREFER_TASK_2_BIG",
        .comp                 = ONESHOT,
        .maxVal               = 536870912, // max pid
        .minVal               = 1,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setTaskPreferCpu_big,
        .unset_func           = unsetTaskPreferCpu_big,
        .init_func            = NULL,
    },

    {
        /* cmd */
        .cmdID                = PERF_RES_SCHED_CPU_PREFER_TASK_2_LITTLE,
        .cmdName              = "PERF_RES_SCHED_CPU_PREFER_TASK_2_LITTLE",
        .comp                 = ONESHOT,
        .maxVal               = 536870912, // max pid
        .minVal               = 1,
        .defaultVal           = -1,
        .normalVal            = -1,
        .sportVal             = -1,
        .force_update         = 0,
        .set_func             = setTaskPreferCpu_little,
        .unset_func           = unsetTaskPreferCpu_little,
        .init_func            = NULL,
    },

};

#endif // ANDROID_PERFSERVICE_RSCFGTBL_H

