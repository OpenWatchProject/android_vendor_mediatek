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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_BOKEH_POLICY_H_
#define _MTK_HARDWARE_MTKCAM_BOKEH_POLICY_H_
//
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <vector>
#include <future>
//
#include <mtkcam/feature/DualCam/IDualCamPolicy.h>
#include <mtkcam/feature/DualCam/IFrame3AControl.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>
#include <isp_tuning/isp_tuning.h>
//
namespace NSCam
{
class DualCamBokehPolicy : public IDualCamPolicy
{
public:
    static MINT32                    totalRefCount;
    MINT32                           mRefCount;

    DualCamBokehPolicy();
    virtual ~DualCamBokehPolicy();
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
    void        setDropDisplayRequest(
                    MINT32 const openId,
                    IMetadata* halMetadata);
    void        updateParams(
                    SyncManagerParams &syncParams);
    void        updateDofLevel(
                    MINT32 const openId,
                    IMetadata* halMetadata);
    void        querySensorActiveDomain();
    void        querySensorCropRegion();

private:
    mutable Mutex            mLock;
    //
    sp<IFrame3AControl>      mIFrame3AControl = nullptr;
    sp<ISyncManagerCallback> mMgrCb = nullptr;
    //
    vector<MINT32>      mvOpenIds;
    //
    MINT32              miMain1Id = -1;
    MINT32              miMain2Id = -1;
    //
    MUINT32             miZoomRatio = 0;
    MINT32              mDofLevel = 0;
    //
    std::shared_ptr<NS3Av3::IHal3A>     mspHal3a_Main = nullptr;
    //
    MTK_SYNC_CAEMRA_STATE mMain1SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    MTK_SYNC_CAEMRA_STATE mMain2SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
    MTK_SYNC_CAEMRA_STATE mPreMain2SensorStatus = MTK_SYNC_CAEMRA_STATE::MTK_SYNC_CAEMRA_INVALID;
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
    MBOOL               mbInit = MTRUE;
    //
    MINT32              mLogLevel = 0;
    //
    MBOOL               mbCanUpdateSetting = MTRUE;
    //
    static              std::future<void> tPolicySetting;
    //
    MRect               mActiveArrayCrop                = MRect(0, 0);
    MRect               mActiveArrayCrop_Main2          = MRect(0, 0);
    MRect               mSensorCropRegion               = MRect(0, 0);
    MRect               mSensorCropRegion_Main2         = MRect(0, 0);
    //
    MBOOL               mbNeedSet3ASync                 = MTRUE;
    //
    MUINT8              mMain1ISPProfile                = NSIspTuning::EIspProfile_N3D_Preview;
    MUINT8              mMain2ISPProfile                = NSIspTuning::EIspProfile_N3D_Preview;
};
};
#endif