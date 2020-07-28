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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONMULTICAM_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONMULTICAM_H_
//
#include "PipelineModelSessionDefault.h"
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/LogicalCam/Type.h>

#include <mtkcam/utils/std/JobQueue.h>
#include <mtkcam/aaa/IHal3A.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {
/******************************************************************************
 * multicam utility
 ******************************************************************************/
class MultiCamMetadataUtility
{
public:
    MultiCamMetadataUtility() = delete;
public:
    static bool filterResultByResultKey(int32_t sensorId, const IMetadata& src, IMetadata& dst);
};
/******************************************************************************
 * multicam notifier utility
 ******************************************************************************/
class MulticamZoomProcessor
{
private:
public:
    enum TYPE
    {
        None,
        SetMasterId,
        SwitchSensorDone,
    };
public:
    MulticamZoomProcessor(
        std::shared_ptr<policy::pipelinesetting::IPipelineSettingPolicy> pSettingPolicy);
    ~MulticamZoomProcessor();
public:
    auto sendNotify(
            TYPE type,
            MUINTPTR arg1,
            MUINTPTR arg2,
            MUINTPTR arg3) -> bool;
private:
    //
    uint32_t mMasterId = std::numeric_limits<uint32_t>::max();
    uint32_t mFrameNo = 0;
    //
    std::weak_ptr<policy::pipelinesetting::IPipelineSettingPolicy> mpSettingPolicy;
};
/******************************************************************************
 *
 ******************************************************************************/
class PipelineModelSessionMultiCam
    : public PipelineModelSessionDefault
{
public:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces (called by Session Factory).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    static  auto    makeInstance(
                        std::string const& name,
                        CtorParams const& rCtorParams
                        ) -> android::sp<IPipelineModelSession>;
    PipelineModelSessionMultiCam(
            std::string const& name,
            CtorParams const& rCtorParams);
    virtual ~PipelineModelSessionMultiCam();
public:     ////    Configuration.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  override base implement
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    auto    configure() -> int override final;
    auto    updateBeforeBuildPipelineContext() -> int override final;
    auto    updateFrame(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result
                    ) -> MVOID override final;
    auto    beginFlush() -> int override final;
    /**
     * A hook function for the reconfiguration.
     *
     * @param[in/out] pConfigInfo2: the configuration information.
     *  Attention: It may change after this call.
     *
     */
    auto    onRequest_Reconfiguration(
                        std::shared_ptr<ConfigInfo2>& pConfigInfo2,
                        policy::pipelinesetting::RequestOutputParams const& reqOutput,
                        std::shared_ptr<ParsedAppRequest>const& pRequest
                    ) -> int override final;

    auto    onRequest_ProcessEvaluatedFrame(
                        std::shared_ptr<policy::pipelinesetting::RequestOutputParams>const& pReqOutputParams __unused,
                        std::shared_ptr<ParsedAppRequest>const& pRequest __unused,
                        std::shared_ptr<ConfigInfo2>const& pConfigInfo2 __unused
                    ) -> int override final;

public:
    std::function<bool(void)> mDualFeatureConfiguration = nullptr;
    std::function<
                bool(
                    MUINT32 const requestNo,
                    MINTPTR const userId,
                    Result const& result)>
                        mDualFeatureFrameUpdate = nullptr;
public:
    auto configure_multicam() -> bool;
    auto configure_multicam_zoom() -> bool;
    auto updateFrame_multicam(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result) -> bool;
private:
    auto    prepareSensorObject() -> bool;
    auto    setSensorSyncToSensorDriver(bool enable) -> bool;
    auto    evaluatePreviewSize(MSize &size) -> bool;
    auto    getMasterCamId(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        Result const& result,
                        MINT32 &masterId) -> bool;
    auto    doSwitchSensorFlow(
                policy::pipelinesetting::RequestOutputParams const& reqOutput) -> bool;
    auto    getP1NodeIdBySensorId(
                        uint32_t const& sensorId,
                        NodeId_T &nodeId) -> bool;
    auto    flushNode(
                        sp<IPipelineContext> pPipelineContext,
                        std::vector<uint32_t> const& sensorIdList) -> bool;
    auto    setSync2A(
                        std::vector<uint32_t> const& sensorIdList) -> bool;
    auto    configNode(
                        sp<IPipelineContext> pPipelineContext,
                        std::vector<uint32_t> const& sensorIdList) -> bool;
    auto    getStreamingSensorId(
                        policy::pipelinesetting::RequestOutputParams const& reqOutput,
                        std::vector<uint32_t>& sensorIdList) -> void;
    auto    ctrl3AOnStreamingSensor(
                        std::vector<uint32_t>& sensorIdList,
                        NS3Av3::E3ACtrl_T cmd) -> bool;
    auto    sendNotify(
                        MulticamZoomProcessor::TYPE type,
                        MUINTPTR arg1,
                        MUINTPTR arg2,
                        MUINTPTR arg3) -> bool;
    auto    handleMarkErrorList(
                        std::shared_ptr<policy::pipelinesetting::RequestOutputParams>const& pReqOutputParams,
                        std::shared_ptr<ParsedAppRequest>const& pRequest) -> bool;
private:
    MINT32 mMultiCamFeatureMode = 0;
    SensorSyncType mSensorSyncType = SensorSyncType::NOT_SUPPORT;
    std::vector<MUINT32> mvSensorDevIdList;
    std::vector<NSCam::IHalSensor*> mvSensorHalInterface;
    // blacklist for vsdof, need to skip some stream id for app callback
    std::vector<int64_t> mvStreamId_BlackList;
    // thermal policy name
    std::string thermal_policy_name;
    bool mbShowLog = false;
    // [zoom]
    std::shared_ptr<MulticamZoomProcessor> mpMulticamZoomProcessor = nullptr;
    std::shared_ptr<NSCam::JobQueue<void()>> mJobQueue = nullptr;
    //
    std::unordered_map<uint32_t, std::shared_ptr<NS3Av3::IHal3A>>
                                            mvHal3AList;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_SESSION_PIPELINEMODELSESSIONMULTICAM_H_

