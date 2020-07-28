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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONBASIC_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONBASIC_H_
//
#include "PipelineModelSessionBase.h"
//
#include <utils/RWLock.h>
//
#include <mtkcam/utils/hw/IScenarioControlV3.h>
#include <impl/ICaptureInFlightRequest.h>
#include <impl/IZslProcessor.h>

#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/ULogDef.h>
#include <utils/Mutex.h>

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
class PipelineModelSessionBasic
    : public PipelineModelSessionBase
    , public NSCam::v3::pipeline::NSPipelineContext::DataCallbackBase
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
    mutable android::RWLock         mRWLock_ConfigInfo2;
    std::shared_ptr<ConfigInfo2>    mConfigInfo2;

protected:  ////    private configuration info (unchangable unless re-configuration)
    mutable android::RWLock         mRWLock_PipelineContext;
    android::sp<IPipelineContextT>  mCurrentPipelineContext;

    /**
     * This flag is used to decide how to build the pipeline context.
     * true: to configure pipeline nodes parallelly and concurrently.
     * false: otherwise
     */
    bool mbUsingParallelNodeToBuildPipelineContext;
    bool mbUsingMultiThreadToBuildPipelineContext;

protected:  ////    private request info (changable)

    /**
     * The current sensor settings.
     */
    std::vector<uint32_t>           mSensorMode;
    std::vector<MSize>              mSensorSize;

protected:  ////    private data members.
    android::sp<IScenarioControlV3> mpScenarioCtrl;
    // node id and open id
    std::unordered_map<NodeId_T, MINT32> mvRootNodeOpenIdList;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Template Methods: member access
    virtual auto    getCurrentConfigInfo2() const -> std::shared_ptr<ConfigInfo2>;
    virtual auto    getCaptureInFlightRequest() -> android::sp<ICaptureInFlightRequest> { return nullptr; }
    virtual auto    getZslProcessor() -> android::sp<IZslProcessor> { return nullptr; }

protected:  ////    Template Methods: called by configure()
    virtual auto    onConfig_ConfigInfo2() -> int;
    virtual auto    onConfig_AppImageStreamMaxBufNum() -> int;
    virtual auto    onConfig_Capture() -> int;
    virtual auto    onConfig_DVFS() -> int;
    virtual auto    onConfig_BeforeBuildingPipelineContext() -> int;
    virtual auto    onConfig_BuildingPipelineContext() -> int;
    virtual auto    onConfig_Ending() -> int;

protected:  ////    Template Methods: called by submitRequest()
    virtual auto    submitOneRequest(
                        std::shared_ptr<ParsedAppRequest>const& request
                    ) -> int override;

protected:  ////    Template Methods: called by submitOneRequest()

    /**
     * A hook function to evaluate a given request.
     *
     * @param[out] reqOutput: the evaluated output for the given request.
     *
     */
    virtual auto    onRequest_EvaluateRequest(
                        policy::pipelinesetting::RequestOutputParams& reqOutput,
                        std::shared_ptr<ParsedAppRequest>const& pRequest,
                        std::shared_ptr<ConfigInfo2>const& pConfigInfo2
                    ) -> int;

    /**
     * A hook function for the reconfiguration.
     *
     * @param[in/out] pConfigInfo2: the configuration information.
     *  Attention: It may change after this call.
     *
     */
    virtual auto    onRequest_Reconfiguration(
                        std::shared_ptr<ConfigInfo2>& pConfigInfo2,
                        policy::pipelinesetting::RequestOutputParams const& reqOutput,
                        std::shared_ptr<ParsedAppRequest>const& pRequest
                    ) -> int;

    /**
     * A hook function to process the evaluated result, including building pipeline frame(s)
     * and queuing the pipeline frame(s) into the pipeline.
     *
     */
    virtual auto    onRequest_ProcessEvaluatedFrame(
                        std::shared_ptr<policy::pipelinesetting::RequestOutputParams>const& pReqOutputParams,
                        std::shared_ptr<ParsedAppRequest>const& pRequest,
                        std::shared_ptr<ConfigInfo2>const& pConfigInfo2
                    ) -> int;

    virtual auto    onRequest_Ending(
                        policy::pipelinesetting::RequestOutputParams const& reqOutput
                    ) -> int;

protected:  ////    Operations (Configuration Stage).
    // enter scenario in configrue stage, maybe need to re-design
    virtual auto    configureDVFSControl(
                        uint32_t scenario,
                        uint32_t featureFlag,
                        std::shared_ptr<ConfigInfo2> pUpdatedConfigInfo2 = nullptr
                    ) -> int;

protected:  ////    Operations (Request Stage).

    /**
     * Process frame(s) which have been evaluated.
     *
     * @param[in] pReqOutputParams: the request output parameters
     *  resulted from IPipelineSettingPolicy::evaluateRequest().
     *
     * @param[in] pAppMetaControl: the original control App metadata
     *  sent from the application/frameworks.
     *
     * @param[in] pAppRequest: the original request sent from the application/frameworks.
     *
     * @param[in] pConfigInfo2: the configuration information.
     *
     * @param[in] pPipelineContext: the PipelineContext to queue.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    struct ProcessEvaluatedFrame
    {
        std::shared_ptr<policy::pipelinesetting::RequestOutputParams>
                                                    pReqOutputParams;
        std::shared_ptr<IMetadata>                  pAppMetaControl;
        std::shared_ptr<ParsedAppRequest>           pAppRequest;
        std::shared_ptr<ConfigInfo2>                pConfigInfo2;
        android::sp<IPipelineContextT>              pPipelineContext;
    };
    virtual auto    processEvaluatedFrame(
                        ProcessEvaluatedFrame const& in
                    ) -> int;

    /**
     * Process frame(s) which have been evaluated.
     *
     * @param[out] lastFrameNo: the last frame number.
     *
     * @param[in] in: input parameters
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    onProcessEvaluatedFrame(
                        uint32_t& lastFrameNo,
                        ProcessEvaluatedFrame const& in
                    ) -> int                                                = 0;

public:     ////    Utility.
    static  auto    print(android::Printer& printer, ConfigInfo2 const& o);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces (called by Session Factory).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
                    PipelineModelSessionBasic(
                        std::string const& name,
                        CtorParams const& rCtorParams);

public:     ////    Configuration.
    virtual auto    configure() -> int;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelSession Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    dumpState(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                        ) -> void override;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NSPipelineContext::DataCallbackBase Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual auto    onMetaResultAvailable(MetaResultAvailable&& arg) -> void override;
    virtual MVOID   onNextCaptureCallBack(MUINT32 requestNo, MINTPTR nodeId, MUINT32 requestCnt, MBOOL bSkipCheck) override;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONBASIC_H_

