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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "FOVWarpNode.h"
#if !SUPPORT_WPE
#include "GPUWarpStream.h"
#endif
#include <mtkcam/feature/DualCam/FOVHal.h>
#include <DpBlitStream.h>
#include <mtkcam/drv/iopipe/PortMap.h>


#define PIPE_CLASS_TAG "FOVWarpNode"
#define PIPE_TRACE TRACE_FOVWARP_NODE
#include <featurePipe/core/include/PipeLog.h>

#define NORMAL_PIPE_NAME "fovWarp"


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using namespace NSCam::NSIoPipe;
#if SUPPORT_WPE
using namespace NSCam::NSIoPipe::NSWpe;
#endif
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_WDMAO;

FOVWarpNode::FOVWarpNode(const char *name)
    : StreamingFeatureNode(name)
#if !SUPPORT_WPE
    , mWarpStream(NULL)
#endif
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mFOVDatas);
    this->addWaitQueue(&mFullIOImgDatas);
    TRACE_FUNC_EXIT();
}

FOVWarpNode::~FOVWarpNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID FOVWarpNode::setOutputBufferPool(const android::sp<IBufferPool> &pool)
{
    TRACE_FUNC_ENTER();
    mOutputBufferPool = pool;
    TRACE_FUNC_EXIT();
}

