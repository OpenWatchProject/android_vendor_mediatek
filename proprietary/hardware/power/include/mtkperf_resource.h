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

/*
 * command table
 */

enum {
    /* MAJOR = 1 CPU freq*/
    PERF_RES_CPUFREQ_MIN_CLUSTER_0                   = 0x00400000,
    PERF_RES_CPUFREQ_MIN_CLUSTER_1                   = 0x00400100,
    PERF_RES_CPUFREQ_MAX_CLUSTER_0                   = 0x00404000,
    PERF_RES_CPUFREQ_MAX_CLUSTER_1                   = 0x00404100,
    PERF_RES_CPUFREQ_MIN_HL_CLUSTER_0                = 0x00408000,
    PERF_RES_CPUFREQ_MIN_HL_CLUSTER_1                = 0x00408100,
    PERF_RES_CPUFREQ_MAX_HL_CLUSTER_0                = 0x0040c000,
    PERF_RES_CPUFREQ_MAX_HL_CLUSTER_1                = 0x0040c100,
    PERF_RES_CPUFREQ_CCI_FREQ                        = 0x00410000,
    PERF_RES_CPUFREQ_PERF_MODE                       = 0x00414000,

    /* MAJOR = 2 CPU core*/
    PERF_RES_CPUCORE_MIN_CLUSTER_0                   = 0x00800000,
    PERF_RES_CPUCORE_MIN_CLUSTER_1                   = 0x00800100,
    PERF_RES_CPUCORE_MAX_CLUSTER_0                   = 0x00804000,
    PERF_RES_CPUCORE_MAX_CLUSTER_1                   = 0x00804100,

    /* MAJOR = 3 GPU*/
    PERF_RES_GPU_FREQ_MIN                            = 0x00c00000,
    PERF_RES_GPU_FREQ_MAX                            = 0x00c04000,
    PERF_RES_GPU_FREQ_LOW_LATENCY                    = 0x00c08000,
    PERF_RES_GPU_GED_BENCHMARK_ON                    = 0x00c0c000,
    PERF_RES_GPU_GED_MARGIN_MODE                     = 0x00c0c100,
    PERF_RES_GPU_GED_TIMER_BASE_DVFS_MARGIN          = 0x00c0c200,
    PERF_RES_GPU_GED_LOADING_BASE_DVFS_STEP          = 0x00c0c300,
    PERF_RES_GPU_GED_CWAITG                          = 0x00c0c400,
    PERF_RES_GPU_GED_GX_BOOST                        = 0x00c0c500,

    /* MAJOR = 4 DRAM (VCORE, CM MGR, EMI)*/
    PERF_RES_DRAM_OPP_MIN                            = 0x01000000,
    PERF_RES_DRAM_VCORE_MIN                          = 0x01004000,
    PERF_RES_DRAM_VCORE_MIN_LP3                      = 0x01004100,
    PERF_RES_DRAM_VCORE_BW_ENABLE                    = 0x01008000,
    PERF_RES_DRAM_VCORE_BW_THRES                     = 0x01008100,
    PERF_RES_DRAM_VCORE_BW_THRESH_LP3                = 0x01008200,
    PERF_RES_DRAM_VCORE_POLICY                       = 0x0100c000,
    PERF_RES_DRAM_CM_MGR                             = 0x01010000,

