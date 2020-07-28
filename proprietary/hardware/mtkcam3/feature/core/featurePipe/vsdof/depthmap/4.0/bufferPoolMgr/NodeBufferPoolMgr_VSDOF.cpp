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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

/**
 * @file NodeBufferPoolMgr_VSDOF.cpp
 * @brief BufferPoolMgr for VSDOF
*/

// Standard C header file
#include <future>
#include <thread>
// Android system/core header file
#include <ui/gralloc_extra.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/def/dpecommon_v20.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/def/IPostProcDef.h>
// Module header file
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
// Local header file
#include "NodeBufferPoolMgr_VSDOF.h"
#include "NodeBufferHandler.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipeUtils.h"
#include "../StageExecutionTime.h"
#include "./bufferSize/NodeBufferSizeMgr.h"
// Logging header file
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "NodeBufferPoolMgr_VSDOF"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSIoPipe;
using NSCam::NSIoPipe::NSPostProc::INormalStream;

#define SUPPORT_RECORD 0

/*******************************************************************************
* Global Define
********************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferPoolMgr_VSDOF class - Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferPoolMgr_VSDOF::
NodeBufferPoolMgr_VSDOF(
    PipeNodeBitSet& nodeBitSet,
    sp<DepthMapPipeSetting> pPipeSetting,
    sp<DepthMapPipeOption> pPipeOption
)
: mNodeBitSet(nodeBitSet)
, mpPipeOption(pPipeOption)
, mpPipeSetting(pPipeSetting)
{
    // decide size mgr
    mpBufferSizeMgr = new NodeBufferSizeMgr(pPipeOption);
    MBOOL bRet = this->initializeBufferPool();

    if(!bRet)
    {
        MY_LOGE("Failed to initialize buffer pool set! Cannot continue..");
        uninit();
        return;
    }

    this->buildImageBufferPoolMap();
    this->buildBufScenarioToTypeMap();

}

NodeBufferPoolMgr_VSDOF::
~NodeBufferPoolMgr_VSDOF()
{
    MY_LOGD("[Destructor] +");
    uninit();
    MY_LOGD("[Destructor] -");
}


MBOOL
NodeBufferPoolMgr_VSDOF::
uninit()
{
    // destroy buffer pools
    FatImageBufferPool::destroy(mpRectInBufPool_Main1_CAP);
    #if(SUPPORT_RECORD == 1)
    FatImageBufferPool::destroy(mpRectInBufPool_Main2_VR);
    #endif
    FatImageBufferPool::destroy(mpRectInBufPool_Main2_PV);
    FatImageBufferPool::destroy(mpRectInBufPool_Main2_CAP);
    FatImageBufferPool::destroy(mpFEOB_BufPool);
    FatImageBufferPool::destroy(mpFEOC_BufPool);
    FatImageBufferPool::destroy(mpFMOB_BufPool);
    FatImageBufferPool::destroy(mpFMOC_BufPool);
    FatImageBufferPool::destroy(mpFEBInBufPool_Main1);
    FatImageBufferPool::destroy(mpFEBInBufPool_Main2);
    FatImageBufferPool::destroy(mpFECInBufPool_Main1);
    FatImageBufferPool::destroy(mpFECInBufPool_Main2);
    TuningBufferPool::destroy(mpTuningBufferPool);
    TuningBufferPool::destroy(mpPQTuningBufferPool);
    TuningBufferPool::destroy(mpDpPQParamTuningBufferPool);
    #ifdef GTEST
    FatImageBufferPool::destroy(mFEHWInput_StageB_Main1);
    FatImageBufferPool::destroy(mFEHWInput_StageB_Main2);
    FatImageBufferPool::destroy(mFEHWInput_StageC_Main1);
    FatImageBufferPool::destroy(mFEHWInput_StageC_Main2);
    #endif
    FatImageBufferPool::destroy(mMYSImgBufPool);
    FatImageBufferPool::destroy(mIMG3OmgBufPool);
    FatImageBufferPool::destroy(mInternalFDImgBufPool);
    //----------------------N3D section--------------------------------//
    GraphicBufferPool::destroy(mN3DWarpingMatrix_Main2_X);
    GraphicBufferPool::destroy(mN3DWarpingMatrix_Main2_X_CAP);
    GraphicBufferPool::destroy(mN3DWarpingMatrix_Main2_Y);
    GraphicBufferPool::destroy(mN3DWarpingMatrix_Main2_Y_CAP);
    //----------------------WPE section--------------------------------//
    FatImageBufferPool::destroy(mDefaultMaskBufPool_Main2);
    FatImageBufferPool::destroy(mWarpImgBufPool_Main1);
    FatImageBufferPool::destroy(mWarpImgBufPool_Main1_CAP);
    FatImageBufferPool::destroy(mWarpMaskBufPool_Main1);
    FatImageBufferPool::destroy(mWarpMaskBufPool_Main1_CAP);

    FatImageBufferPool::destroy(mWarpImgBufPool_Main2);
    FatImageBufferPool::destroy(mWarpImgBufPool_Main2_CAP);
    FatImageBufferPool::destroy(mWarpMaskBufPool_Main2);
    FatImageBufferPool::destroy(mWarpMaskBufPool_Main2_CAP);
    //----------------------DPE section--------------------------------//
    FatImageBufferPool::destroy(mDVLRImgBufPool);
    FatImageBufferPool::destroy(mCFMImgBufPool);
    FatImageBufferPool::destroy(mNOCImgBufPool);
    FatImageBufferPool::destroy(mASFCRMImgBufPool);
    FatImageBufferPool::destroy(mASFRDImgBufPool);
    FatImageBufferPool::destroy(mASFHFImgBufPool);
    FatImageBufferPool::destroy(mDMWImgBufPool);
    //----------------------GF section--------------------------------//
    FatImageBufferPool::destroy(mpInternalDMBGImgBufPool);

    if(mpBufferSizeMgr != nullptr)
        delete mpBufferSizeMgr;

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
SmartFatImageBuffer
NodeBufferPoolMgr_VSDOF::
request(DepthMapBufferID id, BufferPoolScenario scen)
{
    ssize_t index;
    if((index=mBIDtoImgBufPoolMap_Default.indexOfKey(id)) >= 0)
    {
        TIME_THREHOLD(10, "request buffer id=%d %s", id, DepthMapPipeNode::onDumpBIDToName(id));
        sp<FatImageBufferPool> pBufferPool = mBIDtoImgBufPoolMap_Default.valueAt(index);
        return pBufferPool->request();
    }
    else if((index=mBIDtoImgBufPoolMap_Scenario.indexOfKey(id)) >= 0)
    {
        ScenarioToImgBufPoolMap ScenarioBufMap = mBIDtoImgBufPoolMap_Scenario.valueAt(index);
        if((index=ScenarioBufMap.indexOfKey(scen))>=0)
        {
            TIME_THREHOLD(10, "request buffer id=%d %s", id, DepthMapPipeNode::onDumpBIDToName(id));
            sp<FatImageBufferPool> pBufferPool = ScenarioBufMap.valueAt(index);
            return pBufferPool->request();
        }
    }
    return NULL;
}

SmartGraphicBuffer
NodeBufferPoolMgr_VSDOF::
requestGB(DepthMapBufferID id, BufferPoolScenario scen)
{

    ssize_t index;
    if((index=mBIDtoGraBufPoolMap_Scenario.indexOfKey(id)) >= 0)
    {
        TIME_THREHOLD(10, "request GB buffer id=%d %s", id, DepthMapPipeNode::onDumpBIDToName(id));
        ScenarioToGraBufPoolMap ScenarioBufMap = mBIDtoGraBufPoolMap_Scenario.valueAt(index);
        if((index=ScenarioBufMap.indexOfKey(scen))>=0)
        {
            sp<GraphicBufferPool> pBufferPool = ScenarioBufMap.valueAt(index);
            VSDOF_LOGD("requestGB  bufferID=%d", id);
            SmartGraphicBuffer smGraBuf = pBufferPool->request();
            return smGraBuf;
        }
    }
    return NULL;
}

SmartTuningBuffer
NodeBufferPoolMgr_VSDOF::
requestTB(DepthMapBufferID id, BufferPoolScenario scen)
{
    TIME_THREHOLD(10, "request tuning buffer id=%d %s", id, DepthMapPipeNode::onDumpBIDToName(id));
    SmartTuningBuffer smTuningBuf = nullptr;
    if(id == BID_P2A_TUNING)
    {
        smTuningBuf = mpTuningBufferPool->request();
        memset(smTuningBuf->mpVA, 0, mpTuningBufferPool->getBufSize());
    }
    else if(id == BID_PQ_PARAM)
    {
        smTuningBuf = mpPQTuningBufferPool->request();
        memset(smTuningBuf->mpVA, 0, mpPQTuningBufferPool->getBufSize());
    }
    else if(id == BID_DP_PQ_PARAM)
    {
        smTuningBuf = mpDpPQParamTuningBufferPool->request();
        memset(smTuningBuf->mpVA, 0, mpDpPQParamTuningBufferPool->getBufSize());
    }

    if(smTuningBuf.get() == nullptr)
        MY_LOGE("Cannot find the TuningBufferPool with scenario:%d of buffer id:%d!!", scen, id);
    return smTuningBuf;
}

BufferPoolHandlerPtr
NodeBufferPoolMgr_VSDOF::
createBufferPoolHandler()
{
    BaseBufferHandler* pPtr = new NodeBufferHandler(this);
    return pPtr;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
queryBufferType(
    DepthMapBufferID bid,
    BufferPoolScenario scen,
    DepthBufferType& rOutBufType
)
{
    ssize_t index;

    if((index=mBIDToScenarioTypeMap.indexOfKey(bid))>=0)
    {
        BufScenarioToTypeMap scenMap = mBIDToScenarioTypeMap.valueAt(index);
        if((index=scenMap.indexOfKey(scen))>=0)
        {
            rOutBufType = scenMap.valueAt(index);
            return MTRUE;
        }
    }
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferPoolMgr_VSDOF class - Private Functinos
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
NodeBufferPoolMgr_VSDOF::
initBT160ColorSpaceForYUVGB(sp<GraphicBufferPool> pool)
{
    GraphicBufferPool::CONTAINER_TYPE poolContents = pool->getPoolContents();
    for(sp<GraphicBufferHandle>& handle : poolContents)
    {
        // config
        android::sp<NativeBufferWrapper> pNativBuffer = handle->mGraphicBuffer;
        // config graphic buffer to BT601_FULL
        gralloc_extra_ion_sf_info_t info;
        gralloc_extra_query(pNativBuffer->getHandle(), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
        gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
        gralloc_extra_perform(pNativBuffer->getHandle(), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
    }
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initializeBufferPool()
{
    CAM_ULOGM_TAGLIFE("NodeBufferPoolMgr_VSDOF::initializeBufferPool");
    VSDOF_INIT_LOG("+");

    MBOOL bRet = MTRUE;
    std::future<MBOOL> vFutures[eDPETHMAP_PIPE_NODE_SIZE+1];
    int index = 0;

    // P2A buffer pool
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_P2A))
    {
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initP2ABufferPool, this);
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initFEFMBufferPool, this);
    }
    // N3D buffer pool
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_N3D))
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initN3DWPEBufferPool, this);
    // DPE buffer pool
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_DPE))
    {
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initDVSBufferPool, this);

        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initDVPBufferPool, this);
    }
    // GF buffer pool
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_GF))
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initGFBufferPool, this);

    // wait all futures
    for(int idx = index-1; idx >=0; --idx)
    {
        bRet &= vFutures[idx].get();
    }

    VSDOF_INIT_LOG("-");
    return bRet;
}



MBOOL
NodeBufferPoolMgr_VSDOF::
initP2ABufferPool()
{
    VSDOF_INIT_LOG("+");
    const P2ABufferSize& rP2A_VR=mpBufferSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    const P2ABufferSize& rP2A_PV=mpBufferSizeMgr->getP2A(eSTEREO_SCENARIO_PREVIEW);
    const P2ABufferSize& rP2A_CAP=mpBufferSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);

    /**********************************************************************
     * Rectify_in/FD/Tuning/FE/FM buffer pools
     **********************************************************************/
    // VR, Rect_in2 (main2)
    #if(SUPPORT_RECORD == 1)
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main2_VR, "RectInBufPool_Main2_VR",
                        rP2A_VR.mRECT_IN_SIZE_MAIN2,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    #endif
    // PV, Rect_in2
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main2_PV, "RectInPV_BufPool_Main2",
                        rP2A_PV.mRECT_IN_SIZE_MAIN2,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    // CAP, Rect_in1
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main1_CAP, "RectInBufPool_Main1_CAP",
                        rP2A_CAP.mRECT_IN_SIZE_MAIN1,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    // Capture, Rect_in2
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main2_CAP, "RectInGra_BufPool",
                        rP2A_CAP.mRECT_IN_SIZE_MAIN2,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    // TuningBufferPool creation
    mpTuningBufferPool = TuningBufferPool::create("VSDOF_TUNING_P2A", INormalStream::getRegTableSize());
    mpPQTuningBufferPool = TuningBufferPool::create("PQTuningPool", sizeof(PQParam));
    mpDpPQParamTuningBufferPool = TuningBufferPool::create("PQTuningPool", sizeof(DpPqParam));

    // IMG3O bufers
    CREATE_IMGBUF_POOL(mIMG3OmgBufPool, "IMG3OmgBufPool", mpPipeSetting->mszRRZO_Main1,
                        eImgFmt_YV12, FatImageBufferPool::USAGE_HW);
    // Internal FD
    CREATE_IMGBUF_POOL(mInternalFDImgBufPool, "mInternalFDImgBufPool", rP2A_PV.mFD_IMG_SIZE,
                        eImgFmt_YUY2, eBUFFER_USAGE_SW_READ_OFTEN|FatImageBufferPool::USAGE_HW)
    ALLOCATE_BUFFER_POOL(mInternalFDImgBufPool, VSDOF_WORKING_BUF_SET*2);

    // allcate buffer
    #if(SUPPORT_RECORD == 1)
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_VR, CALC_BUFFER_COUNT(P2A_RUN_MS+N3D_RUN_MS+WPE_RUN_MS));
    #endif
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_PV, CALC_BUFFER_COUNT(P2A_RUN_MS+N3D_RUN_MS+WPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main1_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpTuningBufferPool, VSDOF_DEPTH_P2FRAME_SIZE + 2);
    ALLOCATE_BUFFER_POOL(mIMG3OmgBufPool, CALC_BUFFER_COUNT(P2A_RUN_MS)+QUEUE_DATA_BACKUP);
    ALLOCATE_BUFFER_POOL(mpPQTuningBufferPool, VSDOF_DEPTH_P2FRAME_SIZE);
    ALLOCATE_BUFFER_POOL(mpDpPQParamTuningBufferPool, VSDOF_DEPTH_P2FRAME_SIZE);
    ALLOCATE_BUFFER_POOL(mInternalFDImgBufPool, CALC_BUFFER_COUNT(P2ABAYER_RUN_MS));
    // init MY_S
    CREATE_IMGBUF_POOL(mMYSImgBufPool, "mMYSImgBufPool", rP2A_VR.mMYS_SIZE, eImgFmt_NV12,
                        eBUFFER_USAGE_SW_READ_OFTEN|FatImageBufferPool::USAGE_HW);
    // MY_S : from P2ABayer to GF
    ALLOCATE_BUFFER_POOL(mMYSImgBufPool, CALC_BUFFER_COUNT(TOTAL_RUN_MS));
    VSDOF_INIT_LOG("-");
    return MTRUE;
}


