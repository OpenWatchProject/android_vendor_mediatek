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
// SWNR
#include <mtkcam/aaa/ICaptureNR.h>

#include "SWNRNode.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "SWNRNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// MET tags
#define DO_BM_SWNR "doSoftwareNR"

// buffer alloc size
#define BUFFER_ALLOC_SIZE 1
#define TUNING_ALLOC_SIZE 1

#define WAIT_BUF_TIMEOUT_NS     100000000LL

#include <PipeLog.h>

#include <DpBlitStream.h>
#include "../util/vsdof_util.h"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
using namespace NS3Av3;
/*******************************************************************************
 *
 ********************************************************************************/
SWNRNode::
SWNRNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
    this->addWaitQueue(&mImgInfoRequests);

    miEnableSWNR = ::property_get_int32("vendor.debug.bmdenoise.swnr", 1);
}
/*******************************************************************************
 *
 ********************************************************************************/
SWNRNode::
~SWNRNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    MY_LOGD("onData(request) +");

    MBOOL ret = MFALSE;
    switch(id)
    {
        case BMDENOISE_RESULT_TO_SWNR:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    MY_LOGD("onData(request) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
onData(
    DataID id,
    ImgInfoMapPtr& pImgInfo)
{
    MY_LOGD("onData(ImgInfo) +");

    MBOOL ret = MFALSE;
    switch(id)
    {
        case BMDENOISE_RESULT_TO_SWNR:
            mImgInfoRequests.enque(pImgInfo);
            ret = MTRUE;
            break;
        default:
            MY_LOGE("onData(ImgInfoMapPtr) unknown data id :%d", id);
            ret = MFALSE;
            break;
    }

    MY_LOGD("onData(ImgInfo) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_BEGIN("SWNRNode::onInit");
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }

    MY_LOGD("PreProcessNode::onInit=>new DpBlitStream");
    mpDpStream = new DpBlitStream();
    CAM_TRACE_END();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
onUninit()
{
    CAM_TRACE_NAME("SWNRNode::onUninit");
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    cleanUp();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
SWNRNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(mpDpStream!= nullptr)
        delete mpDpStream;
    mBufPool.uninit();
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
SWNRNode::
getISOFromMeta(IMetadata* pMeta)
{
    MINT32 ISO = 0;
    MINT32 debugISO = 0;
    IMetadata exifMeta;

    if( tryGetMetadata<IMetadata>(pMeta, MTK_3A_EXIF_METADATA, exifMeta) ) {
        if(!tryGetMetadata<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, ISO)){
            MY_LOGE("Get ISO from meta failed, use default value:%d", ISO);
        }
    }
    else {
        MY_LOGE("no tag: MTK_3A_EXIF_METADATA, use default value:%d", ISO);
    }

    debugISO = ::property_get_int32("vendor.debug.bmdenoise.iso", -1);

    MY_LOGD("metaISO:%d/debugISO:%d", ISO, debugISO);

    if(debugISO != -1 && debugISO >= 0){
        ISO = debugISO;
    }
    return ISO;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
doSoftwareNR(PipeRequestPtr request, ImgInfoMapPtr imgInfo)
{
    if(miEnableSWNR != 1){
        MY_LOGD("skip SWNR");
        return MFALSE;
    }
    CAM_TRACE_CALL();
    MINT32 reqNo = request->getRequestNo();
    MY_LOGD("+, reqID=%d", reqNo);

    sp<IStopWatchCollection> pStopWatchCollection = request->getStopWatchCollection();

    // preprare data
    // input
    SmartImageBuffer smpBuf_denoise_result = imgInfo->getSmartBuffer(BID_DENOISE_FINAL_RESULT);
    SmartImageBuffer smpBuf_temp_YUV = mBufPool.getBufPool(BID_SWNR_IN_YV12)->request();
    IMetadata* pMeta_InHal_main1 = request->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_InHal_main2 = request->getMetadata(BID_META_IN_HAL_MAIN2);
    IMetadata* pMeta_OutHal_main1 = request->getMetadata(BID_META_OUT_HAL);
    IMetadata* pMeta_inApp = request->getMetadata(BID_META_IN_APP);

    // output
    sp<IImageBuffer> pBuf_out_yuv = request->getImageBuffer(BID_JPEG_YUV);

    if(pBuf_out_yuv == nullptr){
        MY_LOGE("pBuf_out_yuv is nullptr!");
        return MFALSE;
    }

    // format transform
    {
        sp<IStopWatch> pStopWatch = pStopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_software_nr:input_format_convert");
        if (!formatConverter(smpBuf_denoise_result->mImageBuffer.get(), smpBuf_temp_YUV->mImageBuffer.get())) {
            MY_LOGE("doSoftwareNR input format convert failed");
            return MFALSE;
        }
    }

    auto dumpBuffer = [&](SmartImageBuffer& smpBuf, const char* name, const char* postfix){
        if(shouldDumpRequest(request)){
            char filepath[1024];
            snprintf(filepath, 1024, "/sdcard/bmdenoise/%d/%s", reqNo, getName());

            // make path
            MY_LOGD("makePath: %s", filepath);
            makePath(filepath, 0660);

            char writepath[1024];
            snprintf(writepath,
                1024, "%s/%s_%dx%d_%d.%s",
                filepath, name,
                smpBuf->mImageBuffer->getImgSize().w, smpBuf->mImageBuffer->getImgSize().h, smpBuf->mImageBuffer->getBufStridesInBytes(0),
                postfix
            );

            smpBuf->mImageBuffer->saveToFile(writepath);
        }
    };

    #ifdef BIT_TRUE
        dumpBuffer(smpBuf_temp_YUV, "BID_SWNR_IN_YV12", ".yuv");
    #endif

    // do SWNR
    {
        sp<IStopWatch> pStopWatch = pStopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_software_nr:do_swnr");

        MINT32 sensorIdx = mSensorIdx_Main1;
        MINT32 iso = 0;
        MINT32 isMFHR = request->getParam(PID_MFHR_IS_ON);

        MY_LOGD("MFHR isON : %d", isMFHR);
        if(isMFHR == 1){
            sensorIdx = mSensorIdx_Main2;
            iso = getISOFromMeta(pMeta_InHal_main2);
        }else{
            sensorIdx = mSensorIdx_Main1;
            iso = getISOFromMeta(pMeta_InHal_main1);
        }

        ISwNR::SWNRParam swnrParam;
        swnrParam.isDualCam = MTRUE;
        std::unique_ptr<ISwNR> swnr = std::unique_ptr<ISwNR>(MAKE_SwNR(sensorIdx));
        swnrParam.iso = iso;
        MY_LOGD("Run SwNR with iso=%d, sensorIdx:%d", swnrParam.iso, sensorIdx);
        {
            if (!swnr->doSwNR(swnrParam, smpBuf_temp_YUV->mImageBuffer.get())) {
                MY_LOGE("SWNR failed!");
                return MFALSE;
            }
            else {
                MY_LOGD("SWNR successed");

                if (pMeta_OutHal_main1 != nullptr){
                    MY_LOGD("SWNR update exif to hal out");
                    trySetMetadata<MUINT8>(pMeta_OutHal_main1, MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
                    swnr->getDebugInfo((*pMeta_OutHal_main1));
                }
            }
        }
    }

    #ifdef BIT_TRUE
        dumpBuffer(smpBuf_temp_YUV, "BID_SWNR_OUT_YV12", ".yuv");
    #endif

    // MDP rotate, resize & format transform to output YUV
    {
        sp<IStopWatch> pStopWatch = pStopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_software_nr:mdp_convert_result");
        sMDP_Config config;
        config.pSrcBuffer = smpBuf_temp_YUV->mImageBuffer.get();
        config.pDstBuffer = pBuf_out_yuv.get();
        config.pDpStream = mpDpStream;
        config.rotAngle = eTransformToDegree(getJpegRotation(pMeta_inApp));
        if(!excuteMDP(config)){
            MY_LOGE("convert result failed!");
        }
    }

    MY_LOGD("-, reqID=%d", reqNo);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    initBufferPool();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    ImgInfoMapPtr imgInfo = nullptr;
    PipeRequestPtr pipeRequest = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }
    if( !mImgInfoRequests.deque(imgInfo) )
    {
        MY_LOGD("mImgInfoRequests.deque() failed");
        return MFALSE;
    }
    if( !mRequests.deque(pipeRequest) )
    {
        MY_LOGD("mRequests.deque() failed");
        return MFALSE;
    }

    this->incExtThreadDependency();
    {
        sp<IStopWatch> pStopWatch  = pipeRequest->getStopWatchCollection()->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_software_nr");
        if(!doSoftwareNR(pipeRequest, imgInfo)){
            MY_LOGE("failed doing doPostProcess, please check above errors!");
            this->decExtThreadDependency();
            return MFALSE;
        }
    }

    dumpRequestData(BID_JPEG_YUV, pipeRequest);
    dumpRequestData(BID_THUMB_YUV, pipeRequest);
    handleData(SWNR_TO_HELPER, pipeRequest);
    handleData(EXIT_HIGH_PERF, pipeRequest->getRequestNo());
    this->decExtThreadDependency();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
SWNRNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    int allocateSize = 1;
    int tuningSize = 0;

    StereoArea area;

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_SWNR_IN_YV12);
        NSBMDN::BufferConfig c = {
            "BID_SWNR_IN_YV12",
            BID_SWNR_IN_YV12,
            (MUINT32)area.size.w,
            (MUINT32)area.size.h,
            eImgFmt_YV12,
            ImageBufferPool::USAGE_HW,
            MTRUE,
            MFALSE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }
    if(!mBufPool.init(vBufConfig, tuningSize)){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SWNRNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}
