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
// Android system/core header file
#include <ui/gralloc_extra.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/drv/def/dpecommon.h>
#include <mtkcam/drv/iopipe/PostProc/DpeUtility.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
// Module header file
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
// Local header file
#include "NodeBufferPoolMgr_VSDOF.h"
#include "NodeBufferHandler.h"
#include "../DepthMapPipe_Common.h"
#include "../bufferConfig/BufferConfig_VSDOF.h"
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
/*******************************************************************************
* Global Define
********************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NodeBufferPoolMgr_VSDOF class - Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NodeBufferPoolMgr_VSDOF::
NodeBufferPoolMgr_VSDOF(
    PipeNodeBitSet& nodeBitSet,
    sp<DepthMapPipeOption> pPipeOption
)
: mNodeBitSet(nodeBitSet)
, mpPipeOption(pPipeOption)
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
    this->buildRequestBufferMap();
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
    ImageBufferPool::destroy(mpRectInBufPool_Main1_VR);
    ImageBufferPool::destroy(mpRectInBufPool_Main2_VR);
    ImageBufferPool::destroy(mpRectInBufPool_Main1_PV);
    ImageBufferPool::destroy(mpRectInBufPool_Main2_PV);
    ImageBufferPool::destroy(mpRectInBufPool_Main1_CAP);
    ImageBufferPool::destroy(mpCCInBufPool_Main1);
    ImageBufferPool::destroy(mpCCInBufPool_Main2);
    ImageBufferPool::destroy(mpFDBufPool_CAP);
    ImageBufferPool::destroy(mpFEOB_BufPool);
    ImageBufferPool::destroy(mpFEOC_BufPool);
    ImageBufferPool::destroy(mpFMOB_BufPool);
    ImageBufferPool::destroy(mpFMOC_BufPool);
    ImageBufferPool::destroy(mpFEBInBufPool_Main1);
    ImageBufferPool::destroy(mpFEBInBufPool_Main2);
    ImageBufferPool::destroy(mpFECInBufPool_Main1);
    ImageBufferPool::destroy(mpFECInBufPool_Main2);
    GraphicBufferPool::destroy(mpRectInBufPool_Main2_CAP);
    TuningBufferPool::destroy(mpTuningBufferPool);
    #ifdef GTEST
    ImageBufferPool::destroy(mFEHWInput_StageB_Main1);
    ImageBufferPool::destroy(mFEHWInput_StageB_Main2);
    ImageBufferPool::destroy(mFEHWInput_StageC_Main1);
    ImageBufferPool::destroy(mFEHWInput_StageC_Main2);
    #endif
    ImageBufferPool::destroy(mMYSImgBufPool);
    ImageBufferPool::destroy(mDSMVYImgBufPool_CAP);
    //----------------------N3D section--------------------------------//
    ImageBufferPool::destroy(mN3DImgBufPool);
    ImageBufferPool::destroy(mN3DMaskBufPool);
    ImageBufferPool::destroy(mN3DImgBufPool_CAP);
    ImageBufferPool::destroy(mN3DMaskBufPool_CAP);
    ImageBufferPool::destroy(mLDCBufPool);
    //----------------------DPE section--------------------------------//
    ImageBufferPool::destroy(mDMPBuffPool);
    ImageBufferPool::destroy(mCFMBuffPool);
    ImageBufferPool::destroy(mRespBuffPool);
    ImageBufferPool::destroy(mDMPBuffPool_CAP);
    ImageBufferPool::destroy(mCFMBuffPool_CAP);
    ImageBufferPool::destroy(mRespBuffPool_CAP);
    //----------------------OCC section--------------------------------//
    ImageBufferPool::destroy(mOCCNOCImgBufPool);
    ImageBufferPool::destroy(mOCCNOCImgBufPool_CAP);
    //----------------------WMF section--------------------------------//
    ImageBufferPool::destroy(mpInternalDMWImgBufPool);
    ImageBufferPool::destroy(mpInternalDMWImgBufPool_CAP);
    //----------------------GF section--------------------------------//
    if(mpBufferSizeMgr != nullptr)
        delete mpBufferSizeMgr;

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
SmartImageBuffer
NodeBufferPoolMgr_VSDOF::
request(DepthMapBufferID id, BufferPoolScenario scen)
{
    ssize_t index;
    if((index=mBIDtoImgBufPoolMap_Default.indexOfKey(id)) >= 0)
    {
        sp<ImageBufferPool> pBufferPool = mBIDtoImgBufPoolMap_Default.valueAt(index);
        return pBufferPool->request();
    }
    else
    {
        ScenarioToImgBufPoolMap ScenarioBufMap = mBIDtoImgBufPoolMap_Scenario.valueFor(id);
        if((index=ScenarioBufMap.indexOfKey(scen))>=0)
        {
            sp<ImageBufferPool> pBufferPool = ScenarioBufMap.valueAt(index);
            return pBufferPool->request();
        }
        return NULL;
    }
}

SmartGraphicBuffer
NodeBufferPoolMgr_VSDOF::
requestGB(DepthMapBufferID id, BufferPoolScenario scen)
{
    if(scen == eBUFFER_POOL_SCENARIO_CAPTURE &&
            id == BID_P2A_OUT_RECT_IN2)
    {
        SmartGraphicBuffer smGraBuf = mpRectInBufPool_Main2_CAP->request();
        // config
        android::sp<NativeBufferWrapper> pRectInGraBuf = smGraBuf->mGraphicBuffer;
        // config graphic buffer to BT601_FULL
        gralloc_extra_ion_sf_info_t info;
        gralloc_extra_query(pRectInGraBuf->getHandle(), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
        gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
        gralloc_extra_perform(pRectInGraBuf->getHandle(), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
        return smGraBuf;
    }
    else
    {
        return NULL;
    }
}

SmartTuningBuffer
NodeBufferPoolMgr_VSDOF::
requestTB(DepthMapBufferID id, BufferPoolScenario scen)
{
    if(id == BID_P2A_TUNING || id == BID_P2A_OUT_TUNING_MAIN1)
    {
        SmartTuningBuffer smTuningBuf = mpTuningBufferPool->request();
        memset(smTuningBuf->mpVA, 0, mpTuningBufferPool->getBufSize());
        return smTuningBuf;
    }
    else
    {
        MY_LOGE("Cannot find the TuningBufferPool with scenario:%d of buffer id:%d!!", scen, id);
            return NULL;
    }
}

BufferPoolHandlerPtr
NodeBufferPoolMgr_VSDOF::
createBufferPoolHandler()
{
    BaseBufferHandler* pPtr = new NodeBufferHandler(this, mRequestBufferIDMap, mRequestMetaIDMap);
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
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_P2AFM))
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
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_OCC))
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
    const P2ABufferSize& rP2AFM_VR=mpBufferSizeMgr->getP2A(eSTEREO_SCENARIO_RECORD);
    const P2ABufferSize& rP2AFM_PV=mpBufferSizeMgr->getP2A(eSTEREO_SCENARIO_PREVIEW);
    const P2ABufferSize& rP2AFM_CAP=mpBufferSizeMgr->getP2A(eSTEREO_SCENARIO_CAPTURE);

    /**********************************************************************
     * Rectify_in/CC_in/FD/Tuning/FE/FM buffer pools
     **********************************************************************/
    // VR, Rect_in1 (main1)
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main1_VR, "RectInBufPool_Main1_VR",
                        rP2AFM_VR.mRECT_IN_SIZE_MAIN1,
                        eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);
    // VR, Rect_in2 (main2)
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main2_VR, "RectInBufPool_Main2_VR",
                        rP2AFM_VR.mRECT_IN_SIZE_MAIN2,
                        eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);

    // PV, Rect_in1
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main1_PV, "RectInBufPool_Main1_PV",
                        rP2AFM_PV.mRECT_IN_SIZE_MAIN1,
                        eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);
    // PV, Rect_in2
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main2_PV, "RectInPV_BufPool_Main2",
                        rP2AFM_PV.mRECT_IN_SIZE_MAIN2,
                        eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);

    // CAP, Rect_in1
    CREATE_IMGBUF_POOL(mpRectInBufPool_Main1_CAP, "RectInBufPool_Main1_CAP",
                        rP2AFM_CAP.mRECT_IN_SIZE_MAIN1,
                        eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);
    // Capture, Rect_in2
    CREATE_GRABUF_POOL(mpRectInBufPool_Main2_CAP, "RectInGra_BufPool",
                        rP2AFM_CAP.mRECT_IN_SIZE_MAIN2, HAL_PIXEL_FORMAT_YV12,
                        GraphicBufferPool::USAGE_HW_TEXTURE);

    // CC_in
    CREATE_IMGBUF_POOL(mpCCInBufPool_Main1, "CCin_BufPool_Main1", rP2AFM_VR.mCCIN_SIZE_MAIN1,
                        eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mpCCInBufPool_Main2, "CCin_BufPool_Main2", rP2AFM_VR.mCCIN_SIZE_MAIN2,
                        eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);
    // FD when Capture
    CREATE_IMGBUF_POOL(mpFDBufPool_CAP, "FDBufPool_CAP", rP2AFM_CAP.mFD_IMG_SIZE,
                        eImgFmt_YUY2, ImageBufferPool::USAGE_HW, MTRUE);
    // TuningBufferPool creation
    mpTuningBufferPool = TuningBufferPool::create("VSDOF_TUNING_P2A", INormalStream::getRegTableSize());

    // allcate buffer
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main1_VR, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_VR, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main1_PV, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_PV, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main1_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpRectInBufPool_Main2_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpCCInBufPool_Main1, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpCCInBufPool_Main2, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpFDBufPool_CAP, VSDOF_WORKING_BUF_SET);
    ALLOCATE_BUFFER_POOL(mpTuningBufferPool, VSDOF_DEPTH_P2FRAME_SIZE + 2);

    // init MY_S
    CREATE_IMGBUF_POOL(mMYSImgBufPool, "mMYSImgBufPool", rP2AFM_VR.mMYS_SIZE, eImgFmt_YV12,
                        eBUFFER_USAGE_SW_READ_OFTEN|ImageBufferPool::USAGE_HW, MTRUE);
    // init crop MV_Y for OCC CROP_MV_Y which is input for WMF
    CREATE_IMGBUF_POOL(mDSMVYImgBufPool_CAP, "mDSMVYImgBufPool_CAP", rP2AFM_CAP.mMYS_SIZE, eImgFmt_Y8,
                        eBUFFER_USAGE_SW_READ_OFTEN|ImageBufferPool::USAGE_HW, MTRUE);

    // MY_S : from P2ABayer to OCC, (lifetime expand to two times.)
    ALLOCATE_BUFFER_POOL(mMYSImgBufPool, VSDOF_WORKING_BUF_SET*2);
    ALLOCATE_BUFFER_POOL(mDSMVYImgBufPool_CAP, VSDOF_WORKING_BUF_SET);

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
                        eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW, MTRUE);
    // query the FMO buffer size from FEO size
    queryFMOBufferSize(szFEOBufferSize, szFMOBufferSize);
    // create buffer pool
    CREATE_IMGBUF_POOL(mpFMOB_BufPool, "FMB_BufPool", szFMOBufferSize,
                        eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW, MTRUE);

    // FE/FM buffer pool - stage C
    iBlockSize = StereoSettingProvider::fefmBlockSize(2);
    // query FEO/FMO size and create pool
    szFEBufSize = rP2aSize.mFEC_INPUT_SIZE_MAIN1;
    queryFEOBufferSize(szFEBufSize, iBlockSize, szFEOBufferSize);
    CREATE_IMGBUF_POOL(mpFEOC_BufPool, "FEC_BufPool", szFEOBufferSize,
                        eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW, MTRUE);
    queryFMOBufferSize(szFEOBufferSize, szFMOBufferSize);
    CREATE_IMGBUF_POOL(mpFMOC_BufPool, "FMC_BufPool", szFMOBufferSize,
                        eImgFmt_STA_BYTE, ImageBufferPool::USAGE_HW, MTRUE);

    // create the FE input buffer pool - stage B (the seocond FE input buffer)
    //FEB Main1 input
    CREATE_IMGBUF_POOL(mpFEBInBufPool_Main1, "FE1BInputBufPool", rP2aSize.mFEB_INPUT_SIZE_MAIN1,
                    eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);

    //FEB Main2 input
    CREATE_IMGBUF_POOL(mpFEBInBufPool_Main2, "FE2BInputBufPool", rP2aSize.mFEB_INPUT_SIZE_MAIN2,
                    eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);

    //FEC Main1 input
    CREATE_IMGBUF_POOL(mpFECInBufPool_Main1, "FE1CInputBufPool", rP2aSize.mFEC_INPUT_SIZE_MAIN1,
                    eImgFmt_YUY2, ImageBufferPool::USAGE_HW, MTRUE);

    //FEC Main2 input
    CREATE_IMGBUF_POOL(mpFECInBufPool_Main2, "FE2CInputBufPool", rP2aSize.mFEC_INPUT_SIZE_MAIN2,
                    eImgFmt_YUY2, ImageBufferPool::USAGE_HW, MTRUE);

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
    CREATE_IMGBUF_POOL(mFEHWInput_StageB_Main1, "mFEHWInput_StageB_Main1", rP2aSize.mFEB_INPUT_SIZE_MAIN1, eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mFEHWInput_StageB_Main2, "mFEHWInput_StageB_Main2", rP2aSize.mFEBO_AREA_MAIN2.size, eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mFEHWInput_StageC_Main1, "mFEHWInput_StageC_Main1", rP2aSize.mFEC_INPUT_SIZE_MAIN1, eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mFEHWInput_StageC_Main2, "mFEHWInput_StageC_Main2", rP2aSize.mFECO_AREA_MAIN2.size, eImgFmt_YV12, ImageBufferPool::USAGE_HW, MTRUE);
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

    // INIT N3D output buffer pool - for MV/SV
    CREATE_IMGBUF_POOL(mN3DImgBufPool, "N3DImgBufPool", rN3DSize.mWARP_IMG_SIZE,
                        eImgFmt_YV12, usage, MTRUE);
    // INIT N3D output buffer pool - for MASK_M/MASK_Y
    CREATE_IMGBUF_POOL(mN3DMaskBufPool, "N3DMaskBufPool", rN3DSize.mWARP_MASK_SIZE,
                        eImgFmt_Y8, usage, MTRUE);
    // INIT N3D output buffer - for LDC
    CREATE_IMGBUF_POOL(mLDCBufPool, "LDCBufPool", rN3DSize.mLDC_SIZE,
                        eImgFmt_Y8, ImageBufferPool::USAGE_SW, MTRUE);

    // INIT N3D output buffer pool - for MV/SV
    CREATE_IMGBUF_POOL(mN3DImgBufPool_CAP, "mN3DImgBufPool_CAP", rN3DSize_CAP.mWARP_IMG_SIZE,
                        eImgFmt_YV12, usage, MTRUE);
    // INIT N3D output buffer pool - for MASK_M/MASK_Y
    CREATE_IMGBUF_POOL(mN3DMaskBufPool_CAP, "mN3DMaskBufPool_CAP", rN3DSize_CAP.mWARP_MASK_SIZE,
                        eImgFmt_Y8, usage, MTRUE);

    // for MV+SV -> double size
    ALLOCATE_BUFFER_POOL(mN3DImgBufPool, VSDOF_WORKING_BUF_SET*2);
    ALLOCATE_BUFFER_POOL(mN3DImgBufPool_CAP, VSDOF_WORKING_BUF_SET*2);
    // for MASK_M+MASK_Y -> double size
    ALLOCATE_BUFFER_POOL(mN3DMaskBufPool, VSDOF_WORKING_BUF_SET*2);
    ALLOCATE_BUFFER_POOL(mN3DMaskBufPool_CAP, VSDOF_WORKING_BUF_SET*2);
    // for LDC - double size for LDC need to transfer 2 nodes
    ALLOCATE_BUFFER_POOL(mLDCBufPool, VSDOF_WORKING_BUF_SET*2);
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
                                        eImgFmt_Y16, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mCFMBuffPool, "CFMBufPool", rDPESize.mCFM_SIZE,
                                        eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mRespBuffPool, "RESPOBufPool", rDPESize.mRESPO_SIZE,
                                        eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);
    // CAP
    CREATE_IMGBUF_POOL(mDMPBuffPool_CAP, "DMPBufPool_CAP", rDPESize_CAP.mDMP_SIZE,
                                        eImgFmt_Y16, ImageBufferPool::USAGE_HW|ImageBufferPool::USAGE_SW, MTRUE);
    CREATE_IMGBUF_POOL(mCFMBuffPool_CAP, "CFMBufPool_CAP", rDPESize_CAP.mCFM_SIZE,
                                        eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mRespBuffPool_CAP, "RESPOBufPool_CAP", rDPESize_CAP.mRESPO_SIZE,
                                        eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);

    //DMP, CFM, RESPO have all Left/Right side ->  double size
    ALLOCATE_BUFFER_POOL(mDMPBuffPool, VSDOF_WORKING_BUF_SET*2 + BUFFRPOOL_EXTRA_SIZE_FOR_LAST_DMP);
    ALLOCATE_BUFFER_POOL(mCFMBuffPool, VSDOF_WORKING_BUF_SET*2);
    ALLOCATE_BUFFER_POOL(mRespBuffPool, VSDOF_WORKING_BUF_SET*2);
    // Capture DMP: each run need  2*(BID_DPE_INTERNAL_LAST_DMP + BID_DPE_OUT_DMP_L + (DPE_RUN-1) * BID_DPE_INTERNAL_DMP )
    ALLOCATE_BUFFER_POOL(mDMPBuffPool_CAP, VSDOF_WORKING_BUF_SET*2*(2+StereoSettingProvider::getDPECaptureRound()-1));
    ALLOCATE_BUFFER_POOL(mCFMBuffPool_CAP, VSDOF_WORKING_BUF_SET*2);
    ALLOCATE_BUFFER_POOL(mRespBuffPool_CAP, VSDOF_WORKING_BUF_SET*2);

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

    //Capture
    // OCC NOC
    const OCCBufferSize& rOCCSize_CAP = mpBufferSizeMgr->getOCC(eSTEREO_SCENARIO_CAPTURE);
    CREATE_IMGBUF_POOL(mOCCNOCImgBufPool_CAP, "OCCNOCBufPool_CAP", rOCCSize_CAP.mOCCNOC_SIZE,
                                        eImgFmt_Y8, usage, MTRUE);
    ALLOCATE_BUFFER_POOL(mOCCNOCImgBufPool_CAP, VSDOF_WORKING_BUF_SET*2);   //OCC+NOC
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
                        , eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);
    CREATE_IMGBUF_POOL(mpInternalDMWImgBufPool_CAP, "InternalDMWImgBufPool_CAP", szDMWSize_CAP
                        , eImgFmt_Y8, ImageBufferPool::USAGE_HW, MTRUE);

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
    ALLOCATE_BUFFER_POOL(mpInternalDMWImgBufPool_CAP, VSDOF_WORKING_BUF_SET * 3);

    VSDOF_INIT_LOG("-");
    return MTRUE;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
