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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONDEFAULT_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONDEFAULT_H_
//
#include "PipelineModelSessionBasic.h"
//
#include <utils/RWLock.h>
//
#include <impl/ICaptureInFlightRequest.h>
#include <impl/INextCaptureListener.h>
//
#include <impl/IZslProcessor.h>
#include <impl/IAppRaw16Reprocessor.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>


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
class PipelineModelSessionDefault
    : public PipelineModelSessionBasic
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    using IFrameBuilderT = NSCam::v3::pipeline::NSPipelineContext::IFrameBuilder;
    using GroupFrameType = NSCam::v3::pipeline::NSPipelineContext::GroupFrameType;
    using TrackFrameResultParams = NSCam::v3::pipeline::NSPipelineContext::TrackFrameResultParams;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    private data members.
    android::sp<IZslProcessor>           mpZslProcessor;

    android::sp<IAppRaw16Reprocessor>    mpAppRaw16Reprocessor;

    // Capture In Flight Request
    android::sp<ICaptureInFlightRequest> mpCaptureInFlightRequest;
    android::sp<INextCaptureListener>    mpNextCaptureListener;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations for ZSL.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    auto            configZSL() -> void;

    auto            flushZslPendingReq(
                        const android::sp<IPipelineContextT>& pPipelineContext
                    ) -> int;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations (IAppRaw16Reprocessor).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    /**
     * Try to handle an App RAW16 reprocess frame if it is.
     *
     * @param[out] pFrameBuilder
     *  The frame builder used to add Hal meta/image stream buffers into.
     *
     * @param[in] pReqResult
     *  The evaluated result for the request.
     *
     * @param[in] isMainFrame
     *  true indicates it's a main frame.
     *
     * @param[in] pAppMetaControl
     *  The Request App metadata control, sent at the request stage.
     *
     * @param[in] pAppRequest
     *  The App request, sent at the request stage.
     *
     * @param[in] pConfigInfo2
     *  The configuration information.
     *
     * @param[in] pAppRaw16Reprocessor
     *  The IAppRaw16Reprocessor instance. Callers must ensure it's valid.
     *
     * @return
     *  true indicates it's an App RAW16 reprocess frame; otherwise, it's not.
     */
    struct TryHandleAppRaw16ReprocessFrameParams
    {
        using RequestResultParams = NSCam::v3::pipeline::policy::pipelinesetting::RequestResultParams;

        std::shared_ptr<NSPipelineContext::IFrameBuilder>
                                                        pFrameBuilder = nullptr;
        RequestResultParams const*                      pReqResult = nullptr;
        bool                                            isMainFrame = true;
        std::shared_ptr<IMetadata>                      pAppMetaControl = nullptr;
        std::shared_ptr<ParsedAppRequest>               pAppRequest = nullptr;
        std::shared_ptr<ConfigInfo2>                    pConfigInfo2 = nullptr;
        android::sp<IAppRaw16Reprocessor>               pAppRaw16Reprocessor = nullptr;
    };
    static auto     tryHandleAppRaw16ReprocessFrame(
                        TryHandleAppRaw16ReprocessFrameParams const& params
                    ) -> bool;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Template Methods: member access

    virtual auto    getCurrentPipelineContext() const -> android::sp<IPipelineContextT> override;
    virtual auto    getCaptureInFlightRequest() -> android::sp<ICaptureInFlightRequest> override;
    virtual auto    getZslProcessor() -> android::sp<IZslProcessor> override;
    virtual auto    getAppRaw16Reprocessor() const -> android::sp<IAppRaw16Reprocessor>;

protected:  ////    Template Methods: called by configure()
    virtual auto    onConfig_Capture() -> int;
    virtual auto    onConfig_BeforeBuildingPipelineContext() -> int;

protected:  ////    Template Methods: called by submitOneRequest()
    virtual auto    onRequest_Reconfiguration(
                        std::shared_ptr<ConfigInfo2>& pConfigInfo2,
                        policy::pipelinesetting::RequestOutputParams const& reqOutput,
                        std::shared_ptr<ParsedAppRequest>const& pRequest
                    ) -> int override;

    virtual auto    processReconfiguration(
                        policy::pipelinesetting::RequestOutputParams const& rcfOutputParam,
                        std::shared_ptr<ConfigInfo2>& pConfigInfo2,
                        MUINT32 requestNo
                    ) -> int;

protected:  ////    Operations (Configuration Stage).

    // create capture related instance
    virtual auto    configureCaptureInFlight(const int maxJpegNum, const int maxImgoNum) -> int;

protected:  ////    Operations (Request Stage).

    virtual auto    onProcessEvaluatedFrame(
                        uint32_t& lastFrameNo,
                        ProcessEvaluatedFrame const& in
                    ) -> int override;

    virtual auto    convertRequestResultToFrameBuilder(
                        GroupFrameType frameType,
                        policy::pipelinesetting::RequestResultParams const& reqResult,
                        ProcessEvaluatedFrame const& in
                    ) -> std::shared_ptr<IFrameBuilderT>;

                    /**
                     * process tryKeepFrameForAppRaw16Reprocess by each IPipelineFrame
                     * before submitting it to IPipelineContext.
                     */
    virtual auto    tryKeepFrameForAppRaw16Reprocess(
                        android::sp<IPipelineFrame> pPipelineFrame,
                        policy::pipelinesetting::RequestResultParams const* pReqResult,
                        std::shared_ptr<IMetadata> pAppMetaControl,
                        std::shared_ptr<ParsedAppRequest> request
                    ) -> void;

protected:  ////    Operations.

    virtual auto    updateFrameTimestamp(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result,
                        int64_t timestampStartOfFrame
                    ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces (called by Session Factory).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    struct ExtCtorParams
                    {
                        android::sp<IAppRaw16Reprocessor>   pAppRaw16Reprocessor = nullptr;
                    };
    static  auto    makeInstance(
                        std::string const& name,
                        CtorParams const& rCtorParams,
                        ExtCtorParams const& rExtCtorParams
                        ) -> android::sp<IPipelineModelSession>;
    static  auto    makeInstance(
                        std::string const& name,
                        CtorParams const& rCtorParams
                        ) -> android::sp<IPipelineModelSession>;

                    PipelineModelSessionDefault(
                        std::string const& name,
                        CtorParams const& rCtorParams,
                        ExtCtorParams const& rExtCtorParams);
                    PipelineModelSessionDefault(
                        std::string const& name,
                        CtorParams const& rCtorParams);
    virtual         ~PipelineModelSessionDefault();

public:     ////    Configuration.
    virtual auto    configure() -> int override;
    virtual auto    updateBeforeBuildPipelineContext() -> int;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual MVOID   updateFrame(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result
                    ) override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelSession Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                        ) -> void override;

    virtual auto    beginFlush() -> int override;

    virtual auto    endFlush() -> void override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSPipelineContext::DataCallbackBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual MVOID   onNextCaptureCallBack(
                        MUINT32   requestNo,
                        MINTPTR   nodeId,
                        MUINT32   requestCnt,
                        MBOOL     bSkipCheck
                    ) override;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONDEFAULT_H_

