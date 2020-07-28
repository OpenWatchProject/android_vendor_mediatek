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

#ifndef _MTK_HARDWARE_MTKCAM_DUALCAMZOOM_POLICY_H_
#define _MTK_HARDWARE_MTKCAM_DUALCAMZOOM_POLICY_H_

#include <vector>
#include <queue>
#include <future>
#include <functional>
#include <map>

#include <mtkcam/feature/DualCam/IDualCamPolicy.h>
#include <mtkcam/feature/DualCam/IFrame3AControl.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>
#include <utils/Mutex.h>

#include "DualCamZoom/CameraContext.h"

using namespace std;
using namespace android;

#define DISABLE_AAA_SYNC (1)
#define ENABLE_AAA_SYNC  (2)

namespace NSCam
{
using OpenId = MINT32;

class CameraContext;

class DualCamZoomPolicy : public IDualCamPolicy
{
    // ************* IDualCamPolicy interface **************************************
    public:
        MVOID setCameraOpenIds(vector<MINT32> ids) override;

        MVOID setObject(void* obj, MINT32 arg1) override;

        MVOID setParameter(std::string str) override;

        MVOID addCallBack(sp<ISyncManagerCallback> Cb) override;

        MVOID updateSetting(MINT32 const i4OpenId, IMetadata* appMetadata, IMetadata* halMetadata, SyncManagerParams& syncParams) override;

        MVOID updateAfterP1(MINT32 const i4OpenId, MUINT32 const frameNo, void* obj, MINT32 arg1) override;

        MVOID updateFeatureMask( MUINT32 &featureMask) override;

        MVOID lock(void* arg1) override;

        MVOID unlock(void* arg1) override;

        MINT32 getPreviewCameraId(MINT32 *dropOthers) override;
    // *******************************************************************************
    public:
        DualCamZoomPolicy();

        ~DualCamZoomPolicy() override;

    private:
        enum
        {
            DUALCAM_FOV_ONLINE_WIDE = 1 << 0,
            DUALCAM_FOV_ONLINE_TELE = 1 << 1,
        };

        struct JudgeInfo
        {
            MINT32 ZoomDrop;
            MINT32 AAAJudgeIdFromWide;
            MINT32 AAAJudgeIdFromTele;
            MINT32 is4KVideo;
        };

    private:
        inline MBOOL       isInvalidOpenId(OpenId opendId) const;

    private:
        MBOOL       determineDropRequest(MINT32 openId, JudgeInfo info);

        MVOID       setDropMode(OpenId openId, IMetadata* halMetadata, MBOOL isDrop);

        MBOOL       checkDoingOnlineFOV(OpenId openId, MUINT32 zoomRatio, MINT32 IsRec);

    private:
        static std::future<MVOID>   sThermalPolicyBGWorker;

        static MINT32               sTotalRefCount;
        MINT32                      mRefCount;

    private:
        vector<MINT32>                      mOpenIds;
        std::map<OpenId, sp<CameraContext>> mCameraContexts;

        sp<IFrame3AControl>                 mIFrame3AControl;

        MUINT32                             mPreviousZoomRatio;

        OpenId                              mPreviewCameraOpenId;

        sp<ISyncManagerCallback>            mSyncManagerCallback;

    // fov online calibration
    private:
        MINT32              mFovOnlineStatus;
    // strob
    private:
        MINT32              mPauseAFCamId;
        MINT32              mDoCapCamId;
        MINT32              mResumeAFCount;
    // status lock
    private:
        MINT32              mLockStatus;
        mutable Mutex       mLock;
    };
} // NSCam

#endif // _MTK_HARDWARE_MTKCAM_DUALCAMZOOM_POLICY_H_
