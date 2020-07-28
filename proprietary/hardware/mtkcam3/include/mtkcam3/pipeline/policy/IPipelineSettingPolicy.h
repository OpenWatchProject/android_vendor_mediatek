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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IPIPELINESETTINGPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IPIPELINESETTINGPOLICY_H_
//
#include "types.h"
//
#include <mtkcam3/pipeline/pipeline/IPipelineContext.h>
#include <mtkcam3/pipeline/stream/IStreamInfoBuilder.h>
//
#include <map>
#include <memory>
#include <vector>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace pipelinesetting {


/**
 * Used on the output of evaluateConfiguration().
 *
 */
struct ConfigurationOutputParams
{
    /**
     * @param pStreamingFeatureSetting: The streaming feature settings.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     */
    StreamingFeatureSetting*                    pStreamingFeatureSetting = nullptr;

    /**
     * @param pStreamingFeatureSetting: The capture feature settings.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     */
    CaptureFeatureSetting*                      pCaptureFeatureSetting = nullptr;


    /**
     * It indicates which pipeline nodes are needed.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     */
    PipelineNodesNeed*                          pPipelineNodesNeed = nullptr;

    /**
     * The pipeline topology.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     */
    PipelineTopology*                           pPipelineTopology = nullptr;

    /**
     * The sensor settings.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     * During reconfiguration, due to sensorSetting changing, callers must fill in the updated settings,
     * and enable related flags in ConfigurationInputParams, so that they won't be modified in SensorSettingPolicy.
     */
    std::vector<SensorSetting>*                 pSensorSetting = nullptr;

    /**
     * P1 hardware settings.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     */
    std::vector<P1HwSetting>*                   pP1HwSetting = nullptr;

    /**
     * P1 DMA need.
     *
     * The value shows which dma are needed.
     * For example,
     *      ((*pvP1DmaNeed)[0] & P1_IMGO) != 0 indicates that IMGO is needed for sensorId[0].
     *      ((*pvP1DmaNeed)[1] & P1_RRZO) != 0 indicates that RRZO is needed for sensorId[1].
     */
    std::vector<uint32_t>*                      pP1DmaNeed = nullptr;

    /**
     * P1-specific stream info configuration.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     */
    std::vector<ParsedStreamInfo_P1>*           pParsedStreamInfo_P1 = nullptr;

    /**
     * Non-P1-specific stream info configuration.
     *
     * Callers must ensure it's not a nullptr before this call.
     * Callee will fill its content on this call.
     */
    ParsedStreamInfo_NonP1*                     pParsedStreamInfo_NonP1 = nullptr;

    bool*                                       pIsZSLMode = nullptr;

};


/**
 * Used on the input of evaluateConfiguration().
 *
 */
struct ConfigurationInputParams
{
   /**
   * Flags to tell which params need to be modified during reconfiguration.
   * Callers must enable the flag for each param needs to be modified.
   */
    bool                                        bypassSensorSetting = false;

    /**
   * Flags to tell if nodeNeedPolicy is no needed.
   * Callers must enable the flag for each param needs to be modified.
   */
    bool                                        bypassNodeNeedPolicy = false;

};


/**
 * Used in the following structures:
 *      RequestOutputParams
 */
struct RequestResultParams
{
    using NodeSet       = NSCam::v3::pipeline::NSPipelineContext::NodeSet;
    using NodeEdgeSet   = NSCam::v3::pipeline::NSPipelineContext::NodeEdgeSet;
    using IOMapSet      = NSCam::v3::pipeline::NSPipelineContext::IOMapSet;

    /**
     *  Pipeline nodes need.
     *  true indicates its corresponding pipeline node is needed.
     */
    PipelineNodesNeed                           nodesNeed;

    /**
     *  Node set of a pipeline used
     */
    std::vector<NodeId_T>                       nodeSet;

    /**
     * The root nodes of a pipeline.
     */
    NodeSet                                     roots;

    /**
     * The edges to connect pipeline nodes.
     */
    NodeEdgeSet                                 edges;

    /**
     * IOMapSet for all pipeline nodes.
     */
    std::unordered_map<NodeId_T, IOMapSet>      nodeIOMapImage;
    std::unordered_map<NodeId_T, IOMapSet>      nodeIOMapMeta;

    /**
     * P1 DMA need, sent at the request stage.
     *
     * The value shows which dma are needed.
     * For example,
     *      (needP1Dma[0] & P1_IMGO) != 0 indicates that IMGO is needed for sensorId[0].
     *      (needP1Dma[1] & P1_RRZO) != 0 indicates that RRZO is needed for sensorId[1].
     */
    std::vector<uint32_t>                       needP1Dma;

