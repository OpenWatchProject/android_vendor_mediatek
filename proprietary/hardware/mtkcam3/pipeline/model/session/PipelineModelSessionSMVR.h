/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONSMVR_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONSMVR_H_
//
#include "PipelineModelSessionBase.h"
//
#include <utils/RWLock.h>
//
#include <mtkcam/utils/hw/IScenarioControlV3.h>

using IScenarioControlV3
    = NSCam::v3::pipeline::model::IScenarioControlV3;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/******************************************************************************
 *
 ******************************************************************************/
class PipelineModelSessionSMVR
    : public PipelineModelSessionBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    2nd configuration info (unchangable unless re-configuration)
    struct ConfigInfo2
    {
        StreamingFeatureSetting                     mStreamingFeatureSetting;
        CaptureFeatureSetting                       mCaptureFeatureSetting;
        PipelineNodesNeed                           mPipelineNodesNeed;
        PipelineTopology                            mPipelineTopology;
        std::vector<SensorSetting>                  mvSensorSetting;
        std::vector<P1HwSetting>                    mvP1HwSetting;
        std::vector<uint32_t>                       mvP1DmaNeed;
        std::vector<ParsedStreamInfo_P1>            mvParsedStreamInfo_P1;
        ParsedStreamInfo_NonP1                      mParsedStreamInfo_NonP1;
        bool                                        mIsZSLMode = false;
    };
    std::shared_ptr<ConfigInfo2>    mConfigInfo2;

protected:  ////    private configuration info (unchangable unless re-configuration)
    mutable android::RWLock         mRWLock_PipelineContext;
    android::sp<IPipelineContextT>  mCurrentPipelineContext;

protected:  ////    private data members.
    android::sp<IScenarioControlV3>   mpScenarioCtrl;

protected:  ////    private request info (changable)

    /**
     * The current sensor settings.
     */
    std::vector<uint32_t>           mSensorMode;
    std::vector<MSize>              mSensorSize;

    // android.control.availableHighSpeedVideoConfigurations
    uint32_t                        mAeTargetFpsMin;
    uint32_t                        mAeTargetFpsMax;
    uint32_t                        mDefaultBatchSize;




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Template Methods.

    virtual auto    getCurrentPipelineContext() const -> android::sp<IPipelineContextT> override;

    virtual auto    submitOneRequest(
                        std::shared_ptr<ParsedAppRequest>const& request
                    ) -> int override;

private:    ////    Configuration Stage.

    // enter scenario in configrue stage, maybe need to re-design
    auto            configureDVFSControl() -> int;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual MVOID   updateFrame(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result
                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelSession Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    submitRequest(
                        std::vector<std::shared_ptr<UserRequestParams>>const& requests,
                        uint32_t& numRequestProcessed
                        ) -> int override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces (called by Session Factory).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    static  auto    makeInstance(
                        CtorParams const& rCtorParams
                        ) -> android::sp<IPipelineModelSession>;

                    PipelineModelSessionSMVR(CtorParams const& rCtorParams);

public:     ////    Configuration.
    virtual auto    configure() -> int;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONDEFAULT_H_