MBOOL
NodeBufferPoolMgr_VSDOF::
initFEFMBufferPool()
{
    VSDOF_INIT_LOG("+");
    const P2ABufferSize& rP2aSize = mpBufferSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);
    /**********************************************************************
     * FE/FM has 3 stage A,B,C, currently only apply 2 stage FEFM: stage=B(1),C(2)
     **********************************************************************/
    // FE/FM buffer pool - stage B
    MUINT32 iBlockSize = StereoSettingProvider::fefmBlockSize(1);
    // query the FEO buffer size from FE input buffer size
    MSize szFEBufSize = rP2aSize.mFEB_INPUT_SIZE_MAIN1;
    MSize szFEOBufferSize, szFMOBufferSize;
    queryFEOBufferSize(szFEBufSize, iBlockSize, szFEOBufferSize);
    // create buffer pool
    CREATE_IMGBUF_POOL(mpFEOB_BufPool, "FEB_BufPoll", szFEOBufferSize,
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);
    // query the FMO buffer size from FEO size
    queryFMOBufferSize(szFEOBufferSize, szFMOBufferSize);
    // create buffer pool
    CREATE_IMGBUF_POOL(mpFMOB_BufPool, "FMB_BufPool", szFMOBufferSize,
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);

    // FE/FM buffer pool - stage C
    iBlockSize = StereoSettingProvider::fefmBlockSize(2);
    // query FEO/FMO size and create pool
    szFEBufSize = rP2aSize.mFEC_INPUT_SIZE_MAIN1;
    queryFEOBufferSize(szFEBufSize, iBlockSize, szFEOBufferSize);
    CREATE_IMGBUF_POOL(mpFEOC_BufPool, "FEC_BufPool", szFEOBufferSize,
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);
    queryFMOBufferSize(szFEOBufferSize, szFMOBufferSize);
    CREATE_IMGBUF_POOL(mpFMOC_BufPool, "FMC_BufPool", szFMOBufferSize,
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW);

    // create the FE input buffer pool - stage B (the seocond FE input buffer)
    //FEB Main1 input
    CREATE_IMGBUF_POOL(mpFEBInBufPool_Main1, "FE1BInputBufPool", rP2aSize.mFEB_INPUT_SIZE_MAIN1,
                    eImgFmt_YV12, FatImageBufferPool::USAGE_HW);

    //FEB Main2 input
    CREATE_IMGBUF_POOL(mpFEBInBufPool_Main2, "FE2BInputBufPool", rP2aSize.mFEB_INPUT_SIZE_MAIN2,
                    eImgFmt_YV12, FatImageBufferPool::USAGE_HW);

    //FEC Main1 input
    CREATE_IMGBUF_POOL(mpFECInBufPool_Main1, "FE1CInputBufPool", rP2aSize.mFEC_INPUT_SIZE_MAIN1,
                    eImgFmt_YUY2, FatImageBufferPool::USAGE_HW);

    //FEC Main2 input
    CREATE_IMGBUF_POOL(mpFECInBufPool_Main2, "FE2CInputBufPool", rP2aSize.mFEC_INPUT_SIZE_MAIN2,
                    eImgFmt_YUY2, FatImageBufferPool::USAGE_HW);

    // FEO/FMO buffer pool- ALLOCATE buffers : Main1+Main2 -> two working set
    int FEFMO_BUFFER_COUNT = 2 * CALC_BUFFER_COUNT(P2A_RUN_MS+P2ABAYER_RUN_MS+N3D_RUN_MS+N3D_LEARNING_RUN_MS);
    ALLOCATE_BUFFER_POOL(mpFEOB_BufPool, FEFMO_BUFFER_COUNT)
    ALLOCATE_BUFFER_POOL(mpFEOC_BufPool, FEFMO_BUFFER_COUNT)
    ALLOCATE_BUFFER_POOL(mpFMOB_BufPool, FEFMO_BUFFER_COUNT)
    ALLOCATE_BUFFER_POOL(mpFMOC_BufPool, FEFMO_BUFFER_COUNT)

    // FEB/FEC_Input buffer pool- ALLOCATE buffers : 2 (internal working buffer in Burst trigger)
    ALLOCATE_BUFFER_POOL(mpFEBInBufPool_Main1, 2)
    ALLOCATE_BUFFER_POOL(mpFEBInBufPool_Main2, 2)
    ALLOCATE_BUFFER_POOL(mpFECInBufPool_Main1, 2)
    ALLOCATE_BUFFER_POOL(mpFECInBufPool_Main2, 2)

    #ifdef GTEST
    // FE HW Input For UT
    CREATE_IMGBUF_POOL(mFEHWInput_StageB_Main1, "mFEHWInput_StageB_Main1", rP2aSize.mFEB_INPUT_SIZE_MAIN1, eImgFmt_YV12, FatImageBufferPool::USAGE_HW);
    CREATE_IMGBUF_POOL(mFEHWInput_StageB_Main2, "mFEHWInput_StageB_Main2", rP2aSize.mFEBO_AREA_MAIN2.size, eImgFmt_YV12, FatImageBufferPool::USAGE_HW);
    CREATE_IMGBUF_POOL(mFEHWInput_StageC_Main1, "mFEHWInput_StageC_Main1", rP2aSize.mFEC_INPUT_SIZE_MAIN1, eImgFmt_YV12, FatImageBufferPool::USAGE_HW);
    CREATE_IMGBUF_POOL(mFEHWInput_StageC_Main2, "mFEHWInput_StageC_Main2", rP2aSize.mFECO_AREA_MAIN2.size, eImgFmt_YV12, FatImageBufferPool::USAGE_HW);
    ALLOCATE_BUFFER_POOL(mFEHWInput_StageB_Main1, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mFEHWInput_StageB_Main2, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mFEHWInput_StageC_Main1, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mFEHWInput_StageC_Main2, VSDOF_WORKING_BUF_SET);
    #endif
    VSDOF_INIT_LOG("-");

    return MTRUE;
}


