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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IP2NODEDECISIONPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IP2NODEDECISIONPOLICY_H_
//
#include "types.h"
//
#include <functional>
#include <memory>
#include <vector>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


namespace p2nodedecision {


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

    /**
     * Request App face detection intent, resulted at the request stage.
     *
     * true indicates it intents to enable the face detection.
     */
    bool                                        isFdEnabled = false;
    bool                                        needThumbnail = false;


    /*************************************************************************
     * Configuration info.
     *
     * The final configuration information of the pipeline decided at the
     * configuration stage are as below.
     *
     **************************************************************************/

    /**
     * Configuration stream info, built up at the configuration stage.
     *
     * Parsed Non-P1 stream info.
     */
    ParsedStreamInfo_NonP1 const*               pConfiguration_StreamInfo_NonP1 = nullptr;
    /**
     * Parsed main1 stream info.
     */
    ParsedStreamInfo_P1 const*                  pConfiguration_StreamInfo_P1 = nullptr;

    /**
     * Configuration pipeline nodes, built up at the configuration stage.
     *
     * true indicates its corresponding pipeline node is built-up and enabled
     * at the configuration stage.
     */
    bool                                        hasP2CaptureNode = false;
    bool                                        hasP2StreamNode = false;
};


/**
 * Used on the output of evaluateRequest().
 */
struct RequestOutputParams
{
    bool                                        needP2CaptureNode = false;
    MSize                                       maxP2CaptureSize;
    bool                                        needP2StreamNode = false;
    MSize                                       maxP2StreamSize;
    bool                                        needOutputRawFromCaptureNode = false;

    std::vector<StreamId_T>                     vImageStreamId_from_CaptureNode;
    std::vector<StreamId_T>                     vImageStreamId_from_StreamNode;

    std::unordered_map<uint32_t, std::vector<StreamId_T>>
                                                vImageStreamId_from_CaptureNode_Physical;
    std::unordered_map<uint32_t, std::vector<StreamId_T>>
                                                vImageStreamId_from_StreamNode_Physical;

    std::vector<StreamId_T>                     vMetaStreamId_from_CaptureNode;
    std::vector<StreamId_T>                     vMetaStreamId_from_StreamNode;

    std::unordered_map<uint32_t, std::vector<StreamId_T>>
                                                vMetaStreamId_from_CaptureNode_Physical;
    std::unordered_map<uint32_t, std::vector<StreamId_T>>
                                                vMetaStreamId_from_StreamNode_Physical;
    bool                                        needYuvOutputFromP2C = false;
};


};  //namespace p2nodedecision


////////////////////////////////////////////////////////////////////////////////


/**
 * The following nodes belong to P2 node group:
 *      P2CaptureNode
 *      P2StreamingNode
 *
 * The policy is in charge of reporting its requirement at the request stage.
 * Given a request, this policy module is in charge of deciding which P2 nodes are responsible for
 * generating which output image & meta streams.
 *
 *
 * @param[out] out: input parameters
 *
 * @param[in] in: input parameters
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
using FunctionType_P2NodeDecisionPolicy
    = std::function<int(
        p2nodedecision::RequestOutputParams& /*out*/,
        p2nodedecision::RequestInputParams const& /*in*/
    )>;


//==============================================================================


/**
 * Policy instance makers
 *
 */

// default version
FunctionType_P2NodeDecisionPolicy makePolicy_P2NodeDecision_Default();
// App Raw16 reprocessing version
FunctionType_P2NodeDecisionPolicy makePolicy_P2NodeDecision_AppRaw16Reprocess(FunctionType_P2NodeDecisionPolicy f);


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IP2NODEDECISIONPOLICY_H_