    /* MAJOR = 5 Scheduler*/
    PERF_RES_SCHED_BOOST_VALUE_ROOT                  = 0x01400000,
    PERF_RES_SCHED_BOOST_VALUE_FG                    = 0x01400100,
    PERF_RES_SCHED_BOOST_VALUE_BG                    = 0x01400200,
    PERF_RES_SCHED_BOOST_VALUE_TA                    = 0x01400300,
    PERF_RES_SCHED_BOOST_VALUE_RT                    = 0x01400400,
    PERF_RES_SCHED_PREFER_IDLE_ROOT                  = 0x01404000,
    PERF_RES_SCHED_PREFER_IDLE_FG                    = 0x01404100,
    PERF_RES_SCHED_PREFER_IDLE_BG                    = 0x01404200,
    PERF_RES_SCHED_PREFER_IDLE_TA                    = 0x01404300,
    PERF_RES_SCHED_PREFER_IDLE_RT                    = 0x01404400,
    PERF_RES_SCHED_UCLAMP_MIN_ROOT                   = 0x01408000,
    PERF_RES_SCHED_UCLAMP_MIN_FG                     = 0x01408100,
    PERF_RES_SCHED_UCLAMP_MIN_BG                     = 0x01408200,
    PERF_RES_SCHED_UCLAMP_MIN_TA                     = 0x01408300,
    PERF_RES_SCHED_UCLAMP_MIN_RT                     = 0x01408400,
    PERF_RES_SCHED_TUNE_THRES                        = 0x0140c000,
    PERF_RES_SCHED_BOOST                             = 0x01410000,
    PERF_RES_SCHED_MIGRATE_COST                      = 0x01414000,
    PERF_RES_SCHED_CPU_PREFER_TASK_1_BIG             = 0x01418000,
    PERF_RES_SCHED_CPU_PREFER_TASK_1_LITTLE          = 0x01418100,
    PERF_RES_SCHED_CPU_PREFER_TASK_1_RESERVED        = 0x01418200,
    PERF_RES_SCHED_CPU_PREFER_TASK_2_BIG             = 0x01418300,
    PERF_RES_SCHED_CPU_PREFER_TASK_2_LITTLE          = 0x01418400,
    PERF_RES_SCHED_CPU_PREFER_TASK_2_RESERVED        = 0x01418500,
    PERF_RES_SCHED_BTASK_ROTATE                      = 0x0141c000,
    PERF_RES_SCHED_CACHE_AUDIT                       = 0x01420000,
    PERF_RES_SCHED_PLUS_DOWN_THROTTLE_NS             = 0x01424000,
    PERF_RES_SCHED_PLUS_UP_THROTTLE_NS               = 0x01424100,
    PERF_RES_SCHED_PLUS_SYNC_FLAG                    = 0x01424200,
    PERF_RES_SCHED_MTK_PREFER_IDLE                   = 0x01428000,
    PERF_RES_SCHED_HEAVY_TASK_THRES                  = 0x0142c000,
    PERF_RES_SCHED_HEAVY_TASK_AVG_HTASK_AC           = 0x0142c100,
    PERF_RES_SCHED_HEAVY_TASK_AVG_HTASK_THRES        = 0x0142c200,
    PERF_RES_SCHED_WALT                              = 0x01430000,

    /* MAJOR = 6 AI*/
    PERF_RES_AI_VPU_FREQ_MIN_CORE_0                  = 0x01800000,
    PERF_RES_AI_VPU_FREQ_MIN_CORE_1                  = 0x01800100,
    PERF_RES_AI_VPU_FREQ_MAX_CORE_0                  = 0x01804000,
    PERF_RES_AI_VPU_FREQ_MAX_CORE_1                  = 0x01804100,
    PERF_RES_AI_MDLA_FREQ_MIN                        = 0x01808000,
    PERF_RES_AI_MDLA_FREQ_MAX                        = 0x0180c000,

    /* MAJOR = 7 Low power policy */
    PERF_RES_POWER_CPUFREQ_HISPEED_FREQ              = 0x01c00000,
    PERF_RES_POWER_CPUFREQ_MIN_SAMPLE_TIME           = 0x01c04000,
    PERF_RES_POWER_CPUFREQ_ABOVE_HISPEED_DELAY       = 0x01c08000,
    PERF_RES_POWER_CPUFREQ_POWER_MODE                = 0x01c0c000,
    PERF_RES_POWER_HPS_THRESH_UP                     = 0x01c10000,
    PERF_RES_POWER_HPS_THRESH_DOWN                   = 0x01c10100,
    PERF_RES_POWER_HPS_TIMES_UP                      = 0x01c14000,
    PERF_RES_POWER_HPS_TIMES_DOWN                    = 0x01c14100,
    PERF_RES_POWER_HPS_RUSH_BOOST_ENABLE             = 0x01c18000,
    PERF_RES_POWER_HPS_RUSH_BOOST_THRESH             = 0x01c18100,
    PERF_RES_POWER_HPS_HEAVY_TASK                    = 0x01c1c000,
    PERF_RES_POWER_HPS_POWER_MODE                    = 0x01c20000,
    PERF_RES_POWER_PPM_ROOT_CLUSTER                  = 0x01c24000,
    PERF_RES_POWER_PPM_MODE                          = 0x01c28000,
    PERF_RES_POWER_PPM_HICA_VAR                      = 0x01c2c000,
    PERF_RES_POWER_PPM_LIMIT_BIG                     = 0x01c30000,
    PERF_RES_POWER_PPM_SPORTS_MODE                   = 0x01c34000,
    PERF_RES_POWER_PPM_USERLIMIT_BOOST               = 0x01c38000,

