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
/******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "MtkCam/DualCamZoomPolicy/CameraContext"

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// #include <camera_custom_dualzoom.h>

// mtkcam global header file

// Module header file
#include <mtkcam/feature/DualCam/utils/DualCameraUtility.h>

// Local header file
#include "CameraContext.h"


/******************************************************************************
*
*******************************************************************************/
#define COMMON_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGA(fmt, arg...)    CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define COMMON_LOGF(fmt, arg...)    CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define COMMON_FUN_IN()             COMMON_LOGD("+")
#define COMMON_FUN_OUT()            COMMON_LOGD("-")

class scoped_tracer
{
public:
    scoped_tracer(const char* functionName)
    : mFunctionName(functionName)
    {
        CAM_LOGD("[%s] +", mFunctionName);
    }
    ~scoped_tracer()
    {
        CAM_LOGD("[%s] -", mFunctionName);
    }
private:
    const char* const mFunctionName;
};
#define COMMON_SCOPED_TRACER() scoped_tracer ___scoped_tracer(__FUNCTION__ );


/******************************************************************************
*
*******************************************************************************/
#define CAM_CONTEXT_LOGV(fmt, arg...)       CAM_LOGV("[%s][%d] " fmt, __FUNCTION__, getOpenId(), ##arg)
#define CAM_CONTEXT_LOGD(fmt, arg...)       CAM_LOGD("[%s][%d] " fmt, __FUNCTION__, getOpenId(), ##arg)
#define CAM_CONTEXT_LOGI(fmt, arg...)       CAM_LOGI("[%s][%d] " fmt, __FUNCTION__, getOpenId(), ##arg)
#define CAM_CONTEXT_LOGW(fmt, arg...)       CAM_LOGW("[%s][%d] " fmt, __FUNCTION__, getOpenId(), ##arg)
#define CAM_CONTEXT_LOGE(fmt, arg...)       CAM_LOGE("[%s][%d] " fmt, __FUNCTION__, getOpenId(), ##arg)
#define CAM_CONTEXT_LOGA(fmt, arg...)       CAM_LOGA("[%s][%d] " fmt, __FUNCTION__, getOpenId(), ##arg)
#define CAM_CONTEXT_LOGF(fmt, arg...)       CAM_LOGF("[%s][%d] " fmt, __FUNCTION__, getOpenId(), ##arg)

#define CAM_CONTEXT_FUN_IN()                CAM_CONTEXT_LOGD("+")
#define CAM_CONTEXT_FUN_OUT()               CAM_CONTEXT_LOGD("-")

class cam_context_scoped_tracer
{
public:
    cam_context_scoped_tracer(MINT32 openId, const char* functionName)
    : mOpenId(openId)
    , mFunctionName(functionName)
    {
        CAM_LOGD("[%s][%d] +", mFunctionName, mOpenId);
    }
    ~cam_context_scoped_tracer()
    {
        CAM_LOGD("[%s][%d] -", mFunctionName, mOpenId);
    }
private:
    const MINT32        mOpenId;
    const char* const   mFunctionName;
};
#define CAM_CONTEXT_SCOPED_TRACER() cam_context_scoped_tracer ___scoped_tracer(getOpenId(), __FUNCTION__ );


