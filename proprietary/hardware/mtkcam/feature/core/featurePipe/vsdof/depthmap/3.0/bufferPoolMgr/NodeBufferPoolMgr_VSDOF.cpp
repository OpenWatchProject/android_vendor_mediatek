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
#include <mtkcam/drv/def/dpecommon.h>
#include <mtkcam/drv/iopipe/PostProc/DpeUtility.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/def/IPostProcDef.h>
// Module header file
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
// Local header file
#include "NodeBufferPoolMgr_VSDOF.h"
#include "NodeBufferHandler.h"
#include "../DepthMapPipe_Common.h"
#include "../DepthMapPipeNode.h"
#include "../DepthMapPipeUtils.h"
#include "./bufferSize/NodeBufferSizeMgr.h"
// Logging header file
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "NodeBufferPoolMgr_VSDOF"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSIoPipe;
using NSCam::NSIoPipe::NSPostProc::INormalStream;

#define SUPPORT_CAPTURE 0
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
    FatImageBufferPool::destroy(mpRectInBufPool_Main1_VR);
    FatImageBufferPool::destroy(mpRectInBufPool_Main1_PV);
    FatImageBufferPool::destroy(mpRectInBufPool_Main1_CAP);
    GraphicBufferPool::destroy(mpRectInBufPool_Main2_VR);
    GraphicBufferPool::destroy(mpRectInBufPool_Main2_PV);
    GraphicBufferPool::destroy(mpRectInBufPool_Main2_CAP);
    FatImageBufferPool::destroy(mpCCInBufPool_Main1);
    FatImageBufferPool::destroy(mpCCInBufPool_Main2);
    FatImageBufferPool::destroy(mpFDBufPool_CAP);
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
    FatImageBufferPool::destroy(mInternalFDImgBufPool);
    FatImageBufferPool::destroy(mIMG3OmgBufPool);
    //----------------------N3D section--------------------------------//
    FatImageBufferPool::destroy(mN3DImgBufPool_Main1);
    FatImageBufferPool::destroy(mN3DMaskBufPool_Main1);
    FatImageBufferPool::destroy(mN3DImgBufPool_Main1_CAP);
    FatImageBufferPool::destroy(mN3DMaskBufPool_Main1_CAP);
    GraphicBufferPool::destroy(mN3DImgBufPool_Main2);
    GraphicBufferPool::destroy(mN3DMaskBufPool_Main2);
    GraphicBufferPool::destroy(mN3DImgBufPool_Main2_CAP);
    GraphicBufferPool::destroy(mN3DMaskBufPool_Main2_CAP);
    //----------------------DPE section--------------------------------//
    FatImageBufferPool::destroy(mDMPBuffPool);
    FatImageBufferPool::destroy(mCFMBuffPool);
    FatImageBufferPool::destroy(mRespBuffPool);
    FatImageBufferPool::destroy(mDMPBuffPool_CAP);
    FatImageBufferPool::destroy(mCFMBuffPool_CAP);
    FatImageBufferPool::destroy(mRespBuffPool_CAP);
    //----------------------OCC section--------------------------------//
    FatImageBufferPool::destroy(mOCCNOCImgBufPool);
    FatImageBufferPool::destroy(mOCCNOCImgBufPool_CAP);
    FatImageBufferPool::destroy(mDSMVYImgBufPool_CAP);
    //----------------------WMF section--------------------------------//
    FatImageBufferPool::destroy(mpInternalDMWImgBufPool);
    FatImageBufferPool::destroy(mpInternalDMWImgBufPool_CAP);
    //----------------------GF section--------------------------------//
    FatImageBufferPool::destroy(mpInternalDMBGImgBufPool);
    FatImageBufferPool::destroy(mpInternalDepthImgBufPool);

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
            // config
            android::sp<NativeBufferWrapper> pNativBuffer = smGraBuf->mGraphicBuffer;
            // config graphic buffer to BT601_FULL
            gralloc_extra_ion_sf_info_t info;
            gralloc_extra_query(pNativBuffer->getHandle(), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
            gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
            gralloc_extra_perform(pNativBuffer->getHandle(), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
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
MBOOL
NodeBufferPoolMgr_VSDOF::
initializeBufferPool()
{
    CAM_TRACE_BEGIN("NodeBufferPoolMgr_VSDOF::initializeBufferPool");
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
                                &NodeBufferPoolMgr_VSDOF::initN3DBufferPool, this);
    // DPE buffer pool
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_DPE))
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initDPEBufferPool, this);
    // OCC buffer pool
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_OCC) ||
        mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_HWOCC))
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initOCCBufferPool, this);
    // WMF buffer pool
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_WMF))
        vFutures[index++] = std::async(
                                std::launch::async,
                                &NodeBufferPoolMgr_VSDOF::initWMFBufferPool, this);
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
     * Rectify_in/CC_in/FD/Tuning/FE/FM buffer pools
     **********************************************************************/
    // VR, Rect_in1 (main1)
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main1_VR, "RectInBufPool_Main1_VR",
                        rP2A_VR.mRECT_IN_SIZE_MAIN1,
                        eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
    // VR, Rect_in2 (main2)
    CREATE_GRABUF_POOL(mpRectInBufPool_Main2_VR, "RectInBufPool_Main2_VR",
                        rP2A_VR.mRECT_IN_SIZE_MAIN2,
                        HAL_PIXEL_FORMAT_YV12, GraphicBufferPool::USAGE_HW_TEXTURE);

    // PV, Rect_in1
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main1_PV, "RectInBufPool_Main1_PV",
                        rP2A_PV.mRECT_IN_SIZE_MAIN1,
                        eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
    // PV, Rect_in2
    CREATE_GRABUF_POOL(mpRectInBufPool_Main2_PV, "RectInPV_BufPool_Main2",
                        rP2A_PV.mRECT_IN_SIZE_MAIN2,
                        HAL_PIXEL_FORMAT_YV12, GraphicBufferPool::USAGE_HW_TEXTURE);
