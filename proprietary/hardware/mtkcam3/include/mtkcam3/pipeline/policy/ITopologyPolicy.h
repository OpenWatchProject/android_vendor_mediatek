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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ITOPOLOGYPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ITOPOLOGYPOLICY_H_
//
#include "types.h"

#include <functional>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace topology {


/**
 * A structure definition for output parameters
 */
struct RequestOutputParams
{
    using NodeSet       = NSCam::v3::pipeline::NSPipelineContext::NodeSet;
    using NodeEdgeSet   = NSCam::v3::pipeline::NSPipelineContext::NodeEdgeSet;

    /**
     *  Pipeline nodes need.
     *  true indicates its corresponding pipeline node is needed.
     */
    PipelineNodesNeed*                          pNodesNeed = nullptr;
    std::vector<NodeId_T>*                      pNodeSet = nullptr;

    /**
     * The root nodes of a pipeline.
     */
    NodeSet*                                    pRootNodes = nullptr;

    /**
     * The edges to connect pipeline nodes.
     */
    NodeEdgeSet*                                pEdges = nullptr;

};


/**
 * A structure definition for input parameters
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
     * true indicates it intents to create "dummy frame", which means this reqeust will enque
     * to pass1 driver without any target result images.
     */
    bool                                        isDummyFrame = false;

    bool                                        isMainFrame = false;

    int                                         useP1FDYUV = false;

    /**
     * The following info usually results from the P2Node decision policy.
     *
     * Output image/meta stream IDs which P2 streaming node is in charge of outputing.
     * Output image/meta stream IDs which P2 capture node is in charge of outputing.
     */
    bool                                        needP2CaptureNode = false;
    bool                                        needP2StreamNode = false;
    std::vector<StreamId_T> const*              pvImageStreamId_from_StreamNode = nullptr;
    std::vector<StreamId_T> const*              pvMetaStreamId_from_StreamNode = nullptr;
    std::vector<StreamId_T> const*              pvImageStreamId_from_CaptureNode = nullptr;
    std::vector<StreamId_T> const*              pvMetaStreamId_from_CaptureNode = nullptr;

    /**
     * true indicates it intents to enable the face detection.
     */
    bool                                        isFdEnabled = false;

    /**
     * true indicates the APP output raw must be from P2CaptureNode.
     */
    bool                                        isAppRawOutFromP2C = false;

    /**
     * Request App image stream info, sent at the request stage.
     *
     */
    ParsedAppImageStreamInfo const*             pRequest_AppImageStreamInfo = nullptr;

    /**
     * Request App control info, sent at the request stage.
     *
     */
    ParsedMetaControl const*                    pRequest_ParsedAppMetaControl = nullptr;

    /**
     * p1 dma info
     */
    std::vector<uint32_t> const*                pvNeedP1Dma = nullptr;

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

    /*************************************************************************
     * Static info.
     *
     **************************************************************************/

    /**
     * Pipeline static information.
     */
    PipelineStaticInfo const*                   pPipelineStaticInfo = nullptr;

    /**
     * multicam request output parameters.
     */
    MultiCamReqOutputParams* pMultiCamReqOutputParams = nullptr;
};


};  //namespace topology


////////////////////////////////////////////////////////////////////////////////


/**
 * The function type definition.
 * It is used to decide the pipeline topology.
 *
 * @param[out] out: input parameters
 *
 * @param[in] in: input parameters
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
using FunctionType_TopologyPolicy
    = std::function<int(
        topology::RequestOutputParams& /*out*/,
        topology::RequestInputParams const& /*in*/
    )>;


//==============================================================================


/**
 * Policy instance makers
 *
 */

// default version
FunctionType_TopologyPolicy makePolicy_Topology_Default();

// multicam version
FunctionType_TopologyPolicy makePolicy_Topology_Multicam();

// HAL Jpeg version - decorator design pattern
FunctionType_TopologyPolicy makePolicy_Request_Topology_HalJpeg(FunctionType_TopologyPolicy f);

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_ITOPOLOGYPOLICY_H_

