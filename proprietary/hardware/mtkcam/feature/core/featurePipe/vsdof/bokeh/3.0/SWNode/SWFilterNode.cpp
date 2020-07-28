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
//
#include "../bokeh_common.h"
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "SWFilterNode"
//
#include "SWFilterNode.h"
//
#include <PipeLog.h>
#include <string>
//
#include <chrono>
//
#include <DpBlitStream.h>

#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
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
SWFilterNode::
SWFilterNode(
    const char *name,
    Graph_T *graph,
    MINT8 mode  __attribute__((UNUSED)))
    : BokehPipeNode(name, graph)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
}
//************************************************************************
//
//************************************************************************
SWFilterNode::
~SWFilterNode()
{
    MY_LOGD("dctor(%p)", this);
}
//************************************************************************
//
//************************************************************************
MBOOL
SWFilterNode::
onInit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    // create algo hal
    mpBokehHal = std::shared_ptr<BOKEH_HAL>(
                 BOKEH_HAL::createInstance(),
                 [&](auto *p) -> void
                 {
                     MY_LOGD("release bokeh hal");
                     p->destroyInstance();
                     p = nullptr;
                 });
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
SWFilterNode::
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
SWFilterNode::
cleanUp()
{
    FUNC_START;
    mRequests.clear();
    // dump all queue size
    MY_LOGD("mRequests.size(%d)", mRequests.size());
    FUNC_END;
}
//************************************************************************
//
//************************************************************************
MBOOL
SWFilterNode::
onThreadStart()
{
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
SWFilterNode::
onThreadStop()
{
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
SWFilterNode::
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
SWFilterNode::
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
    CAM_TRACE_NAME("SWFilterNode::onThreadLoop");
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
SWFilterNode::
executeAlgo(
    EffectRequestPtr request)
{
    CAM_TRACE_BEGIN("SWFilterNode::GFHALRun");
    MY_LOGD_IF(miPipeLogEnable, "+");
    MINT32 bufferCheak = 1;
    auto getImageBuffer = [&request, &bufferCheak](
        BokehEffectRequestBufferType type,
        SWFilterNode::IN_OUT_TYPE in_out_type) -> IImageBuffer*
        {
            auto bufferTypeToString = [&type]()
            {
                if(BOKEH_ER_BUF_MAIN1 == type)
                {
                    return "BOKEH_ER_BUF_MAIN1";
                }
                else if(BOKEH_ER_BUF_DMBG == type)
                {
                    return "BOKEH_ER_BUF_DMBG";
                }
                else if(BOKEH_ER_BUF_HAL_DEPTHWRAPPER == type)
                {
                    return "BOKEH_ER_BUF_HAL_DEPTHWRAPPER";
                }
                else if(BOKEH_ER_BUF_EXTRADATA == type)
                {
                    return "BOKEH_ER_BUF_EXTRADATA";
                }
                else if(BOKEH_ER_BUF_VSDOF_IMG == type)
                {
                    return "BOKEH_ER_BUF_VSDOF_IMG";
                }
                else if(BOKEH_ER_BUF_DEPTHMAPWRAPPER == type)
                {
                    return "BOKEH_ER_BUF_DEPTHMAPWRAPPER";
                }
                else if(BOKEH_ER_BUF_APP_EXTRA_DATA == type)
                {
                    return "BOKEH_ER_BUF_APP_EXTRA_DATA";
                }
                return "";
            };
            ssize_t keyIndex = -1;
            sp<EffectFrameInfo> frameInfo = nullptr;
            sp<IImageBuffer> frame = nullptr;
            if(SWFilterNode::IN_OUT_TYPE::IN == in_out_type)
            {
                keyIndex = request->vInputFrameInfo.indexOfKey(type);
                if(keyIndex>=0)
                {
                    frameInfo = request->vInputFrameInfo.valueAt(keyIndex);
                    frameInfo->getFrameBuffer(frame);
                }
                else
                {
                    bufferCheak &= 0;
                    MY_LOGE("Get buffer fail. bufferType(%s)", bufferTypeToString());
                    return nullptr;
                }
            }
            else
            {
                keyIndex = request->vOutputFrameInfo.indexOfKey(type);
                if(keyIndex>=0)
                {
                    frameInfo = request->vOutputFrameInfo.valueAt(keyIndex);
                    frameInfo->getFrameBuffer(frame);
                }
                else
                {
                    bufferCheak &= 0;
                    MY_LOGE("Get buffer fail. bufferType(%s)", bufferTypeToString());
                    return nullptr;
                }
            }
            //
            if(frame.get() == nullptr)
            {
                bufferCheak &= 0;
                MY_LOGE("Buffer is invalid. bufferType(%s)", bufferTypeToString());
                return nullptr;
            }
            MY_LOGD("%s: %dx%d", bufferTypeToString(), frame->getImgSize().w, frame->getImgSize().h);
            bufferCheak &= 1;
            return frame.get();
    };
    MBOOL ret = MFALSE;
    //
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::duration<double> elap;
    BOKEH_HAL_PARAMS sBokeh_Hal_Params;
    BOKEH_HAL_OUTPUT sBokeh_Hal_Output;
    IImageBuffer* pExtraData = nullptr;
    MUINT32 extraDataSize = StereoSettingProvider::getExtraDataBufferSizeInBytes();
    MINT32  stereoProfile = request->getRequestParameter()->getInt(VSDOF_FRAME_SENSOR_PROFILE);
    auto pOutExtraData = getImageBuffer(BOKEH_ER_BUF_APP_EXTRA_DATA, SWFilterNode::IN_OUT_TYPE::OUT);
    auto pOutAppDepthWrapper = getImageBuffer(BOKEH_ER_BUF_DEPTHMAPWRAPPER, SWFilterNode::IN_OUT_TYPE::OUT);
    if(mpBokehHal == nullptr)
    {
        MY_LOGE("mpBokehHal is nullptr");
        goto lbExit;
    }
    // get working buffer from buffer pool
    // input
    pExtraData = getImageBuffer(BOKEH_ER_BUF_EXTRADATA, SWFilterNode::IN_OUT_TYPE::IN);
    sBokeh_Hal_Params.extraData = (char *)(pExtraData->getBufVA(0));
    sBokeh_Hal_Params.requestNumber = request->getRequestNo();
    sBokeh_Hal_Params.cleanImage = getImageBuffer(BOKEH_ER_BUF_MAIN1, SWFilterNode::IN_OUT_TYPE::IN);
    sBokeh_Hal_Params.packedDepthmap = getImageBuffer(BOKEH_ER_BUF_HAL_DEPTHWRAPPER, SWFilterNode::IN_OUT_TYPE::IN);

    if (stereoProfile != -1)
    {
        sBokeh_Hal_Params.stereoProfile = (ENUM_STEREO_SENSOR_PROFILE)stereoProfile;
    }
    // output
    sBokeh_Hal_Output.bokehImage = getImageBuffer(BOKEH_ER_BUF_VSDOF_IMG, SWFilterNode::IN_OUT_TYPE::OUT);
    start = std::chrono::system_clock::now();
    // do algo here
    if(bufferCheak == 1)
    {
        // temp skip
        mpBokehHal->Run(sBokeh_Hal_Params, sBokeh_Hal_Output);
    }
    else
    {
        MY_LOGE("some buffer is missing, please check");
        goto lbExit;
    }
    // copy extra data info to output buffer
    if(pOutExtraData != nullptr)
    {
        ::memcpy((void*)pOutExtraData->getBufVA(0), (void*)pExtraData->getBufVA(0), extraDataSize);
    }
    else
    {
        MY_LOGD("output extra data is null, please check");
    }
    if(pOutAppDepthWrapper != nullptr)
    {
        MUINT32 targetSize = pOutAppDepthWrapper->getBufSizeInBytes(0);
        MUINT32 sourceSize = sBokeh_Hal_Params.packedDepthmap->getBufSizeInBytes(0);
        MY_LOGD("copy out source(%d) target(%d)", sourceSize, targetSize);
        ::memcpy(
                    (void*)pOutAppDepthWrapper->getBufVA(0),
                    (void*)sBokeh_Hal_Params.packedDepthmap->getBufVA(0),
                    sourceSize);
    }
    else
    {
        MY_LOGD("output depth wrapper data is null, please check");
    }
    //
    ret = MTRUE;
lbExit:
    elap = std::chrono::system_clock::now() - start;
    VSDOF_PRFLOG("VSDOF_Profile: sw algo processing time(%lf ms) reqID=%d", elap.count()*1000, request->getRequestNo());
    //
    handleData(P2B_MDP_REQUEST, request);
    MY_LOGD_IF(miPipeLogEnable, "-");
    CAM_TRACE_END();
    return ret;
}
