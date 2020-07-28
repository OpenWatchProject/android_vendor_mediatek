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

// Standard C header file
#include <time.h>
#include <iostream>
#include <chrono>
#include <thread>
// Android system/core header file
#include <gtest/gtest.h>
#include <vector>
// mtkcam custom header file

// mtkcam global header file
// Module header file
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/StereoCamEnum.h>
// Local header file
#include "../common/CallbackUTNode.h"
#include "../common/TestDepthMap_Common.h"
#include "../../DepthMapEffectRequest.h"
#include "../../bufferPoolMgr/NodeBufferPoolMgr_VSDOF.h"
#include "../../nodes/N3DNode.h"
#include "../../nodes/P2ANode.h"
#include "../../nodes/P2ABayerNode.h"
#include "../../DataStorage.h"

#define LOG_TAG "MtkCam/DepthPipeUT"
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;
using namespace NSCam::v1::Stereo;

#define TEST_REQ_TYPE eSTATE_NORMAL
#define EIS_ON 0
#define TEST_FLOW_TYPE eDEPTH_FLOW_TYPE_QUEUED_DEPTH //eDEPTH_FLOW_TYPE_STANDARD
#define REQUEST_NEED_FEFM 0
#define TEST_REQ_SIZE 5

DepthMapRequestPtr
prepareEnqueRequest(
    int request_id,
    DepthMapPipeOpState eState,
    sp<BaseBufferPoolMgr> pBuffPoolMgr,
    sp<DepthMapFlowOption> pFlowOption,
    MBOOL isQueueFlow)
{
    MY_LOGD("+: reqID = %d, request state=%d", request_id, eState);
    // new request
    sp<IDepthMapEffectRequest> pDpeRequest =  IDepthMapEffectRequest::createInstance(request_id, WaitingListener::CB);

    DepthMapRequestPtr pRequest = static_cast<DepthMapEffectRequest*>(pDpeRequest.get());
    // load raws
    prepareReqInputBuffer(pRequest);
    // prepare output
    prepareReqOutputBuffer(eState, pRequest);
    if(eState == eSTATE_CAPTURE)
        // prepare metadata
        setupReqMetadata(MFALSE, pRequest, isQueueFlow);
    else
        setupReqMetadata(EIS_ON, pRequest, isQueueFlow);

    // setup request init
    sp<BaseBufferHandler> pBufferPoolHandler = pBuffPoolMgr->createBufferPoolHandler();
    EffectRequestAttrs reqAttrs;
    if(!pFlowOption->queryReqAttrs(pRequest, reqAttrs))
    {
        MY_LOGE("Failed to query request attributes!");
        return MFALSE;
    }
    reqAttrs.needFEFM = REQUEST_NEED_FEFM;
    pRequest->init(pBufferPoolHandler, reqAttrs);

    MY_LOGD("-");
    return pRequest;
}