#if SUPPORT_CAPTURE
    // CAP, Rect_in1
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main1_CAP, "RectInBufPool_Main1_CAP",
                        rP2A_CAP.mRECT_IN_SIZE_MAIN1,
                        eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
    // Capture, Rect_in2
    CREATE_GRABUF_POOL(mpRectInBufPool_Main2_CAP, "RectInGra_BufPool",
                        rP2A_CAP.mRECT_IN_SIZE_MAIN2, HAL_PIXEL_FORMAT_YV12,
                        GraphicBufferPool::USAGE_HW_TEXTURE);
    // FD when Capture
    CREATE_IMGBUF_POOL(mpFDBufPool_CAP, "FDBufPool_CAP", rP2A_CAP.mFD_IMG_SIZE,
                        eImgFmt_YUY2, FatImageBufferPool::USAGE_HW, MTRUE);
#endif
    // CC_in
    CREATE_IMGBUF_POOL(mpCCInBufPool_Main1, "CCin_BufPool_Main1", rP2A_VR.mCCIN_SIZE_MAIN1,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mpCCInBufPool_Main2, "CCin_BufPool_Main2", rP2A_VR.mCCIN_SIZE_MAIN2,
                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
    // TuningBufferPool creation
    mpTuningBufferPool = TuningBufferPool::create("VSDOF_TUNING_P2A", INormalStream::getRegTableSize());
    mpPQTuningBufferPool = TuningBufferPool::create("PQTuningPool", sizeof(PQParam));
    mpDpPQParamTuningBufferPool = TuningBufferPool::create("PQTuningPool", sizeof(DpPqParam));

    // IMG3O bufers
    CREATE_IMGBUF_POOL(mIMG3OmgBufPool, "IMG3OmgBufPool", mpPipeSetting->mszRRZO_Main1,
                        eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
    // allcate buffer
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main1_VR, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_VR, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main1_PV, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_PV, VSDOF_WORKING_EXTRA_BUF_SET);
#if SUPPORT_CAPTURE
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main1_CAP, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_CAP, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpFDBufPool_CAP, VSDOF_WORKING_BUF_SET);
#endif
    ALLOCATE_BUFFER_POOL(mpCCInBufPool_Main1, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpCCInBufPool_Main2, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpTuningBufferPool, VSDOF_DEPTH_P2FRAME_SIZE + 2);
    ALLOCATE_BUFFER_POOL(mIMG3OmgBufPool, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpPQTuningBufferPool, VSDOF_DEPTH_P2FRAME_SIZE);
    ALLOCATE_BUFFER_POOL(mpDpPQParamTuningBufferPool, VSDOF_DEPTH_P2FRAME_SIZE);

    // init MY_S
    CREATE_IMGBUF_POOL(mMYSImgBufPool, "mMYSImgBufPool", rP2A_VR.mMYS_SIZE, eImgFmt_YUY2,
                        eBUFFER_USAGE_SW_READ_OFTEN|FatImageBufferPool::USAGE_HW, MTRUE);
    // MY_S : from P2ABayer to GF, (lifetime expand to 3 times.)
    ALLOCATE_BUFFER_POOL(mMYSImgBufPool, VSDOF_WORKING_BUF_SET*3);