    /**
     * Updated image stream info.
     *
     * For example, they could be Jpeg_YUV and Thumbnail_YUV for capture with rotation.
     */
    std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>>
                                                vUpdatedImageStreamInfo;

    /**
     * Additional metadata
     *
     * The design should avoid to override the app metadata control as possible as we can.
     */
    std::shared_ptr<IMetadata>                  additionalApp;
    std::vector<std::shared_ptr<IMetadata> >    additionalHal;

    /**
     * Need to keep P1 image & meta output buffers or not.
     *
     * true indicates P1 image & meta output buffers for the current request are needed to be kept,
     * and those kept buffers will be used for an App reprocess request which will be sent later.
     */
    bool                                        needKeepP1BuffersForAppReprocess = false;

    /**
     * Used to indicate physical metadata stream id in this request.
     */
    std::unordered_map<uint32_t, std::vector<StreamId_T> >
                                                physicalMetaStreamIds;

};


/**
 * Used on the output of evaluateRequest().
 *
 * Request-stage policy
 *  $ Need to re-configure or not?
 *  $ New sensor mode settings if changed (e.g. 4cell)
 *  $ The frame sequence: pre-dummy frames, main frame, sub frames, post-dummy frames
 *  $ IMGO / RRZO / RSSO settings
 *    . Process RAW (e.g. type3 PD sensor)
 *    . format: UFO/Unpack RAW/Pack RAW (e.g. HDR)
 *    . size (e.g. streaming)
 *  $ Frame rate change (via metadata) (e.g. 60fps capture for special sensors)
 *  $ ZSL flow or non-ZSL flow
 *  $ ZSL selection policy
 *  $ ......
 */
struct RequestOutputParams
{
    /**
     * If this is true, it means this policy requests to re-configure the pipeline. In this case,
     * all the following output results are evaluated based on the after-reconfiguration setting,
     * not the before-reconfiguration setting.
     */
    bool                                        needReconfiguration = false;

    /**
     * The frame sequence is as below:
     *       pre dummy frame 0
     *       pre dummy frame ...
     *       pre dummy frame X-1
     *       pre   sub frame 0
     *       pre   sub frame ...
     *       pre   sub frame P-1
     *            main frame        (should be aligned to the request sent from users)
     *             sub frame 0
     *             sub frame ...
     *             sub frame Y-1
     *      post dummy frame 0
     *      post dummy frame ...
     *      post dummy frame Z-1
     *
     * The policy module is in charge of allocating the memory when needed.
     */
    std::shared_ptr<RequestResultParams>                mainFrame;
    std::vector<std::shared_ptr<RequestResultParams>>   subFrames;
    std::vector<std::shared_ptr<RequestResultParams>>   preSubFrames;
    std::vector<std::shared_ptr<RequestResultParams>>   preDummyFrames;     //  need dummy frames
    std::vector<std::shared_ptr<RequestResultParams>>   postDummyFrames;    //  need dummy frames

    /**
     * ZSL still capture flow is needed if true; otherwise not needed.
     */
    bool                                        needZslFlow = false;
    ZslPolicyParams                             zslPolicyParams;

    /**
     * CShot
     */
    bool                                        bCshotRequest = false;

    /**
     * Reconfig Category
     * 0:No reconfig, 1:Stream reconfig, 2:Capture reconfig
     */
    ReCfgCtg                                    reconfigCategory = ReCfgCtg::NO;

    /**
     *  The result sensor setting
     */
    std::vector<uint32_t>                       sensorMode;
    // boost BWC
    std::vector<BoostControl>                   vboostControl;
    /**
     * [TODO]
     *
     *  $ New sensor mode settings if changed (e.g. 4cell)
     *  $ IMGO / RRZO / RSSO settings
     *    . Process RAW (e.g. type3 PD sensor)
     *    . format: UFO/Unpack RAW/Pack RAW (e.g. HDR)
     *    . size (e.g. streaming)
     *  $ ZSL flow or non-ZSL flow
     *  $ ZSL selection policy
     *  $ ......
     */
    bool                                        keepZslBuffer   = true;

    /**
     * Request output params for multicam.
     */
    MultiCamReqOutputParams                     multicamReqOutputParams;
};


/**
 * Used on the input of evaluateRequest().
 */
struct RequestInputParams
{
    /**************************************************************************
     * Request parameters
     *
     * The parameters related to this capture request is shown as below.
     *
     **************************************************************************/

    /**
     * Request number, sent at the request stage.
     *
     */
    uint32_t                                    requestNo = 0;

    /**
     * Request App image stream info, sent at the request stage.
     *
     */
    ParsedAppImageStreamInfo const*             pRequest_AppImageStreamInfo = nullptr;

