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
#define LOG_TAG "MtkCam/DualCamZoomPolicy/CameraStatus"

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_dualzoom.h>

// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// Module header file
#include <mtkcam/feature/DualCam/utils/DualCameraUtility.h>

// Local header file
#include "CameraStatus.h"
#include "AAAJudgeStrategy.h"

/******************************************************************************
*
*******************************************************************************/
#define STATUS_LOGV(fmt, arg...)    CAM_LOGV("[%s][%d][%s(%d)] " fmt, __FUNCTION__, getOpenId(), getName().c_str(), static_cast<MINT32>(getStatus()), ##arg)
#define STATUS_LOGD(fmt, arg...)    CAM_LOGD("[%s][%d][%s(%d)] " fmt, __FUNCTION__, getOpenId(), getName().c_str(), static_cast<MINT32>(getStatus()), ##arg)
#define STATUS_LOGI(fmt, arg...)    CAM_LOGI("[%s][%d][%s(%d)] " fmt, __FUNCTION__, getOpenId(), getName().c_str(), static_cast<MINT32>(getStatus()), ##arg)
#define STATUS_LOGW(fmt, arg...)    CAM_LOGW("[%s][%d][%s(%d)] " fmt, __FUNCTION__, getOpenId(), getName().c_str(), static_cast<MINT32>(getStatus()), ##arg)
#define STATUS_LOGE(fmt, arg...)    CAM_LOGE("[%s][%d][%s(%d)] " fmt, __FUNCTION__, getOpenId(), getName().c_str(), static_cast<MINT32>(getStatus()), ##arg)
#define STATUS_LOGA(fmt, arg...)    CAM_LOGA("[%s][%d][%s(%d)] " fmt, __FUNCTION__, getOpenId(), getName().c_str(), static_cast<MINT32>(getStatus()), ##arg)
#define STATUS_LOGF(fmt, arg...)    CAM_LOGF("[%s][%d][%s(%d)] " fmt, __FUNCTION__, getOpenId(), getName().c_str(), static_cast<MINT32>(getStatus()), ##arg)

#define STATUS_FUN_IN()             STATUS_LOGD("+");
#define STATUS_FUN_OUT()            STATUS_LOGD("-");

using namespace NS3Av3;

class status_scoped_tracer
{
public:
    status_scoped_tracer(MINT32 openId, const char* functionName, const std::string& statusName, NSCam::MTK_SYNC_CAEMRA_STATE status)
    : mOpenId(openId)
    , mFunctionName(functionName)
    , mStatusName(statusName)
    , mStatus(status)
    {
        CAM_LOGD("[%s][%d][%s(%d)] +", mFunctionName, mOpenId, mStatusName.c_str(), static_cast<MINT32>(mStatus));
    }
    ~status_scoped_tracer()
    {
        CAM_LOGD("[%s][%d][%s(%d)] -", mFunctionName, mOpenId, mStatusName.c_str(), static_cast<MINT32>(mStatus));
    }
private:
    const MINT32                        mOpenId;
    const char* const                   mFunctionName;
    const std::string                   mStatusName;
    const NSCam::MTK_SYNC_CAEMRA_STATE  mStatus;
};
#define STATUS_SCOPED_TRACER() status_scoped_tracer ___scoped_tracer(getOpenId(), __FUNCTION__, getName(), getStatus());


