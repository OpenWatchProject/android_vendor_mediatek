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

#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include "WPENode.h"


#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "WPENode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#define WPE_NORMAL_PIPE_NAME "DenoiseWPENode"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
/*******************************************************************************
 *
 ********************************************************************************/
WPENode::
WPENode(
    const char* name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , mBufPool(name)
    , miOpenId(openId)
{
    MY_LOGD("ctor: %s", name);
    this->addWaitQueue(&mJobQueue);

    mWarpConfigMain1 = StereoSettingProvider::getWarpingBufferConfig(eSTEREO_SENSOR_MAIN1);
    mWarpConfigMain2 = StereoSettingProvider::getWarpingBufferConfig(eSTEREO_SENSOR_MAIN2);
}
/*******************************************************************************
 *
 ********************************************************************************/
WPENode::
~WPENode()
{
    MY_LOGD("dtor");
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
onData(DataID id, ImgInfoMapPtr& data)
{
    TRACE_FUNC_ENTER();
    MBOOL bRet = MFALSE;

    switch(id)
    {
        case N3D_TO_WPE_WARP_DATA:
            mJobQueue.enque(data);
            bRet = MTRUE;
            break;
        default:
            MY_LOGW("non-handle DataID = %d", id);
            break;
    }

    TRACE_FUNC_EXIT();
    return bRet;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
onInit()
{
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
onUninit()
{
    TRACE_FUNC_ENTER();
    mBufPool.uninit();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    TRACE_FUNC_ENTER();

    if(mpINormalStream == nullptr)
    {
        mpINormalStream = INormalStream::createInstance(miOpenId);
        mpINormalStream->init(WPE_NORMAL_PIPE_NAME, NSCam::NSIoPipe::EStreamPipeID_WarpEG);
    }

    initBufferPool();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
onThreadStop()
{
    TRACE_FUNC_ENTER();
    if(mpINormalStream != nullptr)
    {
        mpINormalStream->uninit(WPE_NORMAL_PIPE_NAME);
        mpINormalStream->destroyInstance();
        mpINormalStream = nullptr;
    }
    TRACE_FUNC_EXIT();
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
onThreadLoop()
{
    TRACE_FUNC_ENTER();

    ImgInfoMapPtr pImgInfoMap = nullptr;

    if(!waitAllQueue())
    {
        return MFALSE;
    }

    if(!mJobQueue.deque(pImgInfoMap))
    {
        MY_LOGE("mJobQueue.deque() failed.");
        return MFALSE;
    }

    MBOOL bRet = this->processWPE(pImgInfoMap)

    TRACE_FUNC_EXIT();

    return bRet;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
processWPE(ImgInfoMapPtr pInfoMap)
{
    PipeRequestPtr pRequest = pInfoMap->getRequestPtr();

    SmartImageBuffer smBuf_RectIn1 = pInfoMap->getSmartBuffer(BID_P2AFM_OUT_RECT_IN1_CAP);
    SmartImageBuffer smBuf_MV_Y = mBufPool.getBufPool(BID_WPE_OUT_MV_Y)->request();
    SmartImageBuffer gridArray1[3] = {nullptr};
    gridArray1[0] = pInfoMap->getSmartBuffer(BID_N3D_OUT_WARP_GRID_1_X);
    gridArray1[1] = pInfoMap->getSmartBuffer(BID_N3D_OUT_WARP_GRID_1_Y);
    gridArray1[2] = (mWarpConfigMain1.PLANE_COUNT == 3) ? pInfoMap->getSmartBuffer(BID_N3D_OUT_WARP_GRID_1_Z) : nullptr;

    if(!this->doWPEWarpOnImg(mWarpConfigMain1, smBuf_RectIn1, gridArray1, smBuf_MV_Y))
    {
        MY_LOGE("Failed to warp main1 rect in to MV_Y!");
        return MFALSE;
    }

    SmartImageBuffer smBuf_SV_CC = pInfoMap->getSmartBuffer(BID_N3D_OUT_SV_CC_Y);
    SmartImageBuffer smBuf_SV_Y = mBufPool.getBufPool(BID_WPE_OUT_SV_Y)->request();
    SmartImageBuffer gridArray2[3] = {nullptr};
    gridArray2[0] = pInfoMap->getSmartBuffer(BID_N3D_OUT_WARP_GRID_2_X);
    gridArray2[1] = pInfoMap->getSmartBuffer(BID_N3D_OUT_WARP_GRID_2_Y);
    gridArray2[2] = (mWarpConfigMain2.PLANE_COUNT == 3) ? pInfoMap->getSmartBuffer(BID_N3D_OUT_WARP_GRID_2_Z) : nullptr;

    if(!this->doWPEWarpOnImg(mWarpConfigMain2, smBuf_SV_CC, gridArray2, smBuf_SV_Y))
    {
        MY_LOGE("Failed to warp main2 rect in to SV_Y!");
        return MFALSE;
    }

    SmartImageBuffer smBuf_PreMask2 = pInfoMap->getSmartBuffer(BID_N3D_OUT_PRE_MASK_S);
    SmartImageBuffer smBuf_MASK_S = mBufPool.getBufPool(BID_WPE_OUT_MASK_S)->request();
    if(!this->doWPEWarpOnImg(mWarpConfigMain2, smBuf_PreMask2, gridArray2, smBuf_MASK_S))
    {
        MY_LOGE("Failed to warp main2 pre mask in to MASK_S!");
        return MFALSE;
    }

    ImgInfoMapPtr pImgInfo_DPE = new ImageBufInfoMap(pRequest);
    pImgInfo_DPE->addSmartBuffer(BID_WPE_OUT_MV_Y, smBuf_MV_Y);
    pImgInfo_DPE->addSmartBuffer(BID_WPE_OUT_SV_Y, smBuf_SV_CC);
    pImgInfo_DPE->addSmartBuffer(BID_WPE_OUT_MASK_S, smBuf_MASK_S);
    // pass down to next DPE node
    pImgInfo_DPE->addSmartBuffer(BID_N3D_OUT_MASK_M,
                                pInfoMap->getSmartBuffer(BID_N3D_OUT_MASK_M));
    pImgInfo_DPE->addSmartBuffer(BID_N3D_OUT_WARPING_MATRIX,
                                pInfoMap->getSmartBuffer(BID_N3D_OUT_WARPING_MATRIX));

    handleData(WPE_TO_DPE_WARP_RESULT, pImgInfo_DPE);

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
doWPEWarpOnImg(
    const WARPING_BUFFER_CONFIG_T& config,
    const SmartImageBuffer& srcImg,
    SmartImageBuffer gridImg[],
    SmartImageBuffer& outputImg
)
{
    TRACE_FUNC_ENTER();

    MY_LOGD("BUFFER_SIZE:%d, PLANE_COUNT:%d", config.BUFFER_SIZE, config.PLANE_COUNT);

    FrameParams frameParam;
    // preare input
    Input input;
    input.mPortID = PORT_WPEI;
    input.mBuffer = srcImg->mImageBuffer.get();
    frameParam.mvIn.push_back(input);

    // prepare WPEParam
    WPEQParams wpeParam;
    if(!this->prepareWPEParam(config, gridImg, wpeParam))
    {
        MY_LOGE("Failed to prepare WPEParam.");
        return MFALSE;
    }

    // insert WPE ExtraParam
    ExtraParam exParams;
    exParams.CmdIdx = EPIPE_WPE_INFO_CMD;
    exParams.moduleStruct = (void*) &wpeParam;
    frameParam.mvExtraParam.push_back(exParams);

    // prepare output
    Output output;
    output.mPortID = PORT_WPEO;
    output.mBuffer = outputImg->mImageBuffer.get();
    frameParam.mvOut.push_back(output);

    // prepare QParam enque
    QParams enqueParam;
    QParams dequeParam;
    enqueParam.mvFrameParams.push_back(frameParam);
    // enque
    MY_LOGD("enque +");
    MBOOL bRet = mpINormalStream->enque(enqueParam);
    if(!bRet)
    {
        MY_LOGE("WPE enque failed!");
        goto lbExit;
    }
    MY_LOGD("enque -");

    MY_LOGD("deque +");
    bRet = mpINormalStream->deque(dequeParam);
    if(!bRet)
    {
        MY_LOGE("WPE deque failed!");
        goto lbExit;
    }
    MY_LOGD("deque -");

lbExit:
    TRACE_FUNC_EXIT();
    return bRet;

}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
WPENode::setupVeciInfo(
    SmartImageBuffer gridImg,
    WarpMatrixInfo& veci
)
{
    veci.width = gridImg->mImageBuffer->getImgSize().w;
    veci.height = gridImg->mImageBuffer->getImgSize().h;
    veci.stride = gridImg->mImageBuffer->getBufSizeInBytes(0);
    veci.virtAddr = gridImg->mImageBuffer->getBufVA(0);
    veci.phyAddr = gridImg->mImageBuffer->getBufPA(0);
    veci.bus_size = NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS;
    veci.addr_offset = 0;
    veci.veci_v_flip_en = 0;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
prepareWPEParam(
    const WARPING_BUFFER_CONFIG_T& config,
    SmartImageBuffer gridImg[],
    WPEQParams& rWPEParam
)
{
    // flush
    gridImg[0]->mImageBuffer->syncCache(eCACHECTRL_FLUSH);
    gridImg[1]->mImageBuffer->syncCache(eCACHECTRL_FLUSH);
    //
    rWPEParam.wpe_mode = NSIoPipe::NSWpe::WPE_MODE_WPEO;
    rWPEParam.vgen_hmg_mode = (config.PLANE_COUNT == 2) ? 0 : 1;

    this->setupVeciInfo(gridImg[0], rWPEParam.warp_veci_info);
    this->setupVeciInfo(gridImg[1], rWPEParam.warp_vec2i_info);

    // check plane 3
    if(config.PLANE_COUNT == 3)
        this->setupVeciInfo(gridImg[3], rWPEParam.warp_vec3i_info);

    // vgen crop
    MCrpRsInfo vgenCropInfo;
    vgenCropInfo.mCropRect = MCropRect(MPoint(0,0), config.BUFFER_SIZE);
    rWPEParam.mwVgenCropInfo.push_back(vgenCropInfo);

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
WPENode::
initBufferPool()
{
    StereoSizeProvider* pSizePrvider = StereoSizeProvider::getInstance();
    // capture only need 1 buffer set
    int buffer_size = 1;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        MSize size = pSizePrvider->getBufferSize(E_MV_Y);
        NSBMDN::BufferConfig c = {
            "WPEWarpOut_M",
            BID_WPE_OUT_MV_Y,
            (MUINT32) size.w,
            (MUINT32) size.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)buffer_size
        };
        vBufConfig.push_back(c);
    }

    {
        MSize size = pSizePrvider->getBufferSize(E_MV_Y);
        NSBMDN::BufferConfig c = {
            "WPEWarpOut_S",
            BID_WPE_OUT_SV_Y,
            (MUINT32) size.w,
            (MUINT32) size.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)buffer_size
        };
        vBufConfig.push_back(c);
    }

    {
        MSize size = pSizePrvider->getBufferSize(E_MASK_S_Y);
        NSBMDN::BufferConfig c = {
            "WarpedMaskMain2",
            BID_WPE_OUT_MASK_S,
            (MUINT32) size.w,
            (MUINT32) size.h,
            eImgFmt_Y8,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)buffer_size
        };
        vBufConfig.push_back(c);
    }



    if(!mBufPool.init(vBufConfig)){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
WPENode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}