    /**
     * Request App metadata control, sent at the request stage.
     *
     * pRequest_ParsedAppMetaControl is a partial parsed result from pRequest_AppControl,
     * just for the purpose of a quick reference.
     */
    IMetadata const*                            pRequest_AppControl = nullptr;
    ParsedMetaControl const*                    pRequest_ParsedAppMetaControl = nullptr;


    /*************************************************************************
     * Configuration info.
     *
     * The final configuration information of the pipeline decided at the
     * configuration stage are as below.
     *
     **************************************************************************/

    /**
     * Configured pipeline nodes, built up at the configuration stage.
     *
     * It indicates which pipeline nodes are configured at the configuration stage.
     */
    PipelineNodesNeed const*                    pConfiguration_PipelineNodesNeed = nullptr;

    /**
     * Configuration stream info, built up at the configuration stage.
     *
     * Parsed Non-P1 stream info.
     */
    ParsedStreamInfo_NonP1 const*               pConfiguration_StreamInfo_NonP1 = nullptr;

    /**
     * Configuration stream info, built up at the configuration stage.
     *
     * Parsed P1 stream info.
     */
    std::vector<ParsedStreamInfo_P1> const*     pConfiguration_StreamInfo_P1 = nullptr;


    /**************************************************************************
     * Current Setting
     *
     **************************************************************************/

    /**
     * The current sensor setting
     *
     * pSensorMode and pSensorSize are the pointers to two arraies of sensor modes
     * and sensor sizes, respectively.
     * The array size is the same to the size of sensor id (i.e. PipelineStaticInfo::sensorId).
     */
    std::vector<uint32_t> const*                pSensorMode = nullptr;
    std::vector<MSize> const*                   pSensorSize = nullptr;
    //
    bool                                        isZSLMode   = false;
    //
    std::vector<P1HwSetting> const*             pP1HwSetting = nullptr;

};

/**
 * Policy data type enum.
 */
enum class PolicyType
{
    /* Request sensor control policy */
    SensorControlPolicy,
};

/**
 *
 */
class IPipelineSettingPolicy
{
public:
    virtual         ~IPipelineSettingPolicy() = default;

    /**
     * The policy is in charge of deciding the maximum buffer number of each App image stream
     * which must be decided at the configuration stage.
     *
     * @param[in/out] pInOut
     *  Before this call, callers must promise each App image stream info instance.
     *  On this call, each App image stream info's 'setMaxBufNum()'
     *  must be called to set up its maximum buffer number.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    decideConfiguredAppImageStreamMaxBufNum(
                        ParsedAppImageStreamInfo* pInOut,
                        StreamingFeatureSetting const* pStreamingFeatureSetting,
                        CaptureFeatureSetting const* pCaptureFeatureSetting
                    ) -> int                                                = 0;

    /**
     * The policy is in charge of deciding the configuration settings at the configuration stage.
     *
     * @param[out] out: output parameters.
     *
     * @param[in] in: input parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    evaluateConfiguration(
                        ConfigurationOutputParams& out,
                        ConfigurationInputParams const& in
                    ) -> int                                                = 0;

    /**
     * The policy is in charge of reporting its requirement at the request stage.
     *
     * @param[out] out:
     *  Callers must ensure it's a non-nullptr.
     *
     * @param[in] in:
     *  Callers must promise its content. The callee is not allowed to modify it.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual auto    evaluateRequest(
                        RequestOutputParams& out,
                        RequestInputParams const& in
                    ) -> int                                                = 0;

    /**
     * Send data to policy
     *
     * @param[in] in:
     *  policyType policy type.
     *  arg1 argument 1.
     *  arg1 argument 2.
     *  arg1 argument 3.
     *
     * @return
     *      true indicates success; otherwise failure.
     */
    virtual auto    sendPolicyDataCallback(
                        PolicyType policyType,
                        MUINTPTR arg1,
                        MUINTPTR arg2,
                        MUINTPTR arg3
                    ) -> bool                                               = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
class IPipelineSettingPolicyFactory
{
public:     ////

    /**
     * A structure for creation parameters.
     */
    struct  CreationParams
    {
        std::shared_ptr<PipelineStaticInfo const>       pPipelineStaticInfo;
        std::shared_ptr<PipelineUserConfiguration const>pPipelineUserConfiguration;

        /**
         * The factory of App stream info builder.
         */
        std::shared_ptr<IStreamInfoBuilderFactory>      pAppStreamInfoBuilderFactory;

    };

    static  auto    createPipelineSettingPolicy(
                        CreationParams const& params
                    ) -> std::shared_ptr<IPipelineSettingPolicy>;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IPIPELINESETTINGPOLICY_H_

