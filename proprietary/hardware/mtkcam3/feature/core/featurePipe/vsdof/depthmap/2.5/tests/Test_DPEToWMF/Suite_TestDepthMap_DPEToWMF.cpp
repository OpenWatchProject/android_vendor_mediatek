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
#include "../../nodes/WMFNode.h"
#include "../../nodes/DPENode.h"
#include "../../nodes/OCCNode.h"


using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;
using namespace NSCam::v1::Stereo;

#define TEST_BM 1
#define TEST_REQ_TYPE eSTATE_NORMAL
#define TEST_FLOW_TYPE eDEPTH_FLOW_TYPE_STANDARD
#define EIS_ON 0

SmartImageBuffer
loadDataFromFile(
    const char* filepath,
    sp<BaseBufferPoolMgr> pPoolMgr,
    DepthMapBufferID bufferID,
    BufferPoolScenario scenario)
{
    sp<ImageBufferPool> pBufPool = pPoolMgr->getImageBufferPool(bufferID, scenario);
    SmartImageBuffer smBuf = pBufPool->request();

    MY_LOGD("filepath =%s Request bufferID:%d size=%dx%d", filepath, bufferID, smBuf->mImageBuffer->getImgSize().w, smBuf->mImageBuffer->getImgSize().h);

    smBuf->mImageBuffer->unlockBuf(pBufPool->getName());
    //smBuf->mImageBuffer->loadFromFile(filepath);
    smBuf->mImageBuffer->lockBuf(pBufPool->getName(), ImageBufferPool::USAGE_HW_AND_SW);

    return smBuf;
}

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
        setupReqMetadata(MFALSE, pRequest, MFALSE);
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
    pRequest->init(pBufferPoolHandler, reqAttrs);

    // ==== setup input buffers for DPE->WMF ====

    sp<BaseBufferPoolMgr> pPoolMgr = pBufferPoolHandler->getBufferPoolMgr();

    BufferPoolScenario scenario = (eState == eSTATE_CAPTURE) ? eBUFFER_POOL_SCENARIO_CAPTURE : eBUFFER_POOL_SCENARIO_PREVIEW;

    SmartImageBuffer smBuf;
    // OCC input MYS
    smBuf = loadDataFromFile("/sdcard/vsdof_data/9_Main1.yuv", pPoolMgr, BID_P2A_OUT_MY_S, scenario);
    pBufferPoolHandler->configExternalOutBuffer(eDPETHMAP_PIPE_NODEID_OCC, BID_P2A_OUT_MY_S,
                                            eDPETHMAP_PIPE_NODEID_OCC, smBuf);
    // OCC input LDC -- LDC　in capture exist in the output request buffer
    if(eState == eSTATE_NORMAL)
    {
        smBuf = loadDataFromFile("/sdcard/vsdof_data/XVEC_N3D.raw", pPoolMgr, BID_N3D_OUT_LDC, scenario);
        pBufferPoolHandler->configExternalOutBuffer(eDPETHMAP_PIPE_NODEID_DPE, BID_N3D_OUT_LDC,
                                            eDPETHMAP_PIPE_NODEID_OCC, smBuf);
    }
    // DPE input MV_Y
    smBuf = loadDataFromFile("/sdcard/vsdof_data/9_00001Main1.yuv", pPoolMgr, BID_N3D_OUT_MV_Y, scenario);
    pBufferPoolHandler->configExternalOutBuffer(eDPETHMAP_PIPE_NODEID_DPE, BID_N3D_OUT_MV_Y,
                                            eDPETHMAP_PIPE_NODEID_DPE, smBuf);
    // DPE input SV_Y
    smBuf = loadDataFromFile("/sdcard/vsdof_data/9_00001Main2.yuv", pPoolMgr, BID_N3D_OUT_SV_Y, scenario);
    pBufferPoolHandler->configExternalOutBuffer(eDPETHMAP_PIPE_NODEID_DPE, BID_N3D_OUT_SV_Y,
                                            eDPETHMAP_PIPE_NODEID_DPE, smBuf);
    // DPE input MASK_M
    smBuf = loadDataFromFile("/sdcard/vsdof_data/9_00001maskA.k2.raw", pPoolMgr, BID_N3D_OUT_MASK_M, scenario);
    pBufferPoolHandler->configExternalOutBuffer(eDPETHMAP_PIPE_NODEID_DPE, BID_N3D_OUT_MASK_M,
                                            eDPETHMAP_PIPE_NODEID_DPE, smBuf);
    // DPE input MASK_S
    smBuf = loadDataFromFile("/sdcard/vsdof_data/9_00001maskM.k2.raw", pPoolMgr, BID_N3D_OUT_MASK_S, scenario);
    pBufferPoolHandler->configExternalOutBuffer(eDPETHMAP_PIPE_NODEID_DPE, BID_N3D_OUT_MASK_S,
                                            eDPETHMAP_PIPE_NODEID_DPE, smBuf);
    MY_LOGD("-");
    return pRequest;
}

