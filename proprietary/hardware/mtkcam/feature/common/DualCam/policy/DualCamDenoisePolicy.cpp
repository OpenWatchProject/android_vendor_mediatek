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

#define LOG_TAG "MtkCam/DualCamDenoisePolicy"

#include "DualCamDenoisePolicy.h"

#include <cutils/properties.h>
//
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe_var.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
// for policy
#include <mtkcam/utils/hw/CamManager.h>
//

#include <algorithm>

using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::VarMap;

std::future<void> DualCamDenoisePolicy::tPolicySetting;

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

#define USE_BACKGROUND_THREAD_PULLING 1

#define DENOISE_WAIT_STABLE_COUNT       (3)
#define DENOISE_WAIT_STANDBY_COUNT      (6)
#define DENOISE_WAIT_CAM_ACTIVE_COUNT   (3)
#define DENOISE_WAIT_LOW_POWER_COUNT    (6)
#define DENOISE_N3D_PERIODIC            (5)

#define THERMAL_POLICY_NAME "thermal_policy_04"
/******************************************************************************
 *  Metadata Access
 ******************************************************************************/
namespace DenoisePolicy
{
template <typename T>
static inline MBOOL
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
MINT32 DualCamDenoisePolicy::totalRefCount = 0;
/******************************************************************************
 *
 ******************************************************************************/
DualCamDenoisePolicy::
DualCamDenoisePolicy():
    mRefCount(totalRefCount)
{
    MY_LOGD(-1, "set thermal policy");
    if(DualCamDenoisePolicy::tPolicySetting.valid())
    {
        MY_LOGD(-1, "have share state");
        DualCamDenoisePolicy::tPolicySetting.get();
    }
    // set thermal policy first.
    DualCamDenoisePolicy::tPolicySetting = std::async(
                std::launch::async,
                []()
                {
                    Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_POLICY_NAME, 1);
                });
    DualCamDenoisePolicy::totalRefCount++;
    mspThermalDetector = new ThermalDetector();
    mspBatteryDetector = new BatteryDetector();
#if USE_BACKGROUND_THREAD_PULLING
    if(mspThermalDetector!=nullptr)
    {
        mspThermalDetector->execute();
    }
    else
    {
        MY_LOGE(-1, "create ThremalDetector fail");
    }
    if(mspBatteryDetector!=nullptr)
    {
        mspBatteryDetector->execute();
    }
    else
    {
        MY_LOGE(-1, "create BatteryDetector fail");
    }
#endif
    // get main1 & main2 openid
    if(!StereoSettingProvider::getStereoSensorIndex(miMain1Id, miMain2Id))
    {
        MY_LOGE(-1, "Cannot get sensor ids from StereoSettingProvider! (%d,%d)", miMain1Id, miMain2Id);
    }
    // create 3a hal for query 3a info
    mspHal3a_Main =
        std::shared_ptr<NS3Av3::IHal3A>(
                MAKE_Hal3A(miMain1Id, LOG_TAG),
                [&](auto *p)->void
                {
                    MY_LOGD(-1, "destroy hal3a miMain1Id");
                    if(p)
                    {
                        p->destroyInstance(LOG_TAG);
                        p = nullptr;
                    }
                }
    );
    if(mspHal3a_Main == nullptr)
    {
        MY_LOGE(-1, "Create hal3a object fail.");
    }
    mLogLevel = property_get_int32("vendor.debug.dualcam.policy", 0);
    mbSkipMain2Check = property_get_int32("vendor.debug.dualcam.skippolicy", 0);
    MY_LOGD(-1, "ctr(%p) m1(%d) m2(%d) mLogLevel(%d)", this, miMain1Id, miMain2Id, mLogLevel);
}
/******************************************************************************
 *
 ******************************************************************************/
