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
//
#include "bokeh_common.h"
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "GuidedFilterNode"
//
#include "GuidedFilterNode.h"
//
#include <PipeLog.h>
#include <string>
//
#include <chrono>
using namespace std;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
//************************************************************************
// utility function
//************************************************************************
#define NS_PER_SEC  1000000000
#define NS_PER_MS   1000000
#define NS_PER_US   1000
static void Wait(int ms)
{
  long waitSec;
  long waitNSec;
  waitSec = (ms * NS_PER_MS) / NS_PER_SEC;
  waitNSec = (ms * NS_PER_MS) % NS_PER_SEC;
  struct timespec t;
  t.tv_sec = waitSec;
  t.tv_nsec = waitNSec;
  if( nanosleep(&t, NULL) != 0 )
  {
  }
}
//************************************************************************
//
//************************************************************************
GuidedFilterNode::
GuidedFilterNode(
    const char *name,
    Graph_T *graph,
    MINT8 mode)
    : BokehPipeNode(name, graph),
      mGFMode(mode)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
}
//************************************************************************
//
//************************************************************************
GuidedFilterNode::
~GuidedFilterNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
onInit()
{
    CAM_TRACE_NAME("GuidedFilterNode::onInit");
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    ENUM_STEREO_SCENARIO scenario = eSTEREO_SCENARIO_PREVIEW;
    const NSCam::EImageFormat imgFmt = eImgFmt_STA_BYTE;
    const MUINT32 usage = ImageBufferPool::USAGE_SW|ImageBufferPool::USAGE_HW;

    CAM_TRACE_BEGIN("GuidedFilterNode::CreateImageBufferPool");
    // create DMG buffer pool
    if(mGFMode==ADVANCE)
    {
        MY_LOGD("GF mode: ADVANCE");
        // DMBG size has same size in preview, record and capture mode.
        StereoArea sOutputDMGBufferArea =
                StereoSizeProvider::getInstance()->getBufferSize(E_DMG, scenario);
        ret = createBufferPool(
                        mpDMGBufPool,
                        sOutputDMGBufferArea.size.w,
                        sOutputDMGBufferArea.size.h,
                        imgFmt,
                        GF_PIPELINE_FRAME_DMG_BUFFER_SIZE,
                        "mpDMGBufPool",
                        usage);
        SmartImageBuffer sbDMGBuffer = nullptr;
        for(int i=0;i<GF_PIPELINE_FRAME_DMG_BUFFER_SIZE;++i)
        {
            sbDMGBuffer = mpDMGBufPool->request();
            setImageBufferValue(
                        sbDMGBuffer,
                        sOutputDMGBufferArea.size.w,
                        sOutputDMGBufferArea.size.h,
                        128);
            sbDMGBuffer = nullptr; // return to pool.
        }
        MY_LOGD("DMGB buffer depth(%d) fmt(%d) size(%dx%d) usage(%d)",
                        GF_PIPELINE_FRAME_DMG_BUFFER_SIZE,
                        imgFmt,
                        sOutputDMGBufferArea.size.w,
                        sOutputDMGBufferArea.size.h,
                        usage);
    }
    else
    {
        MY_LOGD("GF mode: GF_NODE_MODE_NORMAL");
    }
    // create DMBG buffer pool
    StereoArea sOutputDMBGBufferArea =
            StereoSizeProvider::getInstance()->getBufferSize(E_DMBG, scenario);
    ret = ret && createBufferPool(
                    mpDMBGBufPool,
                    sOutputDMBGBufferArea.size.w,
                    sOutputDMBGBufferArea.size.h,
                    imgFmt,
                    GF_PIPELINE_FRAME_DMBG_BUFFER_SIZE,
                    "mpDMBGBufPool",
                    usage);
    if(!ret)
    {
        MY_LOGE("Create buffer pool fail.");
        cleanUp();
    }
    CAM_TRACE_END();

    MY_LOGD("DMGB buffer depth(%d) fmt(%d) size(%dx%d) usage(%d)",
                    GF_PIPELINE_FRAME_DMG_BUFFER_SIZE,
                        imgFmt,
                        sOutputDMBGBufferArea.size.w,
                        sOutputDMBGBufferArea.size.h,
                        usage);

    CAM_TRACE_BEGIN("GuidedFilterNode::GF_HAL::createInstance");
    // create gf_hal
    mpGf_Hal = GF_HAL::createInstance(scenario);
    CAM_TRACE_END();
    if(!mpGf_Hal)
    {
        MY_LOGE("Create GF_HAL fail.");
        cleanUp();
    }
    //
    BokehPipeNode::onInit();
    //
    ret = MTRUE;
    FUNC_END;
    return ret;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
createBufferPool(
    android::sp<ImageBufferPool> &pPool,
    MUINT32 width,
    MUINT32 height,
    NSCam::EImageFormat format,
    MUINT32 bufCount,
    const char* caller,
    MUINT32 bufUsage)
{
    FUNC_START;
    MBOOL ret = MFALSE;
    pPool = ImageBufferPool::create(caller, width, height, format, bufUsage);
    if(pPool == nullptr)
    {
        ret = MFALSE;
        MY_LOGE("Create [%s] failed.", caller);
        goto lbExit;
    }
    for(MUINT32 i=0;i<bufCount;++i)
    {
        if(!pPool->allocate())
        {
            ret = MFALSE;
            MY_LOGE("Allocate [%s] working buffer failed.", caller);
            goto lbExit;
        }
    }
    ret = MTRUE;
    FUNC_END;
lbExit:
    return ret;
}
//************************************************************************
//
//************************************************************************
MVOID
GuidedFilterNode::
setImageBufferValue(SmartImageBuffer& buffer,
                MINT32 width,
                MINT32 height,
                MINT32 value)
{
    MUINT8* data = (MUINT8*)buffer->mImageBuffer->getBufVA(0);
    memset(data, value, sizeof(MUINT8) * width * height);
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
onUninit()
{
    FUNC_START;
    cleanUp();
    FUNC_END;
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MVOID
GuidedFilterNode::
cleanUp()
{
    FUNC_START;
    mRequests.clear();
    if(mGFMode == ADVANCE)
    {
        ImageBufferPool::destroy(mpDMGBufPool);
    }
    ImageBufferPool::destroy(mpDMBGBufPool);
    // release gf_hal
    delete mpGf_Hal;
    mpGf_Hal = nullptr;
    // dump all queue size
    MY_LOGD("mRequests.size(%d)", mRequests.size());
    FUNC_END;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
onThreadStart()
{
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
onThreadStop()
{
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
onData(
    DataID data,
    EffectRequestPtr &request)
{
    TRACE_FUNC_ENTER();
    VSDOF_PRFLOG("reqID=%d +", request->getRequestNo());
    MBOOL ret = MFALSE;
    //
    if(request->vInputFrameInfo.size() == 0)
    {
        MY_LOGE("vInputFrameInfo.size() is 0");
        return ret;
    }
    //
    if(request->vOutputFrameInfo.size() == 0)
    {
        MY_LOGE("vOutputFrameInfo.size() is 0");
        return ret;
    }
    //
    switch(data)
    {
        case ID_ROOT_ENQUE:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            break;
    }
    //
    VSDOF_PRFLOG("-");
    TRACE_FUNC_EXIT();
    return ret;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
onThreadLoop()
{
    EffectRequestPtr request;
    //
    if( !waitAllQueue() )
    {
        // mJobs.abort() called
        return MFALSE;
    }
    //
    if( !mRequests.deque(request) )
    {
        return MFALSE;
    }
    CAM_TRACE_NAME("GuidedFilterNode::onThreadLoop");
    //
    if(!executeAlgo(request))
    {
        return MFALSE;
    }
    //
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
executeAlgo(
    EffectRequestPtr request)
{
    CAM_TRACE_BEGIN("GuidedFilterNode::GFHALRun");
    MY_LOGD_IF(miPipeLogEnable, "+");
    MBOOL ret = MFALSE;
    //
    GF_HAL_IN_DATA sInData;
    GF_HAL_OUT_DATA sOutData;
    //
    SmartImageBuffer smDMGImgBuf = nullptr;
    SmartImageBuffer smDMBGImgBuf = nullptr;
    //
    sp<IImageBuffer> pMYS = nullptr;
    sp<IImageBuffer> pDMW = nullptr;
    sp<EffectParameter> pEffectParameter = nullptr;
    //

    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::duration<double> elap;
    //
    if(mpGf_Hal == nullptr)
    {
        MY_LOGE("GF_Hal needs initial first.");
        return MFALSE;
    }
    //
    if(!requireAlgoDataFromRequest(request, sInData, pMYS, pDMW))
    {
        MY_LOGE("get algo buffer fail.");
        ret = MFALSE;
        sInData.depthMap = nullptr;
        sInData.images.clear();
        goto lbExit;
    }
    //
    sInData.depthMap = (MUINT8*)pDMW->getBufVA(0);
    sInData.images.push_back(pMYS.get());
    //
    // create DMG and DMBG buffer for algo.
    if((mGFMode == ADVANCE)&&
			(mpDMGBufPool==nullptr))
    {
        MY_LOGE("mpDMGBufPool is null");
        ret = MFALSE;
        goto lbExit;
    }
    if(mpDMBGBufPool==nullptr)
    {
        MY_LOGE("mpDMBGBufPool is null");
        ret = MFALSE;
        goto lbExit;
    }
    // get working buffer from buffer pool
    if(mGFMode == ADVANCE)
    {
        smDMGImgBuf = mpDMGBufPool->request();
    }
    smDMBGImgBuf = mpDMBGBufPool->request();
    sOutData.dmbg = (MUINT8*)smDMBGImgBuf->mImageBuffer->getBufVA(0);
    //
    start = std::chrono::system_clock::now();
    if(!mpGf_Hal->GFHALRun(sInData, sOutData))
    {
        MY_LOGE("GFHalRun fail");
        // release data
        smDMGImgBuf = nullptr;
        smDMBGImgBuf = nullptr;
        goto lbExit;
    }
    elap = std::chrono::system_clock::now() - start;
    VSDOF_PRFLOG("VSDOF_Profile: gf algo processing time(%lf ms) reqID=%d", elap.count()*1000, request->getRequestNo());

    //
    if(mbDumpImageBuffer)
    {
        pEffectParameter = request->getRequestParameter();
        string filePath = "/sdcard/vsdof/bokeh/result/gf/";
        MY_LOGD("Dump image(%d)+", request->getRequestNo());
        // dump image buffer
        makePath(filePath.c_str(), 0660);
        if(pEffectParameter->getInt(VSDOF_FRAME_SCENARIO) == eSTEREO_SCENARIO_CAPTURE)
        {
            filePath+="Cap_";
        }
        else if(pEffectParameter->getInt(VSDOF_FRAME_SCENARIO) == eSTEREO_SCENARIO_RECORD)
        {
            filePath+="Rec_";
        }
        else
        {
            filePath+="Prv_";
        }
        string saveFileName = "";
        if(mGFMode == ADVANCE)
        {
            saveFileName = filePath+string("DMG_")+
                           to_string(request->getRequestNo())+
                           string(".yuv");
            smDMGImgBuf->mImageBuffer->saveToFile(saveFileName.c_str());
        }
        saveFileName = filePath+string("DMBG_")+
                       to_string(request->getRequestNo())+
                       string(".yuv");
        smDMBGImgBuf->mImageBuffer->saveToFile(saveFileName.c_str());
        saveFileName = filePath+string("MYS_")+
                       to_string(request->getRequestNo())+
                       string(".yuv");
        pMYS->saveToFile(saveFileName.c_str());
        saveFileName = filePath+string("DMW_")+
                       to_string(request->getRequestNo())+
                       string(".yuv");
        pDMW->saveToFile(saveFileName.c_str());
        MY_LOGD("Dump image-");
    }
    //
    if(mGFMode == ADVANCE)
    {
        handleData(GF_BOKEH_DMG, smDMGImgBuf);
    }
    handleData(GF_BOKEH_REQUEST, request);
    handleData(GF_BOKEH_DMBG, smDMBGImgBuf);
    //
    ret = MTRUE;
    MY_LOGD_IF(miPipeLogEnable, "-");
    CAM_TRACE_END();
lbExit:
    return ret;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
requireAlgoDataFromRequest(
    const EffectRequestPtr request,
    GF_HAL_IN_DATA& inData,
    sp<IImageBuffer>& pMYS,
    sp<IImageBuffer>& pDMW)
{
    MY_LOGD_IF(miPipeLogEnable, "+");
    MBOOL ret = MFALSE;
    // check input buffer
    // it needs to contain MYS and DMW for GF node.
    ssize_t keyIndex = -1;
    sp<EffectFrameInfo> frameInfo = nullptr;
    MSize imgSize;
    // get EffectRequest parameter
    const sp<EffectParameter> pEffectParameter
                                = request->getRequestParameter();
    //
    // get MYS buffer.
    if(!getBufferFromRequest(request, BOKEH_ER_BUF_MYS, pMYS))
    {
        MY_LOGE("Get MYS fail.");
        goto lbExit;
    }
    // get DMW buffer.
    if(!getBufferFromRequest(request, BOKEH_ER_BUF_DMW, pDMW))
    {
        MY_LOGE("Get DMW fail.");
        goto lbExit;
    }
    // get Touch flag
    //inData.isAFTriggered = pEffectParameter->getInt(VSDOF_FRAME_AF_TRIGGER_FLAG);
    // get Touch point
    //inData.ptAF = MPoint(
    //                pEffectParameter->getInt(VSDOF_FRAME_AF_POINT_X),
    //                pEffectParameter->getInt(VSDOF_FRAME_AF_POINT_Y));
    // get DOF level
    inData.dofLevel = pEffectParameter->getInt(VSDOF_FRAME_BOKEH_LEVEL);
    // get scenario
    inData.scenario = (StereoHAL::ENUM_STEREO_SCENARIO)pEffectParameter->getInt(VSDOF_FRAME_SCENARIO);
    // get magic number
    inData.magicNumber = pEffectParameter->getInt(VSDOF_FEAME_MAGIC_NUMBER);
    // get convergence offset
    inData.convOffset = pEffectParameter->getFloat(VSDOF_FRAME_CONV_OFFSET);
    //
    if(miPipeLogEnable)
    {
        // printf buffer format.
        MY_LOGD("MY_S image buffer format (%d)", pMYS->getImgFormat());
        MY_LOGD("MY_S image buffer va1 (0x%x)", pMYS->getBufVA(0));
        MY_LOGD("MY_S image buffer va2 (0x%x)", pMYS->getBufVA(1));
        MY_LOGD("MY_S image buffer va3 (0x%x)", pMYS->getBufVA(2));
        MY_LOGD("DMW image buffer format (%d)", pDMW->getImgFormat());
        MY_LOGD("DMW image buffer va (0x%x)", pDMW->getBufVA(0));
        // print MY_S and DMW image buffer size.
        imgSize = pMYS->getImgSize();
        MY_LOGD("MY_S image buffer size w(%d) h(%d)", imgSize.w, imgSize.h);
        imgSize = pDMW->getImgSize();
        MY_LOGD("DMW image buffer size w(%d) h(%d)", imgSize.w, imgSize.h);
        //MY_LOGD("TouchAF flag(%d)", inData.isAFTriggered);
        //MY_LOGD("TouchAF point x(%d) y(%d)", inData.ptAF.x, inData.ptAF.y);
        MY_LOGD("scenario(%d)", inData.scenario);
        MY_LOGD("DOF level(%d)", inData.dofLevel);
        MY_LOGD("magicNumber(%d)", inData.magicNumber);
    }
    ret = MTRUE;
lbExit:
    MY_LOGD_IF(miPipeLogEnable, "-");
    return ret;
}
//************************************************************************
//
//************************************************************************
MBOOL
GuidedFilterNode::
getBufferFromRequest(
    const EffectRequestPtr request,
    BokehEffectRequestBufferType type,
    sp<IImageBuffer>& frame
)
{
    ssize_t keyIndex = -1;
    sp<EffectFrameInfo> frameInfo = nullptr;
    keyIndex = request->vInputFrameInfo.indexOfKey(type);
    if(keyIndex>=0)
    {
        frameInfo = request->vInputFrameInfo.valueAt(keyIndex);
        frameInfo->getFrameBuffer(frame);
    }
    else
    {
        MY_LOGE("Get buffer fail. bufferType(%d)", type);
        return MFALSE;
    }
    //
    if(frame.get() == nullptr)
    {
        MY_LOGE("Buffer is invalid. bufferType(%d)", type);
        return MFALSE;
    }
    return MTRUE;
}