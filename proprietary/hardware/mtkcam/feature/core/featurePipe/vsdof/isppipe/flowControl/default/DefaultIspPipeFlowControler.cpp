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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "./nodes/P2ANode.h"
#include "./nodes/ThirdPartyNode.h"
#include "./nodes/MDPNode.h"
#include "DefaultIspPipeFlowControler.h"
#include "DefaultIspPipeFlow_Common.h"

// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "DefaultIspPipeFlowControler"
#include <featurePipe/core/include/PipeLog.h>


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

namespace
{
    constexpr MINT32 ISP_PIPE_FRAMEBUFFER_SIZE = 3;
    constexpr MINT32 MAX_P2_FRAME = 10;
}

const
std::string&
DefaultIspPipeFlowControler::IDToBIDMapProvider::
getName() const
{
    static const std::string name = "Default";
    return name;
}

const
DataIDToBIDMap&
DefaultIspPipeFlowControler::IDToBIDMapProvider::
getMap() const
{
    static const DataIDToBIDMap map = getDataIDToBIDMap();
    return map;
}

const
std::string&
DefaultIspPipeFlowControler::IspPipeConverter::
getName() const
{
    static const std::string name = "Default";
    return name;
}

const
std::string&
DefaultIspPipeFlowControler::IspPipeConverter::
bufferIdtoString(IspPipeBufferID id) const
{
    IspPipeBufferID BID = id;
#define MAKE_NAME_CASE(name)                                \
    case name:                                              \
    {                                                       \
        static const std::string temp{#name};               \
        return temp;                                        \
    }

    switch(BID)
    {
        MAKE_NAME_CASE(BID_IN_FSRAW_MAIN1);
        MAKE_NAME_CASE(BID_IN_FSRAW_MAIN2);
        MAKE_NAME_CASE(BID_IN_RSRAW_MAIN1);
        MAKE_NAME_CASE(BID_IN_RSRAW_MAIN2);
        MAKE_NAME_CASE(BID_IN_RESIZE_YUV_MAIN1);
        MAKE_NAME_CASE(BID_IN_RESIZE_YUV_MAIN2);
        MAKE_NAME_CASE(BID_IN_FULLSIZE_YUV_MAIN1);
        MAKE_NAME_CASE(BID_IN_FULLSIZE_YUV_MAIN2);
        MAKE_NAME_CASE(BID_IN_LCSO_MAIN1);
        MAKE_NAME_CASE(BID_IN_LCSO_MAIN2);
        // internal P2A buffers
        MAKE_NAME_CASE(BID_PIPE_OUT_PV_YUV0);
        MAKE_NAME_CASE(BID_PIPE_OUT_PV_YUV1);
        MAKE_NAME_CASE(BID_PIPE_OUT_PV_FD);
        MAKE_NAME_CASE(BID_PIPE_OUT_BOKEH_YUV);
        MAKE_NAME_CASE(BID_PIPE_OUT_DEPTH);
        MAKE_NAME_CASE(BID_MDP_OUT_THUMBNAIL_YUV);
        MAKE_NAME_CASE(BID_P2A_OUT_YUV_MAIN1);
        MAKE_NAME_CASE(BID_P2A_OUT_YUV_MAIN2);
        default:
            MY_LOGW("unknown BID:%d", BID);
            return IIspPipeConverter::unknown;
    }

#undef MAKE_NAME_CASE
}

sp<IDataIDToBIDMapProvider>
DefaultIspPipeFlowControler::
onGetDataIDToBIDMapProvider() const
{
    return new IDToBIDMapProvider();
}

sp<IIspPipeConverter>
DefaultIspPipeFlowControler::
onGetIspPipeConverter() const
{
    return new IspPipeConverter();
}

MVOID
DefaultIspPipeFlowControler::
onBuildPipeBufferPool(PipeBufferPoolMgr& mgr) const
{
    const IspPipeSetting& setting = getSetting();

    sp<PipeImgBufferPool> pPVYUVBufferPool = nullptr;
    sp<PipeImgBufferPool> pRRZOBufferPool_Main1 = nullptr;
    sp<PipeImgBufferPool> pRRZOBufferPool_Main2 = nullptr;

    // create/add image buffer
    CREATE_IMGBUF_POOL(
                pPVYUVBufferPool,
                "mpPVYUVBufferPool",
                setting.mszPreviewYUV,
                eImgFmt_NV21,
                PipeImgBufferPool::USAGE_HW
                );

    CREATE_IMGBUF_POOL(
                pRRZOBufferPool_Main1,
                "mpRRZOBufferPool_Main1",
                setting.mszRRZO_Main1,
                eImgFmt_NV21,
                PipeImgBufferPool::USAGE_HW
                );

    CREATE_IMGBUF_POOL(
                pRRZOBufferPool_Main2,
                "mpRRZOBufferPool_Main2",
                setting.mszRRZO_Main2,
                eImgFmt_NV21,
                PipeImgBufferPool::USAGE_HW
                );
    ALLOCATE_BUFFER_POOL(pPVYUVBufferPool, ISP_PIPE_FRAMEBUFFER_SIZE);
    ALLOCATE_BUFFER_POOL(pRRZOBufferPool_Main1, ISP_PIPE_FRAMEBUFFER_SIZE);
    ALLOCATE_BUFFER_POOL(pRRZOBufferPool_Main2, ISP_PIPE_FRAMEBUFFER_SIZE);
    mgr.addPool(BID_P2A_OUT_PV_YUV, pPVYUVBufferPool);
    mgr.addPool(BID_P2A_OUT_YUV_MAIN1, pRRZOBufferPool_Main1);
    mgr.addPool(BID_P2A_OUT_YUV_MAIN2, pRRZOBufferPool_Main2);

    // create/add tuning buffer pool
    sp<TuningBufferPool> pP2ATuningBufferPool = TuningBufferPool::create("VSDOF_TUNING_P2A", INormalStream::getRegTableSize());
    ALLOCATE_BUFFER_POOL(pP2ATuningBufferPool, MAX_P2_FRAME);
    mgr.addTBPool(BID_P2A_TUNING, pP2ATuningBufferPool);

    // set node count
    mgr.setNodeCount(eISP_PIPE_NODE_SIZE);
}

MVOID
DefaultIspPipeFlowControler::
onBuildPipeNodes(IspPipe& pipe, KeyedVector<IspPipeNodeID, IspPipeNode*>& nodeMap) const
{
    MY_LOGD("+");

    if(nodeMap.size() > 0)
    {
        MY_LOGE("argument nodeMap is not empty, clear that, size: %d", nodeMap.size());
        nodeMap.clear();
    }

    #define CREATE_PIPE_NODE(nodeIDEnum, nodeClass, nodeName)\
    {\
        PipeNodeConfigs config(new IspPipeSetting(getSetting()), new IspPipeOption(getOption()));\
        pPipeNode = new nodeClass(nodeName, nodeIDEnum, config);\
        pPipeNode->setDataIDToBIDMapProvider(this->getDataIDToBIDMapProvider());\
        pPipeNode->setIspPipeConverter(this->getIspPipeConverter());\
        nodeMap.add(nodeIDEnum, pPipeNode);\
    }

    IspPipeNode* pPipeNode = nullptr;
    // create node
    CREATE_PIPE_NODE(eISP_PIPE_NODEID_P2A, P2ANode, "P2ANode");
    CREATE_PIPE_NODE(eISP_PIPE_NODEID_TP, ThirdPartyNode, "TPNode");
    CREATE_PIPE_NODE(eISP_PIPE_NODEID_MDP, MDPNode, "MDPNode");
    #undef CREATE_PIPE_NODE

    IspPipeNode* pP2ANode = nodeMap.valueFor(eISP_PIPE_NODEID_P2A);
    IspPipeNode* pTPNode = nodeMap.valueFor(eISP_PIPE_NODEID_TP);
    IspPipeNode* pMDPNode = nodeMap.valueFor(eISP_PIPE_NODEID_MDP);
    // connect graph
    pipe.connectData(P2A_TO_TP_YUV_DATA, P2A_TO_TP_YUV_DATA, *pP2ANode, *pTPNode);
    pipe.connectData(TP_TO_MDP_BOKEHYUV, TP_TO_MDP_BOKEHYUV, *pTPNode, *pMDPNode);
    pipe.connectData(TP_TO_MDP_PVYUV, TP_TO_MDP_PVYUV, *pTPNode, *pMDPNode);
    pipe.connectData(TP_OUT_DEPTH_BOKEH, TP_OUT_DEPTH_BOKEH, *pTPNode, &pipe);
    pipe.connectData(MDP_OUT_THUMBNAIL, MDP_OUT_THUMBNAIL, *pMDPNode, &pipe);
    pipe.connectData(MDP_OUT_YUVS, MDP_OUT_YUVS, *pMDPNode, &pipe);
    pipe.connectData(TO_DUMP_RAWS, TO_DUMP_RAWS, *pP2ANode, &pipe);
    pipe.setRootNode(pP2ANode);

    MY_LOGD("-");
}

MBOOL
DefaultIspPipeFlowControler::
onHandlePipeOnData(IspPipeDataID id, const IspPipeRequestPtr& pRequest)
{
    MBOOL bRet = MFALSE;

    MY_LOGD("+");
    switch(id)
    {
        case TP_OUT_DEPTH_BOKEH:
        case MDP_OUT_THUMBNAIL:
        case MDP_OUT_YUVS:
            bRet = MTRUE;
            break;
        default:
            MY_LOGW("onData non-avaiilable id=%d", id);
    }
    MY_LOGD("-");

    return bRet;
}


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