/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{
using Utility   = DualCameraUtility;
using Helper    = DualZoomPolicyHelper;
using Converter = DualZoomPolicyConverter;
using AAAHelper = Frame3AControlHelper;
/******************************************************************************
 * CameraStatusBase
 ******************************************************************************/
CameraStatusBase::
CameraStatusBase(OpenId openId, const std::string& name, MTK_SYNC_CAEMRA_STATE status)
: mOpenId(openId)
, mName(name)
, mStatus(status)
{
    STATUS_SCOPED_TRACER();
}
//
namespace
{
    // TODO: BAD SMELL, Refactor
    TwinTGMode sTwinTGMode = MTK_P1_TWIN_STATUS_TG_MODE_2;
}
//
TwinTGMode
CameraStatusBase::
getTwinTGMode() const
{
    return sTwinTGMode;
}
//
MTK_SYNC_CAEMRA_STATE
CameraStatusBase::
getStatus() const
{
    return mStatus;
}
//
std::string
CameraStatusBase::
getName() const
{
    return mName;
}
//
OpenId
CameraStatusBase::
getOpenId() const
{
    return mOpenId;
}
//
DropMode
CameraStatusBase::
getDropMode() const
{
    return onGetDropMode();
}
//
MVOID
CameraStatusBase::
handleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    const MINT32 commandQueueSize = mCommandQueue.size();
    if(commandQueueSize > 0)
    {
        STATUS_LOGD("command queue size: %d", commandQueueSize);
        sp<CameraStatusCommand> command = mCommandQueue.front();
        mCommandQueue.pop();

        STATUS_LOGD("execute command: %s", command->getName().c_str());
        sp<CameraStatusBase> receiver = this;
        command->excute(context, receiver, appMetadata, halMetadata, frame3AControl);

        CameraStatusCommand::ControlFlowMode controlFlowMode = command->getControlFlowMode();
        if(!controlFlowMode)
        {
            STATUS_LOGD("control flow mode: %d, doesn't fall through", controlFlowMode);
            return;
        }
    }
    onHandleUpateSettings(context, appMetadata, halMetadata, frame3AControl);
}
//
MVOID
CameraStatusBase::
handleUpateSettingsResult(sp<CameraContext>& context, const VarMap& varMap)
{
    onHandleUpateSettingsResult(context, varMap);
}
//
MVOID
CameraStatusBase::
transit(sp<CameraContext>& context, sp<CameraStatusBase> newStatus, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    if(ForceFlow::isForceActive())
    {
        STATUS_LOGD("force active, no any status tranist");
        return;
    }

    this->onExit(context, appMetadata, halMetadata, frame3AControl);
    context->setStatus(newStatus);
    newStatus->onEntry(context, appMetadata, halMetadata, frame3AControl);
}
//
MVOID
CameraStatusBase::
wakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    onWakeup(context, appMetadata, halMetadata, frame3AControl);
}
//
MVOID
CameraStatusBase::
handleCommand(sp<CameraStatusCommand> command)
{
    STATUS_LOGD("push commamd to queue, name: %s", command->getName().c_str());
    mCommandQueue.push(command);
}
//
MVOID
CameraStatusBase::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{

}
//
MVOID
CameraStatusBase::
onHandleUpateSettingsResult(sp<CameraContext>& context, const VarMap& varMap)
{
    STATUS_SCOPED_TRACER();

    if(Helper::getDynamicTwinSupported())
    {
        TwinTGMode temp = -1;
        temp = varMap.get<MINT32>(VAR_P1RAW_TWIN_STATUS, -1);
        if (temp > -1)
        {
            if(sTwinTGMode != temp)
            {
                STATUS_LOGD("update the TG twin mode: %s(%d) -> %s(%d)",
                    Converter::twinTGModeNameMap[sTwinTGMode], sTwinTGMode, Converter::twinTGModeNameMap[temp], temp);

                sTwinTGMode = temp;
            }
        }
        else
        {
            STATUS_LOGD("failed to get the twin status");
        }
    }
}
//
MVOID
CameraStatusBase::
onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{

}
//
MVOID
CameraStatusBase::
onEntry(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{

}
//
MVOID
CameraStatusBase::
onExit(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{

}
//
CameraStatusBase::
~CameraStatusBase()
{
    STATUS_SCOPED_TRACER();
}
//
DropMode
CameraStatusBase::
onGetDropMode() const
{
    STATUS_LOGD("the operator is not implemented, default is return MTK_DUALZOOM_DROP_NEED_P1");
    return MTK_DUALZOOM_DROP_NEED_P1;
}
/******************************************************************************
 * GoToLowFPSCameraStatus
 ******************************************************************************/
sp<CameraStatusBase>
GoToLowFPSCameraStatus::
createInstance(OpenId openId)
{
    return new GoToLowFPSCameraStatus(openId);
}
//
GoToLowFPSCameraStatus::
GoToLowFPSCameraStatus(OpenId openId)
: CameraStatusBase(openId, "go_to_low_fps_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_LOWFPS)
, mWaitEnterLowFpsCount(0)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
GoToLowFPSCameraStatus::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    mWaitEnterLowFpsCount++;

    STATUS_LOGD("wainEnterCount: %d", mWaitEnterLowFpsCount);
    if(mWaitEnterLowFpsCount >= DualZoomPolicySettings::waitActiveToLowPowerCount)
    {
        STATUS_LOGD("transit status to LowFPSCameraStatus, wainEnterCount: %d", mWaitEnterLowFpsCount);
        transit(context, LowFPSCameraStatus::createInstance(getOpenId()), appMetadata, halMetadata, frame3AControl);
    }
}
//
MVOID
GoToLowFPSCameraStatus::
onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("wake up, create transitCommand for transit status to active_status");
    sp<CameraStatusCommand> command = TransitCommand::createInstance(ActiveCameraStatus::createInstance);
    handleCommand(command);
}
//
/******************************************************************************
 * LowFPSCameraStatus
 ******************************************************************************/
sp<CameraStatusBase>
LowFPSCameraStatus::
createInstance(OpenId openId)
{
    return new LowFPSCameraStatus(openId);
}
//
LowFPSCameraStatus::
LowFPSCameraStatus(OpenId openId)
: CameraStatusBase(openId, "low_fps_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_LOWFPS)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
LowFPSCameraStatus::
onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("wake up, create transitCommand for transit status to active_status");
    sp<CameraStatusCommand> command = TransitCommand::createInstance(ActiveCameraStatus::createInstance);
    handleCommand(command);
}
//
MVOID
LowFPSCameraStatus::
onEntry(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("set low frame rate");
    AAAHelper::setLowFrameRate(getOpenId(), appMetadata, halMetadata,frame3AControl);
}
//
MVOID
LowFPSCameraStatus::
onExit(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("set high frame rate");
    AAAHelper::setHighFrameRate(getOpenId(), appMetadata, halMetadata,frame3AControl);
}
//
/******************************************************************************
 * GoToActiveCameraStatus
 ******************************************************************************/
sp<CameraStatusBase>
GoToActiveCameraStatus::
createInstance(OpenId openId)
{
    return new GoToActiveCameraStatus(openId);
}
//
GoToActiveCameraStatus::
GoToActiveCameraStatus(OpenId openId)
: CameraStatusBase(openId, "go_to_active_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE)
, mWaitEnterActiveCount(0)
, mWaitTGModeTwoCount(0)
, mIsHWStart(false)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
GoToActiveCameraStatus::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    if(!waitForTwinTGModeTwoDone(context, appMetadata, halMetadata, frame3AControl))
    {
        STATUS_LOGD("switch to twin TG mode Two is not done");
        return;
    }

    mWaitEnterActiveCount++;
    STATUS_LOGD("waitEnterActiveCount: %d", mWaitEnterActiveCount);

    const OpenId openId = getOpenId();

        std::unique_ptr <
                    IHal3A,
                    std::function<void(IHal3A*)>
                    > hal3a
            (
                MAKE_Hal3A(openId, LOG_TAG),
                [](IHal3A* p){ if (p) p->destroyInstance(LOG_TAG); }
            );

    if (hal3a.get() == nullptr) {
        STATUS_LOGE("create IHal3A instance failed!");
        return;
    }

    if(!mIsHWStart)
    {
        OpenId otherOpenId = -1;
        if(Helper::tryGetCorrespondingCameraOpenId(openId, otherOpenId))
        {
            // (1) set p1 streaming start
            Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_P1NODE_SENSOR_STATUS, MTK_P1_SENSOR_STATUS_STREAMING);
            // (2) init sync AE
            hal3a->send3ACtrl(E3ACtrl_Sync3A_Sync2ASetting, otherOpenId, openId);
            //mp3A ->send3ACtrl(E3ACtrl_Sync3A_Sync2ASetting, otherOpenId, openId);
            // (3) set other as master camera
            AAAHelper::setMasterCamera(otherOpenId, openId, appMetadata, halMetadata, frame3AControl);
            // (4) enable 3 A sync
            AAAHelper::enable3ASync(openId, appMetadata, halMetadata, frame3AControl);
            //
            mIsHWStart = true;
            //
            STATUS_LOGD("hw steaming start and enable 3A sync");
        }
        else
        {
            STATUS_LOGE("failed to get the corresponding openId: %d to start hw steaming and 3A sync", otherOpenId);
        }
    }
    else
    {
        STATUS_LOGD("transit status to ActiveCameraStatus, waitTGModeTwoDoneCount: %d, waitEnterActiveCount, %d", mWaitTGModeTwoCount, mWaitEnterActiveCount);
        transit(context, ActiveCameraStatus::createInstance(openId), appMetadata, halMetadata, frame3AControl);
    }
}
//
MBOOL
GoToActiveCameraStatus::
waitForTwinTGModeTwoDone(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    MBOOL isTGModeTwoDone = true;
    if(Helper::getDynamicTwinSupported())
    {
        mWaitTGModeTwoCount++;
        STATUS_LOGD("waitTGModeTwoDoneCount: %d", mWaitTGModeTwoCount);

        const OpenId openId = getOpenId();
        const TwinTGMode currentTGTwinMode = getTwinTGMode();
        const MBOOL isTwinTGModeTwo = (currentTGTwinMode == MTK_P1_TWIN_STATUS_TG_MODE_2);
        if(!isTwinTGModeTwo)
        {
            STATUS_LOGD("current twin TG mode is %s(%d)", Converter::twinTGModeNameMap[currentTGTwinMode], currentTGTwinMode);

            OpenId dstOpenId = -1;
            if(Helper::tryGetCorrespondingCameraOpenId(openId, dstOpenId))
            {
                sp<CameraStatusCommand> command = SwitchTwinModeCommand::createInstance(MTK_P1_TWIN_STATUS_TG_MODE_2);
                context->sendCommand(dstOpenId, command);
                STATUS_LOGD("send commed: %s to camera openId: %d", command->getName().c_str(), dstOpenId);
            }
            else
            {
                STATUS_LOGE("failed to get the corresponding openId: %d", dstOpenId);
            }
            isTGModeTwoDone = false;
        }
        else
        {
            STATUS_LOGE("wait TG mode two done");
        }
    }
    else
    {
        STATUS_LOGD("not support dymanic twin, wait done");
    }
    return isTGModeTwoDone;
}
//
/******************************************************************************
 * GoToActiveCameraStatusForTest
 ******************************************************************************/