/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{
using Utility   = DualCameraUtility;
using Helper    = DualZoomPolicyHelper;
using Converter = DualZoomPolicyConverter;
/******************************************************************************
 * CameraContext
 ******************************************************************************/
sp<CameraContext>
CameraContext::
createInstance(OpenId openId, const MRect& activeArray)
{
    COMMON_SCOPED_TRACER();

    sp<CameraContext> ret = new CameraContext(openId, activeArray);

    // status
    sp<CameraStatusBase> cameraStatus = nullptr;
    if(Helper::getEnableHWControlFlow())
    {
        cameraStatus = ActiveCameraStatus::createInstance(openId);
        COMMON_LOGD("enable hw control flow, create ActiveCameraStatus");
    }
    else
    {
        cameraStatus = ActiveCameraStatusForTest::createInstance(openId);
        COMMON_LOGD("disable hw control flow, create ActiveCameraStatusForTest");
    }
    ret->setStatus(cameraStatus);

    // aaa judge strategy
    sp<AAAJudgeStrategy> strategy = nullptr;
    if(openId == DUALZOOM_WIDE_CAM_ID)
    {
        strategy = WideAAAJudgeStrategy::createInstance();
    }
    else if(openId == DUALZOOM_TELE_CAM_ID)
    {
        strategy = TeleAAAJudgeStrategy::createInstance();
    }
    else
    {
        COMMON_LOGE("invalid camera openId: %d", openId);
        strategy = DefauleAAAJudgeStrategy::createInstance(openId);
    }
    ret->setAAAJudgeStrategy(strategy);

    return ret;
}
//
sp<CameraContext>
CameraContext::
getInvalidInstance(OpenId openId)
{
    COMMON_SCOPED_TRACER();

    sp<CameraContext> ret = new CameraContext(openId, MRect(0, 0));
    return ret;
}
//
MTK_SYNC_CAEMRA_STATE
CameraContext::
getStatus(OpenId dstOpenId)
{
    return CameraContextManager::getInstance()->getStatus(dstOpenId);
}
//
wp<CameraContext>
CameraContext::
getCameraContext(OpenId dstOpenId)
{
    return CameraContextManager::getInstance()->getCameraContext(dstOpenId);
}
//
CameraContext::
CameraContext(OpenId openId, const MRect& activeArray)
: mOpenId(openId)
, mActiveArray(activeArray)
{
    CAM_CONTEXT_SCOPED_TRACER();

    mCameraStatus = InvalidCameraStatus::createInstance(getOpenId());
    mAAAJudgeStrategy = DefauleAAAJudgeStrategy::createInstance(getOpenId());

    CameraContextManager::getInstance()->add(getOpenId(), this);
}
//
OpenId
CameraContext::
getOpenId() const
{
    return mOpenId;
}
//
MRect
CameraContext::
getActiveArray() const
{
    return mActiveArray;
}
//
DropMode
CameraContext::
getDropMode() const
{
    return mCameraStatus->getDropMode();
}
//
OpenId
CameraContext::
getAAAJudgeOpenId() const
{
    return mAAAJudgeOpenId;
}
//
MINT32
CameraContext::
getAELv() const
{
    return mAAAJudgeStrategy->getAELv();
}
//
MINT32
CameraContext::
getAFDac() const
{
    return mAAAJudgeStrategy->getAFDac();
}
//
MBOOL
CameraContext::
getIsAFDone() const
{
    return mAAAJudgeStrategy->getIsAFDone();
}
//
MVOID
CameraContext::
setAAAJudgeStrategy(sp<AAAJudgeStrategy> aaaJudgeStrategy)
{
    CAM_CONTEXT_SCOPED_TRACER();

    CAM_CONTEXT_LOGD("set AAA judge strategy, previous: %s, current: %s", mAAAJudgeStrategy->getName().c_str(), aaaJudgeStrategy->getName().c_str());
    mAAAJudgeStrategy = aaaJudgeStrategy;
}
//
MVOID
CameraContext::
handleAAAJudge(sp<IFrame3AControl>& frame3AControl)
{
    OpenId temp = mAAAJudgeStrategy->calculate(frame3AControl);
    if(temp != mAAAJudgeOpenId)
    {
        CAM_CONTEXT_LOGD("change AAA judge openId, name: %s, previous: %d, current: %d", mAAAJudgeStrategy->getName().c_str(), mAAAJudgeOpenId, temp);
        mAAAJudgeOpenId = temp;
    }
}
//
MVOID
CameraContext::
wakeup(IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    sp<CameraContext> context = this;
    mCameraStatus->wakeup(context, appMetadata, halMetadata, frame3AControl);
}
//
MVOID
CameraContext::
handleUpateSettings(IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    sp<CameraContext> context = this;
    mCameraStatus->handleUpateSettings(context, appMetadata, halMetadata, frame3AControl);
}
//
MVOID
CameraContext::
handleUpateSettingsResult(const VarMap& varMap)
{
    sp<CameraContext> context = this;
    mCameraStatus->handleUpateSettingsResult(context, varMap);
}
//
MVOID
CameraContext::
sendCommand(OpenId dstOpenId, sp<CameraStatusCommand> command)
{
    CAM_CONTEXT_SCOPED_TRACER();

    sp<CameraContext> dstCameraContext = CameraContextManager::getInstance()->getCameraContext(dstOpenId).promote();
    if(dstCameraContext != nullptr)
    {
        CAM_CONTEXT_LOGD("send command to openId: %d, command name: %s", dstOpenId, command->getName().c_str());
        dstCameraContext->handleCommand(command);
    }
    else
    {
        CAM_CONTEXT_LOGE("failed to send command, the camera context is not existing, openId: %d, command name: %s", dstOpenId, command->getName().c_str());
    }
}
//
MVOID
CameraContext::
handleCommand(sp<CameraStatusCommand> command)
{
    CAM_CONTEXT_LOGD("push command, command name: %s", command->getName().c_str());
    mCameraStatus->handleCommand(command);
}
//
CameraContext::
~CameraContext()
{
    CAM_CONTEXT_SCOPED_TRACER();

    CameraContextManager::getInstance()->remove(getOpenId(), this);
}
//
MTK_SYNC_CAEMRA_STATE
CameraContext::
getStatus()
{
    return mCameraStatus->getStatus();
}
//
MVOID
CameraContext::
setStatus(sp<CameraStatusBase> newStatus)
{
    CAM_CONTEXT_SCOPED_TRACER();

    sp<CameraStatusBase> oldStatus = mCameraStatus;
    mCameraStatus = newStatus;

    CAM_CONTEXT_LOGD("transit status from %s(%u) to %s(%u)",
        oldStatus->getName().c_str(), static_cast<MUINT32>(oldStatus->getStatus()), mCameraStatus->getName().c_str(), static_cast<MUINT32>(mCameraStatus->getStatus()));
}
//
/******************************************************************************
 * CameraContextManager
 ******************************************************************************/
CameraContextManager::
CameraContextManager()
{

}
//
sp<CameraContextManager>
CameraContextManager::
getInstance()
{
    static sp<CameraContextManager> singleton = new CameraContextManager();
    return singleton;
}
//
MVOID
CameraContextManager::
add(OpenId openId, wp<CameraContext> cameraContext)
{
    COMMON_SCOPED_TRACER();

    std::lock_guard<std::mutex> guard(mMutex);
    {
        if(!Utility::isKeyExisting(mCameraContextTable, openId))
        {
            mCameraContextTable.insert(pair<MINT32, android::wp<CameraContext>>(openId, cameraContext));
            COMMON_LOGD("insert camera context, openId: %d", openId);
        }
        else
        {
            COMMON_LOGE("failed to add camera the context, the context is existing (openId: %d)", openId);
        }
    }
}
//
MVOID
CameraContextManager::
remove(OpenId openId, wp<CameraContext> cameraContext)
{
    COMMON_SCOPED_TRACER();
    std::lock_guard<std::mutex> guard(mMutex);
    {
        if(Utility::isKeyExisting(mCameraContextTable, openId))
        {
            COMMON_LOGD("erase camera context, openId: %d", openId);
            mCameraContextTable.erase(openId);
        }
        else
        {
            COMMON_LOGE("failed to remove the camera context, the context is not existing (openId: %d)", openId);
        }
    }
}
//
wp<CameraContext>
CameraContextManager::
getCameraContext(OpenId dstOpenId)
{
    COMMON_SCOPED_TRACER();

    wp<CameraContext> ret = nullptr;
    std::lock_guard<std::mutex> guard(mMutex);
    {
        if(Utility::isKeyExisting(mCameraContextTable, dstOpenId))
        {
            ret = mCameraContextTable[dstOpenId];
        }
        else
        {
            COMMON_LOGD("failed to get camera context, the key(OpenId: %d) is not existing", dstOpenId);
        }
    }
    return ret;
}
//
MTK_SYNC_CAEMRA_STATE
CameraContextManager::
getStatus(OpenId dstOpenId)
{
    COMMON_SCOPED_TRACER();

    MTK_SYNC_CAEMRA_STATE ret = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    const sp<CameraContext> cameraContext = getCameraContext(dstOpenId).promote();
    if(cameraContext != nullptr)
    {
        ret = cameraContext->getStatus();
    }
    else
    {
        COMMON_LOGD("failed to get camera status, the camera context is not existing(OpenId: %d)", dstOpenId);
    }
    return ret;
}
//
} // NSCam