#if SUPPORT_CAPTURE
    // init crop MV_Y for OCC CROP_MV_Y which is input for WMF
    CREATE_IMGBUF_POOL(mDSMVYImgBufPool_CAP, "mDSMVYImgBufPool_CAP", rP2A_CAP.mMYS_SIZE, eImgFmt_Y8,
                        eBUFFER_USAGE_SW_READ_OFTEN|FatImageBufferPool::USAGE_HW, MTRUE);
    ALLOCATE_BUFFER_POOL(mDSMVYImgBufPool_CAP, VSDOF_WORKING_BUF_SET);
#endif
    // init BID_P2A_OUT_INTERNAL_FD, same size with MY_S
    CREATE_IMGBUF_POOL(mInternalFDImgBufPool, "mInternalFDImgBufPool", rP2A_VR.mFD_IMG_SIZE, eImgFmt_YUY2,
                        eBUFFER_USAGE_SW_READ_OFTEN|FatImageBufferPool::USAGE_HW, MTRUE)
    ALLOCATE_BUFFER_POOL(mInternalFDImgBufPool, VSDOF_WORKING_BUF_SET*2);

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
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW, MTRUE);
    // query the FMO buffer size from FEO size
    queryFMOBufferSize(szFEOBufferSize, szFMOBufferSize);
    // create buffer pool
    CREATE_IMGBUF_POOL(mpFMOB_BufPool, "FMB_BufPool", szFMOBufferSize,
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW, MTRUE);

    // FE/FM buffer pool - stage C
    iBlockSize = StereoSettingProvider::fefmBlockSize(2);
    // query FEO/FMO size and create pool
    szFEBufSize = rP2aSize.mFEC_INPUT_SIZE_MAIN1;
    queryFEOBufferSize(szFEBufSize, iBlockSize, szFEOBufferSize);
    CREATE_IMGBUF_POOL(mpFEOC_BufPool, "FEC_BufPool", szFEOBufferSize,
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW, MTRUE);
    queryFMOBufferSize(szFEOBufferSize, szFMOBufferSize);
    CREATE_IMGBUF_POOL(mpFMOC_BufPool, "FMC_BufPool", szFMOBufferSize,
                        eImgFmt_STA_BYTE, FatImageBufferPool::USAGE_HW, MTRUE);

    // create the FE input buffer pool - stage B (the seocond FE input buffer)
    //FEB Main1 input
    CREATE_IMGBUF_POOL(mpFEBInBufPool_Main1, "FE1BInputBufPool", rP2aSize.mFEB_INPUT_SIZE_MAIN1,
                    eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);

    //FEB Main2 input
    CREATE_IMGBUF_POOL(mpFEBInBufPool_Main2, "FE2BInputBufPool", rP2aSize.mFEB_INPUT_SIZE_MAIN2,
                    eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);

    //FEC Main1 input
    CREATE_IMGBUF_POOL(mpFECInBufPool_Main1, "FE1CInputBufPool", rP2aSize.mFEC_INPUT_SIZE_MAIN1,
                    eImgFmt_YUY2, FatImageBufferPool::USAGE_HW, MTRUE);

    //FEC Main2 input
    CREATE_IMGBUF_POOL(mpFECInBufPool_Main2, "FE2CInputBufPool", rP2aSize.mFEC_INPUT_SIZE_MAIN2,
                    eImgFmt_YUY2, FatImageBufferPool::USAGE_HW, MTRUE);

    // FEO/FMO buffer pool- ALLOCATE buffers : Main1+Main2 -> two working set
    ALLOCATE_BUFFER_POOL(mpFEOB_BufPool, 2*VSDOF_WORKING_BUF_SET)
    ALLOCATE_BUFFER_POOL(mpFEOC_BufPool, 2*VSDOF_WORKING_BUF_SET)
    ALLOCATE_BUFFER_POOL(mpFMOB_BufPool, 2*VSDOF_WORKING_BUF_SET)
    ALLOCATE_BUFFER_POOL(mpFMOC_BufPool, 2*VSDOF_WORKING_BUF_SET)

    // FEB/FEC_Input buffer pool- ALLOCATE buffers : 2 (internal working buffer in Burst trigger)
    ALLOCATE_BUFFER_POOL(mpFEBInBufPool_Main1, 2)
    ALLOCATE_BUFFER_POOL(mpFEBInBufPool_Main2, 2)
    ALLOCATE_BUFFER_POOL(mpFECInBufPool_Main1, 2)
    ALLOCATE_BUFFER_POOL(mpFECInBufPool_Main2, 2)

    #ifdef GTEST
    // FE HW Input For UT
    CREATE_IMGBUF_POOL(mFEHWInput_StageB_Main1, "mFEHWInput_StageB_Main1", rP2aSize.mFEB_INPUT_SIZE_MAIN1, eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mFEHWInput_StageB_Main2, "mFEHWInput_StageB_Main2", rP2aSize.mFEBO_AREA_MAIN2.size, eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mFEHWInput_StageC_Main1, "mFEHWInput_StageC_Main1", rP2aSize.mFEC_INPUT_SIZE_MAIN1, eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mFEHWInput_StageC_Main2, "mFEHWInput_StageC_Main2", rP2aSize.mFECO_AREA_MAIN2.size, eImgFmt_YV12, FatImageBufferPool::USAGE_HW, MTRUE);
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
initN3DBufferPool()
{
    VSDOF_INIT_LOG("+");
    // n3d size : no difference between scenarios
    const N3DBufferSize& rN3DSize = mpBufferSizeMgr->getN3D(eSTEREO_SCENARIO_RECORD);
    const N3DBufferSize& rN3DSize_CAP = mpBufferSizeMgr->getN3D(eSTEREO_SCENARIO_CAPTURE);
    // SW read/write, hw read
    MUINT32 usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;

    // INIT N3D output buffer pool - for MV
    CREATE_IMGBUF_POOL(mN3DImgBufPool_Main1, "N3DImgBufPool", rN3DSize.mWARP_IMG_SIZE,
                        eImgFmt_YV12, usage, MTRUE);
    // SV
    CREATE_GRABUF_POOL(mN3DImgBufPool_Main2, "mN3DImgBufPool_Main2",
                        rN3DSize.mWARP_IMG_SIZE, HAL_PIXEL_FORMAT_YV12, GraphicBufferPool::USAGE_HW_TEXTURE);

    // INIT N3D output buffer pool - for MASK_M/MASK_Y
    CREATE_IMGBUF_POOL(mN3DMaskBufPool_Main1, "N3DMaskBufPool", rN3DSize.mWARP_MASK_SIZE,
                        eImgFmt_Y8, usage, MTRUE);
    CREATE_GRABUF_POOL(mN3DMaskBufPool_Main2, "N3DMaskBufPool_Main2", rN3DSize.mWARP_MASK_SIZE,
                        HAL_PIXEL_FORMAT_YV12, usage);
#if SUPPORT_CAPTURE
    // INIT N3D output buffer pool - for MV
    CREATE_IMGBUF_POOL(mN3DImgBufPool_Main1_CAP, "mN3DImgBufPool_Main1_CAP", rN3DSize_CAP.mWARP_IMG_SIZE,
                        eImgFmt_YV12, usage, MTRUE);
    CREATE_GRABUF_POOL(mN3DImgBufPool_Main2_CAP, "mN3DImgBufPool_Main2_CAP", rN3DSize_CAP.mWARP_IMG_SIZE,
                        HAL_PIXEL_FORMAT_YV12, usage);
    // INIT N3D output buffer pool - for MASK_M/MASK_Y
    CREATE_IMGBUF_POOL(mN3DMaskBufPool_Main1_CAP, "mN3DMaskBufPool_Main1_CAP", rN3DSize_CAP.mWARP_MASK_SIZE,
                        eImgFmt_Y8, usage, MTRUE);
    CREATE_GRABUF_POOL(mN3DMaskBufPool_Main2_CAP, "mN3DMaskBufPool_Main2_CAP", rN3DSize_CAP.mWARP_MASK_SIZE,
                        HAL_PIXEL_FORMAT_YV12, usage);
#endif
    // allocate
    ALLOCATE_BUFFER_POOL(mN3DImgBufPool_Main1, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mN3DImgBufPool_Main2, VSDOF_WORKING_EXTRA_BUF_SET);
    //
    ALLOCATE_BUFFER_POOL(mN3DMaskBufPool_Main1, VSDOF_WORKING_EXTRA_BUF_SET);
    ALLOCATE_BUFFER_POOL(mN3DMaskBufPool_Main2, VSDOF_WORKING_EXTRA_BUF_SET);
#if SUPPORT_CAPTURE
    ALLOCATE_BUFFER_POOL(mN3DImgBufPool_Main1_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mN3DImgBufPool_Main2_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mN3DMaskBufPool_Main1_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mN3DMaskBufPool_Main2_CAP, VSDOF_WORKING_BUF_SET);
#endif
    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initDPEBufferPool()
{
    VSDOF_INIT_LOG("+");
    #define BUFFRPOOL_EXTRA_SIZE_FOR_LAST_DMP 2
    // DPE size : no difference between scenarios
    const DPEBufferSize& rDPESize = mpBufferSizeMgr->getDPE(eSTEREO_SCENARIO_RECORD);
    const DPEBufferSize& rDPESize_CAP = mpBufferSizeMgr->getDPE(eSTEREO_SCENARIO_CAPTURE);
    // allocate with the stride size as the width
    CREATE_IMGBUF_POOL(mDMPBuffPool, "DMPBufPool", rDPESize.mDMP_SIZE,
                                        eImgFmt_Y16, FatImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mCFMBuffPool, "CFMBufPool", rDPESize.mCFM_SIZE,
                                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mRespBuffPool, "RESPOBufPool", rDPESize.mRESPO_SIZE,
                                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
#if SUPPORT_CAPTURE
    // CAP
    CREATE_IMGBUF_POOL(mDMPBuffPool_CAP, "DMPBufPool_CAP", rDPESize_CAP.mDMP_SIZE,
                                        eImgFmt_Y16, FatImageBufferPool::USAGE_HW|FatImageBufferPool::USAGE_SW, MTRUE);
    CREATE_IMGBUF_POOL(mCFMBuffPool_CAP, "CFMBufPool_CAP", rDPESize_CAP.mCFM_SIZE,
                                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mRespBuffPool_CAP, "RESPOBufPool_CAP", rDPESize_CAP.mRESPO_SIZE,
                                        eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
#endif
    //DMP, CFM, RESPO have all Left/Right side ->  double size
    ALLOCATE_BUFFER_POOL(mDMPBuffPool, VSDOF_WORKING_BUF_SET*2 + BUFFRPOOL_EXTRA_SIZE_FOR_LAST_DMP);
    ALLOCATE_BUFFER_POOL(mCFMBuffPool, VSDOF_WORKING_BUF_SET*2);
    ALLOCATE_BUFFER_POOL(mRespBuffPool, VSDOF_WORKING_BUF_SET*2);

#if SUPPORT_CAPTURE
    // Capture DMP: each run need  2*(BID_DPE_INTERNAL_LAST_DMP + BID_DPE_OUT_DMP_L + (DPE_RUN-1) * BID_DPE_INTERNAL_DMP )
    ALLOCATE_BUFFER_POOL(mDMPBuffPool_CAP, VSDOF_WORKING_BUF_SET*2*(2+StereoSettingProvider::getDPECaptureRound()-1));
    ALLOCATE_BUFFER_POOL(mCFMBuffPool_CAP, VSDOF_WORKING_BUF_SET*2);
    ALLOCATE_BUFFER_POOL(mRespBuffPool_CAP, VSDOF_WORKING_BUF_SET*2);
#endif

    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initOCCBufferPool()
{
    VSDOF_INIT_LOG("+");
    MUINT32 usage = eBUFFER_USAGE_SW_WRITE_OFTEN | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READ;
    // OCC size : no difference between scenarios
    const OCCBufferSize& rOCCSize = mpBufferSizeMgr->getOCC(eSTEREO_SCENARIO_RECORD);
    // OCC NOC
    CREATE_IMGBUF_POOL(mOCCNOCImgBufPool, "OCCNOCBufPool", rOCCSize.mOCCNOC_SIZE,
                                        eImgFmt_Y8, usage, MTRUE);
    ALLOCATE_BUFFER_POOL(mOCCNOCImgBufPool, VSDOF_WORKING_BUF_SET*2);   //OCC+NOC
#if SUPPORT_CAPTURE
    //Capture
    // OCC NOC
    const OCCBufferSize& rOCCSize_CAP = mpBufferSizeMgr->getOCC(eSTEREO_SCENARIO_CAPTURE);
    CREATE_IMGBUF_POOL(mOCCNOCImgBufPool_CAP, "OCCNOCBufPool_CAP", rOCCSize_CAP.mOCCNOC_SIZE,
                                        eImgFmt_Y8, usage, MTRUE);
    ALLOCATE_BUFFER_POOL(mOCCNOCImgBufPool_CAP, VSDOF_WORKING_BUF_SET*2);   //OCC+NOC
#endif
    VSDOF_INIT_LOG("-");

    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initWMFBufferPool()
{
    VSDOF_INIT_LOG("+");

    MSize szDMWSize = mpBufferSizeMgr->getWMF(eSTEREO_SCENARIO_RECORD).mDMW_SIZE;
    MSize szDMWSize_CAP = mpBufferSizeMgr->getWMF(eSTEREO_SCENARIO_CAPTURE).mDMW_SIZE;

    CREATE_IMGBUF_POOL(mpInternalDMWImgBufPool, "InternalDMWBufPool", szDMWSize
                        , eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
#if SUPPORT_CAPTURE
    CREATE_IMGBUF_POOL(mpInternalDMWImgBufPool_CAP, "InternalDMWImgBufPool_CAP", szDMWSize_CAP
                        , eImgFmt_Y8, FatImageBufferPool::USAGE_HW, MTRUE);
#endif
    // DMW + DMW_INTERNAL + HOLEFILL_INTERNAL -> 3 sets
    // When QUEUED_DEPTH flow type, need one extra buffer size for queued DMW
    if(mpPipeOption->mFlowType == eDEPTH_FLOW_TYPE_QUEUED_DEPTH)
    {
        ALLOCATE_BUFFER_POOL(mpInternalDMWImgBufPool, VSDOF_WORKING_BUF_SET * 4);
    }
    else
    {
        ALLOCATE_BUFFER_POOL(mpInternalDMWImgBufPool, VSDOF_WORKING_BUF_SET * 3);
    }
#if SUPPORT_CAPTURE
    ALLOCATE_BUFFER_POOL(mpInternalDMWImgBufPool_CAP, VSDOF_WORKING_BUF_SET * 3);
#endif

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
                        , eImgFmt_Y8, FatImageBufferPool::USAGE_SW, MTRUE);
    // extra one size means the one stored in the DepthStorge
    ALLOCATE_BUFFER_POOL(mpInternalDMBGImgBufPool, VSDOF_WORKING_BUF_SET + 1)

    if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH)
    {
        CREATE_IMGBUF_POOL(mpInternalDepthImgBufPool, "mpInternalDepthImgBufPool", szDepthMapSize
                            , eImgFmt_Y8, FatImageBufferPool::USAGE_SW, MTRUE);
        // extra one size means the one stored in the DepthStorge
        ALLOCATE_BUFFER_POOL(mpInternalDepthImgBufPool, VSDOF_WORKING_BUF_SET)
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
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_CC_IN1, mpCCInBufPool_Main1);
        mBIDtoImgBufPoolMap_Default.add(BID_P2A_OUT_CC_IN2, mpCCInBufPool_Main2);
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
        // FD
#if SUPPORT_CAPTURE
        ScenarioToImgBufPoolMap FDImgBufMap;
        FDImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpFDBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_FDIMG, FDImgBufMap);
#endif
        // Rectify Main1 : BID_P2A_OUT_RECT_IN1
        ScenarioToImgBufPoolMap RectInImgBufMap_Main1;
        RectInImgBufMap_Main1.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpRectInBufPool_Main1_PV);
        RectInImgBufMap_Main1.add(eBUFFER_POOL_SCENARIO_RECORD, mpRectInBufPool_Main1_VR);
#if SUPPORT_CAPTURE
        RectInImgBufMap_Main1.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpRectInBufPool_Main1_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_RECT_IN1, RectInImgBufMap_Main1);
        // MYS
        ScenarioToImgBufPoolMap MYSImgBufMap;
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mMYSImgBufPool);
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mMYSImgBufPool);
#if SUPPORT_CAPTURE
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDSMVYImgBufPool_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_MY_S, MYSImgBufMap);
        // INTERNAL_FD
        ScenarioToImgBufPoolMap InternalFDImgBufMap;
        InternalFDImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mInternalFDImgBufPool);
        InternalFDImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mInternalFDImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_INTERNAL_FD, InternalFDImgBufMap);
        // IMG3O
        ScenarioToImgBufPoolMap IMG3OImgBufMap;
        IMG3OImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mIMG3OmgBufPool);
        IMG3OImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mIMG3OmgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_INTERNAL_IMG3O, IMG3OImgBufMap);

        // ============ graphic buffer pool mapping =============
        // Rectify Main2 : BID_P2A_OUT_RECT_IN2
        ScenarioToGraBufPoolMap RectInGraBufMap_Main2;
        RectInGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpRectInBufPool_Main2_PV);
        RectInGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_RECORD, mpRectInBufPool_Main2_VR);