initGFBufferPool()
{
    VSDOF_INIT_LOG("+");
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
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_P2AFM))
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
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_P2AFM))
    {
        // FD
        ScenarioToImgBufPoolMap FDImgBufMap;
        FDImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpFDBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_FDIMG, FDImgBufMap);
        // Rectify Main1 : BID_P2A_OUT_RECT_IN1
        ScenarioToImgBufPoolMap RectInImgBufMap_Main1;
        RectInImgBufMap_Main1.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpRectInBufPool_Main1_PV);
        RectInImgBufMap_Main1.add(eBUFFER_POOL_SCENARIO_RECORD, mpRectInBufPool_Main1_VR);
        RectInImgBufMap_Main1.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpRectInBufPool_Main1_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_RECT_IN1, RectInImgBufMap_Main1);
        // Rectify Main2 : BID_P2A_OUT_RECT_IN2
        ScenarioToImgBufPoolMap RectInImgBufMap_Main2;
        RectInImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpRectInBufPool_Main2_PV);
        RectInImgBufMap_Main2.add(eBUFFER_POOL_SCENARIO_RECORD, mpRectInBufPool_Main2_VR);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_RECT_IN2, RectInImgBufMap_Main2);
        // MYS
        ScenarioToImgBufPoolMap MYSImgBufMap;
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mMYSImgBufPool);
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mMYSImgBufPool);
        MYSImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDSMVYImgBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_P2A_OUT_MY_S, MYSImgBufMap);
    }
    // N3D
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_N3D))
    {
        // LDC
        ScenarioToImgBufPoolMap LDCImgBufMap;
        LDCImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mLDCBufPool);
        LDCImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mLDCBufPool);
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_LDC, LDCImgBufMap);
        // MV_Y/SV_Y
        ScenarioToImgBufPoolMap MVY_SVY_ImgBufMap;
        MVY_SVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DImgBufPool);
        MVY_SVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DImgBufPool);
        MVY_SVY_ImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DImgBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_MV_Y, MVY_SVY_ImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_SV_Y, MVY_SVY_ImgBufMap);
        // MASK_M/S
        ScenarioToImgBufPoolMap MASKImgBufMap;
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mN3DMaskBufPool);
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mN3DMaskBufPool);
        MASKImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mN3DMaskBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_MASK_M, MASKImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_N3D_OUT_MASK_S, MASKImgBufMap);
    }
    //DPE Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_DPE))
    {
        ScenarioToImgBufPoolMap DMPImgBufMap;
        DMPImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mDMPBuffPool);
        DMPImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mDMPBuffPool);
        DMPImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDMPBuffPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_DMP_L, DMPImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_DMP_R, DMPImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_INTERNAL_LAST_DMP, DMPImgBufMap);

        ScenarioToImgBufPoolMap DMPInternalImgBufMap;
        DMPInternalImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDMPBuffPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_INTERNAL_DMP, DMPInternalImgBufMap);

        ScenarioToImgBufPoolMap CFMImgBufMap;
        CFMImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mCFMBuffPool);
        CFMImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mCFMBuffPool);
        CFMImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mCFMBuffPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_CFM_R, CFMImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_CFM_L, CFMImgBufMap);

        ScenarioToImgBufPoolMap RESPImgBufMap;
        RESPImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mRespBuffPool);
        RESPImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mRespBuffPool);
        RESPImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mRespBuffPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_RESPO_L, RESPImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_DPE_OUT_RESPO_R, RESPImgBufMap);
    }
    // OCC Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_OCC))
    {
        ScenarioToImgBufPoolMap OCCNOCImgBufMap;
        OCCNOCImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mOCCNOCImgBufPool);
        OCCNOCImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD,  mOCCNOCImgBufPool);
        OCCNOCImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mOCCNOCImgBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_OCC, OCCNOCImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_NOC, OCCNOCImgBufMap);
        // CROP MV_Y
        ScenarioToImgBufPoolMap DSMVYImgBufMap;
        DSMVYImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mDSMVYImgBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_OCC_OUT_DS_MVY, DSMVYImgBufMap);
    }
    // WMF Part
    if(mNodeBitSet.test(eDPETHMAP_PIPE_NODEID_WMF))
    {
        ScenarioToImgBufPoolMap DMWImgBufMap;
        DMWImgBufMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, mpInternalDMWImgBufPool);
        DMWImgBufMap.add(eBUFFER_POOL_SCENARIO_RECORD, mpInternalDMWImgBufPool);
        DMWImgBufMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, mpInternalDMWImgBufPool_CAP);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WMF_HOLEFILL_INTERNAL, DMWImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WMF_DMW_INTERNAL, DMWImgBufMap);
        mBIDtoImgBufPoolMap_Scenario.add(BID_WMF_OUT_DMW, DMWImgBufMap);
    }

    MY_LOGD("-");
    return MTRUE;
}


