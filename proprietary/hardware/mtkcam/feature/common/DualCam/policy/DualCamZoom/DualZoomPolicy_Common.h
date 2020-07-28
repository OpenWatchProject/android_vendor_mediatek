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
#ifndef _MTKCAM_DUALZOOM_POLICY_COMMON_H_
#define _MTKCAM_DUALZOOM_POLICY_COMMON_H_

// Standard C header file
#include <map>

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/IMetadata.h>

// Module header file
#include <mtkcam/feature/DualCam/IDualCamPolicy.h>
#include <mtkcam/feature/DualCam/IFrame3AControl.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>

// Local header file


/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{

using OpenId        = MINT32;
using TwinTGMode    = MINT32;
using DropMode      = MINT32;

/******************************************************************************
 * DualZoomPolicyHelper
 ******************************************************************************/
class DualZoomPolicyHelper final
{
public:
    DualZoomPolicyHelper() = delete;

public:
    static MBOOL getDynamicTwinSupported();

    static MBOOL getWideDymanicTwinSupported();

    static MBOOL getEnableHWControlFlow();

    static MBOOL getWideStandbySupported();

    static MBOOL getMultiDistanceFovSupported();

    static MINT32 getLogLevel();

public:
    static MBOOL tryGetCorrespondingCameraOpenId(OpenId openId, OpenId& correspondingOpenId);

    static OpenId getCorrespondingCameraOpenId(OpenId openId);

    static MBOOL tryGetSensorActiveArray(MINT32 openId, MRect& activeArray);
};
/******************************************************************************
 * DualZoomPolicyConverter
 ******************************************************************************/
class DualZoomPolicyConverter
{
public:
    DualZoomPolicyConverter() = delete;

public:
    template<class T>
    using NameMap = std::map<T, const char*>;

    static NameMap<const MTK_SYNC_CAEMRA_STATE> statusNameMap;

    static NameMap<MINT32> dropModeNameMap;

    static NameMap<MINT32> twinTGModeNameMap;
};
/******************************************************************************
 * DualZoomPolicySettings
 ******************************************************************************/
class DualZoomPolicySettings
{
public:
    DualZoomPolicySettings() = delete;

public:
    static const MINT32 totalResumeAFCount;

    static const MUINT32 switchCameraZoomRatio;

    // status wait count
    static const MINT32 waitStableCount;
    static const MINT32 waitActiveToLowPowerCount;
    static const MINT32 waitStandbyToActiveCount;
    static const MINT32 waitLowFPSToActiveCount;
};
/******************************************************************************
 * OnlineFovThreshold
 ******************************************************************************/
class OnlineFovThreshold
{
public:
    OnlineFovThreshold() = delete;

public:
    static MINT32 getZoomRatio();

    static MINT32 getThermal();

    static MINT32 getWideDAC();

    static MINT32 getTeleDAC();

    static MINT32 getIso();

    static MINT32 getExpTime();
};
/******************************************************************************
 * AAAJudgeSettings
 ******************************************************************************/
class AAAJudgeSettings
{
public:
    AAAJudgeSettings() = delete;

public:
    static MINT32 getAFStrategy();

    static MINT32 getAELVDifferenceThreshold();

    static MINT32 getWideAEIsoLowThreshold();

    static MINT32 getWideAEIsoHighThreshold();

    static MINT32 getWideAEIsoCountThreshold();

    static MINT32 getWideAFDacLowThreshold();

    static MINT32 getWideAFDacHighThreshold();

    static MINT32 getWideAFDacCountThreshold();

    static MINT32 getTeleAFDacThreshold();

    static MINT32 getTeleAEIsoLowThreshold();

    static MINT32 getTeleAEIsoHighThreshold();

    static MINT32 getTeleAEIsoCountThreshold();
};
/******************************************************************************
 * ForceFlow
 ******************************************************************************/
class ForceFlow
{
public:
    ForceFlow() = delete;

public:

    static MBOOL isSkip3ACheck();

    static MBOOL isForceNotDoOnlineFov();

    static MBOOL isForceNoFrameSync();

    static MBOOL isForceNoDrop();

    static MBOOL isForceNoCheckOnlineFovResult();

    static MBOOL isForceActive();
};
/******************************************************************************
 * Frame3AControlHelper
 ******************************************************************************/
class Frame3AControlHelper
{
public:
    Frame3AControlHelper() = delete;

public:
    using Fram3ASettingType = MUINT32;

    static MVOID setFrame3AControl(OpenId openId, Fram3ASettingType type, MBOOL enable3ASync, MBOOL enableHWSync, MINT32 minFps, MINT32 maxFps, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    static MVOID setFrameRate(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl, MBOOL isHigh);

    static MVOID set3ASync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl, MBOOL enable);

    static MVOID setFrameSync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl, MBOOL enable);

    static MVOID setMasterCamera(OpenId masterOpenId, OpenId slaveOpenId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

public:
    static MVOID setHighFrameRate(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    static MVOID setLowFrameRate(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    static MVOID enable3ASync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    static MVOID disablele3ASync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    static MVOID enableFrameSync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);

    static MVOID disableFrameSync(OpenId openId, IMetadata* appMetadata, IMetadata* halMetadata, sp<IFrame3AControl>& frame3AControl);
};
} // NSCam
#endif // _MTKCAM_DUALZOOM_POLICY_COMMON_H_