MBOOL FOVWarpNode::onData(DataID id, const BasicIOImgData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_P2A_TO_FOV_WARP )
    {
        this->mFullIOImgDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FOVWarpNode::onData(DataID id, const FOVData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_FOV_TO_FOV_WARP )
    {
        this->mFOVDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FOVWarpNode::onInit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    sem_init(&mWaitWPESem, 0, 1);
    StreamingFeatureNode::onInit();
#if !SUPPORT_WPE
    mWarpStream = GPUWarpStream::createInstance();
    if( mWarpStream == NULL )
    {
        MY_LOGE("Failed to create GPU warp module");
        ret = MFALSE;
    }
#endif
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FOVWarpNode::onUninit()
{
    TRACE_FUNC_ENTER();
    mOutputBufferPool = NULL;
    sem_destroy(&mWaitWPESem);
#if !SUPPORT_WPE
    if( mWarpStream )
    {
        mWarpStream->destroyInstance();
        mWarpStream = NULL;
    }
#endif
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVWarpNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

#if SUPPORT_WPE
    if( mpINormalStream == nullptr )
    {
        mpINormalStream = INormalStream::createInstance(mSensorIndex);
        mpINormalStream->init(NORMAL_PIPE_NAME, EStreamPipeID_WarpEG);
    }
#else
    if( mWarpStream != NULL )
    {
        // using GPU warp
        MSize fullSize = mPipeUsage.getStreamingSize();
        fullSize.w = align(fullSize.w, 6);
        fullSize.h = align(fullSize.h, 6);
        MBOOL forceRGBA = getPropertyValue(KEY_FORCE_GPU_RGBA, VAL_FORCE_GPU_RGBA);
        mWarpOutBufferPool = GraphicBufferPool::create("fpipe.fov.gpuo",
                                                        fullSize.w,
                                                        fullSize.h,
                                                        forceRGBA ? HAL_PIXEL_FORMAT_RGBA_8888 : HAL_PIXEL_FORMAT_YV12,
                                                        GraphicBufferPool::USAGE_HW_TEXTURE);
        mWarpOutBufferPool->allocate(3);

        if( !mWarpStream->init(mSensorIndex, fullSize, MAX_WARP_SIZE) )
        {
            MY_LOGE("Failed to create warp module");
            ret = MFALSE;
        }
    }
#endif
    if (mPipeUsage.supportVendor() && mOutputBufferPool != NULL)
    {
        Timer timer;
        timer.start();
        mOutputBufferPool->allocate(mPipeUsage.getNumFOVWarpOutBuffer());
        timer.stop();
        MY_LOGD("fov warp %s %d buf in %d ms", STR_ALLOCATE, mPipeUsage.getNumFOVWarpOutBuffer(), timer.getElapsed());
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL FOVWarpNode::onThreadStop()
{
    TRACE_FUNC_ENTER();

#if SUPPORT_WPE
    if( mpINormalStream != nullptr )
    {
        mpINormalStream->uninit(NORMAL_PIPE_NAME);
        mpINormalStream->destroyInstance();
        mpINormalStream = nullptr;
    }
#else
    if ( mWarpStream != NULL )
    {
        this->waitWarpStreamBaseDone();
        mWarpStream->uninit();

        if( mWarpOutBufferPool != NULL )
        {
            IBufferPool::destroy(mWarpOutBufferPool);
            mWarpOutBufferPool = NULL;
        }
    }
#endif

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVWarpNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr request;
    FOVData fovData;
    BasicIOImgData fullIOImg;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }
    if( !mFOVDatas.deque(fovData) )
    {
        MY_LOGE("WarpMapData deque out of sync");
        return MFALSE;
    }
    if( !mFullIOImgDatas.deque(fullIOImg) )
    {
        MY_LOGE("FullImgData deque out of sync");
        return MFALSE;
    }

    if( fovData.mRequest == NULL ||
        fovData.mRequest != fullIOImg.mRequest )
    {
        MY_LOGE("Request out of sync");
        return MFALSE;
    }

    TRACE_FUNC_ENTER();

    request = fovData.mRequest;
    // if not need fov
    if(!request->needFOV())
    {
        ::sem_wait(&(mWaitWPESem));
        TRACE_FUNC("Frame:%d, do not need FOV", request->mRequestNo);
        handleResultData(request, fullIOImg.mData.mIn, fullIOImg.mData.mOut);
        ::sem_post(&(mWaitWPESem));
        return MTRUE;
    }
    else
    {
        int ratio = request->getVar<MUINT32>(VAR_DUALCAM_ZOOM_RATIO, 0 );
        TRACE_FUNC("Frame:%d, need FOV, ratio=%d", request->mRequestNo, ratio);
    }

    if (request->needPrintIO())
    {
        printIO(request, fullIOImg, fovData);
    }

    TRACE_FUNC("Frame %d in FOVWarp", request->mRequestNo);
    if( mPipeUsage.supportEISNode() && request->needEISFullImg() )
    {
        if( mPipeUsage.supportFOVCombineEIS() )
        {
            TRACE_FUNC("FOV combine EIS don't use warp");
            ::sem_wait(&(mWaitWPESem));

            //MDP
            Output displayOutput;
            MCrpRsInfo displayCrop;
            BasicImg fovOutImg;
            if( request->getDisplayOutput(displayOutput) && request->getDisplayCrop(displayCrop) )
            {
                MCropRect crop = displayCrop.mCropRect;
                MSize FullImgSize = fullIOImg.mData.mIn.mBuffer->getImageBufferPtr()->getImgSize();
                crop.p_integral.x -= fullIOImg.mData.mIn.mDomainOffset.w;
                crop.p_integral.y -= fullIOImg.mData.mIn.mDomainOffset.h;
                applyFOVCrop(fovData.mData.mFOVScale, fovData.mData.mFOVShift, crop, FullImgSize);

                MDPWrapper::OUTPUT_ARRAY outputs;
                outputs.push_back(MDPWrapper::MDPOutput(displayOutput, crop));

                request->mTimer.startFOVWarp();

                if( !mMDP.process(fullIOImg.mData.mIn.mBuffer->getImageBufferPtr(), outputs) )
                {
                    MY_LOGE("FOVWarp MDP process fail!");
                }

                request->mTimer.stopFOVWarp();
            }
            else
            {
                MY_LOGD("No display output");
            }

            handleResultData(request, fovOutImg, fullIOImg.mData.mOut);

            ::sem_post(&(mWaitWPESem));
            return MTRUE;
        }
        else if( mPipeUsage.supportVendor() && request->needVendor() && !request->needHWFOVWarp() )
        {
            TRACE_FUNC("Handle FOV+EIS+Vendor");
            ::sem_wait(&(mWaitWPESem));

            //MDP
            Output displayOutput;
            MCrpRsInfo displayCrop;
            BasicImg fovOutImg;

            // for vendor
            MCropRect crop;
            MSize FullImgSize = fullIOImg.mData.mIn.mBuffer->getImageBufferPtr()->getImgSize();
            MSize fovMarginSize = request->getFOVMarginPixel();
            crop.p_integral.x = fovMarginSize.w;
            crop.p_integral.y = fovMarginSize.h;
            crop.s = FullImgSize - fovMarginSize * 2;

            // apply fov
            crop.p_integral.x -= fullIOImg.mData.mIn.mDomainOffset.w;
            crop.p_integral.y -= fullIOImg.mData.mIn.mDomainOffset.h;
            applyFOVCrop(fovData.mData.mFOVScale, fovData.mData.mFOVShift, crop, FullImgSize);

            MDPWrapper::OUTPUT_ARRAY outputs;
            fovOutImg.mBuffer = mOutputBufferPool->requestIIBuffer();
            fovOutImg.mDomainOffset = fullIOImg.mData.mIn.mDomainOffset + fovMarginSize;

            Output fovo;
            fovo.mPortID = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
            fovo.mBuffer = fovOutImg.mBuffer->getImageBufferPtr();
            fovo.mBuffer->setExtParam(FullImgSize - fovMarginSize * 2);
            outputs.push_back(MDPWrapper::MDPOutput(fovo, crop));

            request->mTimer.startFOVWarp();

            if( !mMDP.process(fullIOImg.mData.mIn.mBuffer->getImageBufferPtr(), outputs) )
            {
                MY_LOGE("FOVWarp MDP process fail!");
            }

            request->mTimer.stopFOVWarp();

            handleResultData(request, fovOutImg, fullIOImg.mData.mOut);

            ::sem_post(&(mWaitWPESem));
            return MTRUE;
        }
    }

#if SUPPORT_WPE
    processFOVWarp(request, fovData.mData, fullIOImg.mData);
#else
    processGPUWarp(request, fovData.mData, fullIOImg.mData);
#endif

    TRACE_FUNC_EXIT();
    return MTRUE;
}

double FOVWarpNode::getFOVWarpRatio(const RequestPtr &request)
{
    (void)(request);
    double ratio = 1.0;
    // when EIS is on, input is 31%, output is 25%
    /*
    if( request->needEIS() )
        ratio = mPipeUsage.getEISFactor() / (1.0 * request->getEISFactor() + DUALZOOM_FOV_MARGIN);
    else
        ratio = 100.0 / (100 + DUALZOOM_FOV_MARGIN);
    */
    return ratio;
}

#if SUPPORT_WPE
MBOOL FOVWarpNode::prepareWPEParam(
    const RequestPtr &request,
    const FOVResult &fovResult,
    const MSize& inputBufSize,
    WPEQParams& rWPEParam
)
{
    IImageBuffer* pWarpMap = fovResult.mWarpMap->getImageBufferPtr();

    // flush
    pWarpMap->syncCache(eCACHECTRL_FLUSH);
    //
    rWPEParam.wpe_mode = WPE_MODE_MDP;
    rWPEParam.vgen_hmg_mode = 0; // no z plane
    rWPEParam.warp_veci_info.width = fovResult.mWarpMapSize.w;
    rWPEParam.warp_veci_info.height = fovResult.mWarpMapSize.h;
    rWPEParam.warp_veci_info.stride = fovResult.mWarpMapSize.w * WPE_WARP_DATA_BYTE;
    rWPEParam.warp_veci_info.virtAddr = pWarpMap->getBufVA(0);
    rWPEParam.warp_veci_info.phyAddr = pWarpMap->getBufPA(0);
    rWPEParam.warp_veci_info.bus_size = WPE_BUS_SIZE_32_BITS;
    rWPEParam.warp_veci_info.addr_offset = 0;
    rWPEParam.warp_veci_info.veci_v_flip_en = 0;

    //
    rWPEParam.warp_vec2i_info.width = fovResult.mWarpMapSize.w;
    rWPEParam.warp_vec2i_info.height = fovResult.mWarpMapSize.h;
    rWPEParam.warp_vec2i_info.stride = fovResult.mWarpMapSize.w * WPE_WARP_DATA_BYTE;
    rWPEParam.warp_vec2i_info.virtAddr = pWarpMap->getBufVA(1);
    rWPEParam.warp_vec2i_info.phyAddr = pWarpMap->getBufPA(1);
    rWPEParam.warp_vec2i_info.bus_size = WPE_BUS_SIZE_32_BITS;
    rWPEParam.warp_vec2i_info.addr_offset = 0;
    rWPEParam.warp_vec2i_info.veci_v_flip_en = 0;

    // vgen crop
    MCrpRsInfo vgenCropInfo;
    vgenCropInfo.mCropRect = MCropRect(MPoint(0,0), fovResult.mWarpMapSize);
    rWPEParam.mwVgenCropInfo.push_back(vgenCropInfo);
    // wpe crop
    rWPEParam.wpecropinfo.x_start_point = 0;
    rWPEParam.wpecropinfo.y_start_point = 0;
    rWPEParam.wpecropinfo.x_end_point = fovResult.mWPESize.w - 1;
    rWPEParam.wpecropinfo.y_end_point = fovResult.mWPESize.h - 1;

    debugWPEParam(rWPEParam);

    return MTRUE;
}

MVOID FOVWarpNode::debugWPEParam(const WPEQParams& rWPEParam    __attribute__((unused)))
{
    TRACE_FUNC("rWPEParam.warp_veci_info.widthxheight=%dx%d", rWPEParam.warp_veci_info.width,
                                                            rWPEParam.warp_veci_info.height);
    TRACE_FUNC("rWPEParam.warp_veci_info.stride=%d", rWPEParam.warp_veci_info.stride);
    TRACE_FUNC("rWPEParam.warp_veci_info.virtAddr=%x", rWPEParam.warp_veci_info.virtAddr);
    TRACE_FUNC("rWPEParam.warp_veci_info.phyAddr=%x", rWPEParam.warp_veci_info.phyAddr);
    TRACE_FUNC("rWPEParam.warp_veci_info.bus_size=%d", rWPEParam.warp_veci_info.bus_size);
    TRACE_FUNC("rWPEParam.warp_veci_info.addr_offset=%d", rWPEParam.warp_veci_info.addr_offset);
    TRACE_FUNC("rWPEParam.warp_veci_info.veci_v_flip_en=%d", rWPEParam.warp_veci_info.veci_v_flip_en);

    TRACE_FUNC("rWPEParam.warp_vec2i_info.widthxheight=%dx%d", rWPEParam.warp_vec2i_info.width,
                                                            rWPEParam.warp_veci_info.height);
    TRACE_FUNC("rWPEParam.warp_vec2i_info.stride=%d", rWPEParam.warp_vec2i_info.stride);
    TRACE_FUNC("rWPEParam.warp_vec2i_info.virtAddr=%x", rWPEParam.warp_vec2i_info.virtAddr);
    TRACE_FUNC("rWPEParam.warp_vec2i_info.phyAddr=%x", rWPEParam.warp_vec2i_info.phyAddr);
    TRACE_FUNC("rWPEParam.warp_vec2i_info.bus_size=%d", rWPEParam.warp_vec2i_info.bus_size);
    TRACE_FUNC("rWPEParam.warp_vec2i_info.addr_offset=%d", rWPEParam.warp_vec2i_info.addr_offset);
    TRACE_FUNC("rWPEParam.warp_vec2i_info.veci_v_flip_en=%d", rWPEParam.warp_vec2i_info.veci_v_flip_en);

    TRACE_FUNC("vgenCropInfo.mCropRect=(%d,%d) %dx%d", rWPEParam.mwVgenCropInfo.itemAt(0).mCropRect.p_integral.x,
                rWPEParam.mwVgenCropInfo.itemAt(0).mCropRect.p_integral.y,
                rWPEParam.mwVgenCropInfo.itemAt(0).mCropRect.s.w,
                rWPEParam.mwVgenCropInfo.itemAt(0).mCropRect.s.h);
    TRACE_FUNC("rWPEParam.wpecropinfo=(%d,%d) (%d,%d)", rWPEParam.wpecropinfo.x_start_point,
                rWPEParam.wpecropinfo.y_start_point, rWPEParam.wpecropinfo.x_end_point,
                rWPEParam.wpecropinfo.y_end_point);
}

MBOOL FOVWarpNode::processFOVWarp(const RequestPtr &request, const FOVResult &fovResult, BasicIOImg &fullIOImg)
{
    TRACE_FUNC_ENTER();
    FrameParams frameParam;
    // preare input : fullimg
    Input input;
    input.mPortID = PORT_WPEI;
    input.mBuffer = fullIOImg.mIn.mBuffer->getImageBufferPtr();
    frameParam.mvIn.push_back(input);

    bufferDump(request, fullIOImg.mIn.mBuffer->getImageBuffer().get(), "input");

    // prepare output
    BasicImg fovOutImg;
    if (mPipeUsage.supportVendor() && request->needVendor())
    {
        TRACE_FUNC("Get vendor output buffer");
        // for vendor
        fovOutImg.mBuffer = mOutputBufferPool->requestIIBuffer();
        fovOutImg.mDomainOffset = fullIOImg.mIn.mDomainOffset + request->getFOVMarginPixel();

        Output fovo;
        fovo.mPortID = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
        fovo.mBuffer = fovOutImg.mBuffer->getImageBufferPtr();
        fovo.mBuffer->setExtParam(fovResult.mWPESize);
        frameParam.mvOut.push_back(fovo);

        MCrpRsInfo crop;
        crop.mGroupID = WROTO_CROP_GROUP;
        crop.mCropRect.p_fractional.x = 0;
        crop.mCropRect.p_fractional.y = 0;
        crop.mCropRect.p_integral.x   = 0;
        crop.mCropRect.p_integral.y   = 0;
        crop.mCropRect.s = fovResult.mWPESize;
        crop.mResizeDst = fovResult.mWPESize;
        frameParam.mvCropRsInfo.push_back(crop);
    }
    else if(!this->prepareYUVOutput(request, fovResult, fullIOImg, frameParam))
    {
        TRACE_FUNC("No exist any yuv buffer to generate.");
        handleData(ID_FOV_WARP_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_FRAME_DONE, request));
        return MTRUE;
    }

    // prepare WPEParam
    WPEQParams wpeParam;
    if(!this->prepareWPEParam(request, fovResult, fullIOImg.mIn.mBuffer->getImageBuffer()->getImgSize(), wpeParam))
    {
        MY_LOGE("Failed to fill prepare WPE params!");
        return MFALSE;
    }

    // insert WPE ExtraParam
    ExtraParam exParams;
    exParams.CmdIdx = EPIPE_WPE_INFO_CMD;
    exParams.moduleStruct = (void*) &wpeParam;
    frameParam.mvExtraParam.push_back(exParams);

    // prepare QParam enque
    QParams enqueParam;
    enqueParam.mvFrameParams.push_back(frameParam);
    enqueParam.mpfnCallback = onWPEDone;
    // cookie
    FOVWarpNode::EnqueCookie* cookie = new FOVWarpNode::EnqueCookie(request, this, fovResult, fullIOImg.mOut);
    cookie->fovOutImg = fovOutImg;

    enqueParam.mpCookie = (MVOID*) cookie;
    // start timer
    request->mTimer.startFOVWarp();

    this->incExtThreadDependency();

    MY_LOGD("sensor(%d) Frame %d fov wpe enque start", mSensorIndex, request->mRequestNo);
    MBOOL bRet = mpINormalStream->enque(enqueParam);
    TRACE_FUNC("sensor(%d) Frame %d fov wpe enque done", mSensorIndex, request->mRequestNo);
    if(!bRet)
    {
        MY_LOGE("Failed to enque NormalStream");
        return MFALSE;
    }
    ::sem_wait(&(mWaitWPESem));

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVWarpNode::prepareYUVOutput(const RequestPtr &request, const FOVResult &fovResult, BasicIOImg &fullIOImg, FrameParams& rFrameParam)
{
    MBOOL bExistOutput = MFALSE;

    // get output : display
    Output yuvOut;
    if( request->getDisplayOutput(yuvOut) )
    {
        TRACE_FUNC("Get display output!");
        bExistOutput = MTRUE;
        rFrameParam.mvOut.push_back(yuvOut);

        if( fovResult.mDisplayCrop.s.w != 0 )
        {
            MCrpRsInfo crop;
            if (yuvOut.mPortID.index == EPortIndex_WROTO)
            {
                crop.mGroupID = 3; //WROT
            }
            else if (yuvOut.mPortID.index == EPortIndex_WDMAO)
            {
                crop.mGroupID = 2; //WDMA
            }
            else
            {
                MY_LOGE("Error!!!!, FOVWarp only support WROT and WDMA but port is %d", yuvOut.mPortID.index);
                crop.mGroupID = 3; //WROT
            }
            crop.mCropRect.p_fractional.x = 0;
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.p_integral.x   = fovResult.mDisplayCrop.p.x;
            crop.mCropRect.p_integral.y   = fovResult.mDisplayCrop.p.y;
            crop.mCropRect.s = fovResult.mDisplayCrop.s;
            crop.mResizeDst = yuvOut.mBuffer->getImgSize();
            rFrameParam.mvCropRsInfo.push_back(crop);
        }
        else
            MY_LOGE("Does not exist the display crop info");
        // set clear zoom for preview
        int noclearzoom = property_get_int32("debug.dualcam.noclearzoom", 0);
        if(!noclearzoom)
        {
            PQParam *pqParam = new PQParam();
            MINT32 openId = request->mVarMap.get<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, -1);
            if(openId != -1)
            {
                ExtraParam extraParam;
                extraParam.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
                request->getPQParam(*pqParam);
                extraParam.moduleStruct = static_cast<void*>(pqParam);
                rFrameParam.mvExtraParam.push_back(extraParam);
            }
        }
    }

    // get output : record
    if( request->needEISFullImg() )
    {
        TRACE_FUNC("Get record output!");
        bExistOutput = MTRUE;

        MINT32 portIndex = (yuvOut.mPortID.index == EPortIndex_WDMAO) ? EPortIndex_WROTO : EPortIndex_WDMAO;

        Output output;
        output.mPortID = PortID(EPortType_Memory, portIndex, PORTID_OUT);
        output.mBuffer = fullIOImg.mOut.mBuffer->getImageBufferPtr();
        rFrameParam.mvOut.push_back(output);

        MSize eisFullImgSize = request->getEISInputSize();

        MCrpRsInfo crop;
        crop.mGroupID = (portIndex == EPortIndex_WDMAO) ? WDMAO_CROP_GROUP : WROTO_CROP_GROUP;
        crop.mCropRect = MCropRect(eisFullImgSize.w, eisFullImgSize.h);
        crop.mResizeDst = eisFullImgSize;
        rFrameParam.mvCropRsInfo.push_back(crop);

        fullIOImg.mOut.mDomainOffset += request->getFOVMarginPixel();
    }
    else if( request->getRecordOutput(yuvOut) )
    {
        TRACE_FUNC("Get record output!");
        bExistOutput = MTRUE;
        rFrameParam.mvOut.push_back(yuvOut);

        if( fovResult.mRecordCrop.s.w != 0 )
        {
            MCrpRsInfo crop;
            if (yuvOut.mPortID.index == EPortIndex_WROTO)
            {
                crop.mGroupID = 3; //WROT
            }
            else if (yuvOut.mPortID.index == EPortIndex_WDMAO)
            {
                crop.mGroupID = 2; //WDMA
            }
            else
            {
                MY_LOGE("Error!!!!, FOVWarp only support WROT and WDMA but port is %d", yuvOut.mPortID.index);
                crop.mGroupID = 2; //WDMA
            }
            crop.mCropRect.p_fractional.x = 0;
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.p_integral.x   = fovResult.mRecordCrop.p.x;
            crop.mCropRect.p_integral.y   = fovResult.mRecordCrop.p.y;
            crop.mCropRect.s = fovResult.mRecordCrop.s;
            crop.mResizeDst = yuvOut.mBuffer->getImgSize();
            rFrameParam.mvCropRsInfo.push_back(crop);
        }
        else
            MY_LOGE("Does not exist the record crop info");
    }

    // output : extra yuv
    Output extra;
    if( request->getExtraOutput(extra) )
    {
        TRACE_FUNC("Get extra output!");
        bExistOutput = MTRUE;
        rFrameParam.mvOut.push_back(extra);

        if( fovResult.mExtraCrop.s.w != 0 )
        {
            MCrpRsInfo crop;
            if (extra.mPortID.index == EPortIndex_WROTO)
            {
                crop.mGroupID = 3; //WROT
            }
            else if (extra.mPortID.index == EPortIndex_WDMAO)
            {
                crop.mGroupID = 2; //WDMA
            }
            else
            {
                MY_LOGE("Error!!!!, FOVWarp only support WROT and WDMA but port is %d", extra.mPortID.index);
                crop.mGroupID = 2; //WDMA
            }
            crop.mCropRect.p_fractional.x = 0;
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.p_integral.x   = fovResult.mExtraCrop.p.x;
            crop.mCropRect.p_integral.y   = fovResult.mExtraCrop.p.y;
            crop.mCropRect.s = fovResult.mExtraCrop.s;
            crop.mResizeDst = extra.mBuffer->getImgSize();
            rFrameParam.mvCropRsInfo.push_back(crop);
        }
        else
            MY_LOGE("Does not exist the extra crop info");
    }

    return bExistOutput;
}

MVOID FOVWarpNode::onWPEDone(QParams& rParams)
{
    FOVWarpNode::EnqueCookie* pCookie = (FOVWarpNode::EnqueCookie*) rParams.mpCookie;
    pCookie->pNode->onHandleWPEDone(rParams);
}

MVOID FOVWarpNode::onHandleWPEDone(QParams& rParams)
{
    FOVWarpNode::EnqueCookie* pCookie = (FOVWarpNode::EnqueCookie*) rParams.mpCookie;
    RequestPtr request = pCookie->request;
    BasicImg fullOutImg = pCookie->fullOutImg;
    BasicImg fovOutImg = pCookie->fovOutImg;
    // release cz object
    for(auto item : rParams.mvFrameParams)
    {
        for(auto frame : item.mvExtraParam)
        {
            if(frame.CmdIdx == EPIPE_MDP_PQPARAM_CMD)
            {
                delete (PQParam*)frame.moduleStruct;
            }
        }
    }

    // stop timer
    request->mTimer.stopFOVWarp();
    MY_LOGD("sensor(%d) Frame %d, fov wpe enque done in %d ms, result = %d", mSensorIndex, request->mRequestNo, request->mTimer.getElapsedFOVWarp(), rParams.mDequeSuccess);
    // buffer dump
    Output output;
    if(request->getDisplayOutput(output))
    {
        bufferDump(request, output.mBuffer, "display1");
    }
    if( request->getRecordOutput(output) )
    {
        bufferDump(request, output.mBuffer, "record");
    }
    if( request->getExtraOutput(output) )
    {
        bufferDump(request, output.mBuffer, "extra");
    }

    if( request->needDump() )
    {
        if( fullOutImg.mBuffer != NULL )
        {
            fullOutImg.mBuffer->getImageBuffer()->syncCache(eCACHECTRL_INVALID);
            dumpData(request, fullOutImg.mBuffer->getImageBufferPtr(), "fovFullout");
        }
    }
    handleResultData(request, fovOutImg, fullOutImg);
    pCookie->fovOutImg.mBuffer = NULL;
    ::sem_post(&(mWaitWPESem));
lbExit:
    // delete the cookie
    delete pCookie;
    this->decExtThreadDependency();

    TRACE_FUNC_EXIT();
}

#else

MBOOL FOVWarpNode::processGPUWarp(const RequestPtr &request, const FOVResult &fovResult, BasicIOImg &fullIOImg)
{
    TRACE_FUNC_ENTER();
    WarpParam param;
    ImgBuffer outBuf = mWarpOutBufferPool->requestIIBuffer();
    BasicImg fovOutImg;

    param.mRequest      = request;
    param.mIn           = fullIOImg.mIn.mBuffer;
    param.mWarpMap      = BasicImg(fovResult.mWarpMap);
    param.mByPass       = MFALSE;
    param.mWarpOut      = outBuf;
    param.mInSize       = param.mIn->getImageBuffer()->getImgSize();
    param.mOutSize      = fovResult.mWPESize;
    param.mDomainOffset = fullIOImg.mIn.mDomainOffset;
    outBuf->getImageBufferPtr()->setExtParam(param.mOutSize);
    if (mPipeUsage.supportVendor() && request->needVendor())
    {
        param.mMDPOut.clear();

        fovOutImg.mBuffer = mOutputBufferPool->requestIIBuffer();
        fovOutImg.mDomainOffset = fullIOImg.mIn.mDomainOffset + request->getFOVMarginPixel();

        Output fovo;
        fovo.mPortID = PortID(EPortType_Memory, EPortIndex_WROTO, PORTID_OUT);
        fovo.mBuffer = fovOutImg.mBuffer->getImageBufferPtr();
        fovo.mBuffer->setExtParam(fovResult.mWPESize);

        MCropRect cropRect(fovResult.mWPESize.w, fovResult.mWPESize.h);
        param.mMDPOut.push_back(MDPWrapper::MDPOutput(fovo, cropRect));
    }
    else
    {
        prepareMDPOutput(request, fovResult, fullIOImg.mOut, param);
    }
    ::sem_wait(&(mWaitWPESem));

    // start timer
    request->mTimer.startFOVWarp();
    this->incExtThreadDependency();
    EnqueCookie cookie(request, this, fovResult, fullIOImg.mOut);
    cookie.fovOutImg = fovOutImg;
    this->enqueWarpStreamBase(mWarpStream, param, cookie);
    TRACE_FUNC("sensor(%d) Frame %d fov gpu enque done", mSensorIndex, request->mRequestNo);

    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL FOVWarpNode::prepareMDPOutput(const RequestPtr &request, const FOVResult &fovResult, BasicImg &fullOutImg, WarpParam &param)
{
    MBOOL bExistOutput = MFALSE;
    param.mMDPOut.clear();

    // get output : display
    Output yuvOut;
    if( request->getDisplayOutput(yuvOut) && fovResult.mDisplayCrop.s.w )
    {
        MCropRect crop(fovResult.mDisplayCrop.p, fovResult.mDisplayCrop.s);
        param.mMDPOut.push_back(MDPWrapper::MDPOutput(yuvOut, crop));
        TRACE_FUNC("Display YUV (%d,%d) (%d,%d)",
                    crop.p_integral.x, crop.p_integral.y, crop.s.w, crop.s.h);

        bExistOutput = MTRUE;
    }

    // get output : record
    if( request->needEISFullImg() )
    {
        MSize eisFullImgSize = request->getEISInputSize();
        MCropRect crop(eisFullImgSize.w, eisFullImgSize.h);
        Output output;
        output.mPortID = PortID(EPortType_Memory, EPortIndex_WDMAO, PORTID_OUT);
        output.mBuffer = fullOutImg.mBuffer->getImageBufferPtr();
        param.mMDPOut.push_back(MDPWrapper::MDPOutput(output, crop));
        fullOutImg.mDomainOffset += request->getFOVMarginPixel();
        TRACE_FUNC("EISFull YUV (%d,%d)", crop.s.w, crop.s.h);

        bExistOutput = MTRUE;
    }
    else if( request->getRecordOutput(yuvOut) && fovResult.mRecordCrop.s.w )
    {
        MCropRect crop(fovResult.mRecordCrop.p, fovResult.mRecordCrop.s);
        param.mMDPOut.push_back(MDPWrapper::MDPOutput(yuvOut, crop));
        TRACE_FUNC("Record YUV (%d,%d) (%d,%d)",
                    crop.p_integral.x, crop.p_integral.y, crop.s.w, crop.s.h);

        bExistOutput = MTRUE;
    }

    // output : extra yuv
    Output extra;
    MCrpRsInfo extraCrop;
    if( request->getExtraOutput(extra) && fovResult.mExtraCrop.s.w )
    {
        MCropRect crop(fovResult.mExtraCrop.p, fovResult.mExtraCrop.s);
        param.mMDPOut.push_back(MDPWrapper::MDPOutput(extra, crop));
        TRACE_FUNC("Extra YUV (%d,%d) (%d,%d)",
                    crop.p_integral.x, crop.p_integral.y, crop.s.w, crop.s.h);

        bExistOutput = MTRUE;
    }

    if( !bExistOutput )
    {
        MY_LOGE("FOVWarp no exist any yuv buffer");
    }

    return bExistOutput;
}

MVOID FOVWarpNode::onWarpStreamBaseCB(const WarpParam &param, const EnqueCookie &cookie)
{
    RequestPtr request = cookie.request;
    BasicImg fullOutImg = cookie.fullOutImg;
    BasicImg fovOutImg = cookie.fovOutImg;
    // stop timer
    request->mTimer.stopFOVWarp();
    TRACE_FUNC("sensor(%d) Frame %d, fov gpu warp done in %d ms, result = %d",
                mSensorIndex, request->mRequestNo, request->mTimer.getElapsedFOVWarp(), param.mResult);

    handleResultData(request, fovOutImg, fullOutImg);

    ::sem_post(&(mWaitWPESem));
    this->decExtThreadDependency();

    TRACE_FUNC_EXIT();
}
#endif

MVOID FOVWarpNode::bufferDump(const RequestPtr &request, IImageBuffer* pImgBuf, const char* name)
{
    if(!request->needDump())
        return;

    TRACE_FUNC("Frame:%d", request->mRequestNo);
    char filepath[1024];
    makePath("/sdcard/wpe", 0660);
    snprintf(filepath, 1024, "/sdcard/wpe/%s_%d_%dx%d.yuv", name, request->mRequestNo, pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);
    pImgBuf->saveToFile(filepath);
    TRACE_FUNC_EXIT();
}

MVOID FOVWarpNode::printIO(const RequestPtr &request, const BasicIOImgData &fullIOImg, const FOVData &fovData)
{
    MCrpRsInfo displayCrop;
    if(request->getDisplayCrop(displayCrop))
    {
        MY_LOGD("display crop() (%d, %d) %dx%d",
            displayCrop.mCropRect.p_integral.x, displayCrop.mCropRect.p_integral.y,
            displayCrop.mCropRect.s.w, displayCrop.mCropRect.s.h);
    }
    MCrpRsInfo recordCrop;
    if(request->getRecordCrop(recordCrop))
    {
        MY_LOGD("record crop() (%d, %d) %dx%d",
            recordCrop.mCropRect.p_integral.x, recordCrop.mCropRect.p_integral.y,
            recordCrop.mCropRect.s.w, recordCrop.mCropRect.s.h);
    }
     MY_LOGD("fullImg size:(%dx%d) request->fovMargin(%d, %d)",
        fullIOImg.mData.mIn.mBuffer->getImageBufferPtr()->getImgSize().w,
        fullIOImg.mData.mIn.mBuffer->getImageBufferPtr()->getImgSize().h,
        request->getFOVMarginPixel().w, request->getFOVMarginPixel().h);
     MY_LOGD("fovData scale(%f) shift(%d, %d) wpeSize(%dx%d) wpeDisplay crop(%d, %d)(%dx%d) wpeRecord crop(%d, %d)(%dx%d)",
         fovData.mData.mFOVScale, fovData.mData.mFOVShift.x, fovData.mData.mFOVShift.y,
         fovData.mData.mWPESize.w, fovData.mData.mWPESize.h,
         fovData.mData.mDisplayCrop.p.x, fovData.mData.mDisplayCrop.p.y, fovData.mData.mDisplayCrop.s.w, fovData.mData.mDisplayCrop.s.h,
         fovData.mData.mRecordCrop.p.x, fovData.mData.mRecordCrop.p.y, fovData.mData.mRecordCrop.s.w, fovData.mData.mRecordCrop.s.h);
}

MVOID FOVWarpNode::handleResultData(
    const RequestPtr &request,
    const BasicImg &fullImg,
    const BasicImg &eisImg
)
{
    TRACE_FUNC_ENTER();
    if (mPipeUsage.supportVendor())
    {
        handleData(ID_FOV_WARP_TO_VENDOR, BasicIOImgData(BasicIOImg(fullImg, eisImg), request));
    }
    else if( mPipeUsage.supportEISNode() )
    {
        handleData(ID_FOV_WARP_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_DISPLAY_DONE, request));
        handleData(ID_FOV_TO_EIS_FULLIMG, BasicImgData(eisImg, request));
    }
    else
    {
        handleData(ID_FOV_WARP_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_FRAME_DONE, request));
    }
    TRACE_FUNC_EXIT();
    return;
}

MVOID FOVWarpNode::applyFOVCrop(
    const float &scale,
    const MPoint &shift,
    MCropRect &crop,
    const MSize maxSize
)
{
    crop.s.w *= scale;
    crop.s.h *= scale;
    crop.p_integral += shift;
    if ((crop.p_integral.x + crop.s.w) > maxSize.w)
    {
        crop.p_integral.x = maxSize.w - crop.s.w;
    }
    if ((crop.p_integral.y + crop.s.h) > maxSize.h)
    {
        crop.p_integral.y = maxSize.h - crop.s.h;
    }
    crop.p_integral.x = (crop.p_integral.x < 0) ? 0 : crop.p_integral.x;
    crop.p_integral.y = (crop.p_integral.y < 0) ? 0 : crop.p_integral.y;
    TRACE_FUNC("fov crop(%d, %d)%dx%d", crop.p_integral.x, crop.p_integral.y, crop.s.w, crop.s.h);

}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
