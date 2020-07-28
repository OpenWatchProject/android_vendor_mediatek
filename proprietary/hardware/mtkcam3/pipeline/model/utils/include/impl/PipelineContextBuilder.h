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

#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_UTILS_INCLUDE_IMPL_PIPELINECONTEXTBUILDER_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_UTILS_INCLUDE_IMPL_PIPELINECONTEXTBUILDER_H_

#include <mtkcam3/pipeline/pipeline/IPipelineContextBuilder.h>
#include <mtkcam/utils/hw/IResourceConcurrency.h>
#include <impl/types.h>

#include <string>
#include <vector>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/**
 * Unsed on the call buildPipelineContext().
 */
struct BuildPipelineContextInputParams
{
    std::string                                 pipelineName;

    /**
     * Pipeline static info. and user configuration given at the configuration stage.
     */
    PipelineStaticInfo const*                   pPipelineStaticInfo = nullptr;
    PipelineUserConfiguration const*            pPipelineUserConfiguration = nullptr;
    PipelineUserConfiguration2 const*           pPipelineUserConfiguration2 = nullptr;

    /**
     * Non-P1-specific stream info configuration.
     */
    ParsedStreamInfo_NonP1 const*               pParsedStreamInfo_NonP1 = nullptr;

    /**
     * P1-specific stream info configuration.
     */
    std::vector<ParsedStreamInfo_P1> const*     pParsedStreamInfo_P1 = nullptr;

    /**
     *  BuildStream with zsl settings.
     */
    bool                                        bIsZslEnabled = false;

    /**
     * The sensor settings.
     */
    std::vector<SensorSetting> const*           pSensorSetting = nullptr;

    /**
     * P1 hardware settings.
     */
    std::vector<P1HwSetting> const*             pvP1HwSetting = nullptr;

    /**
     * It indicates which pipeline nodes are needed.
     */
    PipelineNodesNeed const*                    pPipelineNodesNeed = nullptr;

    /**
     * Pipeline topology.
     */
    PipelineTopology const*                     pPipelineTopology = nullptr;

    /**
     * The streaming feature settings.
     */
    StreamingFeatureSetting const*              pStreamingFeatureSetting = nullptr;

    /**
     * The capture feature settings.
     */
    CaptureFeatureSetting const*                pCaptureFeatureSetting = nullptr;

    /**
     * Batch size
     *
     * A batch size is a divisor of fps / 30. For example, if fps is 300, a batch size could only be
     * one of only be 1, 2, 5, or 10.
     *
     * See static_android.control.availableHighSpeedVideoConfigurations under
     * https://android.googlesource.com/platform/system/media/+/master/camera/docs/docs.html
     */
    uint32_t                                    batchSize = 0;

    /**
     * Old pipeline context.
     *
     * Passing 'nullptr' indicates no old pipeline context.
     * Callers must make sure "pOldPipelineContext" and "out" are different instances.
     */
    android::sp<NSPipelineContext::IPipelineContext> const
                                                pOldPipelineContext;

    android::sp<NSPipelineContext::IDataCallback> const
                                                pDataCallback = nullptr;

    /**
     * using multi-thread to init/config each node.
     */
    bool                                        bUsingParallelNodeToBuildPipelineContext = true;
    /**
     * using multi-thread to background config context.
     */
    bool                                        bUsingMultiThreadToBuildPipelineContext = false;

    /**
     * true indicates it's a reconfiguration flow.
     */
    bool                                        bIsReconfigure = false;
    /**
     * ResourceConcurrency for p1node synchronization.
     * Callers should check whether it is nullptr or not.
     */
    android::sp<IResourceConcurrency>           pP1NodeResourceConcurrency = nullptr;

    /**
     * isSwitchSensor is used to indicates current config is sensor switch flow or not.
     */
    bool                                        bIsSwitchSensor = false;
};

/**
 * Unsed on the call buildPipelineContext().
 */
struct BuildPipelineContextOutputParams
{
    /**
     * Store root nodeid and opendid.
     */
    std::unordered_map<NodeId_T, MINT32> *mvRootNodeOpenIdList = nullptr;
};


/**
 * Generate a new pipeline context.
 *
 * @param[out] out: a new-created pipeline context.
 * @param[out] outParam: a output parameters.
 *
 * @param[in] in: input parameters.
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
auto buildPipelineContext(
    android::sp<NSPipelineContext::IPipelineContext>& out,
    BuildPipelineContextInputParams const& in,
    BuildPipelineContextOutputParams *outParam = nullptr
) -> int;

/**
 * Reconfig P1 by original pipeline context.
 *
 * @param[out] pContext: a original pipeline context and conifg P1 for new setting.
 *
 * @param[in] in: input parameters.
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
auto reconfigureP1ForPipelineContext(
    android::sp<NSPipelineContext::IPipelineContext> pContext,
    BuildPipelineContextInputParams const& in,
    BuildPipelineContextOutputParams *outParam = nullptr
) -> int;


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_PIPELINE_MODEL_UTILS_INCLUDE_IMPL_PIPELINECONTEXTBUILDER_H_