    /* MAJOR = 8 FPS policy (FPSGO) */
    PERF_RES_FPS_FSTB_FPS_LOWER                      = 0x02000000,
    PERF_RES_FPS_FSTB_FPS_UPPER                      = 0x02000100,
    PERF_RES_FPS_FPSGO_ENABLE                        = 0x02004000,
    PERF_RES_FPS_FSTB_FORCE_VAG                      = 0x02008000,
    PERF_RES_FPS_FBT_SHORT_RESCUE_NS                 = 0x0200c000,
    PERF_RES_FPS_FBT_MIN_RESCUE_PERCENT              = 0x0200c100,
    PERF_RES_FPS_FBT_DEQTIME_BOUND                   = 0x02010000,
    PERF_RES_FPS_FBT_FLOOR_BOUND                     = 0x02014000,
    PERF_RES_FPS_FBT_KMIN                            = 0x02018000,
    PERF_RES_FPS_FSTB_SOFT_FPS_LOWER                 = 0x0201c000,
    PERF_RES_FPS_FSTB_SOFT_FPS_UPPER                 = 0x0201c100,
    PERF_RES_FPS_FBT_BHR_OPP                         = 0x02020000,
    PERF_RES_FPS_FBT_BHR                             = 0x02024000,
    PERF_RES_FPS_EARA_BENCH                          = 0x02028000,
    PERF_RES_FPS_FPSGO_GPU_BLOCK_BOOST               = 0x0202c000,
    PERF_RES_FPS_FBT_BOOST_TA                        = 0x02030000,
    PERF_RES_FPS_FPSGO_MARGIN_MODE                   = 0x02034000,
    PERF_RES_FPS_FPSGO_MARGIN_MODE_DBNC_A            = 0x02034100,
    PERF_RES_FPS_FPSGO_MARGIN_MODE_DBNC_B            = 0x02034200,
    PERF_RES_FPS_EARA_THERMAL_ENABLE                 = 0x02038000,
    PERF_RES_FPS_FBT_RESCUE_F                        = 0x0203c000,
    PERF_RES_FPS_FBT_RESCUE_PERCENT                  = 0x0203c100,
    PERF_RES_FPS_FBT_ULTRA_RESCUE                    = 0x0203c200,
    PERF_RES_FPS_FPSGO_ADJ_LOADING                   = 0x02040000,
    PERF_RES_FPS_FPSGO_ADJ_CNT                       = 0x02040100,
    PERF_RES_FPS_FPSGO_ADJ_DEBNC_CNT                 = 0x02040200,
    PERF_RES_FPS_FPSGO_ADJ_LOADING_TIMEDIFF          = 0x02040300,
    PERF_RES_FPS_FPSGO_LLF_TH                        = 0x02044000,

    /* MAJOR = 9 Display (DFPS, display, video mode) */
    PERF_RES_DISP_DFPS_MODE                          = 0x02400000,
    PERF_RES_DISP_DFPS_FPS                           = 0x02400100,
    PERF_RES_DISP_VIDEO_MODE                         = 0x02404000,
    PERF_RES_DISP_DECOUPLE                           = 0x02408000,
    PERF_RES_DISP_IDLE_TIME                          = 0x0240c000,

