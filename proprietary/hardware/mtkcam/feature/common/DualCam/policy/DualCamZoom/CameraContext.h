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
#ifndef _MTKCAM_DUALZOOM_CAMERA_COMTEXT_H_
#define _MTKCAM_DUALZOOM_CAMERA_COMTEXT_H_

// Standard C header file
#include <map>

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe_var.h>

// Local header file
#include "DualZoomPolicy_Common.h"
#include "CameraStatus.h"
#include "CameraStatusCommand.h"
#include "AAAJudgeStrategy.h"


/******************************************************************************
* namespace start
*******************************************************************************/
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::VarMap;

namespace NSCam
{
/******************************************************************************
 * CameraContext
 ******************************************************************************/
class CameraStatusBase;
class CameraStatusCommand;
class CameraContext : public virtual RefBase
{
public:
    static sp<CameraContext> createInstance(OpenId openId, const MRect& activeArray);

    static sp<CameraContext> getInvalidInstance(OpenId openId);

    static MTK_SYNC_CAEMRA_STATE getStatus(OpenId dstOpenId);

    static wp<CameraContext> getCameraContext(OpenId dstOpenId);

public:
    OpenId getOpenId() const;

    MRect getActiveArray() const;

public:
    DropMode    getDropMode() const;

    OpenId      getAAAJudgeOpenId() const;

    MINT32      getAELv() const;

    MINT32      getAFDac() const;

    MBOOL       getIsAFDone() const;

public:

    MVOID setAAAJudgeStrategy(sp<AAAJudgeStrategy> aaaJudgeStrategy);

    MVOID handleAAAJudge(sp<IFrame3AControl>& frame3AControl);

    MVOID setStatus(sp<CameraStatusBase> newStatus);

    MVOID wakeup(IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    MVOID handleUpateSettings(IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    MVOID handleUpateSettingsResult(const VarMap& varMap);

    MVOID sendCommand(OpenId dstOpenId, sp<CameraStatusCommand> command);

    MVOID handleCommand(sp<CameraStatusCommand> command);

    ~CameraContext();

    MTK_SYNC_CAEMRA_STATE getStatus();

private:
    CameraContext(OpenId openId, const MRect& activeArray);

private:
    const OpenId                    mOpenId;
    const struct MRect              mActiveArray;
    sp<CameraStatusBase>            mCameraStatus;

    sp<AAAJudgeStrategy>            mAAAJudgeStrategy;
    OpenId                          mAAAJudgeOpenId;
};
//
/******************************************************************************
 * CameraContextManager
 ******************************************************************************/
class CameraContextManager : public RefBase
{
public:
    static sp<CameraContextManager> getInstance();

public:
    MVOID add(OpenId openId, wp<CameraContext> cameraContext);

    MVOID remove(OpenId openId, wp<CameraContext> cameraContext);

    wp<CameraContext> getCameraContext(OpenId dstOpenId);

    MTK_SYNC_CAEMRA_STATE getStatus(OpenId dstOpenId);

private:
    CameraContextManager();

private:
    std::mutex mMutex;
    std::map<OpenId, wp<CameraContext>> mCameraContextTable;
};
//
} // NSCam
#endif // _MTKCAM_DUALZOOM_CAMERA_COMTEXT_H_