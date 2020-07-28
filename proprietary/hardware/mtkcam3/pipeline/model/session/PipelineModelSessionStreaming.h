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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONSTREAMING_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONSTREAMING_H_
//
#include "PipelineModelSessionDefault.h"
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/Type.h>


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
class PipelineModelSessionStreaming
: public PipelineModelSessionDefault
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces (called by Session Factory).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    static  auto    makeInstance(
                        std::string const& name,
                        CtorParams const& rCtorParams
                        ) -> android::sp<IPipelineModelSession>;

                    PipelineModelSessionStreaming(
                        std::string const& name,
                        CtorParams const& rCtorParams
                    );

    virtual         ~PipelineModelSessionStreaming();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:
    //methods
    virtual auto    processReconfiguration(
                        policy::pipelinesetting::RequestOutputParams const& rcfOutputParam,
                        std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
                        MUINT32 requestNo
                    ) -> int override;

    virtual auto    beginFlush() -> int override;

    auto            waitUntilNodeDrainedAndFlush(
                        NodeId_T const nodeId
                    ) -> MERROR;

    //variables
    mutable android::RWLock         mRWLock_vCapConfigInfo2;
    mutable std::mutex              mMutexLock_Context;
    std::unordered_map<MUINT32, std::shared_ptr<ConfigInfo2>> mvCapConfigInfo2;

private:
    auto            waitUntilP1NodeDrainedAndFlush() -> MERROR;
    auto            waitUntilP2DrainedAndFlush() -> MERROR;

    auto            processReconfigStream(
                        policy::pipelinesetting::RequestOutputParams const& rcfOutputParam,
                        std::shared_ptr<ConfigInfo2>&    pConfigInfo2,
                        MUINT32 requestNo
                    ) -> int;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONSTREAMING_H_

