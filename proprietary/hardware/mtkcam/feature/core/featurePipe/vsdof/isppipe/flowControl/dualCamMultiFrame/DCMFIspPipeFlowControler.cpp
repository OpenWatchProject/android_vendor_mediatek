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
#include <tuple>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "./nodes/RootNode.h"
#include "./nodes/MFNRNode.h"
#include "./nodes/HDRNode.h"
#include "./nodes/DepthNode.h"
#include "./nodes/BokehNode.h"
#include "DCMFIspPipeFlowControler.h"
#include "DCMFIspPipeFlow_Common.h"

// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "DCMFIspPipeFlowControler"
#include <featurePipe/core/include/PipeLog.h>


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {


namespace { // begin anonymous namespace
    using ThreadSched = std::tuple<int, int>;
    using NodeThreadSchedTable = std::map<eDCMFIspPipeNodeID, ThreadSched>;
    const NodeThreadSchedTable gNodeThreadSchedTable = { {eISP_PIPE_DCMF_NODEID_ROOT, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
                                                         {eISP_PIPE_DCMF_NODEID_MFNR, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}},
                                                         {eISP_PIPE_DCMF_NODEID_HDR, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY-4}},
                                                         {eISP_PIPE_DCMF_NODEID_DEPTH, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY-4}},
                                                         {eISP_PIPE_DCMF_NODEID_BOKEH, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY-4}},
                                                         {eISP_PIPE_DCMF_NODEID_FD, {SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY}}};
} // end anonymous namespace


const
std::string&
DCMFIspPipeFlowControler::IDToBIDMapProvider::
getName() const
{
    static const std::string name = "DCMF";
    return name;
}

const
DataIDToBIDMap&
DCMFIspPipeFlowControler::IDToBIDMapProvider::
getMap() const
{
    static const DataIDToBIDMap map = getDataIDToBIDMap();
    return map;
}

const
std::string&
DCMFIspPipeFlowControler::IspPipeConverter::
getName() const
{
    static const std::string name = "DCMF";
    return name;
}

const
std::string&
DCMFIspPipeFlowControler::IspPipeConverter::
bufferIdtoString(IspPipeBufferID id) const
{
    IspPipeBufferID BID = id;
#define MAKE_NAME_CASE(name) \
    case name: return #name;
    switch(BID)
    {
        MAKE_NAME_CASE(BID_IN_FSYUV_MAIN1);
        MAKE_NAME_CASE(BID_IN_SMYUV_MAIN1);
        MAKE_NAME_CASE(BID_IN_RSYUV_MAIN1);
        MAKE_NAME_CASE(BID_IN_RSYUV_MAIN2);
        MAKE_NAME_CASE(BID_OUT_CLEAN_FSYUV);
        MAKE_NAME_CASE(BID_OUT_BOKEH_FSYUV);
        MAKE_NAME_CASE(BID_OUT_DEPTHMAP);
        MAKE_NAME_CASE(BID_FD_INERNAL_YUV);
        default:
            MY_LOGW("unknown BID:%d", BID);
            return IIspPipeConverter::unknown;
    }

#undef MAKE_NAME_CASE
}

sp<IDataIDToBIDMapProvider>
DCMFIspPipeFlowControler::
onGetDataIDToBIDMapProvider() const
{
    return new IDToBIDMapProvider();
}

sp<IIspPipeConverter>
DCMFIspPipeFlowControler::
onGetIspPipeConverter() const
{
    return new IspPipeConverter();
}