TEST(DepthMapPipe_DPEToWMF, Standard)
{
    MY_LOGD("DepthMapPipe_DPEToWMF  TEST_BM=%d", TEST_BM);
#if (TEST_BM == 1)
    UTEnvironmenSetup::Profile profile = {eRatio_16_9, STEREO_SENSOR_PROFILE_REAR_REAR,
                                            E_STEREO_FEATURE_VSDOF, BAYER_AND_MONO};
#else
    UTEnvironmenSetup::Profile profile = {eRatio_16_9, STEREO_SENSOR_PROFILE_REAR_REAR,
                                            E_STEREO_FEATURE_VSDOF, BAYER_AND_BAYER};
#endif
    UTEnvironmenSetup setup("WMFTest", profile);

    // prepare DPE->WMF bitset
    PipeNodeBitSet nodeBitSet;
    nodeBitSet.reset();
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_P2AFM, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_DPE, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_OCC, 1);
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_WMF, 1);
    // used for generate N3D
    nodeBitSet.set(eDPETHMAP_PIPE_NODEID_N3D, 1);

    sp<DepthMapPipeSetting> pSetting;
    sp<DepthMapPipeOption> pOption;
    sp<DepthMapFlowOption> pFlowOption;

    if(!setup.isReadyToUT() || !setupUTDepthMapPipe(TEST_BM, pSetting, pOption, pFlowOption, TEST_FLOW_TYPE))
    {
        MY_LOGE("Failed to setup UT depthmap pipe!");
        EXPECT_TRUE(0);
        return;
    }

    sp<BaseBufferPoolMgr> pBuffPoolMgr = new NodeBufferPoolMgr_VSDOF(nodeBitSet, pOption);
    MY_LOGD("Create Nodes...");
    PipeNodeConfigs config(pFlowOption, pSetting, pOption);
    // create pipe nodes
    DPENode *pDPENode = new DPENode("DPENode", eDPETHMAP_PIPE_NODEID_DPE, config);
    OCCNode *pOCCNode = new OCCNode("OCCNode", eDPETHMAP_PIPE_NODEID_OCC, config);
    WMFNode *pWMFNode = new WMFNode("WMFNode", eDPETHMAP_PIPE_NODEID_WMF, config);
    // use the last node　ｉｄ　ｅｎｕｍ :eDPETHMAP_PIPE_NODE_SIZE
    CallbackUTNode *pCallbackUTNode = new CallbackUTNode("CBUTNode", eDPETHMAP_PIPE_NODE_SIZE, config, finishCB_forListener);
    //
    std::vector<DepthMapPipeNode*> mvNodes;
    mvNodes.push_back(pDPENode);
    mvNodes.push_back(pOCCNode);
    mvNodes.push_back(pWMFNode);
    mvNodes.push_back(pCallbackUTNode);
    // init the basic usage
    DepthInfoStorage* pStorage = new DepthInfoStorage();
    NodeSignal* pNodeSignal = new NodeSignal();
    for(auto pNode : mvNodes)
    {
        pNode->setDepthStorage(pStorage);
        pNode->setNodeSignal(pNodeSignal);
    }
    //
    sp<ImageBufferPool> pPool = pBuffPoolMgr->getImageBufferPool(BID_DPE_INTERNAL_LAST_DMP);
    MY_LOGD("setDMPBufferPool:%x ...", pPool.get());
    pDPENode->setDMPBufferPool(pPool);

    MY_LOGD("Connect Nodes...");
    // connect data
    CONNECT_DATA_AND_SETUPDUMP(pDPENode, pOCCNode, DPE_TO_OCC_MVSV_DMP_CFM);
    CONNECT_DATA_AND_SETUPDUMP(pOCCNode, pWMFNode, OCC_TO_WMF_OMYSN);
    CONNECT_DATA_AND_SETUPDUMP(pWMFNode, pCallbackUTNode, WMF_TO_GF_DMW_MY_S);
    CONNECT_DATA_AND_SETUPDUMP(pWMFNode, pCallbackUTNode, WMF_TO_GF_OND);

    MY_LOGD("Create requests...");
    // build requet
    vector<MUINT32> vReqIDVec;
    vector<DepthMapRequestPtr> vRequestVec;
    for(int i=0;i<1;i++)
    {
        DepthMapRequestPtr pRequest = prepareEnqueRequest(i, TEST_REQ_TYPE, pBuffPoolMgr, pFlowOption,
                                                pOption->mFlowType ==  eDEPTH_FLOW_TYPE_QUEUED_DEPTH);
        vRequestVec.push_back(pRequest);
        vReqIDVec.push_back(i);
    }

    // setup waiting ID list
    vector<DepthMapDataID> vWaitDataID;
    if(TEST_REQ_TYPE == eSTATE_CAPTURE)
        vWaitDataID.push_back(WMF_TO_GF_OND);
    else
        vWaitDataID.push_back(WMF_TO_GF_DMW_MY_S);
    pCallbackUTNode->setWaitingDataIDAndReqID(vWaitDataID, vReqIDVec);
    // init
    for(auto pNode : mvNodes)
    {
        pNode->init();
        pNode->start();
    }
    // insert into DPE node
    for(auto i=0; i<vRequestVec.size();i++)
    {
        auto pRequest = vRequestVec[i];
        MY_LOGD("Insert requests...No: %d", i);
        pDPENode->onData(N3D_TO_DPE_MVSV_MASK, pRequest);
        pOCCNode->onData(P2A_TO_OCC_MY_S, pRequest);
        pOCCNode->onData(N3D_TO_OCC_LDC, pRequest);

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


