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

#ifndef _MTK_HARDWARE_MTKCAM_DENOISE_POLICY_H_
#define _MTK_HARDWARE_MTKCAM_DENOISE_POLICY_H_
//
#include <utils/RefBase.h>
#include <vector>
#include <future>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/feature/DualCam/IDualCamPolicy.h>
#include <mtkcam/feature/DualCam/IFrame3AControl.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>
#include <mtkcam/feature/hdrDetection/Defs.h>
#include "Denoise/ThermalDetector.h"
#include "Denoise/BatteryDetector.h"
//
namespace NSCam
{
class DualCamDenoisePolicy : public IDualCamPolicy
{
public:
    static MINT32                    totalRefCount;
    MINT32                           mRefCount;

    DualCamDenoisePolicy();
    virtual ~DualCamDenoisePolicy();
public:
    MVOID       setCameraOpenIds(
                    vector<MINT32> ids) override;
    MVOID       setObject(
                    void* obj,
                    MINT32 arg1) override;
    MVOID       setParameter(
                    std::string str) override;
    MVOID       addCallBack(
                    sp<ISyncManagerCallback> Cb) override;
    MVOID       updateSetting(
                    MINT32 const i4OpenId,
                    IMetadata* appMetadata,
                    IMetadata* halMetadata,
                    SyncManagerParams &syncParams) override;
    MVOID       updateAfterP1(
                    MINT32 const i4OpenId,
                    MUINT32 const frameNo,
                    void* obj,
                    MINT32 arg1) override;
    MVOID       updateFeatureMask(
                    MUINT32 &featureMask) override;
    MVOID       lock(
                    void* arg1) override;
    MVOID       unlock(
                    void* arg1) override;

    MINT32      getPreviewCameraId(
                    MINT32 *dropOthers) override;

private:
    enum class DECISION_STATUS
    {
        UNSTABLE,
        MAIN2_ENABLE,
        MAIN2_DISABLE,
    };
    status_t    changeCameraMode(
                    MINT32  const               openId,
                    MINT32                      type,
                    MBOOL                       enableAAASync,
                    MBOOL                       enableHWSync,
                    MINT32                      minFps,
                    MINT32                      maxFps,
                    IMetadata*                  appMetadata,
                    IMetadata*                  halMetadata);
    status_t    updateStandbyMode(
                    MINT32  const               openId,
                    MINT32                      &setStatus,
                    IMetadata*                  halMetadata);
    status_t    setMasterCamera(
                    MINT32  const               openId,
                    IMetadata*                  halMetadata);
    void        changeCameraState(
                    MINT32 const                openId,
                    DECISION_STATUS const       canUsingMain2,
                    IMetadata*                  appMetadata,
                    IMetadata*                  halMetadata,
                    MTK_SYNC_CAEMRA_STATE&      state);
private:
    bool        canUsingDualCam(
                    MINT32 const openId,
                    DECISION_STATUS &status,
                    MBOOL isCapture = false);
    void        setMain2DropDisplayRequest(
                    MINT32 const openId,
                    IMetadata* halMetadata);
    void        updateParams(
                    SyncManagerParams &syncParams);
    void        update3AInfo(
                    IMetadata* halMetadata);
    void        updateISO(
                    NS3Av3::DualZoomInfo_T &aaaInfo);
    void        updateFlashFlag(
                    NS3Av3::DualZoomInfo_T &aaaInfo,
                    IMetadata* halMetadata);
    MINT32      getMain2OnIsoValue();

private:
    mutable Mutex            mLock;
    //
    sp<IFrame3AControl>      mIFrame3AControl = nullptr;
    sp<ISyncManagerCallback> mMgrCb = nullptr;
    //
    sp<ThermalDetector>      mspThermalDetector = nullptr;
    sp<BatteryDetector>      mspBatteryDetector = nullptr;
    //
    vector<MINT32>      mvOpenIds;
    //
    MRect               mActiveArrayMain1;
    MRect               mActiveArrayMain2;
    //
    MINT32              miMain1Id = -1;
    MINT32              miMain2Id = -1;
    //
    MBOOL               mbUsingFlash = 0;
    MINT32              miISO = -1;
    MUINT32             miZoomRatio = 0;
    NSCam::HDRMode      mHDRMode = NSCam::HDRMode::OFF;
    MBOOL               mbBatteryDualCamSupportCheck = MFALSE;
    MBOOL               mbThermalDualCamSupportCheck = MFALSE;
    //
    std::shared_ptr<NS3Av3::IHal3A>     mspHal3a_Main = nullptr;
    //
    MTK_SYNC_CAEMRA_STATE mMain1SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    MTK_SYNC_CAEMRA_STATE mMain2SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    //
    MINT32              mWaitStable = 0;
    MINT32              mWaitEnterLP = 0;
    MINT32              mWaitEnterAct = 0;
    //
    // When camera is open, main1 & main2 will be opened directly.
    // For this situation, enable 3a sync and hwsync directly.
    MTK_DUAL_CAM_AAA_SYNC_STATE         mSet3ASyncMode = MTK_DUAL_CAM_AAA_SYNC_STATE::NOT_SET;
    //
    DECISION_STATUS     mbCanUsingMain2 = DECISION_STATUS::UNSTABLE;
    //
    MINT32              mSetMain2SensorStatus = MTK_P1_SENSOR_STATUS_NONE;
    //
    MBOOL               mbInit = MTRUE;
    //
    MUINT8              miDoN3DCount = 0;
    //
    MINT32              mLogLevel = 0;
    //
    MBOOL               mbCanUpdateSetting = MTRUE;
    //
    MUINT8              miUnlockCount = 0;
    //
    MUINT8              miLockCount = 0;
    //
    MBOOL               mbSkipMain2Check = 0;
    //
    MBOOL               mBatteryCheck = MFALSE;
    //
    MBOOL               mThermalCheck = MFALSE;
    //
    static              std::future<void> tPolicySetting;
    //
    MBOOL               mbIsMain2Suspend = MFALSE;

};
};
#endif