MVOID
DCMFIspPipeFlowControler::
onBuildPipeBufferPool(PipeBufferPoolMgr& mgr) const
{
    // TODO: doesn't allocate the max buffer size
    static const MSize depthMapBufferSize = {1600, 1200};
    static const MSize cleanBufferSize = {6000, 4000}; // for 24M sensor
    static const MSize postviewBufferSize = {1600, 1200};

    const IspPipeSetting& setting = getSetting();

    sp<PipeImgBufferPool> pFDBufferPool = nullptr;
    sp<PipeImgBufferPool> pDepthMapBufferPool = nullptr;
    sp<ImageBufferPool>   pCleanBufferPool = nullptr;
    sp<PipeImgBufferPool> pPostviewBufferPool = nullptr;

    // create/add image buffer
    CREATE_IMGBUF_POOL(
                pFDBufferPool,
                "pFDBufferPool",
                setting.mszFDImg,
                eImgFmt_YUY2,
                PipeImgBufferPool::USAGE_HW
                );

    CREATE_IMGBUF_POOL(
                pDepthMapBufferPool,
                "pDepthMapBufferPool",
                depthMapBufferSize,
                eImgFmt_Y8,
                PipeImgBufferPool::USAGE_HW
                );

    CREATE_INDIVI_IMGBUF_POOL(
                pCleanBufferPool,
                "pCleanBufferPool",
                cleanBufferSize,
                eImgFmt_YV12,
                PipeImgBufferPool::USAGE_HW
                );

    CREATE_IMGBUF_POOL(
                pPostviewBufferPool,
                "pPostviewBufferPool",
                postviewBufferSize,
                eImgFmt_NV21,
                PipeImgBufferPool::USAGE_HW
                );

    ALLOCATE_BUFFER_POOL(pFDBufferPool, 1);
    ALLOCATE_BUFFER_POOL(pDepthMapBufferPool, 1);
    ALLOCATE_BUFFER_POOL(pCleanBufferPool, 1);
    ALLOCATE_BUFFER_POOL(pPostviewBufferPool, 1);

    mgr.addPool(BID_FD_INERNAL_YUV, pFDBufferPool);
    mgr.addPool(BID_DEPTHMAP_INERNAL_Y8, pDepthMapBufferPool);
    mgr.addPool(BID_CLEAN_INERNAL_YV12, pCleanBufferPool);
    mgr.addPool(BID_POSTVIEW_NV21, pPostviewBufferPool);

    // set node count
    mgr.setNodeCount(eISP_PIPE_NODE_SIZE);
}

