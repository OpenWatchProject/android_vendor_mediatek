/*
 * Copyright (C) 2011-2014 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __GED_H__
#define __GED_H__

#include <stdint.h>
#include "ged_type.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct GED_TAG
{
    int32_t i32BridgeFD;
    int32_t tid;
}GED;

GED_HANDLE ged_create(void);

void ged_destroy(GED_HANDLE hGed);

GED_ERROR ged_boost_gpu_freq(GED_HANDLE hGed, GED_BOOST_GPU_FREQ_LEVEL eLevel);

GED_ERROR ged_boost_gpu_freq_ext(GED_HANDLE hGed, GED_BOOST_GPU_FREQ_LEVEL eLevel, GED_BOOST_ORIGIN eOrigin);

GED_ERROR ged_gpu_hint_cpu(GED_HANDLE hGed, uint32_t flag);

GED_ERROR ged_hint_force_mdp(GED_HANDLE hGed, int32_t flag, int32_t *mdp_flag);

GED_ERROR ged_notify_sw_vsync(GED_HANDLE hGed, GED_DVFS_UM_QUERY_PACK* psQueryData);

GED_ERROR ged_dvfs_probe(GED_HANDLE hGed, int pid);

GED_ERROR ged_dvfs_um_return(GED_HANDLE hGed, unsigned long gpu_tar_freq, bool bFallback);

GED_ERROR ged_query_info( GED_HANDLE hGed, GED_INFO eType, size_t size, void* retrieve);

GED_ERROR ged_event_notify(GED_HANDLE hGed, GED_DVFS_VSYNC_OFFSET_SWITCH_CMD eEvent, bool bSwitch);

GED_ERROR ged_vsync_calibration(GED_HANDLE hGed, int i32Delay, unsigned long nsVsync_period);

GED_ERROR ged_vsync_notify(GED_HANDLE hGed, unsigned long msVsync_period);

GED_ERROR ged_wait_for_hw_vsync(GED_HANDLE hGed);

int ged_frr_query_target_fps(GED_HANDLE hGed, int pid, uint64_t cid, int fencefd);

int ged_gpu_timestamp(GED_HANDLE hGed, uint64_t ullWnd, int i32FrameID, int fence_fd, int QedBuffer_length, int pid, int isSF);

#if defined (__cplusplus)
}
#endif

#endif
