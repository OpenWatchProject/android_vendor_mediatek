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

#ifndef ANDROID_COMMON_H
#define ANDROID_COMMON_H

#include <string>

using namespace std;

#define PATH_CPUTOPO_CHECK_HMP "/sys/devices/system/cpu/cputopo/is_big_little"
#define PATH_CPUTOPO_NR_CLUSTER "/sys/devices/system/cpu/cputopo/nr_clusters"
#define PATH_CPUTOPO_CLUSTER_CPU "/sys/devices/system/cpu/cputopo/cpus_per_cluster"
#define PATH_PERFMGR_TOPO_CHECK_HMP "/proc/perfmgr/boost_ctrl/topo_ctrl/is_big_little"
#define PATH_PERFMGR_TOPO_NR_CLUSTER "/proc/perfmgr/boost_ctrl/topo_ctrl/nr_clusters"
#define PATH_PERFMGR_TOPO_CLUSTER_CPU "/proc/perfmgr/boost_ctrl/topo_ctrl/cpus_per_cluster"

#define PATH_CPUHOTPLUG_CFG "/sys/devices/system/cpu/cpufreq/hotplug/cpu_num_base"
#define PATH_CPUHOTPLUG_HPS_MIN  "/proc/hps/num_base_perf_serv"
#define PATH_CPUHOTPLUG_HPS_MAX  "/proc/hps/num_limit_power_serv"
#define PATH_CPUHOTPLUG_DOWN_THRESHOLD "/proc/hps/down_threshold"
#define PATH_CPUHOTPLUG_UP_THRESHOLD   "/proc/hps/up_threshold"
#define PATH_CPUHOTPLUG_RUSH_BOOST     "/proc/hps/rush_boost_enabled"
#define PATH_CPUHOTPLUG_HEAVY_TASK     "/proc/hps/heavy_task_enabled"
#define PATH_CPUHOTPLUG_UP_TIMES       "/proc/hps/up_times"
#define PATH_CPUHOTPLUG_DOWN_TIMES     "/proc/hps/down_times"
#define PATH_CPUHOTPLUG_POWER_MODE     "/proc/hps/power_mode"
#define PATH_CPUFREQ_HISPEED_FREQ        "/sys/devices/system/cpu/cpufreq/interactive/hispeed_freq"
#define PATH_CPUFREQ_MIN_SAMPLE_TIME     "/sys/devices/system/cpu/cpufreq/interactive/min_sample_time"
#define PATH_CPUFREQ_ABOVE_HISPEED_DELAY "/sys/devices/system/cpu/cpufreq/interactive/above_hispeed_delay"
#define PATH_SCHED_HTASK_THRESH          "/sys/devices/system/cpu/rq-stats/htasks_thresh"
#define PATH_SCHED_AVG_HTASK_THRESH      "/sys/devices/system/cpu/rq-stats/avg_htasks_thresh"

#define PATH_VCORE "/proc/fliper"
#define PATH_DCM_STATE "/sys/power/dcm_state"
#define PATH_FBC "/proc/perfmgr/fbc"

#define PATH_CPUNUM_POSSIBLE "/sys/devices/system/cpu/possible"
#define PATH_CPU_CPUFREQ "/sys/devices/system/cpu/cpu0/cpufreq"
#define PATH_CPUONLINE_0 "/sys/devices/system/cpu/cpu0/online"
#define PATH_CPUONLINE_1 "/sys/devices/system/cpu/cpu1/online"
#define PATH_CPUONLINE_2 "/sys/devices/system/cpu/cpu2/online"
#define PATH_CPUONLINE_3 "/sys/devices/system/cpu/cpu3/online"
#define PATH_GOVERNOR    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define PATH_CPUPRESENT  "/sys/devices/system/cpu/present"