MVOID
DCMFIspPipeFlowControler::
onBuildPipeNodes(IspPipe& pipe, KeyedVector<IspPipeNodeID, IspPipeNode*>& nodeMap) const
{

    MY_LOGD("+");

    if(nodeMap.size() > 0)
    {
        MY_LOGE("argument nodeMap is not empty, clear that, size: %d", nodeMap.size());
        nodeMap.clear();
    }

    #define CREATE_PIPE_NODE(nodeIDEnum, nodeClass, nodeName, abbreName)\
    {\
        int policy = 0;\
        int priority = 0;\
        auto it = gNodeThreadSchedTable.find(nodeIDEnum);\
        if(it != gNodeThreadSchedTable.end())\
        {\
            policy = std::get<0>(it->second);\
            priority = std::get<1>(it->second);\
        }\
        else\
        {\
            MY_LOGD("can't find nodeIDEnum:" #nodeIDEnum " from table, set default policy and priority");\
        }\
        int newPolicy = ::property_get_int32("vendor.debug.isppipe." #abbreName ".policy", 0);\
        if(newPolicy != policy) policy = newPolicy;\
        int newPriority = ::property_get_int32("vendor.debug.isppipe." #abbreName ".priority", priority);\
        if(newPriority != priority) priority = newPriority;\
        MY_LOGD("create " #nodeClass " , name: %s, policy: %d, priority: %d", nodeName, policy, priority);\
        PipeNodeConfigs config(new IspPipeSetting(getSetting()), new IspPipeOption(getOption()));\
        pPipeNode = new nodeClass(nodeName, nodeIDEnum, config, policy, priority);\
        pPipeNode->setDataIDToBIDMapProvider(this->getDataIDToBIDMapProvider());\
        pPipeNode->setIspPipeConverter(this->getIspPipeConverter());\
        nodeMap.add(nodeIDEnum, pPipeNode);\
    }


    IspPipeNode* pPipeNode = nullptr;
    // create node
    CREATE_PIPE_NODE(eISP_PIPE_DCMF_NODEID_ROOT, RootNode, "RootNode", root);
    CREATE_PIPE_NODE(eISP_PIPE_DCMF_NODEID_MFNR, MFNRNode, "MFNRNode", mfnr);
    CREATE_PIPE_NODE(eISP_PIPE_DCMF_NODEID_HDR, HDRNode, "HDRNode", hdr);
    CREATE_PIPE_NODE(eISP_PIPE_DCMF_NODEID_DEPTH, DepthNode, "DepthNode", depth);
    CREATE_PIPE_NODE(eISP_PIPE_DCMF_NODEID_BOKEH, BokehNode, "BokehNode", bokeh);
    CREATE_PIPE_NODE(eISP_PIPE_DCMF_NODEID_FD, FDNode, "FDNode", fd);
    #undef CREATE_PIPE_NODE

    IspPipeNode* pRootNode = nodeMap.valueFor(eISP_PIPE_DCMF_NODEID_ROOT);
    IspPipeNode* pMFNRNode = nodeMap.valueFor(eISP_PIPE_DCMF_NODEID_MFNR);
    IspPipeNode* pHDRNode = nodeMap.valueFor(eISP_PIPE_DCMF_NODEID_HDR);
    IspPipeNode* pDepthNode = nodeMap.valueFor(eISP_PIPE_DCMF_NODEID_DEPTH);
    IspPipeNode* pBokehNode = nodeMap.valueFor(eISP_PIPE_DCMF_NODEID_BOKEH);
    IspPipeNode* pFDNode = nodeMap.valueFor(eISP_PIPE_DCMF_NODEID_FD);
    // connect graph
    pipe.connectData(ROOT_TO_MFNR, ROOT_TO_MFNR, *pRootNode, *pMFNRNode);
    pipe.connectData(ROOT_TO_DEPTH, ROOT_TO_DEPTH, *pRootNode, *pDepthNode);
    pipe.connectData(ROOT_TO_FD, ROOT_TO_FD, *pRootNode, *pFDNode);
    pipe.connectData(ROOT_TO_HDR, ROOT_TO_HDR, *pRootNode, *pHDRNode);
    pipe.connectData(MFNR_TO_BOKEH, MFNR_TO_BOKEH, *pMFNRNode, *pBokehNode);
    pipe.connectData(HDR_TO_BOKEH, HDR_TO_BOKEH, *pHDRNode, *pBokehNode);
    pipe.connectData(DEPTH_TO_BOKEH, DEPTH_TO_BOKEH, *pDepthNode, *pBokehNode);
    pipe.connectData(FD_TO_BOKEH, FD_TO_BOKEH, *pFDNode, *pBokehNode);
    pipe.connectData(FD_TO_HDR, FD_TO_HDR, *pFDNode, *pHDRNode);
    pipe.connectData(FD_TO_DEPTH, FD_TO_DEPTH, *pFDNode, *pDepthNode);

    pipe.connectData(BOKEH_OUT, BOKEH_OUT, *pBokehNode, &pipe);
    pipe.setRootNode(pRootNode);

    // set the specific settings
    {
        // MFNR_TO_BOKEH, HDR_TO_BOKEH, DEPTH_TO_BOKEH, so the count is 3
        const MINT32 bokehNodeMaxReceivedRequestCount = 3;
        // BokehNode* temp = dynamic_cast<BokehNode*>(pBokehNode);
        BokehNode* temp = (BokehNode*)(pBokehNode);
        temp->setMaxReceivedRequestCount(bokehNodeMaxReceivedRequestCount);
    }
    MY_LOGD("-");
}

MBOOL
DCMFIspPipeFlowControler::
onHandlePipeOnData(IspPipeDataID id, const IspPipeRequestPtr& pRequest)
{
    MBOOL bRet = MFALSE;

    MY_LOGD("+");
    switch(id)
    {
        case BOKEH_OUT:
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