sp<CameraStatusBase>
GoToActiveCameraStatusForTest::
createInstance(OpenId openId)
{
    return new GoToActiveCameraStatusForTest(openId);
}
//
GoToActiveCameraStatusForTest::
GoToActiveCameraStatusForTest(OpenId openId)
: CameraStatusBase(openId, "go_to_active_status_for_test", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_ACTIVE)
, mWaitEnterActiveCount(0)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
GoToActiveCameraStatusForTest::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    mWaitEnterActiveCount++;
    STATUS_LOGD("waitEnterActiveCount: %d", mWaitEnterActiveCount);

    if(mWaitEnterActiveCount > DualZoomPolicySettings::waitStandbyToActiveCount)
    {
        STATUS_LOGD("transit status to ActiveCameraStatusForTest, waitEnterActiveCount: %d", mWaitEnterActiveCount);
        transit(context, ActiveCameraStatusForTest::createInstance(getOpenId()), appMetadata, halMetadata, frame3AControl);
    }
}
//
/******************************************************************************
 * ActiveCameraStatus
 ******************************************************************************/
sp<CameraStatusBase>
ActiveCameraStatus::
createInstance(OpenId openId)
{
    return new ActiveCameraStatus(openId);
}
//
ActiveCameraStatus::
ActiveCameraStatus(OpenId openId)
: CameraStatusBase(openId, "active_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
, mWaitStableCount(0)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
ActiveCameraStatus::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    mWaitStableCount++;
    STATUS_LOGD("waitStableCount: %d", mWaitStableCount);

    if(mWaitStableCount > DualZoomPolicySettings::waitStableCount)
    {
        MINT32 openId = getOpenId();
        sp<CameraStatusBase> trainstStatus = nullptr;
        if (openId == DUALZOOM_WIDE_CAM_ID)
        {
            trainstStatus = Helper::getWideStandbySupported() ? GoToStandbyCameraStatus::createInstance(openId) : GoToLowFPSCameraStatus::createInstance(openId);
        }
        else if(openId == DUALZOOM_TELE_CAM_ID)
        {
            // tele delay into low power mode, only notify wide disable 3A sync
            // aaaSyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::DISABLE;

            STATUS_LOGD("transit status to GoToStandbyCameraStatus, waitStableCount: %d", mWaitStableCount);
            trainstStatus = GoToStandbyCameraStatus::createInstance(openId);
        }
        else
        {
            STATUS_LOGE("failed to transit status, invalid camera openId: %d", openId);
        }

        if(trainstStatus != nullptr)
        {
            STATUS_LOGD("transit status to %s(%d), waitStableCount: %d", trainstStatus->getName().c_str(), static_cast<MUINT32>(trainstStatus->getStatus()), mWaitStableCount);
            transit(context, trainstStatus, appMetadata, halMetadata, frame3AControl);
        }
    }
}
//
MVOID
ActiveCameraStatus::
onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("wakeup, reset waitStableCount (perivous: %d)", mWaitStableCount);
    mWaitStableCount = 0;
}
//
DropMode
ActiveCameraStatus::
onGetDropMode() const
{
    return MTK_DUALZOOM_DROP_NONE;
}
//
/******************************************************************************
 * ActiveCameraStatusForTest
 ******************************************************************************/
