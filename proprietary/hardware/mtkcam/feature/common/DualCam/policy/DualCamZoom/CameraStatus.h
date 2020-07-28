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
#ifndef _MTKCAM_DUALZOOM_CAMERA_STATUS_H_
#define _MTKCAM_DUALZOOM_CAMERA_STATUS_H_

// Standard C header file
#include <queue>

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe_var.h>

// Local header file
#include "DualZoomPolicy_Common.h"
#include "CameraContext.h"
#include "CameraStatusCommand.h"
#include <mtkcam/aaa/IHal3A.h>

/******************************************************************************
* namespace start
*******************************************************************************/
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::VarMap;

namespace NSCam
{
/******************************************************************************
 * CameraStatusBase
 ******************************************************************************/
class CameraContext;
class CameraStatusCommand;
class CameraStatusBase : public virtual RefBase
{
public:
    std::string getName() const;

    MTK_SYNC_CAEMRA_STATE getStatus() const;

    OpenId getOpenId() const;

    DropMode getDropMode() const;

    TwinTGMode getTwinTGMode() const;

public:
    MVOID transit(sp<CameraContext>& context, sp<CameraStatusBase> newStatus, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    MVOID handleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    MVOID handleUpateSettingsResult(sp<CameraContext>& context, const VarMap& varMap);

    MVOID wakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    MVOID handleCommand(sp<CameraStatusCommand> command);

public:
    virtual ~CameraStatusBase();

protected:

    CameraStatusBase(OpenId openId, const std::string& name, MTK_SYNC_CAEMRA_STATE status);


protected:
    virtual MVOID    onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);
    virtual MVOID    onHandleUpateSettingsResult(sp<CameraContext>& context, const VarMap& varMap);
    virtual MVOID    onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);
    virtual MVOID    onEntry(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);
    virtual MVOID    onExit(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

protected:
    virtual DropMode onGetDropMode() const;

private:
    const OpenId                    mOpenId;
    const std::string               mName;
    const MTK_SYNC_CAEMRA_STATE     mStatus;

private:
    std::queue<sp<CameraStatusCommand>> mCommandQueue;
};
//
/******************************************************************************
 * GoToLowFPSCameraStatus
 ******************************************************************************/
class GoToLowFPSCameraStatus : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

    MVOID onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

private:
    GoToLowFPSCameraStatus(OpenId openId);

private:
    MINT32 mWaitEnterLowFpsCount;
};
//
/******************************************************************************
 * LowFPSCameraStatus
 ******************************************************************************/
class LowFPSCameraStatus : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

    MVOID onEntry(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

    MVOID onExit(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

private:
    LowFPSCameraStatus(OpenId openId);
};
//
/******************************************************************************
 * GoToStandbyCameraStatus
 ******************************************************************************/
class GoToStandbyCameraStatus : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

private:
    GoToStandbyCameraStatus(OpenId openId);

protected:
    DropMode onGetDropMode() const override;

private:
    MBOOL waitForTwinTGModeOneDone(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

private:
    MINT32  mWaitEnterStandbyCount;
    MBOOL   mWaitTwinTGModeOneCount;
    MBOOL   mIsHWStandby;
};
//
/******************************************************************************
 * GoToStandbyCameraStatusForTest
 ******************************************************************************/
class GoToStandbyCameraStatusForTest : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId open);

protected:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

private:
    GoToStandbyCameraStatusForTest(OpenId openId);

private:
    MINT32 mWaitEnterStandbyCount;
};
//
/******************************************************************************
 * StandbyCameraStatus
 ******************************************************************************/
class StandbyCameraStatus : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

protected:
    DropMode onGetDropMode() const override;

private:
    StandbyCameraStatus(OpenId openId);
};
//
/******************************************************************************
 * StandbyCameraStatusForTest
 ******************************************************************************/
class StandbyCameraStatusForTest: public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

    MVOID onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

    DropMode onGetDropMode() const override;

private:
    StandbyCameraStatusForTest(OpenId openId);

private:
    MINT32  mStandbyCount;
};
//
/******************************************************************************
 * GoToActiveCameraStatus
 ******************************************************************************/
class GoToActiveCameraStatus : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

private:
    GoToActiveCameraStatus(OpenId open);

private:
    MBOOL waitForTwinTGModeTwoDone(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

private:
    MINT32 mWaitEnterActiveCount;
    MINT32 mWaitTGModeTwoCount;
    MBOOL  mIsHWStart;
};
//
/******************************************************************************
 * GoToActiveCameraStatusForTest
 ******************************************************************************/
class GoToActiveCameraStatusForTest : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

private:
    GoToActiveCameraStatusForTest(OpenId openId);

private:
    MINT32 mWaitEnterActiveCount;
};
//
/******************************************************************************
 * ActiveCameraStatus
 ******************************************************************************/
class ActiveCameraStatus : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

protected:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

    MVOID onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

protected:
    DropMode onGetDropMode() const override;

private:
    ActiveCameraStatus(OpenId openId);

private:
    MINT32  mWaitStableCount;
};
//
/******************************************************************************
 * ActiveCameraStatusForTest
 ******************************************************************************/
class ActiveCameraStatusForTest : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

public:
    MVOID onHandleUpateSettings(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

    MVOID onWakeup(sp<CameraContext>& context, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

protected:
    DropMode onGetDropMode() const override;

private:
    ActiveCameraStatusForTest(OpenId openId);

private:
    MINT32  mWaitStableCount;
};
//
/******************************************************************************
 * InvalidCameraStatus
 ******************************************************************************/
class InvalidCameraStatus : public CameraStatusBase
{
public:
    static sp<CameraStatusBase> createInstance(OpenId openId);

public:
    MVOID onHandleUpateSettings(sp<CameraContext>& context,IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl) override;

protected:
    DropMode onGetDropMode() const override;

private:
    InvalidCameraStatus(OpenId openId);
};
//
} // NSCam
#endif // _MTKCAM_DUALZOOM_CAMERA_STATUS_H_