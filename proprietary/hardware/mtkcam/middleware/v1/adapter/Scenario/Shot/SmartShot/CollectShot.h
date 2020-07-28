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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_COllECTSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_COllECTSHOT_H_

#include "SmartShot.h"

namespace android {
namespace NSShot {
namespace NSSmartShot {

using android::sp;

/******************************************************************************
 *
 ******************************************************************************/
class CollectShot : public SmartShot
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~CollectShot();
                                    CollectShot(
                                        char const*const pszShotName,
                                        uint32_t const   u4ShotMode,
                                        int32_t const    i4OpenId,
                                        bool const       isZsd
                                    );


/* re-implementations of SmartShot */
public:
    virtual MBOOL                   createStreams() override;
    virtual bool                    onCmd_capture() override;

protected:
    virtual status_t                getSelectorData(
                                        IMetadata& rAppSetting,
                                        IMetadata& rHalSetting,
                                        android::sp<IImageBuffer>& pBuffer,
                                        bool  bResetReqRule = false);
    virtual MBOOL                   applyRawBufferSettings(
                                        Vector<SettingSet>  vSettings,
                                        MINT32              shotCount,
                                        Vector<MINT32>&     rvRequestNo,
                                        MINT32              dummyCount = 0,
                                        MINT32              delayedCount = 0
                                    ) override;
    virtual MERROR                  submitCaptureSetting(
                                        MBOOL     mainFrame,
                                        IMetadata appSetting,
                                        IMetadata halSetting
                                    ) override;
    virtual MBOOL                   createPipeline() override;
/* re-implementations of SCallbackListener */
protected:
    virtual MVOID                   onMetaReceived(
                                        MUINT32         const requestNo,
                                        StreamId_T      const streamId,
                                        MBOOL           const errorResult,
                                        IMetadata       const result
                                    ) override;
/* methods */
protected:
    status_t                        returnSelectedBuffers();

private:
    MINT32                          m_frameNum;
    MINT32                          mCurrentReqNum;
    MINT32                          mShotCount;
    bool                            mbUsedLatest;

    //For cpu boost
    std::unique_ptr<
        CpuCtrl,
        std::function< void(CpuCtrl*) >
    >
    mCpuCtrl;
};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSSmartShot
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_COllECTSHOT_H_