sp<CameraStatusBase>
ActiveCameraStatusForTest::
createInstance(OpenId openId)
{
    return new ActiveCameraStatusForTest(openId);
}
//
ActiveCameraStatusForTest::
ActiveCameraStatusForTest(OpenId openId)
: CameraStatusBase(openId, "active_status_for_test", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_ACTIVE)
, mWaitStableCount(0)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
ActiveCameraStatusForTest::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    mWaitStableCount++;
    STATUS_LOGD("waitStableCount: %d", mWaitStableCount);

    if(mWaitStableCount > DualZoomPolicySettings::waitStableCount)
    {
        STATUS_LOGD("transit status to GoToStandbyCameraStatusForTest, waitStableCount: %d", mWaitStableCount);
        transit(context, GoToStandbyCameraStatusForTest::createInstance(getOpenId()), appMetadata, halMetadata, frame3AControl);
    }
}
//
MVOID
ActiveCameraStatusForTest::
onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("wakeup, reset waitStableCount (perivous: %d)", mWaitStableCount);
    mWaitStableCount = 0;
}
//
DropMode
ActiveCameraStatusForTest::
onGetDropMode() const
{
    return MTK_DUALZOOM_DROP_NONE;
}
//
/******************************************************************************
 * GoToStandbyCameraStatus
 ******************************************************************************/