#if SUPPORT_CAPTURE
        RectInGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpRectInBufPool_Main2_CAP);
#endif
        mBIDtoGraBufPoolMap_Scenario.add(BID_P2A_OUT_RECT_IN2, RectInGraBufMap_Main2);
    }
    // N3D
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_N3D))
    {
        // MV_Y
        ScenarioToImgBufPoolMap MVY_ImgBufMap;
        MVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DImgBufPool_Main1);
        MVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DImgBufPool_Main1);
#if SUPPORT_CAPTURE
        MVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DImgBufPool_Main1_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_MV_Y, MVY_ImgBufMap);
        // MASK_M
        ScenarioToImgBufPoolMap MASKImgBufMap;
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DMaskBufPool_Main1);
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DMaskBufPool_Main1);
#if SUPPORT_CAPTURE
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DMaskBufPool_Main1_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_MASK_M, MASKImgBufMap);
        // ============ graphic buffer pool mapping =============
        // SV_Y
        ScenarioToGraBufPoolMap WarpGraBufMap_Main2;
        WarpGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DImgBufPool_Main2);
        WarpGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DImgBufPool_Main2);
#if SUPPORT_CAPTURE
        WarpGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DImgBufPool_Main2_CAP);
#endif
        mBIDtoGraBufPoolMap_Scenario.add(BID_N3D_OUT_SV_Y, WarpGraBufMap_Main2);
        // MASK_S
        ScenarioToGraBufPoolMap MaskGraBufMap_Main2;
        MaskGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DMaskBufPool_Main2);
        MaskGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DMaskBufPool_Main2);