sp<ImageBufferPool>
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

    return NULL;
}

MBOOL
NodeBufferPoolMgr_VSDOF::
buildRequestBufferMap()
{
    MY_LOGD("+");
    // build the IO type map for each buffer of each scenario
    BufferIDMap pvBufferMap, vrBufferMap, capBufferMap;
    // preview
    const NodeBufferSetting* pConfig = preview_buffer_config;
    while(pConfig->bufferID != BID_INVALID)
    {
        pvBufferMap.add(pConfig->bufferID, pConfig->ioType);
        pConfig++;
    }
    // record
    pConfig = record_buffer_config;
    while(pConfig->bufferID != BID_INVALID)
    {
        vrBufferMap.add(pConfig->bufferID, pConfig->ioType);
        pConfig++;
    }
    // capture
    pConfig = capture_buffer_config;
    while(pConfig->bufferID != BID_INVALID)
    {
        capBufferMap.add(pConfig->bufferID, pConfig->ioType);
        pConfig++;
    }

    mRequestBufferIDMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, pvBufferMap);
    mRequestBufferIDMap.add(eBUFFER_POOL_SCENARIO_RECORD, vrBufferMap);
    mRequestBufferIDMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, capBufferMap);

    // meta has no scenario difference
    BufferIDMap pvMetaBufferMap, vrMetaBufferMap, capMetaBufferMap;
    pConfig =metadata_config;
    while(pConfig->bufferID != BID_INVALID)
    {
        pvMetaBufferMap.add(pConfig->bufferID, pConfig->ioType);
        vrMetaBufferMap.add(pConfig->bufferID, pConfig->ioType);
        capMetaBufferMap.add(pConfig->bufferID, pConfig->ioType);
        pConfig++;
    }

    mRequestMetaIDMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, pvMetaBufferMap);
    mRequestMetaIDMap.add(eBUFFER_POOL_SCENARIO_RECORD, vrMetaBufferMap);
    mRequestMetaIDMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, capMetaBufferMap);

    MY_LOGD("-");
    return MTRUE;
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
    // Rect_in2 in capture
    ssize_t idx;
    if((idx=mBIDToScenarioTypeMap.indexOfKey(BID_P2A_OUT_RECT_IN2))>=0)
    {
        BufScenarioToTypeMap& bufScenMap = mBIDToScenarioTypeMap.editValueAt(idx);
        bufScenMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, eBUFFER_GRAPHIC);
    }
    else
    {
        BufScenarioToTypeMap bufScenMap;
        bufScenMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, eBUFFER_GRAPHIC);
        mBIDToScenarioTypeMap.add(BID_P2A_OUT_RECT_IN2, bufScenMap);
    }

    // tuning buffer section
    BufScenarioToTypeMap typeMap;
    typeMap.add(eBUFFER_POOL_SCENARIO_PREVIEW, eBUFFER_TUNING);
    typeMap.add(eBUFFER_POOL_SCENARIO_RECORD, eBUFFER_TUNING);
    typeMap.add(eBUFFER_POOL_SCENARIO_CAPTURE, eBUFFER_TUNING);
    mBIDToScenarioTypeMap.add(BID_P2A_TUNING, typeMap);
    return MTRUE;

}


}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

