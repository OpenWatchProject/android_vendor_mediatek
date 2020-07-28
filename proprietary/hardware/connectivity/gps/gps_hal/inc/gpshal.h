/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __GPS_HAL_H__
#define __GPS_HAL_H__

#include "hardware/gps_mtk.h"
#include <pthread.h>

//=========================================================

#define MAX_EPOLL_EVENT 50
#define GNSS_NAME_LEN (255)

//=========================================================

typedef enum {  // state order is important
    GPSHAL_STATE_UNKNOWN,
    GPSHAL_STATE_RESOURCE,
    GPSHAL_STATE_CLEANUP,
    GPSHAL_STATE_INIT,    // == STOP
    GPSHAL_STATE_STOP,    // == INIT
    GPSHAL_STATE_START
} gpshal_state;

typedef struct {
    int       fd_mnl2hal;
    int       fd_worker_epoll;
    pthread_t thd_worker;

    pthread_mutex_t mutex_gps_state_intent;
    gpshal_state    gps_state_intent;  // what we want
    gpshal_state    gps_state;  // what we are, but we may fail to change it
    int             gps_cap;
    char            gps_name[GNSS_NAME_LEN];
    char            proxy_apps[HAL_NFW_USER_NUM_MAX*HAL_NFW_USER_NAME_LEN];  //NFW user app list

    GpsCallbacks_ext* gps_cbs;
    AGpsCallbacks*    agps_cbs;
    GpsNiCallbacks*   gpsni_cbs;
    AGpsRilCallbacks* agpsril_cbs;
    GpsMeasurementCallbacks_ext*       meas_cbs;
    MeasurementCorrectionCallbacks_ext* meas_correct_cbs;
    GnssVisibilityControlCallback_ext* visibility_control_cbs;
    GpsNavigationMessageCallbacks* navimsg_cbs;
    GpsGeofenceCallbacks* geofence_cbs;
    VzwDebugCallbacks* vzw_debug_cbs;
} gpshal_context_t;

//=========================================================

extern gpshal_context_t g_gpshal_ctx;
extern const AGpsInterface_ext mtk_agps_inf;
extern const GpsNiInterface  mtk_gps_ni_inf;
extern const AGpsRilInterface_ext mtk_agps_ril_inf;
extern const GpsGeofencingInterface mtkGeofence_inf;


//=========================================================

extern const char* gpshal_state_to_string(gpshal_state state);

extern int gpshal_gpscbs_save(GpsCallbacks_ext* src);

extern void gpshal_set_gps_state_intent(gpshal_state newState);

extern void gpshal2mnl_gps_init();
extern void gpshal2mnl_gps_start();
extern void gpshal2mnl_gps_stop();
extern void gpshal2mnl_gps_cleanup();

extern void gpshal_worker_thread(void *arg);

#endif  //  __GPS_HAL_DEBUG_H__