#define PATH_CPUFREQ_LIMIT "/proc/cpufreq/cpufreq_limited_by_hevc"
#define PATH_CPUFREQ_BIG_LIMIT     "/proc/cpufreq/MT_CPU_DVFS_BIG/cpufreq_limited_by_hevc"
#define PATH_CPUFREQ_LITTLE_LIMIT  "/proc/cpufreq/MT_CPU_DVFS_LITTLE/cpufreq_limited_by_hevc"
#define PATH_CPUFREQ_MAX_FREQ      "/proc/cpufreq/cpufreq_limited_max_freq_by_user"
#define PATH_CPUFREQ_MAX_FREQ_BIG  "/proc/cpufreq/MT_CPU_DVFS_BIG/cpufreq_limited_max_freq_by_user"
#define PATH_CPUFREQ_MAX_FREQ_CPU0 "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define PATH_CPUFREQ_MIN_FREQ_CPU0 "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq"
#define PATH_CPUFREQ_LIMIT_PROCFS  "/proc/cpufreq/cpufreq_cur_freq"
#define PATH_CPUFREQ_L_CURR_FREQ   "/proc/cpufreq/MT_CPU_DVFS_L/cpufreq_freq"
#define PATH_CPUFREQ_ROOT          "/proc/cpufreq"
#define PATH_CPUFREQ_POWER_MODE    "/proc/cpufreq/cpufreq_power_mode"

#define PATH_PPM_CORE_BASE     "/proc/ppm/policy/userlimit_min_cpu_core"
#define PATH_PPM_CORE_LIMIT    "/proc/ppm/policy/userlimit_max_cpu_core"
#define PATH_PPM_CORE_CTRL     "/proc/ppm/policy/userlimit_cpu_core"
#define PATH_PERFMGR_CORE_CTRL "/proc/perfmgr/legacy/perfserv_core"
#define PATH_BOOST_CORE_CTRL   "/proc/perfmgr/boost_ctrl/cpu_ctrl/perfserv_core"
#define PATH_PPM_FREQ_BASE     "/proc/ppm/policy/userlimit_min_cpu_freq"
#define PATH_PPM_FREQ_LIMIT    "/proc/ppm/policy/userlimit_max_cpu_freq"
#define PATH_PPM_FREQ_CTRL     "/proc/ppm/policy/userlimit_cpu_freq"
#define PATH_PERFMGR_FREQ_CTRL "/proc/perfmgr/legacy/perfserv_freq"
#define PATH_BOOST_FREQ_CTRL   "/proc/perfmgr/boost_ctrl/cpu_ctrl/perfserv_freq"
#define PATH_PPM_PERF_IDX_MIN  "/proc/ppm/policy/perfserv_min_perf_idx"
#define PATH_PPM_PERF_IDX_MAX  "/proc/ppm/policy/perfserv_max_perf_idx"
#define PATH_PPM_PERF_IDX      "/proc/ppm/policy/perfserv_perf_idx"
#define PATH_PPM_ROOT_CLUSTER  "/proc/ppm/root_cluster"
#define PATH_PPM_MODE          "/proc/ppm/mode"

#define PATH_TURBO_SUPPORT     "/proc/perfmgr/smart/smart_turbo_support"

#define PATH_GPUFREQ_COUNT   "/d/ged/hal/total_gpu_freq_level_count"
#define PATH_GPUFREQ_BASE    "/d/ged/hal/custom_boost_gpu_freq"
#define PATH_GPUFREQ_MAX     "/d/ged/hal/custom_upbound_gpu_freq"
#define PATH_GPUFREQ_EVENT   "/d/ged/hal/event_notify"

#define PATH_THERMAL_PID          "/proc/driver/thermal/ta_fg_pid"
#define PATH_GX_PID               "/sys/module/ged/parameters/gx_top_app_pid"
#define PATH_HARD_USER_LIMIT      "/proc/ppm/policy/hard_userlimit_cpu_freq"


/*-- add for DL/DC --*/
#define DISP_DEV_ID               "/dev/mtk_disp_mgr"

extern int set_value(const char * path, const int value_1, const int value_2);
extern int set_value(const char * path, const int value);
extern int set_value(const char * path, const char *str);
extern int set_value(const char * path, const string *str);
extern int  get_int_value(const char * path);
extern void get_str_value(const char * path, char *str, int len);
extern int  get_cpu_num(void);
extern void get_cputopo_cpu_info(int cluster_num, int *p_cpu_num, int *p_first_cpu);
extern void get_task_comm(const char *path, char *comm);
extern void get_ppm_cpu_freq_info(int cluster_index, int *p_max_freq, int *p_count, int **pp_table);
extern void get_cpu_freq_info(int cpu_index, int *p_max_freq, int *p_count, int **pp_table);
extern void get_gpu_freq_level_count(int *p_count);
extern void set_gpu_freq_level(int level);
extern void set_gpu_freq_level_max(int level);
extern void set_vcore_level(int level);
extern void set_str_cpy(char * desc, const char *src, int desc_max_size);
//extern int set_disp_ctl(int enable);

#endif // ANDROID_COMMON_H