TEST(DepthMapPipe_P2AToN3D_BB, Standard)
//void test()
{
    MY_LOGD("DepthMapPipe_P2AToN3D_BB");
    UTEnvironmenSetup::Profile profile = {eRatio_4_3, STEREO_SENSOR_PROFILE_REAR_REAR,
                                            E_STEREO_FEATURE_VSDOF, BAYER_AND_BAYER};
    UTEnvironmenSetup setup("P2AToN3D", profile);

    if(!setup.isReadyToUT())
    {
        EXPECT_TRUE(0);
        return;
    }

    // prepare DPE->WMF bitset
    PipeNodeBitSet nodeBitSet;
    nodeBitSet.reset();
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2A, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_N3D, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_DPE, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_WMF, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_OCC, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_HWOCC, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_FD, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_GF, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2ABAYER, 1);

    sp<DepthMapPipeSetting> pSetting;
    sp<DepthMapPipeOption> pOption;
    sp<DepthMapFlowOption> pFlowOption;

    if(!setupUTDepthMapPipe(MFALSE, pSetting, pOption, pFlowOption, TEST_FLOW_TYPE))
    {
        MY_LOGE("Failed to setup UT depthmap pipe!");
        EXPECT_TRUE(0);
        return;
    }
    MY_LOGD("Create Nodes...");
    sp<BaseBufferPoolMgr> pBuffPoolMgr = new NodeBufferPoolMgr_VSDOF(nodeBitSet, pSetting, pOption);
    PipeNodeConfigs config(pFlowOption, pSetting, pOption);
    // create pipe nodes
    P2ANode *pP2ANode = new P2ANode("P2ANode", eDPETHMAP_PIPE_NODEID_P2A, config);
    P2ABayerNode *pP2ABayerNode = new P2ABayerNode("P2ABayerNode", eDPETHMAP_PIPE_NODEID_P2ABAYER, config);
    N3DNode *pN3DNode = new N3DNode("N3DNode", eDPETHMAP_PIPE_NODEID_N3D, config);
    // use the last node id: eDPETHMAP_PIPE_NODE_SIZE
    CallbackUTNode *pCallbackUTNode = new CallbackUTNode("CBUTNode", eDPETHMAP_PIPE_NODE_SIZE, config, finishCB_forListener);
    //
    std::vector<DepthMapPipeNode*> mvNodes;
    mvNodes.push_back(pP2ANode);
    mvNodes.push_back(pP2ABayerNode);
    mvNodes.push_back(pN3DNode);
    mvNodes.push_back(pCallbackUTNode);
    // init the basic usage
    DepthInfoStorage* pStorage = new DepthInfoStorage();
    NodeSignal* pNodeSignal = new NodeSignal();
    for(auto pNode : mvNodes)
    {
        pNode->setDepthStorage(pStorage);
        pNode->setNodeSignal(pNodeSignal);
        pNode->setBufferPoolMgr(pBuffPoolMgr);
    }
    // fake one depth info into storage
    DepthBufferInfo depthInfo;
    depthInfo.mpDMBGBuffer = pBuffPoolMgr->request(BID_GF_INTERNAL_DMBG, eBUFFER_POOL_SCENARIO_RECORD);
    pStorage->setStoredData(depthInfo);

    MY_LOGD("Connect Nodes...");
    // connect data
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pN3DNode, P2A_TO_N3D_FEFM_CCin);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pN3DNode, P2A_TO_N3D_NOFEFM_RECT1);
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pN3DNode, P2A_TO_N3D_NOFEFM_RECT2);
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pP2ABayerNode, BAYER_ENQUE);
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pCallbackUTNode, P2A_OUT_MV_F);
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pCallbackUTNode, P2A_OUT_FD);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_MV_F);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_FD);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_MV_F_CAP);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_YUV_DONE);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_TO_FD_IMG);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_TO_OCC_MY_S);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_TO_HWOCC_MY_S);
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pCallbackUTNode, TO_DUMP_RAWS);
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pCallbackUTNode, TO_DUMP_BUFFERS);
    // n3D output
    CONNECT_DATA_AND_SETUPDUMP(pN3DNode, pCallbackUTNode, DEPTHMAP_META_OUT);
    CONNECT_DATA_AND_SETUPDUMP(pN3DNode, pCallbackUTNode, N3D_TO_DPE_MVSV_MASK);
    CONNECT_DATA_AND_SETUPDUMP(pN3DNode, pCallbackUTNode, N3D_OUT_JPS_WARPMTX);
    CONNECT_DATA_AND_SETUPDUMP(pN3DNode, pCallbackUTNode, N3D_TO_FD_EXTDATA_MASK);
    SETUP_DUMP(pP2ANode, TO_DUMP_RAWS);
    SETUP_DUMP(pP2ANode, TO_DUMP_BUFFERS);
    SETUP_DUMP_REQ_START("0","1");

    MY_LOGD("Create requests...");
    // build requet
    vector<MUINT32> vReqIDVec;
    vector<DepthMapRequestPtr> vRequestVec;
    for(int i=0;i<TEST_REQ_SIZE;i++)
    {

        DepthMapRequestPtr pRequest = prepareEnqueRequest(i, TEST_REQ_TYPE, pBuffPoolMgr, pFlowOption, pOption->mFlowType ==  eDEPTH_FLOW_TYPE_QUEUED_DEPTH);
        vRequestVec.push_back(pRequest);
        vReqIDVec.push_back(i);
    }

    // setup waiting ID list
    vector<DepthMapDataID> vWaitDataID;
    if(TEST_REQ_TYPE == eSTATE_NORMAL)
    {
        vWaitDataID.push_back(P2A_OUT_MV_F);
        vWaitDataID.push_back(P2A_OUT_FD);
        vWaitDataID.push_back(P2A_TO_HWOCC_MY_S);
        vWaitDataID.push_back(N3D_TO_DPE_MVSV_MASK);

        if(pOption->mFlowType != eDEPTH_FLOW_TYPE_QUEUED_DEPTH)
        {
            vWaitDataID.push_back(DEPTHMAP_META_OUT);
        }
    }
    else
    {
        vWaitDataID.push_back(P2A_OUT_MV_F_CAP);
        vWaitDataID.push_back(P2A_OUT_YUV_DONE);
        vWaitDataID.push_back(P2A_TO_OCC_MY_S);
        vWaitDataID.push_back(DEPTHMAP_META_OUT);
        vWaitDataID.push_back(P2A_TO_FD_IMG);
        vWaitDataID.push_back(N3D_TO_DPE_MVSV_MASK);
        vWaitDataID.push_back(N3D_OUT_JPS_WARPMTX);
        vWaitDataID.push_back(N3D_TO_FD_EXTDATA_MASK);
    }

    pCallbackUTNode->setWaitingDataIDAndReqID(vWaitDataID, vReqIDVec);
    // init & start
    for(auto pNode : mvNodes)
    {
        pNode->init();
        pNode->start();
    }
    // insert into P2A node
    while(vRequestVec.size() > 0)
    {
        auto pRequest = vRequestVec.back();
        vRequestVec.pop_back();
        MY_LOGD("Insert request!..%d", pRequest->getRequestNo());
        pP2ANode->onData(ROOT_ENQUE, pRequest);

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    MY_LOGD("Start to wait!!....");
    bool bRet = WaitingListener::waitRequest(TEST_REQ_SIZE, 5);
    MY_LOGD("Finish the waiting!!....%d", bRet);
    EXPECT_TRUE(bRet);

    // finalize the nodes
    finalizeTheNodes(mvNodes);
    // release meta
    releaseMetadata(vRequestVec);
    //
    WaitingListener::resetCounter();
    MY_LOGD("delete the nodes!!....");
    // delete
    for(auto pNode : mvNodes)
    {
        delete pNode;
    }
    vRequestVec.clear();
    mvNodes.clear();
    MY_LOGD("Finished!!....");

    return;
}