sp<CameraStatusBase>
GoToStandbyCameraStatus::
createInstance(OpenId openId)
{
    return new GoToStandbyCameraStatus(openId);
}
//
GoToStandbyCameraStatus::
GoToStandbyCameraStatus(OpenId openId)
: CameraStatusBase(openId, "go_to_standby_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY)
, mWaitEnterStandbyCount(0)
, mWaitTwinTGModeOneCount(0)
, mIsHWStandby(false)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
GoToStandbyCameraStatus::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    mWaitEnterStandbyCount++;
    STATUS_LOGD("waitEnterStandbyCount: %d", mWaitEnterStandbyCount);

    const OpenId openId = getOpenId();
    if(!mIsHWStandby)
    {
        Utility::setMetadataAndUpdate<MINT32>(*halMetadata, MTK_P1NODE_SENSOR_STATUS, MTK_P1_SENSOR_STATUS_HW_STANDBY);
        //
        AAAHelper::disablele3ASync(openId, appMetadata, halMetadata, frame3AControl);
        //
        mIsHWStandby = true;
        //
        STATUS_LOGD("hw stream standby and disable 3A sync");
    }
    else if(!waitForTwinTGModeOneDone(context, appMetadata, halMetadata, frame3AControl))
    {
        STATUS_LOGD("wait for switching to twin TG mode one, mWaitTwinTGModeOneCount: %d", mWaitTwinTGModeOneCount);
    }
    else
    {
        STATUS_LOGD("transit status to StandbyCameraStatus, mWaitTwinTGModeOneCount: %d, waitEnterStandbyCount: %d", mWaitTwinTGModeOneCount, mWaitEnterStandbyCount);
        transit(context, StandbyCameraStatus::createInstance(openId), appMetadata, halMetadata, frame3AControl);
    }
}
//
MBOOL
GoToStandbyCameraStatus::
waitForTwinTGModeOneDone(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    MBOOL isTGModeOneDone = true;
    if(Helper::getDynamicTwinSupported())
    {
        mWaitTwinTGModeOneCount++;
        STATUS_LOGD("waitTwinTGModeOneCount: %d", mWaitTwinTGModeOneCount);

        const OpenId openId = getOpenId();
        const MBOOL isWideExecuteDymanicTwin = (Helper::getWideDymanicTwinSupported() && openId == DUALZOOM_WIDE_CAM_ID);
        const MBOOL isTeleExecuteDymanicTwin = (openId == DUALZOOM_TELE_CAM_ID);
        const TwinTGMode currentTGTwinMode = getTwinTGMode();
        const MBOOL isTwinTGModeOne = (currentTGTwinMode == MTK_P1_TWIN_STATUS_TG_MODE_1);
        if((isWideExecuteDymanicTwin || isTeleExecuteDymanicTwin) && !isTwinTGModeOne)
        {
            STATUS_LOGD("switch to twin TG mode one, isWideExecuteDymanicTwin: %d, isTeleExecuteDymanicTwin: %d, isTwinTGModeOne: %d, currentTGTwinMode: %s(%d)",
                isWideExecuteDymanicTwin, isTeleExecuteDymanicTwin, isTwinTGModeOne, Converter::twinTGModeNameMap[currentTGTwinMode], currentTGTwinMode);

            OpenId dstOpenId = -1;
            if(Helper::tryGetCorrespondingCameraOpenId(openId, dstOpenId))
            {
                sp<CameraStatusCommand> command = SwitchTwinModeCommand::createInstance(MTK_P1_TWIN_STATUS_TG_MODE_1);
                context->sendCommand(dstOpenId, command);

                STATUS_LOGD("send commed: %s to camera openId: %d", command->getName().c_str(), dstOpenId);
            }
            else
            {
                STATUS_LOGD("failed to find the corresponding camera openId");
            }
            isTGModeOneDone = false;
        }
        else
        {
            STATUS_LOGD("switch to twin TG mode one done, isWideExecuteDymanicTwin: %d, isTeleExecuteDymanicTwin: %d, isTwinTGModeOne: %d, waitTwinTGModeOneCount: %d",
                isWideExecuteDymanicTwin, isTeleExecuteDymanicTwin, isTwinTGModeOne, mWaitTwinTGModeOneCount);
        }
    }
    else
    {
        STATUS_LOGD("not support dymanic twin, wait done");
    }
    return isTGModeOneDone;
}
//
DropMode
GoToStandbyCameraStatus::
onGetDropMode() const
{
    DropMode ret = mIsHWStandby ? MTK_DUALZOOM_DROP_DIRECTLY : MTK_DUALZOOM_DROP_NEED_P1;
    STATUS_LOGD("isHWStandby: %d, current drop mode: %d", mIsHWStandby, ret);
    return ret;
}
//
/******************************************************************************
 * GoToStandbyCameraStatusForTest
 ******************************************************************************/