    /* MAJOR = 10 CONSYS + MD */
    PERF_RES_NET_WIFI_CAM                            = 0x02800000,
    PERF_RES_NET_WIFI_LOW_LATENCY                    = 0x02804000,
    PERF_RES_NET_WIFI_SMART_PREDICT                  = 0x02804100,
    PERF_RES_NET_NETD_BOOST_UID                      = 0x02808000,
    PERF_RES_NET_NETD_BLOCK_UID                      = 0x02808100,
    PERF_RES_NET_MD_LOW_LATENCY                      = 0x0280c000,
    PERF_RES_NET_MD_GAME_MODE                        = 0x0280c100,
    PERF_RES_NET_MD_CERT_PID                         = 0x0280c200,
    PERF_RES_NET_MD_CRASH_PID                        = 0x0280c300,
    PERF_RES_NET_MD_WEAK_SIG_OPT                     = 0x0280c400,
    PERF_RES_NET_BT_A2DP_LOW_LATENCY                 = 0x02810000,

    /* MAJOR = 11 IO */
    PERF_RES_IO_BOOST_VALUE                          = 0x02c00000,
    PERF_RES_IO_UCLAMP_MIN                           = 0x02c00100,
    PERF_RES_IO_UTIL_MIN                             = 0x02c00200,
    PERF_RES_IO_F2FS_UFS_BOOST                       = 0x02c04000,
    PERF_RES_IO_F2FS_EMMC_BOOST                      = 0x02c08000,
    PERF_RES_IO_BLKDEV_READAHEAD                     = 0x02c0c000,
    PERF_RES_IO_EXT4_DATA_BOOST                      = 0x02c10000,

    /* MAJOR = 12 Misc. (thermal, touch, sys perf) */
    PERF_RES_THERMAL_POLICY                          = 0x03000000,
    PERF_RES_UX_PREDICT_LOW_LATENCY                  = 0x03004000,
    PERF_RES_UX_PREDICT_GAME_MODE                    = 0x03004100,
    PERF_RES_CFP_ENABLE                              = 0x03008000,
    PERF_RES_CFP_POLLING_MS                          = 0x03008100,
    PERF_RES_CFP_UP_LOADING                          = 0x03008200,
    PERF_RES_CFP_DOWN_LOADING                        = 0x03008300,
    PERF_RES_CFP_UP_TIME                             = 0x03008400,
    PERF_RES_CFP_DOWN_TIME                           = 0x03008500,
    PERF_RES_CFP_UP_OPP                              = 0x03008600,
    PERF_RES_CFP_DOWN_OPP                            = 0x03008700,
    PERF_RES_PERF_TASK_TURBO                         = 0x0300c000,
    PERF_RES_TOUCH_CHANGE_RATE                       = 0x03010000,

    /* MAJOR = 13 PowerHAL internal control */
    PERF_RES_POWERHAL_SCREEN_OFF_STATE               = 0x03400000,
    PERF_RES_POWERHAL_SCN_CRASH                      = 0x03400100,
    PERF_RES_POWERHAL_SPORTS_MODE                    = 0x03404000,
    PERF_RES_POWERHAL_SPORTS_MODE_APP_SMART_MODE     = 0x03404100,
    PERF_RES_POWERHAL_TOUCH_BOOST_OPP                = 0x03408000,
    PERF_RES_POWERHAL_TOUCH_BOOST_DURATION           = 0x03408100,
    PERF_RES_POWERHAL_TOUCH_BOOST_ACTIVE_TIME        = 0x03408200,
    PERF_RES_POWERHAL_WHITELIST_APP_LAUNCH_TIME_COLD = 0x0340c000,
    PERF_RES_POWERHAL_WHITELIST_APP_LAUNCH_TIME_WARM = 0x0340c100,
    PERF_RES_POWER_HINT_HOLD_TIME                    = 0x03410000,
    PERF_RES_POWER_HINT_EXT_HINT                     = 0x03410100,
    PERF_RES_POWER_HINT_EXT_HINT_HOLD_TIME           = 0x03410200,

    /* MAJOR = 64 Reserved for customization */
    PERF_RES_CUSTOM_RESOURCE_1                       = 0x10000000,
};

