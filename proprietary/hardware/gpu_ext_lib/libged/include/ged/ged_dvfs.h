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

#ifndef __GED_DVFS_H__
#define __GED_DVFS_H__

#include "ged_type.h"

#define GED_NO_UM_SERVICE -1

#define GED_DVFS_VSYNC_OFFSET_SIGNAL_EVENT 44
#define GED_FPS_CHANGE_SIGNAL_EVENT        45
#define GED_SRV_SUICIDE_EVENT 46
#define GED_MHL4K_VID_SIGNAL_EVENT         48
#define GED_GAS_SIGNAL_EVENT               49
#define GED_SIGNAL_BOOST_HOST_EVENT               50
#define GED_VILTE_VID_SIGNAL_EVENT         51
#define GED_LOW_LATENCY_MODE_SIGNAL_EVENT  52

#define DYNAMIC_VSYNC_OFFSET_PROTOCOL 10002

/* GED_DVFS_DIFF_THRESHOLD (us) */
#define GED_DVFS_DIFF_THRESHOLD 500

#if defined (__cplusplus)
extern "C" {
#endif

GED_ERROR ged_dvfs_policy(GED_HANDLE hGed, unsigned long ul3DFenceDoneTime, unsigned long ulvsync_period,unsigned long t, long phase);
void ged_dvfs_set_vsync_offset(int voffset);

#define GED_EVENT_TOUCH             (1 << 0)
#define GED_EVENT_THERMAL           (1 << 1)
#define GED_EVENT_WFD               (1 << 2)
#define GED_EVENT_MHL               (1 << 3)
#define GED_EVENT_GAS               (1 << 4)
#define GED_EVENT_LOW_POWER_MODE    (1 << 5)
#define GED_EVENT_MHL4K_VID         (1 << 6)
#define GED_EVENT_BOOST_HOST        (1 << 7)
#define GED_EVENT_VR                (1 << 8)
#define GED_EVENT_VILTE_VID         (1 << 9)
#define GED_EVENT_LCD               (1 << 10)
#define GED_EVENT_NETWORK           (1 << 11)
#define GED_EVENT_DOPT_WIFI_SCAN    (1 << 12)
#define GED_EVENT_LOW_LATENCY_MODE  (1 << 13)
#define GED_EVENT_DHWC              (1 << 14)

#define GED_EVENT_FORCE_ON          (1 << 0)
#define GED_EVENT_FORCE_OFF         (1 << 1)
#define GED_EVENT_NOT_SYNC          (1 << 2)

#define GED_VSYNC_OFFSET_NOT_SYNC -2
#define GED_VSYNC_OFFSET_SYNC -3

#if defined (__cplusplus)
}
#endif

#endif