MBOOL
NodeBufferPoolMgr_VSDOF::
initN3DWPEBufferPool()
{
    VSDOF_INIT_LOG("+");
    // n3d size : no difference between scenarios
    const N3DBufferSize& rN3DSize = mpBufferSizeMgr->getN3D(eSTEREO_SCENARIO_RECORD);
    const N3DBufferSize& rN3DSize_CAP = mpBufferSizeMgr->getN3D(eSTEREO_SCENARIO_CAPTURE);
    // SW read/write, hw read
    MUINT32 usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;

    // INIT N3D/WPE output buffer pool
    // MV_Y
    CREATE_IMGBUF_POOL(mWarpImgBufPool_Main1, "WarpImgBufPool_Main1", rN3DSize.mWARP_IMG_SIZE,
                        eImgFmt_Y8, usage);
    CREATE_IMGBUF_POOL(mWarpImgBufPool_Main1_CAP, "mWarpImgBufPool_Main1_CAP", rN3DSize_CAP.mWARP_IMG_SIZE,
                        eImgFmt_Y8, usage);
    // SV_Y
    CREATE_IMGBUF_POOL(mWarpImgBufPool_Main2, "mWarpImgBufPool_Main2", rN3DSize.mWARP_IMG_SIZE,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    CREATE_IMGBUF_POOL(mWarpImgBufPool_Main2_CAP, "mWarpImgBufPool_Main2_CAP", rN3DSize_CAP.mWARP_IMG_SIZE,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    // MASK_M
    CREATE_IMGBUF_POOL(mWarpMaskBufPool_Main1, "N3DMaskBufPool", rN3DSize.mWARP_MASK_SIZE,
                        eImgFmt_Y8, usage);
    CREATE_IMGBUF_POOL(mWarpMaskBufPool_Main1_CAP, "mWarpMaskBufPool_Main1_CAP", rN3DSize_CAP.mWARP_MASK_SIZE,
                        eImgFmt_Y8, usage);
    // MASK_S
    CREATE_IMGBUF_POOL(mWarpMaskBufPool_Main2, "N3DMaskBufPool_Main2", rN3DSize.mWARP_MASK_SIZE,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    CREATE_IMGBUF_POOL(mWarpMaskBufPool_Main2_CAP, "mWarpMaskBufPool_Main2_CAP", rN3DSize_CAP.mWARP_MASK_SIZE,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW);
    // mN3DWarpingMatrix_Main2_X
    CREATE_GRABUF_POOL(mN3DWarpingMatrix_Main2_X, "N3DWarpingMatrix_Main2_X", rN3DSize.mWARP_MAP_SIZE_MAIN2,
                        HAL_PIXEL_FORMAT_RGBA_8888 , GraphicBufferPool::USAGE_HW_TEXTURE);
    CREATE_GRABUF_POOL(mN3DWarpingMatrix_Main2_X_CAP, "N3DWarpingMatrix_Main2_X_CAP", rN3DSize_CAP.mWARP_MAP_SIZE_MAIN2,
                        HAL_PIXEL_FORMAT_RGBA_8888 , GraphicBufferPool::USAGE_HW_TEXTURE);
    // mN3DWarpingMatrix_Main2_Y
    CREATE_GRABUF_POOL(mN3DWarpingMatrix_Main2_Y, "N3DWarpingMatrix_Main2_Y", rN3DSize.mWARP_MAP_SIZE_MAIN2,
                        HAL_PIXEL_FORMAT_RGBA_8888 , GraphicBufferPool::USAGE_HW_TEXTURE);
    CREATE_GRABUF_POOL(mN3DWarpingMatrix_Main2_Y_CAP, "N3DWarpingMatrix_Main2_Y_CAP", rN3DSize_CAP.mWARP_MAP_SIZE_MAIN2,
                        HAL_PIXEL_FORMAT_RGBA_8888 , GraphicBufferPool::USAGE_HW_TEXTURE);
    // WPE_IN_MASK
    CREATE_IMGBUF_POOL(mDefaultMaskBufPool_Main2, "DefaultMaskBufPool", rN3DSize.mWARP_IN_MASK_MAIN2,
                        eImgFmt_Y8, usage);
    // allocate
    ALLOCATE_BUFFER_POOL(mWarpImgBufPool_Main1, CALC_BUFFER_COUNT(N3D_RUN_MS+WPE_RUN_MS+DPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mWarpImgBufPool_Main1_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mWarpImgBufPool_Main2, CALC_BUFFER_COUNT(N3D_RUN_MS+WPE_RUN_MS+DPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mWarpImgBufPool_Main2_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mWarpMaskBufPool_Main1, CALC_BUFFER_COUNT(N3D_RUN_MS+WPE_RUN_MS+DPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mWarpMaskBufPool_Main1_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mWarpMaskBufPool_Main2, CALC_BUFFER_COUNT(N3D_RUN_MS+WPE_RUN_MS+DPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mWarpMaskBufPool_Main2_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mN3DWarpingMatrix_Main2_X, CALC_BUFFER_COUNT(N3D_RUN_MS+WPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mN3DWarpingMatrix_Main2_X_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mN3DWarpingMatrix_Main2_Y, CALC_BUFFER_COUNT(N3D_RUN_MS+WPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mN3DWarpingMatrix_Main2_Y_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mDefaultMaskBufPool_Main2, 2);
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initDVSBufferPool()
{
    VSDOF_INIT_LOG("+");
    #define BUFFRPOOL_EXTRA_SIZE_FOR_LAST_DMP 1
    // DPE size : no difference between scenarios
    const DPEBufferSize& rDPESize = mpBufferSizeMgr->getDPE(eSTEREO_SCENARIO_RECORD);
    // allocate with the stride size as the width
    // DV_LR
    CREATE_IMGBUF_POOL(mDVLRImgBufPool, "DVLRImgBufPool", rDPESize.mDV_LR_SIZE, eImgFmt_Y16,
                        FatImageBufferPool::USAGE_HW)
    // CFM
    CREATE_IMGBUF_POOL(mCFMImgBufPool, "CFMImgBufPool", rDPESize.mCFM_SIZE, eImgFmt_Y8,
                        FatImageBufferPool::USAGE_HW)
    // NOC
    CREATE_IMGBUF_POOL(mNOCImgBufPool, "NOCImgBufPool", rDPESize.mNOC_SIZE, eImgFmt_Y8,
                        FatImageBufferPool::USAGE_HW)
    // alloc
    ALLOCATE_BUFFER_POOL(mDVLRImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS) + BUFFRPOOL_EXTRA_SIZE_FOR_LAST_DMP);
    ALLOCATE_BUFFER_POOL(mCFMImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS+GF_RUN_MS));
    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH)
    {
        ALLOCATE_BUFFER_POOL(mNOCImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS)+QUEUE_DATA_BACKUP);
    }
    else
    {
        ALLOCATE_BUFFER_POOL(mNOCImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS));
    }
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initDVPBufferPool()
{
    VSDOF_INIT_LOG("+");
    const DPEBufferSize& rDPESize = mpBufferSizeMgr->getDPE(eSTEREO_SCENARIO_RECORD);
    // ASF_CRM
    CREATE_IMGBUF_POOL(mASFCRMImgBufPool, "ASFCRMImgBufPool", rDPESize.mASF_CRM_SIZE, eImgFmt_Y8,
                        FatImageBufferPool::USAGE_HW)
    // ASF RD
    CREATE_IMGBUF_POOL(mASFRDImgBufPool, "ASFRDImgBufPool", rDPESize.mASF_RD_SIZE, eImgFmt_Y8,
                        FatImageBufferPool::USAGE_HW)
    // ASF HF
    CREATE_IMGBUF_POOL(mASFHFImgBufPool, "ASFHFImgBufPool", rDPESize.mASF_HF_SIZE, eImgFmt_Y8,
                        FatImageBufferPool::USAGE_HW)
    // DMW
    CREATE_IMGBUF_POOL(mDMWImgBufPool, "DMWImgBufPool", rDPESize.mDMW_SIZE, eImgFmt_Y8,
                        FatImageBufferPool::USAGE_HW)
    // alloc
    ALLOCATE_BUFFER_POOL(mASFCRMImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mASFRDImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mASFHFImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS));
    ALLOCATE_BUFFER_POOL(mDMWImgBufPool, CALC_BUFFER_COUNT(DPE_RUN_MS+GF_RUN_MS));
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initGFBufferPool()
{
    VSDOF_INIT_LOG("+");
    MSize szDMBGSize = mpBufferSizeMgr->getGF(eSTEREO_SCENARIO_RECORD).mDMBG_SIZE;
    MSize szDepthMapSize = mpBufferSizeMgr->getGF(eSTEREO_SCENARIO_RECORD).mDEPTHMAP_SIZE;

    CREATE_IMGBUF_POOL(mpInternalDMBGImgBufPool, "mpInternalDMBGImgBufPool", szDMBGSize
                        , eImgFmt_Y8, FatImageBufferPool::USAGE_SW);
    // extra one size means the one stored in the DepthStorge
    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_VSDOF)
    {
        ALLOCATE_BUFFER_POOL(mpInternalDMBGImgBufPool, CALC_BUFFER_COUNT(GF_RUN_MS)+QUEUE_DATA_BACKUP)
    }
    else
    {
        ALLOCATE_BUFFER_POOL(mpInternalDMBGImgBufPool, CALC_BUFFER_COUNT(GF_RUN_MS))
    }
    VSDOF_INIT_LOG("-");
    return MTRUE;
}


MBOOL
NodeBufferPoolMgr_VSDOF::
buildImageBufferPoolMap()
{
    MY_LOGD("+");

    //---- build the buffer pool without specific scenario ----
    // P2A Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_P2A))
    {
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FE1BO, mpFEOB_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FE2BO, mpFEOB_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FE1CO, mpFEOC_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FE2CO, mpFEOC_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FMBO_RL, mpFMOB_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FMBO_LR, mpFMOB_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FMCO_LR, mpFMOC_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_FMCO_RL, mpFMOC_BufPool);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_FE1B_INPUT, mpFEBInBufPool_Main1);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_FE2B_INPUT, mpFEBInBufPool_Main2);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_FE1C_INPUT, mpFECInBufPool_Main1);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_FE2C_INPUT, mpFECInBufPool_Main2);
        #ifdef GTEST
        mBIDtoImgBufPoolMap_Default.add(BID_FE2_HWIN_MAIN1, mFEHWInput_StageB_Main1);
        mBIDtoImgBufPoolMap_Default.add(BID_FE2_HWIN_MAIN2, mFEHWInput_StageB_Main2);
        mBIDtoImgBufPoolMap_Default.add(BID_FE3_HWIN_MAIN1, mFEHWInput_StageC_Main1);
        mBIDtoImgBufPoolMap_Default.add(BID_FE3_HWIN_MAIN2, mFEHWInput_StageC_Main2);
        #endif

    }

    //---- build the buffer pool WITH specific scenario ----
    // P2A
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_P2A))
    {
        // ============ image buffer pool mapping =============
        // Rectify Main1 : BID_P2A_OUT_RECT_IN1
        ScenarioToImgBufPoolMap RectInImgBufMap_Main1;
        RectInImgBufMap_Main1.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpRectInBufPool_Main1_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_RECT_IN1, RectInImgBufMap_Main1);
        // MYS
        ScenarioToImgBufPoolMap MYSImgBufMap;
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mMYSImgBufPool);
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mMYSImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_MY_S, MYSImgBufMap);
        // IMG3O
        ScenarioToImgBufPoolMap IMG3OImgBufMap;
        IMG3OImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mIMG3OmgBufPool);
        IMG3OImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mIMG3OmgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_INTERNAL_IMG3O, IMG3OImgBufMap);
        // Internal FD
        ScenarioToImgBufPoolMap InternalFDImgBufMap;
        InternalFDImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mInternalFDImgBufPool);
        InternalFDImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mInternalFDImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_INTERNAL_FD, InternalFDImgBufMap);
        // Rectify Main2 : BID_P2A_OUT_RECT_IN2
        ScenarioToImgBufPoolMap RectInImgBufMap_Main2;
        RectInImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpRectInBufPool_Main2_PV);
        #if(SUPPORT_RECORD == 1)
        RectInImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_RECORD, mpRectInBufPool_Main2_VR);
        #endif
        RectInImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpRectInBufPool_Main2_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_RECT_IN2, RectInImgBufMap_Main2);
    }
    // N3D
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_N3D))
    {
        // WPE_IN_MASK
        ScenarioToImgBufPoolMap ImgBufMap_DefaultMask;
        ImgBufMap_DefaultMask.add(eBUFFER_POOL_SCENARIO_PREVIEW, mDefaultMaskBufPool_Main2);
        ImgBufMap_DefaultMask.add(eBUFFER_POOL_SCENARIO_RECORD, mDefaultMaskBufPool_Main2);
        ImgBufMap_DefaultMask.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDefaultMaskBufPool_Main2);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WPE_IN_MASK_S, ImgBufMap_DefaultMask);
        // MV_Y
        ScenarioToImgBufPoolMap MVY_ImgBufMap;
        MVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mWarpImgBufPool_Main1);
        MVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mWarpImgBufPool_Main1);
        MVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mWarpImgBufPool_Main1_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_MV_Y, MVY_ImgBufMap);
        // MASK_M
        ScenarioToImgBufPoolMap MASKImgBufMap;
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mWarpMaskBufPool_Main1);
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mWarpMaskBufPool_Main1);
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mWarpMaskBufPool_Main1_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_MASK_M, MASKImgBufMap);
        // SV_Y
        ScenarioToImgBufPoolMap WarpImgBufMap_Main2;
        WarpImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_PREVIEW, mWarpImgBufPool_Main2);
        WarpImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_RECORD, mWarpImgBufPool_Main2);
        WarpImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_CAPTURE, mWarpImgBufPool_Main2_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WPE_OUT_SV_Y, WarpImgBufMap_Main2);
        // MASK_S
        ScenarioToImgBufPoolMap MaskImgBufMap_Main2;
        MaskImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_PREVIEW, mWarpMaskBufPool_Main2);
        MaskImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_RECORD, mWarpMaskBufPool_Main2);
        MaskImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_CAPTURE, mWarpMaskBufPool_Main2_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WPE_OUT_MASK_S, MaskImgBufMap_Main2);
        // warping matrix (graphic)
        ScenarioToGraBufPoolMap GraBufMap_WarpMtx2_X;
        GraBufMap_WarpMtx2_X.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DWarpingMatrix_Main2_X);
        GraBufMap_WarpMtx2_X.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DWarpingMatrix_Main2_X);
        GraBufMap_WarpMtx2_X.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DWarpingMatrix_Main2_X_CAP);
        mBIDtoGraBufPoolMap_Scenario.add(BID_N3D_OUT_WARPMTX_MAIN2_X, GraBufMap_WarpMtx2_X);

        ScenarioToGraBufPoolMap GraBufMap_WarpMtx2_Y;
        GraBufMap_WarpMtx2_Y.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DWarpingMatrix_Main2_Y);
        GraBufMap_WarpMtx2_Y.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DWarpingMatrix_Main2_Y);
        GraBufMap_WarpMtx2_Y.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DWarpingMatrix_Main2_Y_CAP);
        mBIDtoGraBufPoolMap_Scenario.add(BID_N3D_OUT_WARPMTX_MAIN2_Y, GraBufMap_WarpMtx2_Y);
    }
    //DPE Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_DPE))
    {
        // DV_LR
        ScenarioToImgBufPoolMap imgBufMap_DVLR;
        imgBufMap_DVLR.add(eBUFFER_POOL_SCENARIO_PREVIEW, mDVLRImgBufPool);
        imgBufMap_DVLR.add(eBUFFER_POOL_SCENARIO_RECORD, mDVLRImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DVS_OUT_DV_LR, imgBufMap_DVLR);
        // CFM
        ScenarioToImgBufPoolMap imgBufMap_CFM;
        imgBufMap_CFM.add(eBUFFER_POOL_SCENARIO_PREVIEW, mCFMImgBufPool);
        imgBufMap_CFM.add(eBUFFER_POOL_SCENARIO_RECORD, mCFMImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_CFM_M, imgBufMap_CFM);
        // NOC
        ScenarioToImgBufPoolMap imgBufMap_NOC;
        imgBufMap_NOC.add(eBUFFER_POOL_SCENARIO_PREVIEW, mNOCImgBufPool);
        imgBufMap_NOC.add(eBUFFER_POOL_SCENARIO_RECORD, mNOCImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_NOC_M, imgBufMap_NOC);
        // ASF_CRM
        ScenarioToImgBufPoolMap imgBufMap_CRM;
        imgBufMap_CRM.add(eBUFFER_POOL_SCENARIO_PREVIEW, mASFCRMImgBufPool);
        imgBufMap_CRM.add(eBUFFER_POOL_SCENARIO_RECORD, mASFCRMImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_ASF_OUT_CRM, imgBufMap_CRM);
        // ASF_RD
        ScenarioToImgBufPoolMap imgBufMap_RD;
        imgBufMap_RD.add(eBUFFER_POOL_SCENARIO_PREVIEW, mASFRDImgBufPool);
        imgBufMap_RD.add(eBUFFER_POOL_SCENARIO_RECORD, mASFRDImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_ASF_OUT_RD, imgBufMap_RD);
        // ASF_HF
        ScenarioToImgBufPoolMap imgBufMap_HF;
        imgBufMap_HF.add(eBUFFER_POOL_SCENARIO_PREVIEW, mASFHFImgBufPool);
        imgBufMap_HF.add(eBUFFER_POOL_SCENARIO_RECORD, mASFHFImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_ASF_OUT_HF, imgBufMap_HF);
        // DMW
        ScenarioToImgBufPoolMap imgBufMap_DMW;
        imgBufMap_DMW.add(eBUFFER_POOL_SCENARIO_PREVIEW, mDMWImgBufPool);
        imgBufMap_DMW.add(eBUFFER_POOL_SCENARIO_RECORD, mDMWImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WMF_OUT_DMW, imgBufMap_DMW);

    }
    // GF Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_GF))
    {
        ScenarioToImgBufPoolMap DMBGImgBufMap;
        DMBGImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpInternalDMBGImgBufPool);
        DMBGImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mpInternalDMBGImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_GF_INTERNAL_DMBG, DMBGImgBufMap);
    }

    MY_LOGD("-");
    return MTRUE;
}


