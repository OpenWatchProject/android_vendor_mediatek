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

#define LOG_TAG "MtkCam/vmgr_v"
//
#include "MyUtils.h"
//
#include <mtkcam/pipeline/extension/IVendorManager.h>
#include <mtkcam/pipeline/extension/MFNR.h>
#include <mtkcam/pipeline/extension/Collect.h>
//
#include <mtkcam/pipeline/extension/Vendor.h>

using namespace android;
using namespace NSCam::plugin;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN             MY_LOGD_IF(1<=mLogLevel, "%d ", getOpenId());
#define FUNCTION_OUT            MY_LOGD_IF(1<=mLogLevel, "%d ", getOpenId());


sp<IVendor>
IVendor::
createInstance(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
)
{
    MY_LOGD("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);

    sp<IVendor> rVendor;

    switch (vendorMode) {
    case MTK_PLUGIN_MODE_COPY:
        // rVendor = CopyVendor::createInstance(pcszName, i4OpenId, vendorMode);
        rVendor = mixYuvVendor::createInstance(pcszName, i4OpenId, vendorMode);
        break;
    case MTK_PLUGIN_MODE_MFNR:
        rVendor = MAKE_MFNRVendor(pcszName, i4OpenId, vendorMode);
        break;

    case MTK_PLUGIN_MODE_HDR:
        rVendor = MAKE_HDRVendor(pcszName, i4OpenId, vendorMode);
        break;

    case MTK_PLUGIN_MODE_NR:
        rVendor = NRvendor::createInstance(pcszName, i4OpenId, vendorMode);
        break;

    case MTK_PLUGIN_MODE_BMDN:
    case MTK_PLUGIN_MODE_MFHR:
        rVendor = MAKE_BMDNVendor(pcszName, i4OpenId, vendorMode);
        break;

#if (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#if (MTKCAM_HAVE_VSDOF_MODE == 2)
	case MTK_PLUGIN_MODE_DCMF_3RD_PARTY:
        rVendor = MAKE_DCMFVendor(pcszName, i4OpenId, vendorMode);
		break;
#endif
#endif

#if (MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT == 1)
    case MTK_PLUGIN_MODE_FUSION_3rdParty:
        rVendor = FusionVendor::createInstance(pcszName, i4OpenId, vendorMode);
        break;
#endif

#if (MTKCAM_HAVE_DUALCAM_VSDOF_VENDOR_SUPPORT == 1)
    case MTK_PLUGIN_MODE_VSDOF_3rdParty:
        rVendor = VsdofVendor::createInstance(pcszName, i4OpenId, vendorMode);
        break;
#endif

    case MTK_PLUGIN_MODE_COLLECT:
        rVendor = MAKE_COLLECTVendor(pcszName, i4OpenId, vendorMode);
        break;
    default:
        // do nothing.
        ;
    }

    if (rVendor.get() == nullptr) {
        MY_LOGE("un-supported vendor mode(%" PRId64 ") or create vendor returns NULL",
                vendorMode);
    }

    return rVendor;
}

// ----------------------------------------------------------------------------
// Static variable of all vendors
// ----------------------------------------------------------------------------
Mutex MFNRVendor::sCtrlerAccessLock;
