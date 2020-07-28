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

#ifndef _MTK_HARDWARE_MTKCAM_I_DUALCAM_POLICY_H_
#define _MTK_HARDWARE_MTKCAM_I_DUALCAM_POLICY_H_

#include <utils/RefBase.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam3/feature/DualCam/ISyncManager.h>

#include <vector>
#include <string>
using namespace std;
using namespace android;


#define DUAL_CAM_POLICY_MTK_TK       0
#define DUAL_CAM_POLICY_DENOISE     1
#define DUAL_CAM_POLICY_3RD_PARTY_CP 2
#define DUAL_CAM_POLICY_BOKEH     3

namespace NSCam
{
// for lock function used
struct PolicyLockParams
{
    MBOOL   bUseDualCamShot = MFALSE;
    MBOOL   bIsMain2On = MFALSE;
};
class IDualCamPolicy : public virtual android::RefBase
{
    public:
        static IDualCamPolicy* createInstance(MINT32 scenario);

        IDualCamPolicy() {}
        ~IDualCamPolicy() {}

    public:
        virtual MVOID setCameraOpenIds(vector<MINT32> ids) = 0;
        virtual MVOID setObject(void* obj, MINT32 arg1) = 0;
        virtual MVOID setParameter(std::string str) = 0;
        virtual MVOID addCallBack(sp<ISyncManagerCallback> Cb) = 0;

        // Per IPipelineFrame
        virtual MVOID updateSetting(
            MINT32 const i4OpenId,
            IMetadata* appMetadata, IMetadata* halMetadata,
            SyncManagerParams &syncParams) = 0;

        virtual MVOID updateAfterP1(MINT32 const i4OpenId, MUINT32 const frameNo,
                                    void* obj, MINT32 arg1) = 0;

        virtual MVOID updateFeatureMask(MUINT32 &featureMask) = 0;

        // lock/unlock the state changing
        virtual MVOID lock(void* arg1) = 0;
        virtual MVOID unlock(void* arg1) = 0;

        virtual MINT32 getPreviewCameraId(MINT32 *dropOthers) = 0;
};
} // NSCam

#endif // _MTK_HARDWARE_MTKCAM_I_DUALCAM_POLICY_H_