sp<FatImageBufferPool>
NodeBufferPoolMgr_VSDOF::
getImageBufferPool(
    DepthMapBufferID bufferID,
    BufferPoolScenario scenario
)
{
    ssize_t index=mBIDtoImgBufPoolMap_Default.indexOfKey(bufferID);
    if(index >= 0)
    {
        return mBIDtoImgBufPoolMap_Default.valueAt(index);
    }

    index = mBIDtoImgBufPoolMap_Scenario.indexOfKey(bufferID);
    if(index >= 0)
    {
        ScenarioToImgBufPoolMap bufMap;
        bufMap = mBIDtoImgBufPoolMap_Scenario.valueAt(index);
        if((index = bufMap.indexOfKey(scenario)) >= 0)
            return bufMap.valueAt(index);
    }

    MY_LOGW("Failed to get ImageBufferPool! bufferID=%d, scenario=%d", bufferID, scenario);
    return NULL;
}

sp<GraphicBufferPool>
NodeBufferPoolMgr_VSDOF::
getGraphicImageBufferPool(
    DepthMapBufferID bufferID,
    BufferPoolScenario scenario
)
{
    ssize_t index = mBIDtoGraBufPoolMap_Scenario.indexOfKey(bufferID);
    if(index >= 0)
    {
        ScenarioToGraBufPoolMap bufMap;
        bufMap = mBIDtoGraBufPoolMap_Scenario.valueAt(index);
        if((index = bufMap.indexOfKey(scenario)) >= 0)
            return bufMap.valueAt(index);
    }

    return NULL;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
buildBufScenarioToTypeMap()
{
    // buffer id in default imgBuf pool map -> all scenario are image buffer type
    for(ssize_t idx=0;idx<mBIDtoImgBufPoolMap_Default.size();++idx)
    {
        DepthMapBufferID bid = mBIDtoImgBufPoolMap_Default.keyAt(idx);
        BufScenarioToTypeMap typeMap;
        typeMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, eBUFFER_IMAGE);
        typeMap.add(eBUFFER_POOL_SCENARIO_RECORD, eBUFFER_IMAGE);
        typeMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, eBUFFER_IMAGE);
        mBIDToScenarioTypeMap.add(bid, typeMap);
    }

    // buffer id in scenario imgBuf pool map -> the scenario is image buffer type
    for(ssize_t idx=0;idx<mBIDtoImgBufPoolMap_Scenario.size();++idx)
    {
        DepthMapBufferID bid = mBIDtoImgBufPoolMap_Scenario.keyAt(idx);
        ScenarioToImgBufPoolMap scenToBufMap = mBIDtoImgBufPoolMap_Scenario.valueAt(idx);

        BufScenarioToTypeMap typeMap;
        for(ssize_t idx2=0;idx2<scenToBufMap.size();++idx2)
        {
            BufferPoolScenario sce = scenToBufMap.keyAt(idx2);
            typeMap.add(sce, eBUFFER_IMAGE);
        }
        mBIDToScenarioTypeMap.add(bid, typeMap);
    }
    // graphic buffer section
    for(ssize_t idx=0;idx<mBIDtoGraBufPoolMap_Scenario.size();++idx)
    {
        DepthMapBufferID bid = mBIDtoGraBufPoolMap_Scenario.keyAt(idx);
        ScenarioToGraBufPoolMap scenToBufMap = mBIDtoGraBufPoolMap_Scenario.valueAt(idx);
        ssize_t idx2;
        if((idx2=mBIDToScenarioTypeMap.indexOfKey(bid))>=0)
        {
            BufScenarioToTypeMap& bufScenMap = mBIDToScenarioTypeMap.editValueAt(idx2);
            for(ssize_t idx2=0;idx2<scenToBufMap.size();++idx2)
            {
                BufferPoolScenario sce = scenToBufMap.keyAt(idx2);
                bufScenMap.add(sce, eBUFFER_GRAPHIC);
            }
        }
        else
        {
            BufScenarioToTypeMap typeMap;
            for(ssize_t idx2=0;idx2<scenToBufMap.size();++idx2)
            {
                BufferPoolScenario sce = scenToBufMap.keyAt(idx2);
                typeMap.add(sce, eBUFFER_GRAPHIC);
            }
            mBIDToScenarioTypeMap.add(bid, typeMap);
        }
    }
    // tuning buffer section
    BufScenarioToTypeMap typeMap;
    typeMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, eBUFFER_TUNING);
    typeMap.add(eBUFFER_POOL_SCENARIO_RECORD, eBUFFER_TUNING);
    typeMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, eBUFFER_TUNING);
    mBIDToScenarioTypeMap.add(BID_P2A_TUNING, typeMap);
    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
