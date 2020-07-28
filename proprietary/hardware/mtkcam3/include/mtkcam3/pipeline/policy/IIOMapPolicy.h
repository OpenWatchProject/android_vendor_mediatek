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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IIOMAPPOLICY_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IIOMAPPOLICY_H_
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
namespace iomap {


/**
 * A structure definition for output parameters
 */
struct RequestOutputParams
{
    using IOMapSet = NSCam::v3::pipeline::NSPipelineContext::IOMapSet;

    /**
     * IOMapSet for all pipeline nodes.
     */
    std::unordered_map<NodeId_T, IOMapSet>*     pNodeIOMapImage = nullptr;
    std::unordered_map<NodeId_T, IOMapSet>*     pNodeIOMapMeta = nullptr;
    bool                                        bP2UseImgo = false;

    std::unordered_map<uint32_t, std::vector<StreamId_T>>*
                                                pvMetaStreamId_All_Physical = nullptr;
    MultiCamReqOutputParams*                    pMultiCamReqOutputParams = nullptr;
};


/**
 * A structure definition for input parameters
 */
struct RequestInputParams
{
    int                                         isMainFrame = true;
    int                                         isDummyFrame = false;
    int                                         useP1FDYUV = false;
    int                                         masterIndex = 0;
    bool                                        isAppRawOutFromP2C = false;
    /**************************************************************************
     * Request parameters
     *
     * The parameters related to this capture request is shown as below.
     *
     **************************************************************************/

    /**
     * Pipeline nodes need, sent at the request stage.
     */
    PipelineNodesNeed const*                    pRequest_PipelineNodesNeed = nullptr;

    /**
     * Request App image stream info, sent at the request stage.
     *
     */
    ParsedAppImageStreamInfo const*             pRequest_AppImageStreamInfo = nullptr;

    /**
     * pRequest_ParsedAppMetaControl is a partial parsed result from pRequest_AppControl,
     * just for the purpose of a quick reference.
     */
    ParsedMetaControl const*                    pRequest_ParsedAppMetaControl = nullptr;

    /**
     * The Jpeg orientation is passed to HAL at the request stage.
     * need this???, this info is already in pConfiguration_StreamInfo_NonP1
     */
    //IImageStreamInfo const*                     pRequest_HalImage_Jpeg_YUV = nullptr;

    /**
     * The thumbnail size is passed to HAL at the request stage.
     */
    IImageStreamInfo const*                     pRequest_HalImage_Thumbnail_YUV = nullptr;

    /**
     * P1 DMA need, sent at the request stage.
     *
     * The value shows which dma are needed.
     * For example,
     *      (needP1Dma[0] & P1_IMGO) != 0 indicates that IMGO is needed for sensorId[0].
     *      (needP1Dma[1] & P1_RRZO) != 0 indicates that RRZO is needed for sensorId[1].
     */
    std::vector<uint32_t> const*                pRequest_NeedP1Dma = nullptr;

    /**
     * Using to decide current camera id need P2 or not.
     *
     * The value shows which dma are needed.
     * For example,
     *      (needP1Dma[0] & P2_STREAMING) != 0 indicates that P2Streaming is needed for sensorId[0].
     *      (needP1Dma[1] & P2_STREAMING) != 0 indicates that P2Streaming is needed for sensorId[1].
     */
    std::vector<uint32_t> const*                pRequest_NeedP2Process = nullptr;

    /**
     * Output image/meta stream IDs which P2 streaming node is in charge of outputing.
     */
    std::vector<StreamId_T> const*              pvImageStreamId_from_StreamNode = nullptr;
    std::unordered_map<uint32_t, std::vector<StreamId_T>> const*
                                                pvImageStreamId_from_StreamNode_Physical = nullptr;
    std::vector<StreamId_T> const*              pvMetaStreamId_from_StreamNode = nullptr;
    std::unordered_map<uint32_t, std::vector<StreamId_T>> const*
                                                pvMetaStreamId_from_StreamNode_Physical = nullptr;

    /**
     * Output image/meta stream IDs which P2 capture node is in charge of outputing.
     */
    std::vector<StreamId_T> const*              pvImageStreamId_from_CaptureNode = nullptr;
    std::unordered_map<uint32_t, std::vector<StreamId_T>> const*
                                                pvImageStreamId_from_CaptureNode_Physical = nullptr;
    std::vector<StreamId_T> const*              pvMetaStreamId_from_CaptureNode = nullptr;
    std::unordered_map<uint32_t, std::vector<StreamId_T>> const*
                                                pvMetaStreamId_from_CaptureNode_Physical = nullptr;


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
     * Configuration stream info, built up at the configuration stage.
     *
     * Parsed P1 stream info.
     */
    std::vector<ParsedStreamInfo_P1> const*     pConfiguration_StreamInfo_P1 = nullptr;

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


};  //namespace iomap


////////////////////////////////////////////////////////////////////////////////


/**
 * The function type definition.
 * It is used to decide the I/O Map of P2Nodes, including P2StreamNode and P2CaptureNode.
 *
 * @param[out] out: input parameters
 *
 * @param[in] in: input parameters
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
using FunctionType_IOMapPolicy_P2Node
    = std::function<int(
        iomap::RequestOutputParams& /*out*/,
        iomap::RequestInputParams const& /*in*/
    )>;


/**
 * The function type definition.
 * It is used to decide the I/O Map of Non-P2Node.
 *
 * @param[out] out: input parameters
 *
 * @param[in] in: input parameters
 *
 * @return
 *      0 indicates success; otherwise failure.
 */
using FunctionType_IOMapPolicy_NonP2Node
    = std::function<int(
        iomap::RequestOutputParams const& /*out*/,
        iomap::RequestInputParams const& /*in*/
    )>;


//==============================================================================


/**
 * Policy instance makers
 *
 */

// default version
FunctionType_IOMapPolicy_P2Node makePolicy_IOMap_P2Node_Default();

// multi-cam version
FunctionType_IOMapPolicy_P2Node makePolicy_IOMap_P2Node_multicam();

////////////////////////////////////////////////////////////////////////////////

// default version
FunctionType_IOMapPolicy_NonP2Node makePolicy_IOMap_NonP2Node_Default();

// Hal Jpeg version -decorator design pattern
FunctionType_IOMapPolicy_NonP2Node makePolicy_IOMap_NonP2Node_HalJpeg(
        FunctionType_IOMapPolicy_NonP2Node f);

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_POLICY_IIOMAPPOLICY_H_