#if SUPPORT_CAPTURE
        MaskGraBufMap_Main2.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DMaskBufPool_Main2_CAP);
#endif
        mBIDtoGraBufPoolMap_Scenario.add(BID_N3D_OUT_MASK_S, MaskGraBufMap_Main2);
    }
    //DPE Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_DPE))
    {
        ScenarioToImgBufPoolMap DMPImgBufMap;
        DMPImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mDMPBuffPool);
        DMPImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mDMPBuffPool);
#if SUPPORT_CAPTURE
        DMPImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDMPBuffPool_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_DMP_L, DMPImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_DMP_R, DMPImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_INTERNAL_LAST_DMP, DMPImgBufMap);
#if SUPPORT_CAPTURE
        ScenarioToImgBufPoolMap DMPInternalImgBufMap;
        DMPInternalImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDMPBuffPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_INTERNAL_DMP, DMPInternalImgBufMap);
#endif
        ScenarioToImgBufPoolMap CFMImgBufMap;
        CFMImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mCFMBuffPool);
        CFMImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mCFMBuffPool);
#if SUPPORT_CAPTURE
        CFMImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mCFMBuffPool_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_CFM_R, CFMImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_CFM_L, CFMImgBufMap);

        ScenarioToImgBufPoolMap RESPImgBufMap;
        RESPImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mRespBuffPool);
        RESPImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mRespBuffPool);
