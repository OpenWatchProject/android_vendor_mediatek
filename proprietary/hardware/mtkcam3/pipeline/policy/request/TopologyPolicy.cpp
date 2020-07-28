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

#define LOG_TAG "mtkcam-TopologyPolicy"

#include <mtkcam3/pipeline/policy/ITopologyPolicy.h>

#include "MyUtils.h"

#include <mtkcam3/pipeline/hwnode/NodeId.h>
#include <bitset>
#include <mtkcam/utils/std/ULog.h>
#include <bitset>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/******************************************************************************
 *
 ******************************************************************************/
static inline std::string edgeToString(NodeId_T from, NodeId_T to)
{
    std::string os;
    os += toHexString(from);
    os += "->";
    os += toHexString(to);
    return os;
}


/**
 * Make a function target as a policy - default version
 */
FunctionType_TopologyPolicy makePolicy_Topology_Default()
{
    using namespace topology;

    return [](
        RequestOutputParams& out __unused,
        RequestInputParams const& in __unused
    ) -> int
    {
        auto const pCfgNodesNeed    = in.pConfiguration_PipelineNodesNeed;
        auto const pCfgStream_NonP1 = in.pConfiguration_StreamInfo_NonP1;
        if ( CC_UNLIKELY( !pCfgNodesNeed || !pCfgStream_NonP1 ) ) {
            MY_LOGE("null configuration params");
            return -EINVAL;
        }

        std::vector<uint32_t> const*      pNeedP1Dma = in.pvNeedP1Dma;
        PipelineNodesNeed*                pNodesNeed = out.pNodesNeed;
        std::vector<NodeId_T>*            pNodeSet   = out.pNodeSet;
        RequestOutputParams::NodeSet*     pRootNodes = out.pRootNodes;
        RequestOutputParams::NodeEdgeSet* pEdges     = out.pEdges;

        static constexpr size_t MAX_P1_NUM = (eNODEID_P1Node_END - eNODEID_P1Node_BEGIN);
        std::bitset<MAX_P1_NUM> bNeedConnectToStreaming;
        std::bitset<MAX_P1_NUM> bNeedConnectToCapture;
        bNeedConnectToStreaming.reset();
        bNeedConnectToCapture.reset();

        if(pNeedP1Dma)
        {
            for(size_t i=0; i < pNeedP1Dma->size() ; ++i)
            {
                auto p1Dma = (*pNeedP1Dma)[i];
                if(!p1Dma)
                {
                    pNodesNeed->needP1Node.push_back(false);
                    continue;
                }

                pRootNodes->add(NodeIdUtils::getP1NodeId(i));
                pNodesNeed->needP1Node.push_back(true);
                pNodeSet->push_back(NodeIdUtils::getP1NodeId(i));
                bNeedConnectToStreaming.set(i);
                bNeedConnectToCapture.set(i);
            }
        }
        else
        {
            pRootNodes->add(eNODEID_P1Node);
            pNodesNeed->needP1Node.push_back(true);
            pNodeSet->push_back(eNODEID_P1Node);
            if ( in.pConfiguration_PipelineNodesNeed->needP1Node.size()>1 ) {
                pRootNodes->add(eNODEID_P1Node_main2);
            }
        }

        if ( in.isDummyFrame ) {
            return OK;
        }

        auto const pReqImageSet = in.pRequest_AppImageStreamInfo;
        // jpeg
        if ( pReqImageSet!=nullptr && pReqImageSet->pAppImage_Jpeg.get() ) {
            bool found = false;
            const auto& streamId = pCfgStream_NonP1->pHalImage_Jpeg_YUV->getStreamId();
            for ( const auto& s : *(in.pvImageStreamId_from_CaptureNode) ) {
                if ( s == streamId ) {
                    pEdges->addEdge(eNODEID_P2CaptureNode, eNODEID_JpegNode);
                    found = true;
                    break;
                }
            }

            if ( !found ) {
                for ( const auto& s : *(in.pvImageStreamId_from_StreamNode) ) {
                    if ( s == streamId ) {
                        pEdges->addEdge(eNODEID_P2StreamNode, eNODEID_JpegNode);
                        found = true;
                        break;
                    }
                }
            }

            if ( !found ) {
                MY_LOGE("no p2 streaming&capture node w/ jpeg output");
                return -EINVAL;
            }
            pNodesNeed->needJpegNode = true;
            pNodeSet->push_back(eNODEID_JpegNode);
        }

        //P1RawIspTuningDataPackNode
        if (pCfgNodesNeed->needP1RawIspTuningDataPackNode && pReqImageSet!=nullptr &&pReqImageSet->hasRawOut && !in.isAppRawOutFromP2C)
        {
            MY_LOGD("set P1RawIspTuningDataPackNode");
            if ( (pReqImageSet->pAppImage_Output_IspTuningData_Raw.get())
               || ( in.pRequest_ParsedAppMetaControl != nullptr &&
                        (in.pRequest_ParsedAppMetaControl->control_isp_tuning & MTK_CONTROL_CAPTURE_ISP_TUNING_REQ_RAW) ) ) {
                MY_LOGD("add P1RawIspTuningDataPackNode edge");
                pEdges->addEdge(eNODEID_P1Node, eNODEID_P1RawIspTuningDataPackNode);
                pNodesNeed->needP1RawIspTuningDataPackNode = true;
                pNodeSet->push_back(eNODEID_P1RawIspTuningDataPackNode);
            }
        }
        //P2RawIspTuningDataPackNode
        else if (pCfgNodesNeed->needP2RawIspTuningDataPackNode && in.needP2CaptureNode)
        {
            MY_LOGD("set P2RawIspTuningDataPackNode");
            if ( (pReqImageSet!=nullptr && pReqImageSet->pAppImage_Output_IspTuningData_Raw.get())
               || ( in.pRequest_ParsedAppMetaControl != nullptr &&
                        (in.pRequest_ParsedAppMetaControl->control_isp_tuning & MTK_CONTROL_CAPTURE_ISP_TUNING_REQ_RAW) ) ) {
                MY_LOGD("add P2RawIspTuningDataPackNode edge");
                pEdges->addEdge(eNODEID_P2CaptureNode, eNODEID_P2RawIspTuningDataPackNode);
                pNodesNeed->needP2RawIspTuningDataPackNode = true;
                pNodeSet->push_back(eNODEID_P2RawIspTuningDataPackNode);
            }
        }

        //P2YuvIspTuningDataPackNode
        if (pCfgNodesNeed->needP2YuvIspTuningDataPackNode && in.needP2CaptureNode)
        {
            MY_LOGD("set P2YuvIspTuningDataPackNode");
            if ( (pReqImageSet!=nullptr && pReqImageSet->pAppImage_Output_IspTuningData_Yuv.get())
               || ( in.pRequest_ParsedAppMetaControl != nullptr &&
                        (in.pRequest_ParsedAppMetaControl->control_isp_tuning & MTK_CONTROL_CAPTURE_ISP_TUNING_REQ_YUV) ) ) {
                MY_LOGD("add P2YuvIspTuningDataPackNode edge");
                pEdges->addEdge(eNODEID_P2CaptureNode, eNODEID_P2YuvIspTuningDataPackNode);
                pNodesNeed->needP2YuvIspTuningDataPackNode = true;
                pNodeSet->push_back(eNODEID_P2YuvIspTuningDataPackNode);
            }
        }

        // fd
        if ( in.isFdEnabled ) {
            pNodesNeed->needFDNode = true;
            pNodeSet->push_back(eNODEID_FDNode);
            if ( in.useP1FDYUV )
            {
                pEdges->addEdge(eNODEID_P1Node, eNODEID_FDNode);
            }
            else
            {
                if ( in.needP2CaptureNode && !in.needP2StreamNode )
                {
                    pEdges->addEdge(eNODEID_P2CaptureNode, eNODEID_FDNode);
                }
                else if ( in.needP2StreamNode )
                {
                    pEdges->addEdge(eNODEID_P2StreamNode, eNODEID_FDNode);
                }
            }
        }

        // raw16
        if (pCfgNodesNeed->needRaw16Node
            && pReqImageSet!=nullptr && pReqImageSet->vAppImage_Output_RAW16.size() > 0 ) {
            pNodesNeed->needRaw16Node = true;
            MY_LOGD("add raw16 edge");
            for( const auto& raw16 : pReqImageSet->vAppImage_Output_RAW16 )
            {
                auto const index = in.pPipelineStaticInfo->getIndexFromSensorId(raw16.first);
                auto const Raw16Node = NodeIdUtils::getRaw16NodeId(index);
                auto const P1_Node = NodeIdUtils::getP1NodeId(index);
                MY_LOGD("[%s] %s", std::to_string(index).c_str(), edgeToString(P1_Node, Raw16Node).c_str());
                pNodeSet->push_back(Raw16Node);
                pEdges->addEdge(P1_Node, Raw16Node);
            }
        }

        // raw16(physical)
        if (pCfgNodesNeed->needRaw16Node
            && pReqImageSet!=nullptr && pReqImageSet->vAppImage_Output_RAW16_Physical.size() > 0 ) {
            pNodesNeed->needRaw16Node = true;
            MY_LOGD("add raw16 edge(physical)");
            for( const auto& raw16 : pReqImageSet->vAppImage_Output_RAW16_Physical )
            {
                auto const index = in.pPipelineStaticInfo->getIndexFromSensorId(raw16.first);
                auto const Raw16Node = NodeIdUtils::getRaw16NodeId(index);
                auto const P1_Node = NodeIdUtils::getP1NodeId(index);
                MY_LOGD("[%s] %s", std::to_string(index).c_str(), edgeToString(P1_Node, Raw16Node).c_str());
                pNodeSet->push_back(Raw16Node);
                pEdges->addEdge(P1_Node, Raw16Node);
            }
        }

        // p2 streaming
        if ( in.needP2StreamNode ) {
            pNodesNeed->needP2StreamNode = true;
            pNodeSet->push_back(eNODEID_P2StreamNode);
            for(size_t i=0, j=0;j<bNeedConnectToStreaming.count();i++)
            {
                if(bNeedConnectToStreaming.test(i))
                {
                    j++;
                    pEdges->addEdge(NodeIdUtils::getP1NodeId(i), eNODEID_P2StreamNode);
                    MY_LOGD("needP2StreamNode: %s", edgeToString(NodeIdUtils::getP1NodeId(i), eNODEID_P2StreamNode).c_str());
                }
            }
        }

        // p2 capture
        if ( in.needP2CaptureNode ) {
            pNodesNeed->needP2CaptureNode = true;
            pNodeSet->push_back(eNODEID_P2CaptureNode);
            for(size_t i=0, j=0;j<bNeedConnectToCapture.count();i++)
            {
                if(bNeedConnectToCapture.test(i))
                {
                    j++;
                    pEdges->addEdge(NodeIdUtils::getP1NodeId(i), eNODEID_P2CaptureNode);
                    MY_LOGD("needP2CaptureNode: %s", edgeToString(NodeIdUtils::getP1NodeId(i), eNODEID_P2CaptureNode).c_str());
                }
            }
        }

        // if ( in.pPipelineStaticInfo->isType3PDSensorWithoutPDE ) {
        //     pNodesNeed->needPDENode = true;
        //     MY_LOGE("not defined PDE node yet");
        //     return -EINVAL;
        // }

        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