sp<CameraStatusBase>
GoToStandbyCameraStatusForTest::
createInstance(OpenId openId)
{
    return new GoToStandbyCameraStatusForTest(openId);
}
//
GoToStandbyCameraStatusForTest::
GoToStandbyCameraStatusForTest(OpenId openId)
: CameraStatusBase(openId, "go_to_standby_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_GOTO_STANDBY)
, mWaitEnterStandbyCount(0)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
GoToStandbyCameraStatusForTest::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    mWaitEnterStandbyCount++;
    STATUS_LOGD("waitEnterStandbyCount: %d", mWaitEnterStandbyCount);

    if(mWaitEnterStandbyCount > DualZoomPolicySettings::waitActiveToLowPowerCount)
    {
        STATUS_LOGD("transit status to StandbyCameraStatusForTest, waitEnterStandbyCount: %d", mWaitEnterStandbyCount);
        transit(context, StandbyCameraStatusForTest::createInstance(getOpenId()), appMetadata, halMetadata, frame3AControl);
    }
}
//
/******************************************************************************
 * StandbyCameraStatus
 ******************************************************************************/
sp<CameraStatusBase>
StandbyCameraStatus::
createInstance(OpenId openId)
{
    return new StandbyCameraStatus(openId);
}
//
StandbyCameraStatus::
StandbyCameraStatus(OpenId openId)
: CameraStatusBase(openId, "standby_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
StandbyCameraStatus::
onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("wake up, create transitCommand for transit status to go_to_active_status");
    sp<CameraStatusCommand> command = TransitCommand::createInstance(GoToActiveCameraStatus::createInstance);
    handleCommand(command);
}
//
DropMode
StandbyCameraStatus::
onGetDropMode() const
{
    return MTK_DUALZOOM_DROP_DIRECTLY;
}
//
/******************************************************************************
 * StandbyCameraStatusForTest
 ******************************************************************************/
