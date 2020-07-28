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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifdef LOG_TAG
    #undef LOG_TAG
#endif

#define LOG_TAG "MtkCam/DenoiseDecision"

#include "camera_custom_stereo.h"
//
#include "DualCamDenoisePolicy.h"
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//

#include <algorithm>

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

#define MY_LOGV(id, fmt, arg...)        CAM_LOGV("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)        CAM_LOGD("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)        CAM_LOGI("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)        CAM_LOGW("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)        CAM_LOGE("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGA(id, fmt, arg...)        CAM_LOGA("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGF(id, fmt, arg...)        CAM_LOGF("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_IN()             CAM_LOGI("[%d][%s]+", mRefCount, __FUNCTION__)
#define FUNCTION_OUT()            CAM_LOGI("[%d][%s]-", mRefCount, __FUNCTION__)
/******************************************************************************
 *  Metadata Access
 ******************************************************************************/
namespace DenoisePolicy
{
template <typename T>
static MBOOL
tryGetMetadata(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if (pMetadata == NULL) {
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    //
    return MFALSE;
}
};
/******************************************************************************
 * implement
 ******************************************************************************/
namespace NSCam
{
/******************************************************************************
 *
 ******************************************************************************/
bool
DualCamDenoisePolicy::
canUsingDualCam(
    MINT32 const openId,
    DECISION_STATUS &status,
    MBOOL isCapture
)
{
    bool ret = true;
    bool batteryCheck = true;
    bool thermalCheck = true;
    auto stateToString = [&status]()
    {
        switch(status)
        {
            case DECISION_STATUS::UNSTABLE:
                return "UNSTABLE";
            case DECISION_STATUS::MAIN2_ENABLE:
                return "MAIN2_ENABLE";
            case DECISION_STATUS::MAIN2_DISABLE:
                return "MAIN2_DISABLE";
        }
        return "";
    };
    if(mbSkipMain2Check)
    {
        status = DECISION_STATUS::MAIN2_ENABLE;
        return true;
    }
    // check HDR mode
    if(mHDRMode == NSCam::HDRMode::ON)
    {
        MY_LOGD(openId, "HDR mode is force enable, can not using dual cam");
        status = DECISION_STATUS::MAIN2_DISABLE;
        goto lbExit;
    }
    // check zoom value
    if(miZoomRatio != 100)
    {
        MY_LOGD(openId, "Zoom vlaue is not equal to 100 (%d), cannot using dual cam", miZoomRatio);
        status = DECISION_STATUS::MAIN2_DISABLE;
        goto lbExit;
    }
    // check flash
    if(mbUsingFlash)
    {
        MY_LOGD(openId, "flash detect");
        status = DECISION_STATUS::MAIN2_DISABLE;
        goto lbExit;
    }
    // check battery
    if(mspBatteryDetector != nullptr)
    {
        batteryCheck = mspBatteryDetector->canUsingDualCam();
        if(!batteryCheck)
        {
            MY_LOGD(openId, "Battery check fail, cannot using dual cam");
            status = DECISION_STATUS::MAIN2_DISABLE;
            goto lbExit;
        }
    }
    // check thermal
    if(mspThermalDetector != nullptr)
    {
        thermalCheck= mspThermalDetector->canUsingDualCam();
        if(!thermalCheck)
        {
            MY_LOGD(openId, "Thermal check fail, cannot using dual cam");
            status = DECISION_STATUS::MAIN2_DISABLE;
            goto lbExit;
        }
    }
    // check iso
    {
        if(miISO == -1)
        {
            // this status means 3a is not stable, so pass iso check.
            MY_LOGD(openId, "3a is not stable, skip iso check");
            status = DECISION_STATUS::UNSTABLE;
            goto lbExit;
        }
        else if(miISO < ::getMain2SwichISO(BMDeNoiseMain2Switch::Off))
        {
            int value = ::getMain2SwichISO(BMDeNoiseMain2Switch::Off);
            // if iso less than DENOISE_POLICY_ISO_MAIN2_OFF, it needs to close main2.
            //MY_LOGD(openId, "ISO(%d) less then DENOISE_POLICY_ISO_MAIN2_OFF(%d)", miISO, value);
            status = DECISION_STATUS::MAIN2_DISABLE;
            goto lbExit;
        }
        else if(miISO < ::getMain2SwichISO(BMDeNoiseMain2Switch::On))
        {
            if(mMain2SensorStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
            {
                int value = ::getMain2SwichISO(BMDeNoiseMain2Switch::On);
                // if iso less than DENOISE_POLICY_ISO_MAIN2_ON, it needs to close main2.
                //MY_LOGD(openId, "ISO(%d) less then DENOISE_POLICY_ISO_MAIN2_ON(%d)", miISO, value);
                status = DECISION_STATUS::MAIN2_DISABLE;
                goto lbExit;
            }
        }
    }
    mBatteryCheck = batteryCheck;
    mThermalCheck = thermalCheck;
    status = DECISION_STATUS::MAIN2_ENABLE;
lbExit:
    MY_LOGD(openId, "[%s] HDRMode(%u) Zoom(%d) flash(%d) Battery(%d) Thermal(%d) ISO(%d)",
            stateToString(),
            (MUINT8)mHDRMode,
            miZoomRatio,
            mbUsingFlash,
            batteryCheck,
            thermalCheck,
            miISO);
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamDenoisePolicy::
update3AInfo(
    IMetadata* halMetadata)
{
    if(mspHal3a_Main == nullptr)
    {
        MY_LOGE(-1, "mspHal3a_Main is nullptr");
        return;
    }
    // get 3a info
    NS3Av3::DualZoomInfo_T aaaInfo;
    mspHal3a_Main->send3ACtrl(NS3Av3::E3ACtrl_GetDualZoomInfo, (MINTPTR)&aaaInfo, 0);
    // this interface will get real iso, and it is not suitable to denoise.
    // updateISO(aaaInfo);
    updateFlashFlag(aaaInfo, halMetadata);
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamDenoisePolicy::
updateISO(
    NS3Av3::DualZoomInfo_T &aaaInfo
)
{
    miISO = aaaInfo.i4AEIso;
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamDenoisePolicy::
updateFlashFlag(
    NS3Av3::DualZoomInfo_T &aaaInfo,
    IMetadata* halMetadata
)
{
    MUINT8 aeMode = 0;
    if(!DenoisePolicy::tryGetMetadata<MUINT8>(halMetadata, MTK_CONTROL_AE_MODE, aeMode))
    {
        MY_LOGW(-1, "cannot get MTK_CONTROL_AE_MODE");
    }
    else
    {
        // update flash
        if(aeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)
        {
            mbUsingFlash = MTRUE;
        }
        else if(aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH || aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE)
        {
            // check bv trigger
            if(aaaInfo.bIsAEBvTrigger)
            {
                mbUsingFlash = MTRUE;
            }
            else
            {
                mbUsingFlash = MFALSE;
            }
        }else{
            mbUsingFlash = MFALSE;
        }

        MY_LOGD_IF(mLogLevel>0, -1, "aeMode(%d) bIsAEBvTrigger(%d) mbUsingFlash(%d)",
            aeMode,
            aaaInfo.bIsAEBvTrigger,
            mbUsingFlash
        );
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
DualCamDenoisePolicy::
getMain2OnIsoValue(
)
{
    return ::getMain2SwichISO(BMDeNoiseMain2Switch::On);
}
};