//TEST(DepthMapPipe_P2AToN3D_BM, Standard)
void test()
{
    MY_LOGD("DepthMapPipe_P2AToN3D_BM");
    UTEnvironmenSetup::Profile profile = {eRatio_16_9, STEREO_SENSOR_PROFILE_REAR_REAR,
                                            E_STEREO_FEATURE_VSDOF, BAYER_AND_MONO};
    UTEnvironmenSetup setup("P2AToN3D", profile);

    if(!setup.isReadyToUT())
    {
        EXPECT_TRUE(0);
        return;
    }

    // prepare DPE->WMF bitset
    PipeNodeBitSet nodeBitSet;
    nodeBitSet.reset();
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2A, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2ABAYER, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_N3D, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_WMF, 1);

    sp<DepthMapPipeSetting> pSetting;
    sp<DepthMapPipeOption> pOption;
    sp<DepthMapFlowOption> pFlowOption;

    if(!setupUTDepthMapPipe(MTRUE, pSetting, pOption, pFlowOption, TEST_FLOW_TYPE))
    {
        MY_LOGE("Failed to setup UT depthmap pipe!");
        EXPECT_TRUE(0);
        return;
    }
    sp<BaseBufferPoolMgr> pBuffPoolMgr = new NodeBufferPoolMgr_VSDOF(nodeBitSet, pSetting, pOption);
    PipeNodeConfigs config(pFlowOption, pSetting, pOption);
    // create pipe nodes
    P2ANode *pP2ANode = new P2ANode("P2ANode", eDPETHMAP_PIPE_NODEID_P2A, config);
    N3DNode *pN3DNode = new N3DNode("N3DNode", eDPETHMAP_PIPE_NODEID_N3D, config);
    P2ABayerNode *pP2ABayerNode = new P2ABayerNode("pP2ABayerNode", eDPETHMAP_PIPE_NODEID_P2ABAYER, config);
    // use the last node id: eDPETHMAP_PIPE_NODE_SIZE
    CallbackUTNode *pCallbackUTNode = new CallbackUTNode("CBUTNode", eDPETHMAP_PIPE_NODE_SIZE, config, finishCB_forListener);
    //
    std::vector<DepthMapPipeNode*> mvNodes;
    mvNodes.push_back(pP2ANode);
    mvNodes.push_back(pN3DNode);
    mvNodes.push_back(pP2ABayerNode);
    mvNodes.push_back(pCallbackUTNode);
    // init the basic usage
    DepthInfoStorage* pStorage = new DepthInfoStorage();
    NodeSignal* pNodeSignal = new NodeSignal();
    for(auto pNode : mvNodes)
    {
        pNode->setDepthStorage(pStorage);
        pNode->setNodeSignal(pNodeSignal);
    }
    // fake one depth info into storage
    DepthBufferInfo depthInfo;
    depthInfo.mpDMBGBuffer = pBuffPoolMgr->request(BID_WMF_OUT_DMW, eBUFFER_POOL_SCENARIO_RECORD);
    pStorage->setStoredData(depthInfo);

    MY_LOGD("Connect Nodes...");
    // connect data
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pN3DNode, P2A_TO_N3D_FEFM_CCin);
    CONNECT_DATA_AND_SETUPDUMP(pP2ANode, pP2ABayerNode, BAYER_ENQUE);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_MV_F);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_FD);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_MV_F_CAP);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_TO_OCC_MY_S);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_TO_FD_IMG);
    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_YUV_DONE);    CONNECT_DATA_AND_SETUPDUMP(pP2ABayerNode, pCallbackUTNode, P2A_OUT_DMBG);

    CONNECT_DATA_AND_SETUPDUMP(pN3DNode, pCallbackUTNode, N3D_TO_DPE_MVSV_MASK);
    CONNECT_DATA_AND_SETUPDUMP(pN3DNode, pCallbackUTNode, N3D_TO_FD_EXTDATA_MASK);
    CONNECT_DATA_AND_SETUPDUMP(pN3DNode, pCallbackUTNode, DEPTHMAP_META_OUT);

    SETUP_DUMP(pP2ANode, TO_DUMP_RAWS);
    SETUP_DUMP(pP2ANode, TO_DUMP_BUFFERS);

    MY_LOGD("Create requests...");
    // build requet
    vector<MUINT32> vReqIDVec;
    vector<DepthMapRequestPtr> vRequestVec;
    for(int i=0;i<1;i++)
    {
        DepthMapRequestPtr pRequest = prepareEnqueRequest(i, TEST_REQ_TYPE, pBuffPoolMgr, pFlowOption, pOption->mFlowType ==  eDEPTH_FLOW_TYPE_QUEUED_DEPTH);
        vRequestVec.push_back(pRequest);
        vReqIDVec.push_back(i);
    }

    // setup waiting ID list
    vector<DepthMapDataID> vWaitDataID;
    if(TEST_REQ_TYPE == eSTATE_NORMAL)
    {
        vWaitDataID.push_back(P2A_OUT_MV_F);
        vWaitDataID.push_back(P2A_OUT_FD);
        vWaitDataID.push_back(P2A_TO_HWOCC_MY_S);
        vWaitDataID.push_back(DEPTHMAP_META_OUT);
        vWaitDataID.push_back(P2A_OUT_DMBG);
        vWaitDataID.push_back(N3D_TO_DPE_MVSV_MASK);
        if(!REQUEST_NEED_FEFM)
        {
            vWaitDataID.push_back(P2A_TO_N3D_NOFEFM_RECT1);
        }
    }
    else
    {
        vWaitDataID.push_back(P2A_OUT_MV_F_CAP);
        vWaitDataID.push_back(P2A_TO_OCC_MY_S);
        vWaitDataID.push_back(P2A_OUT_YUV_DONE);
        vWaitDataID.push_back(DEPTHMAP_META_OUT);
        vWaitDataID.push_back(P2A_TO_FD_IMG);
        vWaitDataID.push_back(N3D_TO_DPE_MVSV_MASK);
        vWaitDataID.push_back(N3D_OUT_JPS_WARPMTX);
        vWaitDataID.push_back(N3D_TO_FD_EXTDATA_MASK);
    }

    pCallbackUTNode->setWaitingDataIDAndReqID(vWaitDataID, vReqIDVec);
    // init & start
    for(auto pNode : mvNodes)
    {
        pNode->init();
        pNode->start();
    }
    // insert into P2A node
    for(auto i=0; i<vRequestVec.size();i++)
    {
        auto pRequest = vRequestVec[i];
        MY_LOGD("Insert requests...No: %d", i);
        // notify DPE to startup
        pP2ANode->onData(ROOT_ENQUE, pRequest);
    }

    MY_LOGD("Start to wait!!....");
    bool bRet = WaitingListener::waitRequest(vRequestVec.size(), 5);
    MY_LOGD("Finish the waiting!!....%d", bRet);
    EXPECT_TRUE(bRet);

    // finalize the nodes
    finalizeTheNodes(mvNodes);
    // release meta
    releaseMetadata(vRequestVec);
    //
    WaitingListener::resetCounter();
    MY_LOGD("delete the nodes!!....");
    // delete
    for(auto pNode : mvNodes)
    {
        delete pNode;
    }
    vRequestVec.clear();
    mvNodes.clear();
    MY_LOGD("Finished!!....");

    return;
}