getAllPoolImageBuffer(
    DepthMapBufferID id,
    BufferPoolScenario scen,
    std::vector<IImageBuffer*>& rImgVec
)
{
    MBOOL bRet = MFALSE;
    if(mBIDtoImgBufPoolMap_Default.indexOfKey(id) >= 0)
    {
        bRet = MTRUE;
        sp<FatImageBufferPool> pool = mBIDtoImgBufPoolMap_Default.valueFor(id);
        FatImageBufferPool::CONTAINER_TYPE poolContents = pool->getPoolContents();
        for(sp<FatImageBufferHandle>& handle : poolContents)
        {
            rImgVec.push_back(handle->mImageBuffer.get());
        }
    }

    if(mBIDtoImgBufPoolMap_Scenario.indexOfKey(id) >= 0)
    {
        bRet = MTRUE;
        ScenarioToImgBufPoolMap bufMap = mBIDtoImgBufPoolMap_Scenario.valueFor(id);
        if(bufMap.indexOfKey(scen)>=0)
        {
            sp<FatImageBufferPool> pool = bufMap.valueFor(scen);
            FatImageBufferPool::CONTAINER_TYPE poolContents = pool->getPoolContents();
            for(sp<FatImageBufferHandle>& handle : poolContents)
            {
                // if not exist
                if(std::find(rImgVec.begin(), rImgVec.end(), handle->mImageBuffer.get())==rImgVec.end())
                    rImgVec.push_back(handle->mImageBuffer.get());
            }
        }
    }

    if(mBIDtoGraBufPoolMap_Scenario.indexOfKey(id) >= 0)
    {
        bRet = MTRUE;
        ScenarioToGraBufPoolMap bufMap = mBIDtoGraBufPoolMap_Scenario.valueFor(id);
        if(bufMap.indexOfKey(scen)>=0)
        {
            sp<GraphicBufferPool> pool = bufMap.valueFor(scen);
            GraphicBufferPool::CONTAINER_TYPE poolContents = pool->getPoolContents();
            for(sp<GraphicBufferHandle>& handle : poolContents)
            {
                // if not exist
                if(std::find(rImgVec.begin(), rImgVec.end(), handle->mImageBuffer.get())==rImgVec.end())
                    rImgVec.push_back(handle->mImageBuffer.get());
            }
        }
    }

    return bRet;
}


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

