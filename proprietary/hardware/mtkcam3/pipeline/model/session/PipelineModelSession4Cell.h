/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSION4CELL_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSION4CELL_H_
//
#include "PipelineModelSessionDefault.h"
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/Type.h>
#include <future>
#include <semaphore.h>
#include <mtkcam/utils/hw/IResourceConcurrency.h>
#include <cutils/properties.h>
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
class PipelineModelSession4Cell
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

                    PipelineModelSession4Cell(
                        std::string const& name,
                        CtorParams const& rCtorParams
                    );

    virtual         ~PipelineModelSession4Cell();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual MVOID   updateFrame(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result
                    );

protected:
    //methods
    virtual auto    submitOneRequest(
                        std::shared_ptr<ParsedAppRequest>const& request
                    ) -> int override;

    virtual auto    configure() -> int override;

    virtual auto    beginFlush() -> int override;

    virtual auto    endFlush() -> void override;

    auto            waitUntilNodeDrainedAndFlush(
                        NodeId_T const nodeId,
                        android::sp<IPipelineContextT> pPipelineContext
                    ) -> MERROR;

    auto            waitUntilNodeDrained(
                        NodeId_T const nodeId,
                        android::sp<IPipelineContextT> pPipelineContext
                    ) -> MERROR;
    //variables
    mutable android::RWLock         mRWLock_vCapConfigInfo2;
    std::unordered_map<MUINT32, std::shared_ptr<ConfigInfo2>> mvCapConfigInfo2;
    sem_t                           mP1ConfigLock;
    mutable android::RWLock         mRWLock_vCapFut;
    std::vector<std::future<int>>   mvCaptureFuture;

    //ResourceConcurrency for multi-thread capture
    android::sp<IResourceConcurrency>
                                    mP1NodeConcurrency = 0;
    MBOOL                           mbNeedRcgSensor = MFALSE;
    MINT32                          mbDebugLogLevel;
private:
    auto            processReconfiguration_4cell(
                        std::shared_ptr<policy::pipelinesetting::RequestOutputParams> pReqOutParm,
                        std::shared_ptr<ConfigInfo2>& pConfigInfo2 __unused,
                        std::shared_ptr<ParsedAppRequest>const& request,
                        std::shared_ptr<IMetadata> pAppControl,
                        MUINT32 requestNo
                    ) -> int;

    auto            waitUntilP1NodeDrainedAndFlush(
                        android::sp<IPipelineContextT> pPipelineContext
                    ) -> MERROR;

    auto            waitUntilP1NodeDrained(
                        android::sp<IPipelineContextT> pPipelineContext
                    ) -> MERROR;

    auto            waitUntilP2DrainedAndFlush(
                        android::sp<IPipelineContextT> pPipelineContext
                    ) -> MERROR;

    auto            waitUntilP2JpegDrainedAndFlush(
                        android::sp<IPipelineContextT> pPipelineContext
                    ) -> MERROR;

    auto            processReconfigStream(
                        std::shared_ptr<ConfigInfo2>&    pConfigInfo2,
                        MUINT32 requestNo
                    ) -> int;

    auto            doCapture(
                        std::shared_ptr<policy::pipelinesetting::RequestOutputParams> pReqOutParm,
                        std::shared_ptr<ParsedAppRequest>const request,
                        std::shared_ptr<IMetadata> pAppControl,
                        MUINT32 requestNo
                    )-> int;

    auto            waitUntilCaptureDone() -> int;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSION4CELL_H_