DualCamDenoisePolicy::
~DualCamDenoisePolicy()
{
    FUNCTION_IN();
    if(DualCamDenoisePolicy::tPolicySetting.valid())
    {
        MY_LOGD(-1, "have share state");
        DualCamDenoisePolicy::tPolicySetting.get();
    }
    MY_LOGD(-1, "set thermal policy");
    DualCamDenoisePolicy::tPolicySetting = std::async(
                std::launch::async,
                [this]()
                {
                    Utils::CamManager::getInstance()->setThermalPolicy(THERMAL_POLICY_NAME, 0);
                });
    //
    mIFrame3AControl = NULL;
    //
    mMgrCb = NULL;
    //
    if(mspBatteryDetector != nullptr)
    {
#if USE_BACKGROUND_THREAD_PULLING
        mspBatteryDetector->requestExit();
        mspBatteryDetector->join();
#endif
        mspBatteryDetector = nullptr;
    }
    //
    if(mspThermalDetector != nullptr)
    {
#if USE_BACKGROUND_THREAD_PULLING
        mspThermalDetector->requestExit();
        mspThermalDetector->join();
#endif
        mspThermalDetector = nullptr;
    }
    //
    mvOpenIds.clear();
    MY_LOGD(-1, "dctr(%p)", this);
    FUNCTION_OUT();
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
setCameraOpenIds(
    vector<MINT32> ids
)
{
    // reset openid list
    mvOpenIds.clear();
    mvOpenIds = ids;
    //
    if(ids.size() > 2)
    {
        MY_LOGW(-1, "should not happened, please check flow");
    }
    //
    mMain1SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    mMain2SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    // find main1 openid
    std::vector<MINT32>::iterator it = std::find(ids.begin(), ids.end(), miMain1Id);
    mMain1SensorStatus = (it != ids.end()) ? MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE : MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    // find main2 openid
    it = std::find(ids.begin(), ids.end(), miMain2Id);
    mMain2SensorStatus = (it != ids.end()) ? MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT : MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    //
    MY_LOGD(-1, "m1(%hhu) m2(%hhu)", mMain1SensorStatus, mMain2SensorStatus);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
setObject(
    void* obj,
    MINT32 arg1
)
{
    if(obj != nullptr)
    {
        mIFrame3AControl = (IFrame3AControl*)obj;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
setParameter(
    std::string str
)
{

}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
addCallBack(
    sp<ISyncManagerCallback> Cb
)
{
    MY_LOGD(-1,"set callback");
    if(Cb == nullptr)
    {
        MY_LOGD(-1, "set nullptr?");
    }
    mMgrCb = Cb;
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
updateSetting(
    MINT32 const i4OpenId,
    IMetadata* appMetadata,
    IMetadata* halMetadata,
    SyncManagerParams &syncParams
)
{
    Mutex::Autolock _l(mLock);
    if(appMetadata == nullptr)
    {
        MY_LOGE(i4OpenId, "appMetadata is nullptr");
        return;
    }
    if(halMetadata == nullptr)
    {
        MY_LOGE(i4OpenId, "halMetadata is nullptr");
        return;
    }
    // Check main2 open condition, when main1 update setting.
    if(i4OpenId == miMain1Id)
    {
        if(mbCanUpdateSetting)
        {
            // update params
            updateParams(syncParams);
            // update 3a info for dualcam check
            update3AInfo(appMetadata);
            canUsingDualCam(i4OpenId, mbCanUsingMain2);
        }
    }
    if(i4OpenId == miMain2Id)
    {
        {
            if(mbCanUpdateSetting)
            {
                changeCameraState(i4OpenId, mbCanUsingMain2, appMetadata, halMetadata, mMain2SensorStatus);
            }
            setMain2DropDisplayRequest(i4OpenId, halMetadata);
            updateStandbyMode(i4OpenId, mSetMain2SensorStatus, halMetadata);
        }
        // skip 3dnr
        {
            IMetadata::IEntry tag_3dnr(MTK_NR_FEATURE_3DNR_MODE);
            tag_3dnr.push_back(MTK_NR_FEATURE_3DNR_MODE_OFF, Type2Type<MINT32>());
            appMetadata->update(MTK_NR_FEATURE_3DNR_MODE, tag_3dnr);
        }
    }
    if(mMain1SensorStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE &&
       mMain2SensorStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
    {
        // do frame sync check
        IMetadata::IEntry tag(MTK_DUALZOOM_DO_FRAME_SYNC);
        tag.push_back(1, Type2Type<MINT32>());
        halMetadata->update(MTK_DUALZOOM_DO_FRAME_SYNC, tag);
    }
    if(mSet3ASyncMode == MTK_DUAL_CAM_AAA_SYNC_STATE::ENABLE)
    {
        setMasterCamera(i4OpenId, halMetadata);
        // enable 3sync & hwsync
        changeCameraMode(i4OpenId, F3A_TYPE_3A_SYNC | F3A_TYPE_FRAME_SYNC, true, true, 0, 0,
                         appMetadata, halMetadata);
    }
    if(mSet3ASyncMode == MTK_DUAL_CAM_AAA_SYNC_STATE::DISABLE)
    {
        setMasterCamera(i4OpenId, halMetadata);
        // enable 3sync & hwsync
        changeCameraMode(i4OpenId, F3A_TYPE_3A_SYNC | F3A_TYPE_FRAME_SYNC, false, false, 0, 0,
                         appMetadata, halMetadata);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
updateAfterP1(
    MINT32 const i4OpenId,
    MUINT32 const frameNo,
    void* obj,
    MINT32 arg1
)
{
    auto afStateToString = [](MUINT8 state)
    {
        switch(state)
        {
            case MTK_CONTROL_AF_STATE_INACTIVE:
                return "AF_STATE_INACTIVE";
            case MTK_CONTROL_AF_STATE_PASSIVE_SCAN:
                return "AF_STATE_PASSIVE_SCAN";
            case MTK_CONTROL_AF_STATE_PASSIVE_FOCUSED:
                return "AF_STATE_PASSIVE_FOCUSED";
            case MTK_CONTROL_AF_STATE_ACTIVE_SCAN:
                return "AF_STATE_ACTIVE_SCAN";
            case MTK_CONTROL_AF_STATE_FOCUSED_LOCKED:
                return "AF_STATE_FOCUSED_LOCKED";
            case MTK_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
                return "AF_STATE_NOT_FOCUSED_LOCKED";
            case MTK_CONTROL_AF_STATE_PASSIVE_UNFOCUSED:
                return "AF_STATE_PASSIVE_UNFOCUSED";
        }
        return "None";
    };
    auto lensStateToString = [](MUINT8 state)
    {
        switch(state)
        {
            case MTK_LENS_STATE_STATIONARY:
                return "LENS_STATE_STATIONARY";
            case MTK_LENS_STATE_MOVING:
                return "LENS_STATE_MOVING";
        }
        return "None";
    };
    bool result = false;
    bool afCheck = false;
    //
    MUINT8 afState = 0;
    MUINT8 lensState = 0;
    MINT32 main2Id = 0;
    MBOOL bNeedN3D = MFALSE;
    //
    FeaturePipeParam* pipeParam = nullptr;
    if(obj == nullptr)
    {
        MY_LOGD(-1, "obj is null");
        return;
    }

    if(i4OpenId == miMain2Id)
    {
        return;
    }
    pipeParam = (FeaturePipeParam*)obj;
    // get iso
    {
        Mutex::Autolock _l(mLock);
        if(!pipeParam->mVarMap.tryGet<MINT32>(VAR_DUALCAM_EXIF_ISO, miISO))
        {
            MY_LOGW(-1, "cannot get exif iso");
        }
    }
    if(mMain2SensorStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
    {
        miDoN3DCount = 0;
        goto lbExit;
    }
    if(!pipeParam->mVarMap.tryGet<MUINT8>(VAR_DUALCAM_AF_STATE, afState))
    {
        MY_LOGW(-1, "cannot get af state");
        goto lbExit;
    }
    // MTK_LENS_STATE_STATIONARY,
    // MTK_LENS_STATE_MOVING
    if(!pipeParam->mVarMap.tryGet<MUINT8>(VAR_DUALCAM_LENS_STATE, lensState))
    {
        MY_LOGW(-1, "cannot get lens state");
        goto lbExit;
    }
    MY_LOGD_IF(mLogLevel>0, -1, "afstate(%s) lensState(%s)", afStateToString(afState), lensStateToString(lensState));
    //
    if(!pipeParam->mVarMap.tryGet<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, main2Id))
    {
        miDoN3DCount = 0;
        MY_LOGW(-1, "cannot get VAR_DUALCAM_FOV_SLAVE_ID");
        goto lbExit;
    }
    //
    afCheck = (afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN) ||
              (afState == MTK_CONTROL_AF_STATE_ACTIVE_SCAN) ||
              (lensState == MTK_LENS_STATE_MOVING);
    // if af check is true, it means af is not stable.
    // In this case, reset counting;
    if(afCheck)
    {
        miDoN3DCount = 0;
    }
    else
    {
        if(miDoN3DCount == 0 || miDoN3DCount == DENOISE_N3D_PERIODIC)
        {
            bNeedN3D = MTRUE;
            //pipeParam->mVarMap.set<MUINT8>(VAR_DUALCAM_DO_N3D, 1);
            miDoN3DCount %= DENOISE_N3D_PERIODIC;
        }
        else
        {
            bNeedN3D = MFALSE;
            //pipeParam->mVarMap.set<MUINT8>(VAR_DUALCAM_DO_N3D, 0);
        }
        miDoN3DCount++;
    }
    result = true;
lbExit:
    if(bNeedN3D)
    {
        NSCamFeature::NSFeaturePipe::ENABLE_N3D(pipeParam->mFeatureMask);
    }
    else
    {
        NSCamFeature::NSFeaturePipe::DISABLE_N3D(pipeParam->mFeatureMask);
    }
    if(!result)
    {
        miDoN3DCount = 0;
    }
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
updateFeatureMask(
    MUINT32 &featureMask
)
{
    //NSCamFeature::NSFeaturePipe::ENABLE_N3D(featureMask);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
lock(
    void* arg1
)
{
    FUNCTION_IN();
    Mutex::Autolock _l(mLock);
    if(arg1 == nullptr)
    {
        MY_LOGD(-1, "arg1 is nullptr");
        return;
    }
    mbCanUpdateSetting = MFALSE;
    PolicyLockParams* params = (PolicyLockParams*) arg1;
    MY_LOGD(-1, "HDRMode(%u) Zoom(%d) flash(%d) Battery(%d) Thermal(%d) ISO(%d) s(%d) m(%d)",
            (MUINT8)mHDRMode,
            miZoomRatio,
            mbUsingFlash,
            mBatteryCheck,
            mThermalCheck,
            miISO,
            mbIsMain2Suspend,
            mMain2SensorStatus);
    if(params != nullptr)
    {
        if(mbSkipMain2Check)
        {
            MY_LOGD(-1, "main2 force on");
            params->bUseDualCamShot = MTRUE;
            params->bIsMain2On = MTRUE;
            goto lbExit;
        }
        // check if main2 is active
        if(mMain2SensorStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY)
        {
            MY_LOGD(-1, "main2 is not active");
            params->bUseDualCamShot = MFALSE;
            params->bIsMain2On = MFALSE;
            goto lbExit;
        }
        else if(mMain2SensorStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
        {
            MY_LOGD(-1, "main2 is prepare to active");
            params->bUseDualCamShot = MFALSE;
            params->bIsMain2On = MTRUE;
            goto lbExit;
        }
        else
        {
            MY_LOGD(-1, "main2 is active");
            params->bUseDualCamShot = MTRUE;
            params->bIsMain2On = MTRUE;
            // check iso value, if ISO < getMain2OnIsoValue cannot use denoise shot
            if(miISO < getMain2OnIsoValue())
            {
                MY_LOGD(-1, "main2 is open, but iso value is less than open ISO %d", getMain2OnIsoValue());
                params->bUseDualCamShot = MFALSE;
            }
            goto lbExit;
        }
    }
lbExit:
    miLockCount ++;
    FUNCTION_OUT();
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamDenoisePolicy::
unlock(
    void* arg1
)
{
    FUNCTION_IN();
    Mutex::Autolock _l(mLock);
    miUnlockCount ++;
    MY_LOGD(-1, "unlockCnt(%d) lockCnt(%d)", miUnlockCount, miLockCount);
    if(miUnlockCount % 2 == 0)
    {
        MY_LOGD(-1, "doule Unlock check");
        miUnlockCount = 0;
        miLockCount --;
    }
    if(miLockCount == 0){
        MY_LOGD(-1, "can update");
        mbCanUpdateSetting = MTRUE;
    }
    FUNCTION_OUT();
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
DualCamDenoisePolicy::
getPreviewCameraId(MINT32 *dropOthers)
{
    if (dropOthers)
    {
        *dropOthers = 0;
    }
    return miMain1Id;
}

/******************************************************************************
 *
 ******************************************************************************/
void
DualCamDenoisePolicy::
setMain2DropDisplayRequest(
    MINT32 const openId,
    IMetadata* halMetadata
)
{
    typedef MTK_SYNC_CAEMRA_STATE STATE;
    MBOOL drop_all = MFALSE;
    MBOOL drop_p2  = MFALSE;

    if( mbIsMain2Suspend &&
        mMain2SensorStatus != STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE
    ){
        drop_all = MTRUE;
    }else{
        switch(mMain2SensorStatus){
            case STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY:
                drop_p2 = MTRUE;
                break;
            case STATE::MTK_SYNC_CAEMRA_STANDBY:
                drop_all = MTRUE;
                break;
            case STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE:
                drop_p2 = MTRUE;
                break;
            case STATE::MTK_SYNC_CAEMRA_ACTIVE:
                // need p1 + p2
                break;
            case STATE::MTK_SYNC_CAMERA_INIT:
                drop_p2 = MTRUE;
                break;
            default:
                MY_LOGE(openId, "unknown status:%d, drop all", mMain2SensorStatus);
                drop_all = MTRUE;
        }
    }

    MY_LOGD(openId, "main2 status(%d) susp(%d) drop_all(%d) drop_p2(%d)",
        mMain2SensorStatus, mbIsMain2Suspend, drop_all, drop_p2
    );

    if(drop_all){
        MY_LOGD_IF(mLogLevel>0, openId, "drop request");
        IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
        tag_drop.push_back(MTK_DUALZOOM_DROP_DIRECTLY, Type2Type<MINT32>());
        halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
    }else if(drop_p2){
        MY_LOGD_IF(mLogLevel>0, openId, "need p1");
        IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
        tag_drop.push_back(MTK_DUALZOOM_DROP_NEED_P1, Type2Type<MINT32>());
        halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
    }else{
        MY_LOGD_IF(mLogLevel>0, openId, "need p1 and p2");
        IMetadata::IEntry tag_drop(MTK_DUALZOOM_DROP_REQ);
        tag_drop.push_back(MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE, Type2Type<MINT32>());
        halMetadata->update(MTK_DUALZOOM_DROP_REQ, tag_drop);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamDenoisePolicy::
updateParams(
    SyncManagerParams &syncParams
)
{
    miZoomRatio = syncParams.miZoomRatio;
    mHDRMode = syncParams.mHDRMode;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCamDenoisePolicy::
changeCameraMode(
    MINT32  const               openId,
    MINT32                      type,
    MBOOL                       enableAAASync,
    MBOOL                       enableHWSync,
    MINT32                      minFps,
    MINT32                      maxFps,
    IMetadata*                  appMetadata,
    IMetadata*                  halMetadata)
{
    Frame3ASetting setting =
    {
        .openId   = openId,
        .is3ASyncEnabled = enableAAASync,
        .hwSyncMode = (MUINT32)enableHWSync,
        .minFps          = minFps,
        .maxFps          = maxFps,
    };
    if (mIFrame3AControl != NULL)
    {
        mIFrame3AControl->set(&setting, type, appMetadata, halMetadata);
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
DualCamDenoisePolicy::
updateStandbyMode(
    MINT32  const               openId,
    MINT32                      &setStatus,
    IMetadata*                  halMetadata)
{
    if (setStatus == MTK_P1_SENSOR_STATUS_HW_STANDBY)
    {
        IMetadata::IEntry tag(MTK_P1NODE_SENSOR_STATUS);
        tag.push_back(MTK_P1_SENSOR_STATUS_HW_STANDBY, Type2Type<MINT32>());
        halMetadata->update(MTK_P1NODE_SENSOR_STATUS, tag);
        setStatus = MTK_P1_SENSOR_STATUS_NONE;
        MY_LOGD(openId, "set streaming off");
        mbIsMain2Suspend = MTRUE;
    }
    else if (setStatus == MTK_P1_SENSOR_STATUS_STREAMING)
    {
        {
            IMetadata::IEntry tag(MTK_P1NODE_SENSOR_STATUS);
            tag.push_back(MTK_P1_SENSOR_STATUS_STREAMING, Type2Type<MINT32>());
            halMetadata->update(MTK_P1NODE_SENSOR_STATUS, tag);
        }
		//For fix AE resume issue
        //{
        //    IMetadata::IEntry tag(MTK_P1NODE_RESUME_SHUTTER_TIME_US);
        //    tag.push_back(10000, Type2Type<MINT32>()); // need ask 3a owner, set temp value first
        //    halMetadata->update(MTK_P1NODE_RESUME_SHUTTER_TIME_US, tag);
        //}
        setStatus = MTK_P1_SENSOR_STATUS_NONE;
        MY_LOGD(openId, "set streaming on");
        mbIsMain2Suspend = MFALSE;
    }
    return OK;
}
/****************************************setMasterCamera**************************************
 *
 ******************************************************************************/
status_t
DualCamDenoisePolicy::
setMasterCamera(
    MINT32  const               openId,
    IMetadata*                  halMetadata)
{
    Frame3ASetting setting =
    {
        .openId   = openId,
        .masterId = miMain1Id,
        .slaveId  = miMain2Id,
    };
    MINT32 type = F3A_TYPE_MASTER_SLAVE_SET;

    if (mIFrame3AControl != NULL)
    {
        mIFrame3AControl->set(&setting, type, NULL, halMetadata);
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
void
DualCamDenoisePolicy::
changeCameraState(
    MINT32  const               openId,
    DECISION_STATUS    const    canUsingMain2,
    IMetadata*                  appMetadata,
    IMetadata*                  halMetadata,
    MTK_SYNC_CAEMRA_STATE&      state
)
{
    auto stateToString = [](MTK_SYNC_CAEMRA_STATE &state)
    {
        switch(state)
        {
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY:
                return "GO_TO_STANDBY";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY:
                return "STANDBY";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE:
                return "GO_TO_ACTIVE";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE:
                return "ACTIVE";
            case MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT:
                return "INIT";
        }
        return "";
    };
    auto decision_to_string = [](DECISION_STATUS status)
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
    MY_LOGD(openId, "main2(%s) state(%s) mWaitEnterLP(%d) mWaitStable(%d) mWaitEnterAct(%d)",
                decision_to_string(canUsingMain2),
                stateToString(state),
                mWaitEnterLP,
                mWaitStable,
                mWaitEnterAct);
    //
    if(state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAMERA_INIT)
    {
        if(miISO != -1)
        {
            if(canUsingMain2 == DECISION_STATUS::MAIN2_ENABLE)
            {
                // prepare to active
                mSet3ASyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::ENABLE;
                //
                state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE;
                //
                mWaitEnterAct = DENOISE_WAIT_CAM_ACTIVE_COUNT;
                //
                mWaitStable = DENOISE_WAIT_STABLE_COUNT;
            }
            else if(canUsingMain2 == DECISION_STATUS::MAIN2_DISABLE)
            {
                // go to standby
                state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY;
                mWaitEnterLP = DENOISE_WAIT_STANDBY_COUNT;
                mSet3ASyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::DISABLE;
            }
        }
    }
    else if(state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY)
    {
        if(mWaitEnterLP == 0)
        {
            if(state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY)
            {
                MY_LOGD(openId, "standby done");
                state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY;
            }
        }
        else
        {
            mWaitEnterLP--;
            if(mWaitEnterLP == (DENOISE_WAIT_LOW_POWER_COUNT >> 1))
            {
                mSetMain2SensorStatus = MTK_P1_SENSOR_STATUS_HW_STANDBY;
            }
        }
    }
    else if(state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY)
    {
        if(mWaitStable > 0)
        {
            MY_LOGD(openId, "need wait stable (%d)", mWaitStable);
            mWaitStable--;
        }
        if(mWaitStable == 0 && (canUsingMain2 == DECISION_STATUS::MAIN2_ENABLE))
        {
            MY_LOGD(openId, "go to active");
            mSetMain2SensorStatus = MTK_P1_SENSOR_STATUS_STREAMING;
            // tele leave low power mode, enable wide 3A sync
            mSet3ASyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::ENABLE;
            //
            state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE;
            //
            mWaitEnterAct = DENOISE_WAIT_CAM_ACTIVE_COUNT;
            //
            mWaitStable = DENOISE_WAIT_STABLE_COUNT;
        }
    }
    else if (state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE)
    {
        if (mWaitEnterAct <= 0)
        {
            MY_LOGD(openId, "active done");
            state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE;
        }
        else
        {
            mWaitEnterAct--;
        }
    }
    //
    if(state == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
    {
        if(mWaitStable > 0)
        {
            MY_LOGD(openId, "need wait stable (%d)", mWaitStable);
            mWaitStable--;
        }
        //
        /*if(mWaitStable == 0 && mbInit)
        {
            MY_LOGD(openId, "init stage, enable hwsync & 3async");
            // tele leave low power mode, enable wide 3A sync
            mSet3ASyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::ENABLE;
            //
            state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE;
            //
            mWaitEnterAct = DENOISE_WAIT_CAM_ACTIVE_COUNT;
            //
            mWaitStable = DENOISE_WAIT_STABLE_COUNT;
            mbInit = MFALSE;
        }
        else */if(mWaitStable == 0 && (canUsingMain2 == DECISION_STATUS::MAIN2_DISABLE))
        {
            MY_LOGD(openId, "go to standby mode");
            state = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY;
            mSet3ASyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::DISABLE;
            mWaitEnterLP = DENOISE_WAIT_STANDBY_COUNT;
        }
    }

}

};