#if SUPPORT_CAPTURE
        RESPImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mRespBuffPool_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_RESPO_L, RESPImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_RESPO_R, RESPImgBufMap);
    }
    // OCC Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_OCC) ||
        mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_HWOCC))
    {
        ScenarioToImgBufPoolMap OCCNOCImgBufMap;
        OCCNOCImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mOCCNOCImgBufPool);
        OCCNOCImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD,  mOCCNOCImgBufPool);
#if SUPPORT_CAPTURE
        OCCNOCImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mOCCNOCImgBufPool_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_OCC, OCCNOCImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_NOC, OCCNOCImgBufMap);
        // CROP MV_Y
#if SUPPORT_CAPTURE
        ScenarioToImgBufPoolMap DSMVYImgBufMap;
        DSMVYImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDSMVYImgBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_DS_MVY, DSMVYImgBufMap);
#endif
    }
    // WMF Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_WMF))
    {
        ScenarioToImgBufPoolMap DMWImgBufMap;
        DMWImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpInternalDMWImgBufPool);
        DMWImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mpInternalDMWImgBufPool);
#if SUPPORT_CAPTURE
        DMWImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpInternalDMWImgBufPool_CAP);
#endif
        mBIDtoImgBufPoolMap_Scenario.add(BID_WMF_HOLEFILL_INTERNAL, DMWImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WMF_DMW_INTERNAL, DMWImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WMF_OUT_DMW, DMWImgBufMap);
    }
    // GF Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_GF))
    {
        ScenarioToImgBufPoolMap DMBGImgBufMap;
        DMBGImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpInternalDMBGImgBufPool);
        DMBGImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mpInternalDMBGImgBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_GF_INTERNAL_DMBG, DMBGImgBufMap);

        if(mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_DEPTH ||
            mpPipeOption->mFeatureMode == eDEPTHNODE_MODE_MTK_UNPROCESS_DEPTH)
        {
            ScenarioToImgBufPoolMap DepthImgBufMap;
            DepthImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpInternalDepthImgBufPool);
            DepthImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mpInternalDepthImgBufPool);
            mBIDtoImgBufPoolMap_Scenario.add(BID_GF_INTERNAL_DEPTHMAP, DepthImgBufMap);
        }
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