sp<CameraStatusBase>
StandbyCameraStatusForTest::
createInstance(OpenId openId)
{
    return new StandbyCameraStatusForTest(openId);
}
//
StandbyCameraStatusForTest::
StandbyCameraStatusForTest(OpenId openId)
: CameraStatusBase(openId, "standby_status_for_test", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_STANDBY)
, mStandbyCount(0)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
StandbyCameraStatusForTest::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    OpenId openId = getOpenId();

    // just for debug use
    mStandbyCount++;
    STATUS_LOGD("standbyCount: %d", mStandbyCount);
}
//
MVOID
StandbyCameraStatusForTest::
onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();

    STATUS_LOGD("wake up, create transitCommand for transit status to go_to_active_status");
    sp<CameraStatusCommand> command = TransitCommand::createInstance(GoToActiveCameraStatusForTest::createInstance);
    handleCommand(command);
}
//
DropMode
StandbyCameraStatusForTest::
onGetDropMode() const
{
    return MTK_DUALZOOM_DROP_DIRECTLY;
}
//
/******************************************************************************
 * InvalidCameraStatus
 ******************************************************************************/
sp<CameraStatusBase>
InvalidCameraStatus::
createInstance(OpenId openId)
{
    return new InvalidCameraStatus(openId);
}
//
InvalidCameraStatus::
InvalidCameraStatus(OpenId openId)
: CameraStatusBase(openId, "invalid_status", MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID)
{
    STATUS_SCOPED_TRACER();
}
//
MVOID
InvalidCameraStatus::
onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl)
{
    STATUS_SCOPED_TRACER();
}
//
DropMode
InvalidCameraStatus::
onGetDropMode() const
{
    return MTK_DUALZOOM_DROP_DIRECTLY;
}
//
} // NSCam
