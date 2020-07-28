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

#define LOG_TAG "MtkCam/DualCamZoomPolicy"

#include "DualCamZoomPolicy.h"

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>

#include <cutils/properties.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe_var.h>
#include <mtkcam/feature/3dnr/util_3dnr.h>
#include <mtkcam/feature/3dnr/lmv_state.h>

#include <mtkcam/feature/DualCam/DualCam.Common.h>
#include <mtkcam/feature/DualCam/utils/DualCameraUtility.h>


#include <mtkcam/drv/mem/cam_cal_drv.h>
// for policy
#include <mtkcam/utils/hw/CamManager.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
//#include <mtkcam/aaa/ISync3A.h>

#include "DualCamZoom/DualZoomPolicy_Common.h"

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#include <camera_custom_dualzoom.h>
#include <camera_custom_dualzoom_func.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#endif

using NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe;
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::VarMap;

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(id, fmt, arg...)    CAM_LOGV("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGD(id, fmt, arg...)    CAM_LOGD("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGI(id, fmt, arg...)    CAM_LOGI("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGW(id, fmt, arg...)    CAM_LOGW("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGE(id, fmt, arg...)    CAM_LOGE("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGA(id, fmt, arg...)    CAM_LOGA("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)
#define MY_LOGF(id, fmt, arg...)    CAM_LOGF("[%d]id:%d[%s] " fmt, mRefCount, id, __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_IN()             CAM_LOGI("[%d][%s]+", mRefCount, __FUNCTION__)
#define FUNCTION_OUT()            CAM_LOGI("[%d][%s]-", mRefCount, __FUNCTION__)

class my_scoped_tracer
{
/******************************************************************************
 *
 ******************************************************************************/
public:
    my_scoped_tracer(MINT32 openId, MINT32 refCount, const char* functionName)
    : mOpenId(openId)
    , mRefCount(refCount)
    , mFunctionName(functionName)
    {
        CAM_LOGD("[%d]id:%d[%s] +", mRefCount, mOpenId, mFunctionName);
    }
    ~my_scoped_tracer()
    {
        CAM_LOGD("[%d]id:%d[%s] -", mRefCount, mOpenId, mFunctionName);
    }
private:
    const MINT32        mOpenId;
    const MINT32        mRefCount;
    const char* const   mFunctionName;
};
#define MY_SCOPED_TRACER(id) my_scoped_tracer ___scoped_tracer(id, mRefCount, __FUNCTION__ );

namespace
{
    static constexpr char* sDualZoomThermalPolicyName = "thermal_policy_07";
}
/******************************************************************************
*
*******************************************************************************/
namespace NSCam
{
using Utility   = DualCameraUtility;
using Helper    = DualZoomPolicyHelper;
using Converter = DualZoomPolicyConverter;
using AAAHelper = Frame3AControlHelper;
//
/******************************************************************************
 * Initialize
 ******************************************************************************/
MINT32 DualCamZoomPolicy::sTotalRefCount = 0;
std::future<MVOID> DualCamZoomPolicy::sThermalPolicyBGWorker;
//
/******************************************************************************
 * DualCamZoomPolicy
 ******************************************************************************/
DualCamZoomPolicy::
DualCamZoomPolicy()
    : mRefCount(sTotalRefCount)
    , mIFrame3AControl(nullptr)
    , mPreviousZoomRatio(0)
    , mPreviewCameraOpenId(-1)
    , mSyncManagerCallback(nullptr)
    , mFovOnlineStatus(0)
    , mPauseAFCamId(-1)
    , mDoCapCamId(-1)
    , mResumeAFCount(0)
    , mLockStatus(false)
{
    MY_SCOPED_TRACER(-1);

    DualCamZoomPolicy::sTotalRefCount++;

    if(DualCamZoomPolicy::sThermalPolicyBGWorker.valid())
    {
        MY_LOGD(-1, "have share state");
        DualCamZoomPolicy::sThermalPolicyBGWorker.get();
    }

    auto turnOnThermalPolicy = [](){ Utils::CamManager::getInstance()->setThermalPolicy(const_cast<char*>(sDualZoomThermalPolicyName), 1); };
    DualCamZoomPolicy::sThermalPolicyBGWorker = std::async(std::launch::async, turnOnThermalPolicy);

    mCameraContexts[DUALZOOM_WIDE_CAM_ID] = CameraContext::getInvalidInstance(DUALZOOM_WIDE_CAM_ID);
    mCameraContexts[DUALZOOM_TELE_CAM_ID] = CameraContext::getInvalidInstance(DUALZOOM_TELE_CAM_ID);

    // for 3DNR/LMV
    LmvStateMachine::createInstance(LmvStateMachine::STATE_ON_WIDE);
}
//
/******************************************************************************
 *
 ******************************************************************************/
DualCamZoomPolicy::
~DualCamZoomPolicy()
{
    MY_SCOPED_TRACER(-1);

    // for 3DNR/LMV
    LmvStateMachine::destroyInstance();

    mCameraContexts.clear();
    mOpenIds.clear();

    if(DualCamZoomPolicy::sThermalPolicyBGWorker.valid())
    {
        MY_LOGD(-1, "have share state");
        DualCamZoomPolicy::sThermalPolicyBGWorker.get();
    }

    auto turnOffThermalPolicy = [](){ Utils::CamManager::getInstance()->setThermalPolicy(const_cast<char*>(sDualZoomThermalPolicyName), 0); };
    DualCamZoomPolicy::sThermalPolicyBGWorker = std::async(std::launch::async, turnOffThermalPolicy);

    DualCamZoomPolicy::sTotalRefCount--;

    mIFrame3AControl = nullptr;
    mSyncManagerCallback = nullptr;
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
setCameraOpenIds(vector<MINT32> ids)
{
    MY_SCOPED_TRACER(-1);

    Mutex::Autolock _l(mLock);

    MY_LOGD(-1, "argument openIds size: %lu", ids.size());

    mOpenIds.clear();
    for (MUINT32 i = 0; i < ids.size(); i++)
    {
        OpenId openId = ids[i];

        MRect activeArray;
        if (openId == DUALZOOM_WIDE_CAM_ID)
        {
            if(Helper::tryGetSensorActiveArray(openId, activeArray))
            {
                mCameraContexts[openId] = nullptr;

                mCameraContexts[openId] = CameraContext::createInstance(openId, activeArray);

                mPreviewCameraOpenId = openId;

                mOpenIds.push_back(openId);

                MY_LOGD(openId, "create wide camera context, Id: %d, address: %p", openId, mCameraContexts[openId].get());
            }
        }
        else if (openId == DUALZOOM_TELE_CAM_ID)
        {
            if(Helper::tryGetSensorActiveArray(openId, activeArray))
            {
                mCameraContexts[openId] = nullptr;

                mCameraContexts[openId] = CameraContext::createInstance(openId, activeArray);
                mOpenIds.push_back(openId);

                MY_LOGD(openId, "create tele camera context, Id: %d, address: %p", openId, mCameraContexts[openId].get());
            }
        }
        else
        {
            MY_LOGE(openId, "invalid camera openId: %d", openId);
        }
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
setObject(void* obj, MINT32 arg1)
{
    if (obj != nullptr)
    {
        mIFrame3AControl = reinterpret_cast<IFrame3AControl*>(obj);
        MY_LOGD(-1, "set frame3AControl: %p", mIFrame3AControl.get());
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
setParameter(std::string str)
{
    Mutex::Autolock _l(mLock);
    if (str.compare(VAR_DUALCAM_FOV_ONLINE) == 0)
    {
        MY_LOGD(-1, "not implement in this version");
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy
::addCallBack(sp<ISyncManagerCallback> Cb)
{
    mSyncManagerCallback = Cb;
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
updateSetting( MINT32 const i4OpenId, IMetadata* appMetadata, IMetadata* halMetadata, SyncManagerParams &syncParams)
{
    if(isInvalidOpenId(i4OpenId))
    {
        MY_LOGE(i4OpenId, "illegal follow, invalid camera openId");
        return;
    }

    Mutex::Autolock _l(mLock);

    MINT32 isDropReq = false;
    JudgeInfo info;

    MINT32 zoomRatio = 0;
    MRect crop(0, 0);

    sp<CameraContext> currentCameraContext;
    if (Utility::tryGetMetadata(*appMetadata, MTK_SCALER_CROP_REGION, crop))
    {
        currentCameraContext = mCameraContexts[i4OpenId];
        MRect activeArray = currentCameraContext->getActiveArray();

        zoomRatio = ((activeArray.s.w * 100)) / crop.s.w;
        Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_DUALZOOM_ZOOM_FACTOR, zoomRatio);

        MY_LOGD(i4OpenId, "openId: %d, crop info: (%d, %d, %d, %d), active array: (%d, %d, %d, %d), zoomRatio: %d",
            i4OpenId, crop.p.x, crop.p.y, crop.s.w, crop.s.h, activeArray.p.x, activeArray.p.y, activeArray.s.w, activeArray.s.h, zoomRatio);
    }
    else
    {
        MY_LOGE(i4OpenId, "illegal follow, failed to get the MTK_SCALER_CROP_REGION");
        return;
    }

    if (syncParams.mPartialUpdate)
    {
        MY_LOGD(i4OpenId, "do partial update settings.");
    }
    else
    {
        MTK_SYNC_CAEMRA_STATE wideStatus = mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getStatus();
        MTK_SYNC_CAEMRA_STATE teleStatus = mCameraContexts[DUALZOOM_TELE_CAM_ID]->getStatus();
        if ((wideStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE) && (teleStatus!= MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE))
        {
            MY_LOGE(i4OpenId, "illegal follow, no cameras are active!!");
            return;
        }

        MY_LOGD(i4OpenId, "current status(01), wide: %s(%d), tele: %s(%d)", Converter::statusNameMap[wideStatus],
            static_cast<MINT32>(wideStatus), Converter::statusNameMap[teleStatus], static_cast<MINT32>(teleStatus));

        if (mLockStatus)
        {
            if(i4OpenId == mPreviewCameraOpenId)
            {
                if (currentCameraContext->getStatus() != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
                {
                    MY_LOGE(i4OpenId, "illegal follow, please check!! previewCamID: %d, state: %d", mPreviewCameraOpenId, static_cast<MINT32>(currentCameraContext->getStatus()));
                    return;
                }

                isDropReq = false;
                MY_LOGD(i4OpenId, "lock status: previewCamID: %d, wide state: %d, tele state: %d",
                    mPreviewCameraOpenId, static_cast<MINT32>(wideStatus), static_cast<MINT32>(teleStatus));
            }
            else
            {
                isDropReq = true;
                MY_LOGD(i4OpenId, "lock status, not preview camera and drop request");

            }
        }
        else
        {
            // (1) determine the status, that maybe transit the status, and will decision the drop type
            currentCameraContext->handleUpateSettings(appMetadata, halMetadata, mIFrame3AControl);
            {
                MTK_SYNC_CAEMRA_STATE newWideStatus = mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getStatus();
                MTK_SYNC_CAEMRA_STATE newTeleStatus = mCameraContexts[DUALZOOM_TELE_CAM_ID]->getStatus();
                if((wideStatus != newWideStatus) || (teleStatus != newTeleStatus))
                {
                    wideStatus = newWideStatus;
                    teleStatus = newTeleStatus;
                    MY_LOGD(i4OpenId, "current status(02), wide: %s(%d), tele: %s(%d)",
                        Converter::statusNameMap[wideStatus], static_cast<MINT32>(wideStatus),
                        Converter::statusNameMap[teleStatus], static_cast<MINT32>(teleStatus));
                }
            }

            // (2) determine the zoom ratio is changed
            MBOOL isZoomRatioChange = false;
            if (mPreviousZoomRatio != syncParams.miZoomRatio)
            {
                MY_LOGD(i4OpenId, "zoom ratio change prev/ curr: %d /%d", mPreviousZoomRatio, syncParams.miZoomRatio);
                mPreviousZoomRatio = syncParams.miZoomRatio;
                isZoomRatioChange = true;
            }

            // (3) 3A judge
            currentCameraContext->handleAAAJudge(mIFrame3AControl);
            MBOOL isAAAForceCamId = false;
            MINT32 aaaJudgeOpenIdFromWide  = mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getAAAJudgeOpenId();
            MINT32 aaaJudgeOpenIdFromTele  = mCameraContexts[DUALZOOM_TELE_CAM_ID]->getAAAJudgeOpenId();
            isAAAForceCamId = (aaaJudgeOpenIdFromWide == i4OpenId) || (aaaJudgeOpenIdFromTele == i4OpenId);
            MY_LOGD(i4OpenId, "force 3A camera open Id, wide: %d, tele: %d, isAAAForceCamId: %d",
                aaaJudgeOpenIdFromWide, aaaJudgeOpenIdFromTele, isAAAForceCamId);

            // (4) zoom drop
            MBOOL isZoomRatioRange = false;
            if (i4OpenId == DUALZOOM_WIDE_CAM_ID)
            {
                isZoomRatioRange = syncParams.miZoomRatio < DualZoomPolicySettings::switchCameraZoomRatio;
                MY_LOGD(i4OpenId, "isZoomRatioRange: %d(%d < %d)", isZoomRatioRange, syncParams.miZoomRatio, DualZoomPolicySettings::switchCameraZoomRatio);
            }
            else if (i4OpenId == DUALZOOM_TELE_CAM_ID)
            {
                isZoomRatioRange = syncParams.miZoomRatio >= DualZoomPolicySettings::switchCameraZoomRatio;
                MY_LOGD(i4OpenId, "isZoomRatioRange: %d(%d >= %d)", isZoomRatioRange, syncParams.miZoomRatio, DualZoomPolicySettings::switchCameraZoomRatio);
            }

            // (5) wake up for un-stable condition
            {
                MBOOL isWakeup = (isZoomRatioChange || isZoomRatioRange || isAAAForceCamId);
                MY_LOGD(i4OpenId, "isZoomRatioChange: %d, isZoomRatioRange: %d,  isAAAForceCamId: %d, isWakeup: %d",
                    isZoomRatioChange, isZoomRatioRange, isAAAForceCamId , isWakeup);

                if(isWakeup)
                {
                    // in wakeup, just reset count or notify next the same openId request to transit status
                    currentCameraContext->wakeup(appMetadata, halMetadata, mIFrame3AControl);
                }
            }

            // determine the request is drop or not
            info.ZoomDrop = !isZoomRatioRange;
            info.AAAJudgeIdFromWide = aaaJudgeOpenIdFromWide;
            info.AAAJudgeIdFromTele = aaaJudgeOpenIdFromTele;
            info.is4KVideo = syncParams.mb4KVideo;
            isDropReq = determineDropRequest(i4OpenId, info);
        }

        setDropMode(i4OpenId, halMetadata, isDropReq);

        // ************************** on-line calibratin ********************************
        if ((wideStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE) && (teleStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE))
        {
            do {
                if (ForceFlow::isForceNotDoOnlineFov())
                {
                    MY_LOGD(i4OpenId, "force not do fov online frame sync");
                    break;
                }

                if(!ForceFlow::isForceNoCheckOnlineFovResult())
                {
                    if(!checkDoingOnlineFOV(i4OpenId, syncParams.miZoomRatio, syncParams.mIsVideoRec))
                    {
                        MY_LOGD(i4OpenId, "check doing online fov failed");
                        break;
                    }
                }
                else
                {
                    MY_LOGD(i4OpenId, "force no check online fov result");
                }

                if (ForceFlow::isForceNoFrameSync())
                {
                    MY_LOGD(i4OpenId, "force no framce sync");
                    break;
                }

                MY_LOGD(i4OpenId, "start frame sync to do online fov");
                Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_DUALZOOM_DO_FRAME_SYNC, 1);

                if (isDropReq && mIFrame3AControl.get())
                {
                    Frame3ASetting_t setting =
                    {
                        .openId   = i4OpenId,
                        .dropMode = MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE,
                    };
                    mIFrame3AControl->set(&setting, F3A_TYPE_DROP_MODE, nullptr, halMetadata);
                }
            } while (0);

            MY_LOGD(i4OpenId, "enable frame sync");
            AAAHelper::enableFrameSync(i4OpenId, appMetadata, halMetadata, mIFrame3AControl);
        }
        else
        {
            MY_LOGD(i4OpenId, "disable frame sync");
            AAAHelper::disableFrameSync(i4OpenId, appMetadata, halMetadata, mIFrame3AControl);
            mFovOnlineStatus = 0;
        }
        // ************************** END on-line calibratin ********************************


        // ************************** fix strobe issue ********************************
        if ((wideStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID) && (teleStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID))
        {
            if (mResumeAFCount > 0 && mDoCapCamId == i4OpenId)
            {
                mResumeAFCount--;
            }

            MUINT8 aePrecapture = -1;
            if(!Utility::tryGetMetadata(*appMetadata, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, aePrecapture))
            {
                if(aePrecapture == MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START)
                {
                    mPauseAFCamId = Helper::getCorrespondingCameraOpenId(i4OpenId);
                    mDoCapCamId = i4OpenId;
                    mResumeAFCount = 30;
                    MY_LOGD(i4OpenId, "strobe trace - 01");
                }
            }

            if ((mPauseAFCamId == i4OpenId) && (mResumeAFCount == DualZoomPolicySettings::totalResumeAFCount))
            {
                Utility::setMetadataAndUpdate<MUINT8>(*halMetadata, MTK_FOCUS_PAUSE, 1);
                MY_LOGD(i4OpenId, "strobe trac- 02");
            }

            if ((mPauseAFCamId == i4OpenId) && (mResumeAFCount == 0))
            {
                Utility::setMetadataAndUpdate<MUINT8>(*halMetadata, MTK_FOCUS_PAUSE, 0);
                mPauseAFCamId = -1;
                mDoCapCamId = -1;
                mResumeAFCount = 0;
                MY_LOGD(i4OpenId, "strobe trace - 03");
            }
        }


        if (isDropReq)
        {
            /*// for strop
            IMetadata::IEntry entry = appMetadata->entryFor(MTK_CONTROL_AE_MODE);
            entry.push_back(MTK_CONTROL_AE_MODE_ON, Type2Type<MINT32>());
            appMetadata->update(MTK_CONTROL_AE_MODE, entry);*/

            MY_LOGD(i4OpenId, "request is dropped");
        }
        else
        {
            if ((wideStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID) && (teleStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID))
            {
                // change the master camera
                AAAHelper::setMasterCamera(i4OpenId, Helper::getCorrespondingCameraOpenId(i4OpenId), appMetadata, halMetadata, mIFrame3AControl);
                if (i4OpenId != mPreviewCameraOpenId)
                {
                    MY_LOGD(i4OpenId, "switch main camera : %d -> %d", mPreviewCameraOpenId, i4OpenId);
                    mPreviewCameraOpenId = i4OpenId;
                    if (mSyncManagerCallback != nullptr)
                    {
                        mSyncManagerCallback->onEvent(mPreviewCameraOpenId, MTK_SYNCMGR_MSG_NOTIFY_MASTER_ID, 0, nullptr);
                    }
                }

                // for strop
                if (i4OpenId == mPauseAFCamId)
                {
                    Utility::setMetadataAndUpdate<MUINT8>(*halMetadata, MTK_FOCUS_PAUSE, 0);
                    mPauseAFCamId = -1;
                    mDoCapCamId = -1;
                    mResumeAFCount = 0;
                    MY_LOGD(i4OpenId, "strobe trace - 04");
                }
            }
        }
        // ************************** End fix strobe issue ********************************


        // ************************** 3DNR LMV, per frame setting Begin *******************
        // ask LmvStateMachine to check
        LmvStateMachine* lmv = LmvStateMachine::getInstance();

        if (lmv != nullptr && mOpenIds.size() > 1)
        {
            MBOOL doSend = false;

            if ((i4OpenId == DUALZOOM_WIDE_CAM_ID && lmv->getCurrentState() == LmvStateMachine::STATE_ON_WIDE) ||
                (i4OpenId == DUALZOOM_TELE_CAM_ID && lmv->getCurrentState() == LmvStateMachine::STATE_ON_TELE))
            {
                LmvStateMachine::SensorId id = (mPreviewCameraOpenId == DUALZOOM_WIDE_CAM_ID) ? LmvStateMachine::WIDE : LmvStateMachine::TELE;

                LmvStateMachine::Action act;
                lmv->switchTo(id, &act);

                if (LmvStateMachine::DO_NOTHING != act.cmd)
                {
                    MY_LOGD(i4OpenId, "id = %d, act.cmd = %d, i4OpenId = %d", id, act.cmd, i4OpenId);
                    doSend = true;

                    MY_LOGD(i4OpenId, "add MTK_NR_FEATURE_LMV_SEND_SWITCH_OUT on CamID %d", i4OpenId);
                    Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_LMV_SEND_SWITCH_OUT, 1);
                }
            }

            // double check per-frame for retry (to trigger driver)
            if (!doSend)
            {
                LmvStateMachine::SensorId id =
                    (i4OpenId == DUALZOOM_WIDE_CAM_ID) ? LmvStateMachine::WIDE : LmvStateMachine::TELE;
                doSend = lmv->needEnque(id, true);
            }

            if (doSend)
            {
                // avoid drop frame for 3DNR LMV state machine
                // dummy frame
                if (isDropReq)
                {
                    MINT32 dropValue = MTK_DUALZOOM_DROP_NONE;
                    Utility::tryGetMetadata<MINT32>(*halMetadata, MTK_DUALZOOM_DROP_REQ, dropValue);

                    if (dropValue > MTK_DUALZOOM_DROP_DIRECTLY)
                    {
                        MY_LOGD(i4OpenId, "add MTK_DUALZOOM_DROP_NEED_SYNCMGR on CamID %d", i4OpenId);

                        if (mIFrame3AControl.get() != nullptr)
                        {
                            Frame3ASetting_t setting =
                            {
                                .openId = i4OpenId,
                                .dropMode = MTK_DUALZOOM_DROP_NEED_SYNCMGR,
                            };

                            mIFrame3AControl->set(&setting, F3A_TYPE_DROP_MODE, nullptr, halMetadata);
                        }
                        else
                        {
                            Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_DUALZOOM_DROP_REQ, MTK_DUALZOOM_DROP_NEED_SYNCMGR);
                        }
                    }
                }
            }
        }

        MINT32 nrFeature3dnrMode = (mPreviewCameraOpenId == i4OpenId) ? MTK_NR_FEATURE_3DNR_MODE_ON : MTK_NR_FEATURE_3DNR_MODE_OFF;
        Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_DUALZOOM_3DNR_MODE, nrFeature3dnrMode);
        // ************************** END 3DNR LMV, per frame setting *******************

        if (syncParams.miZoomRatio != 0)
        {
            Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_DUALZOOM_ZOOMRATIO, syncParams.miZoomRatio);
        }
    }

    // partial update setting
    // fov rect info
    {
        MRect fovCrop;
        MSize marginSize = MSize(0, 0);
        if(Utility::tryGetMetadata(*halMetadata, MTK_DUALZOOM_FOV_MARGIN_PIXEL, marginSize))
        {
            marginSize.h = marginSize.w * crop.s.h / crop.s.w;
        }

        if ((marginSize.w != 0) && (marginSize.h != 0))
        {
            if (crop.s.w > marginSize.w)
            {
                fovCrop.s.w = crop.s.w - marginSize.w;
                fovCrop.s.h = crop.s.h - marginSize.h;
                fovCrop.s.w = fovCrop.s.w + (fovCrop.s.w & 0x01);
                fovCrop.s.h = fovCrop.s.h + (fovCrop.s.h & 0x01);
            }
            Utility::setMetadataAndUpdate<MSize>(*halMetadata, MTK_DUALZOOM_FOV_MARGIN_PIXEL, marginSize);

            IMetadata::IEntry fovRectInfoEntry(MTK_DUALZOOM_FOV_RECT_INFO);
            //
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getActiveArray().s.w);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getActiveArray().s.h);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_TELE_CAM_ID]->getActiveArray().s.w);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_TELE_CAM_ID]->getActiveArray().s.h);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, crop.s.w);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, crop.s.h);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, fovCrop.s.w);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, fovCrop.s.h);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, crop.p.x + (crop.s.w >> 1));
            Utility::setMetadata<MINT32>(fovRectInfoEntry, crop.p.y + (crop.s.h >> 1));
            // AF data
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getAFDac());
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_TELE_CAM_ID]->getAFDac());
            Utility::setMetadata<MINT32>(fovRectInfoEntry, 1);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, 1);
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getIsAFDone());
            Utility::setMetadata<MINT32>(fovRectInfoEntry, mCameraContexts[DUALZOOM_TELE_CAM_ID]->getIsAFDone());
            //
            Utility::updateMetadata(*halMetadata, fovRectInfoEntry);

            MY_LOGD(i4OpenId, "crop : %dx%d, fovCrop : %dx%d", crop.s.w, crop.s.h, fovCrop.s.w, fovCrop.s.h);
        }
        // fov calibration
        {
            IMetadata::IEntry fovCalbInfoEntry(MTK_DUALZOOM_FOV_CALB_INFO);
            Utility::setMetadata<MINT32>(fovCalbInfoEntry, mCameraContexts[i4OpenId]->getActiveArray().s.w);
            Utility::setMetadata<MINT32>(fovCalbInfoEntry, mCameraContexts[i4OpenId]->getActiveArray().s.h);
            Utility::setMetadata<MINT32>(fovCalbInfoEntry, crop.s.w);
            Utility::setMetadata<MINT32>(fovCalbInfoEntry, crop.s.h);
            Utility::setMetadata<MINT32>(fovCalbInfoEntry, zoomRatio);
            Utility::updateMetadata(*halMetadata, fovCalbInfoEntry);
        }
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
updateAfterP1(MINT32 const i4OpenId, MUINT32 const frameNo, void* obj, MINT32 arg1)
{
    if(isInvalidOpenId(i4OpenId))
    {
        MY_LOGE(i4OpenId, "illegal follow, invalid camera openId");
    }
    else if (obj == nullptr)
    {
        MY_LOGE(i4OpenId, "invalid argument, obj is nullptr");
    }
    else
    {
        FeaturePipeParam* pipeParam = (FeaturePipeParam*)obj;

        // update for 3DNR/LMV
        LmvStateMachine* lmv = LmvStateMachine::getInstance();
        if ((lmv != nullptr) && (mOpenIds.size() > 0))
        {
            LmvStateMachine::SensorId id = LmvStateMachine::SENSOR_ID_MAX;
            if (i4OpenId == DUALZOOM_WIDE_CAM_ID)
            {
                id = LmvStateMachine::WIDE;
            }
            else if(i4OpenId == DUALZOOM_TELE_CAM_ID)
            {
                id = LmvStateMachine::TELE;
            }
            else
            {
                MY_LOGE(i4OpenId, "invalid camera openId: %d", i4OpenId);
            }

            if(id != LmvStateMachine::SENSOR_ID_MAX)
            {
                MINT32 lmv_result = -1;
                lmv_result = pipeParam->mVarMap.get<MINT32>(VAR_LMV_SWITCH_OUT_RST, -1);
                if (lmv_result >= 0)
                {
                    lmv->notifySwitchResult(id, (LmvStateMachine::SwitchResult)lmv_result);
                }

                MINT32 lmv_validity = -1;
                lmv_validity = pipeParam->mVarMap.get<MINT32>(VAR_LMV_VALIDITY, -1);
                if (lmv_validity >= 0)
                {
                    lmv->notifyLmvValidity(id, lmv_validity);
                }
            }
        }

        Mutex::Autolock _l(mLock);
        {
            sp<CameraContext> currentCameraContext = mCameraContexts[i4OpenId];
            currentCameraContext->handleUpateSettingsResult(pipeParam->mVarMap);
        }
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
updateFeatureMask(MUINT32 &featureMask)
{
#if (MTKCAM_HAVE_DUAL_ZOOM_VENDOR_FOV_SUPPORT==1)
    NSCamFeature::NSFeaturePipe::ENABLE_VENDOR_FOV(featureMask);
#else
    NSCamFeature::NSFeaturePipe::ENABLE_FOV(featureMask);
#endif // MTKCAM_HAVE_DUAL_ZOOM_VENDOR_FOV_SUPPORT
}
//
/******************************************************************************
 *
 ******************************************************************************/
MINT32
DualCamZoomPolicy::
getPreviewCameraId(MINT32 *dropOthers)
{
    if (dropOthers)
    {
        if (!ForceFlow::isForceNoDrop())
        {
            *dropOthers = 1;
        }
        else
        {
            *dropOthers = 0;
        }
    }
    else
    {
        MY_LOGD(-1, "skip drop other");
    }
    return mPreviewCameraOpenId;
}
//
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamZoomPolicy::
determineDropRequest(MINT32 openId, JudgeInfo info)
{
    MBOOL ret = false;

    const MTK_SYNC_CAEMRA_STATE wideStatus = mCameraContexts[DUALZOOM_WIDE_CAM_ID]->getStatus();
    const MTK_SYNC_CAEMRA_STATE teleStatus = mCameraContexts[DUALZOOM_TELE_CAM_ID]->getStatus();
    if (openId == DUALZOOM_WIDE_CAM_ID)
    {
        if (wideStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
        {
            ret = true;
            MY_LOGD(openId, "drop, wide is not active, status: %s(%u)", Converter::statusNameMap[wideStatus], static_cast<MUINT32>(wideStatus));
        }
        else if (teleStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
        {
            if ((teleStatus == MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID) && info.ZoomDrop)
            {
                ret = true;
                MY_LOGD(openId, "drop, wide is active, tele is invalid and wide is in zoom drop");
            }
            else
            {
                ret = false;
                MY_LOGD(openId, "not drop, wide is active, tele is not invalid or wide is not in zoom drop");
            }
        }
        else if (info.is4KVideo)
        {
            ret = false;
            MY_LOGD(openId, "not drop, is4KVideo for wide");
        }
        else if ((info.AAAJudgeIdFromWide != -1) || (info.AAAJudgeIdFromTele != -1))
        {
            if ((info.AAAJudgeIdFromWide == openId) || (info.AAAJudgeIdFromTele == openId))
            {
                ret = false;
                MY_LOGD(openId, "no drop, satisfy 3A judge for wide");
            }
            else
            {
                ret = true;
                MY_LOGD(openId, "drop, not satisfy 3A judge for wide");
            }
        }
        else if (info.ZoomDrop)
        {
            MY_LOGD(openId, "drop, satisfy zoom drop for wide");
            ret = true;
        }
    }
    else if (openId == DUALZOOM_TELE_CAM_ID)
    {
        if (wideStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
        {
            ret = false;
            MY_LOGD(openId, "no drop, wide is not active, status: %s(%u)", Converter::statusNameMap[wideStatus], static_cast<MUINT32>(wideStatus));
        }
        else if (teleStatus != MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
        {
            ret = true;
            MY_LOGD(openId, "drop, tele is not active, status: %s(%u)", Converter::statusNameMap[teleStatus], static_cast<MUINT32>(teleStatus));
        }
        else if (info.is4KVideo)
        {
            ret = true;
            MY_LOGD(openId, "drop, is4KVideo for tele");
        }
        else if ((info.AAAJudgeIdFromWide != -1) || (info.AAAJudgeIdFromTele != -1))
        {
            if ((info.AAAJudgeIdFromWide == openId) || (info.AAAJudgeIdFromTele == openId))
            {
                ret = false;
                MY_LOGD(openId, "no drop, satisty 3A judgeg for tele");
            }
            else
            {
                ret = true;
                MY_LOGD(openId, "drop, no satisty 3A judge for tele");
            }
        }
        else if (info.ZoomDrop)
        {
            ret = true;
            MY_LOGD(openId, "drop, zoom drop for tele");
        }
    }
    return ret;
}
//
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamZoomPolicy::
checkDoingOnlineFOV(OpenId openId, MUINT32 zoomRatio, MINT32 isRecording)
{
    MINT32 ret = MTRUE;
    Frame3ASetting_t setting =
    {
        .openId   = openId
    };

    if (isRecording || mLockStatus)
        return MFALSE;

    if (openId == DUALZOOM_TELE_CAM_ID)
    {
        goto lbEXIT;
    }

    {
        // 0. check zoom ratio
        MINT32 thresholdZoomRatio = OnlineFovThreshold::getZoomRatio();
        if (zoomRatio > (MUINT32)thresholdZoomRatio) {
            MY_LOGD(openId, "zoomRatio(%d) too large (>%d)", zoomRatio, 1000);
            ret = MFALSE;
            goto lbEXIT;
        }
    }
    //
    if (mIFrame3AControl != nullptr)
    {
        mIFrame3AControl->get(&setting, F3A_TYPE_3A_INFO | F3A_TYPE_FOV_ONLINE_INFO);
    }

    {
        // 1. check thermal
        MINT32 thresholdThermal = OnlineFovThreshold::getThermal();
        if (setting.temperature > thresholdThermal) {
            MY_LOGD(openId, "temperature(%d) too high (>%d)", setting.temperature, thresholdThermal);
            ret = MFALSE;
            goto lbEXIT;
        }
    }
    //
    if(!Helper::getMultiDistanceFovSupported())
    {
        // 2. check DAC
        MINT32 thresholdDAC;
        MINT32 normalized_value;
        if (openId == DUALZOOM_WIDE_CAM_ID)
        {
            thresholdDAC = OnlineFovThreshold::getWideDAC();
        }
        else if(openId == DUALZOOM_TELE_CAM_ID)
        {
            thresholdDAC = OnlineFovThreshold::getTeleDAC();
        }
        else
        {
            MY_LOGE(openId, "invalid camera openId: %d", openId);
        }

        if (setting.maxDac - setting.minDac == 0) {
            MY_LOGE(openId, "maxDac == minDac (%d), something wrong", setting.maxDac);
        } else {
            normalized_value = (setting.AFDAC - setting.minDac) * 1000 / (setting.maxDac - setting.minDac);
            if (normalized_value >= thresholdDAC) {
                MY_LOGD(openId, "dac(%d) too high (>%d)", normalized_value, thresholdDAC);
                ret = MFALSE;
                goto lbEXIT;
            }
        }
    }
    //
    {
        // 3. check ISO
        MINT32 thresholdISO = OnlineFovThreshold::getIso();
        if (setting.isoValue >= thresholdISO) {
            MY_LOGD(openId, "iso(%d) too high (>%d)", setting.isoValue, thresholdISO);
            ret = MFALSE;
            goto lbEXIT;
        }
    }
    //
    {
        // 4. check exposure time
        MINT32 thresholdExpTime = OnlineFovThreshold::getExpTime();
        if (setting.expTime > thresholdExpTime) {
            MY_LOGD(openId, "exposure time(%d) too long (%d)", setting.expTime, thresholdExpTime);
            ret = MFALSE;
        }
    }
    //
lbEXIT:
    if (openId == DUALZOOM_WIDE_CAM_ID) {
        mFovOnlineStatus = ret ? (mFovOnlineStatus | DUALCAM_FOV_ONLINE_WIDE) : (mFovOnlineStatus & ~DUALCAM_FOV_ONLINE_WIDE);
    }
    MY_LOGD(openId, "mFovOnlineStatus(%d)", mFovOnlineStatus);
    if (mFovOnlineStatus & DUALCAM_FOV_ONLINE_WIDE)
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
lock(void* arg1)
{
    Mutex::Autolock _l(mLock);
    mLockStatus = true;
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
unlock(void* arg1)
{
    Mutex::Autolock _l(mLock);
    mLockStatus = false;
}
//
/******************************************************************************
 *
 ******************************************************************************/
MVOID
DualCamZoomPolicy::
setDropMode(OpenId openId, IMetadata* halMetadata, MBOOL isDrop)
{
    if(isInvalidOpenId(openId))
    {
        MY_LOGE(openId, "illegal follow, invalid camera openId");
    }
    if (ForceFlow::isForceNoDrop())
    {
        MY_LOGD_IF((Helper::getLogLevel() > 2), openId, "force no drop request");
    }
    else if (mIFrame3AControl == nullptr)
    {
        MY_LOGE(openId, "frame3AControl is nullptr, drop directly");
        Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_DUALZOOM_DROP_REQ, MTK_DUALZOOM_DROP_DIRECTLY);
    }
    else
    {
        const DropMode dropMode = mCameraContexts[openId]->getDropMode();
        if (isDrop && (dropMode == MTK_DUALZOOM_DROP_NONE))
        {
            Frame3ASetting_t setting =
            {
                .openId   = openId,
                .dropMode = MTK_DUALZOOM_DROP_NEED_P1,
            };
            mIFrame3AControl->set(&setting, F3A_TYPE_DROP_MODE, nullptr, halMetadata);

            MY_LOGD(openId, "drop request and drop type is none, set drop after P1");
        }
        else
        {
            Frame3ASetting_t setting =
            {
                .openId   = openId,
                .dropMode = dropMode,
            };
            mIFrame3AControl->set(&setting, F3A_TYPE_DROP_MODE, nullptr, halMetadata);

            MY_LOGD(openId, "set drop mode: %s(%d)", Converter::dropModeNameMap[dropMode], dropMode);
        }
    }
}
//
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DualCamZoomPolicy::
isInvalidOpenId(OpenId openId) const
{
    MBOOL valid = (openId == DUALZOOM_WIDE_CAM_ID) || (openId == DUALZOOM_TELE_CAM_ID);
    return !valid;
}
//
/******************************************************************************
 *
 ******************************************************************************/
} // NSCam
