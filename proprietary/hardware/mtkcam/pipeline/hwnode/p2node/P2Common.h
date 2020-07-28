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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_P2COMMON_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_P2COMMON_H_
#ifndef LOG_TAG
#define LOG_TAG "MtkCam/P2Node"
#endif

#include <mtkcam/utils/std/Log.h>
#include <vector>
#include <mtkcam/utils/std/Trace.h>
#include <cutils/properties.h>
#include "hwnode_utilities.h"
#include "BaseNode.h"
// using generic namespace
using namespace android;
using namespace NSCam::v3;
using namespace std;

/******************************************************************************
 *
 ******************************************************************************/

//
#define P2_THREAD_POLICY    (SCHED_OTHER)
#define P2_THREAD_PRIORITY  (ANDROID_PRIORITY_FOREGROUND-2)
//
#define P2_DEBUG_DUMP_PATH  "/sdcard/camera_dump"
#define P2_DEBUG_DEQUE      (0)
#define P2_DEBUG_LOG        (0)
#define P2_DEBUG_FUNC       (P2_DEBUG_LOG && (0))
//
#define SUPPORT_3A               (1)
#define SUPPORT_ISP              (1)
#define SUPPORT_EIS              (1)
#define SUPPORT_EIS_MV           (0)
#define SUPPORT_MNR              (!MTKCAM_HAVE_IVENDOR_SUPPORT)
#define SUPPORT_SWNR             (!MTKCAM_HAVE_IVENDOR_SUPPORT)
#define SUPPORT_IMG3O_PORT       (1)
#define SUPPORT_MFBO_PORT        (1)
#define SUPPORT_FD_PORT          (1)
#define SUPPORT_PLUGIN           (1)
#define SUPPORT_3RD_PARTY        (MTKCAM_HAVE_IVENDOR_SUPPORT)
#define SUPPORT_DRE              (1)
#define FORCE_EIS_ON             (SUPPORT_EIS  && (0))
#define FORCE_BURST_ON           (0)
#define FORCE_VENC_STREAM_ON     (0)
#define FORCE_MNR_ON             (SUPPORT_MNR  && (0))
#define FORCE_SWNR_ON            (SUPPORT_SWNR && (0))

#define DRE_CAP_NAME             ("CapP2")
#define DRE_PRV_NAME             ("PrvP2")

/******************************************************************************
 * Log
 ******************************************************************************/
#ifdef DEFINE_OPEN_ID
#define MY_LOGDO(fmt, arg...)        CAM_LOGD("[%s] (%d) " fmt, __FUNCTION__, DEFINE_OPEN_ID, ##arg)
#define MY_LOGIO(fmt, arg...)        CAM_LOGI("[%s] (%d) " fmt, __FUNCTION__, DEFINE_OPEN_ID, ##arg)
#define MY_LOGWO(fmt, arg...)        CAM_LOGW("[%s] (%d) " fmt, __FUNCTION__, DEFINE_OPEN_ID, ##arg)
#define MY_LOGEO(fmt, arg...)        CAM_LOGE("[%s] (%d) " fmt, __FUNCTION__, DEFINE_OPEN_ID, ##arg)
#define MY_LOGDO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGDO(__VA_ARGS__); } }while(0)
#define MY_LOGIO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGIO(__VA_ARGS__); } }while(0)
#define MY_LOGWO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGWO(__VA_ARGS__); } }while(0)
#define MY_LOGEO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGEO(__VA_ARGS__); } }while(0)
#else
#define MY_LOGDO(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGIO(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGWO(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGEO(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGDO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGDO(__VA_ARGS__); } }while(0)
#define MY_LOGIO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGIO(__VA_ARGS__); } }while(0)
#define MY_LOGWO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGWO(__VA_ARGS__); } }while(0)
#define MY_LOGEO_IF(cond, ...)       do { if ( (cond) ) { MY_LOGEO(__VA_ARGS__); } }while(0)
#endif
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#if P2_DEBUG_FUNC
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif
//
#define ASSERT_TRUE(pridicate, message)     \
    do {                                    \
        if (!(pridicate)) {                 \
            MY_LOGE(message);               \
        }                                   \
    } while(0)


/******************************************************************************
 * AEE Assert
 ******************************************************************************/
#if (HWNODE_HAVE_AEE_FEATURE)
#include <aee.h>
#ifdef AEE_ASSERT
#undef AEE_ASSERT
#endif
#define AEE_ASSERT(String) \
    do { \
        CAM_LOGE("ASSERT("#String") fail"); \
        aee_system_exception( \
            LOG_TAG, \
            NULL, \
            DB_OPT_DEFAULT, \
            String); \
    } while(0)
#else
#define AEE_ASSERT(String)
#endif

#endif
