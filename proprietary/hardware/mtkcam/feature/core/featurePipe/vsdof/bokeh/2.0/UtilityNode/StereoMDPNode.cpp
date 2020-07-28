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
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
//
#include "StereoMDPNode.h"
//
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "StereoMDPNode"
//
#include <PipeLog.h>
//
#include <DpBlitStream.h>
#include <vsdof/util/vsdof_util.h>
//
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;
/*******************************************************************************
 *
 ********************************************************************************/
StereoMDPNode::
StereoMDPNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BokehPipeNode(name, graph)
    , miOpenId(openId)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
    //this->addWaitQueue(&mWDMABufQueue);
}
/*******************************************************************************
 *
 ********************************************************************************/
StereoMDPNode::
~StereoMDPNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
onData(
    DataID id,
    EffectRequestPtr &request)
{
    FUNC_START;
    TRACE_FUNC_ENTER();

    MBOOL ret;
    switch(id)
    {
        case P2B_MDP_REQUEST:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            break;
    }

    TRACE_FUNC_EXIT();
    FUNC_END;
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
onInit()
{
    CAM_TRACE_NAME("StereoMDPNode::onInit");
    TRACE_FUNC_ENTER();
    FUNC_START;
    //
    mpDpStream = new DpBlitStream();
    //
    BokehPipeNode::onInit();
    FUNC_END;
    TRACE_FUNC_EXIT();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
onUninit()
{
    CAM_TRACE_NAME("StereoMDPNode::onUninit");
    FUNC_START;
    cleanUp();
    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
StereoMDPNode::
cleanUp()
{
    FUNC_START;
    if(mpDpStream!= nullptr)
        delete mpDpStream;
    FUNC_END;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
onThreadStart()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
onThreadLoop()
{
    //FUNC_START;
    EffectRequestPtr request = nullptr;
    SmartImageBuffer wdma_img = nullptr;

    if( !waitAllQueue() )
    {
        // mJobs.abort() called
        //MY_LOGD("waitAllQueue failed");
        return MFALSE;
    }
    if( !mRequests.deque(request) )
    {
        MY_LOGD("mRequests.deque() failed");
        return MFALSE;
    }
    CAM_TRACE_NAME("StereoMDPNode::onThreadLoop");
    // check scenario
    const sp<EffectParameter> params = request->getRequestParameter();
    MINT32 scenarioId = params->getInt(VSDOF_FRAME_SCENARIO);
    MINT32 gsensorOri = params->getInt(VSDOF_FRAME_G_SENSOR_ORIENTATION);
    //
    // only process in capture scenario.
    // In capture scenario, MDPnode needs to rotate CleanImg,
    // and create thumbnail image.
    if(scenarioId == StereoHAL::ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
    {
        if(!processCaptureRequest(request, gsensorOri))
        {
            MY_LOGE("processCaptureRequest fail.");
        }
    }
    handleData(MDP_OUT, request);
    //FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
processCaptureRequest(
    EffectRequestPtr request,
    MINT32 rot)
{
    CAM_TRACE_NAME("StereoMDPNode::processCaptureRequest");
    FrameInfoPtr pTNSrcFrame = nullptr;
    FrameInfoPtr pTNFrame = nullptr;
    FrameInfoPtr pCleanImgFrame = nullptr;
    FrameInfoPtr pInputMainFrame = nullptr;
    //
    sp<IImageBuffer> pTNSrcImgBuffer = nullptr;
    sp<IImageBuffer> pTNImgBuffer = nullptr;
    sp<IImageBuffer> pCleanImgBuffer = nullptr;
    sp<IImageBuffer> pInputMainImgBuffer = nullptr;
    // process vsdof image start
    // get dst buffer
    if(!getFrameInfoFromRequest(
                    request,
                    pTNSrcFrame,
                    BOKEH_ER_BUF_VSDOF_IMG,
                    MFALSE // VSDOF_IMG is put in out buffer list.
                    ))
    {
        MY_LOGE("Get VSDOF_IMG frame fail.");
        return MFALSE;
    }
    pTNSrcFrame->getFrameBuffer(pTNSrcImgBuffer);
    if(pTNSrcImgBuffer == nullptr)
    {
        MY_LOGE("Thumbnail image buffer is null");
        return MFALSE;
    }
    // get dst buffer (thumbnail)
    if(!getFrameInfoFromRequest(
                    request,
                    pTNFrame,
                    BOKEH_ER_BUF_THUMBNAIL,
                    MFALSE
                    ))
    {
        MY_LOGE("Get thumbnail frame fail.");
        return MFALSE;
    }
    pTNFrame->getFrameBuffer(pTNImgBuffer);
    if(pTNImgBuffer == nullptr)
    {
        MY_LOGE("Thumbnail image buffer is null");
        return MFALSE;
    }
    // excute MDP to scale down image
    //if(!processImageByMDP(pTNSrcImgBuffer, pTNImgBuffer, (360-rot)%360))
    if(!processImageByMDPWithCrop(pTNSrcImgBuffer, pTNImgBuffer, (360-rot)%360))
    {
        MY_LOGE("scaleDownImage fail.");
        return MFALSE;
    }
    // process vsdof image end
    //
    // process clean image rot process start
    // get dst buffer
    if(!getFrameInfoFromRequest(
                    request,
                    pCleanImgFrame,
                    BOKEH_ER_BUF_CLEAN_IMG,
                    MFALSE // VSDOF_IMG is put in out buffer list.
                    ))
    {
        MY_LOGE("Get CLEAN_IMG frame fail.");
        return MFALSE;
    }
    pCleanImgFrame->getFrameBuffer(pCleanImgBuffer);
    if(pCleanImgBuffer == nullptr)
    {
        MY_LOGE("clean image buffer is null");
        return MFALSE;
    }
    // get src
    if(!getFrameInfoFromRequest(
                    request,
                    pInputMainFrame,
                    BOKEH_ER_BUF_MAIN1,
                    MTRUE // VSDOF_IMG is put in out buffer list.
                    ))
    {
        MY_LOGE("Get VSDOF_IMG frame fail.");
        return MFALSE;
    }
    pInputMainFrame->getFrameBuffer(pInputMainImgBuffer);
    if(pInputMainImgBuffer == nullptr)
    {
        MY_LOGE("input main image buffer is null");
        return MFALSE;
    }
    // excute MDP to rotate image
    if(!processImageByMDP(pInputMainImgBuffer, pCleanImgBuffer, rot))
    {
        MY_LOGE("scaleDownImage fail.");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
processImageByMDP(
    sp<IImageBuffer> srcBuf,
    sp<IImageBuffer> dstBuf,
    MINT32 rot)
{
    CAM_TRACE_NAME("StereoMDPNode::processImageByMDP");
    if(mpDpStream == nullptr)
    {
        MY_LOGE("mpDpStream is null.");
        return MFALSE;
    }
    sMDP_Config config;
    config.pDpStream = mpDpStream;
    config.pSrcBuffer = srcBuf.get();
    config.pDstBuffer = dstBuf.get();
    config.rotAngle = rot;
    if(!excuteMDP(config))
    {
        MY_LOGE("excuteMDP fail.");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
processImageByMDPWithCrop(
    sp<IImageBuffer> srcBuf,
    sp<IImageBuffer> dstBuf,
    MINT32 rot)
{
    CAM_TRACE_NAME("StereoMDPNode::processImageByMDPWithCrop");
    if(mpDpStream == nullptr)
    {
        MY_LOGE("processImageByMDPWithCrop - mpDpStream is null.");
        return MFALSE;
    }
    sMDP_Config config;
    config.pDpStream = mpDpStream;
    config.pSrcBuffer = srcBuf.get();
    config.pDstBuffer = dstBuf.get();
    config.rotAngle = rot;
    if(!excuteMDPWithCrop(config))
    {
        MY_LOGE("excuteMDPWithCrop fail.");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoMDPNode::
getFrameInfoFromRequest(
    EffectRequestPtr request,
    FrameInfoPtr& frame,
    MINT32 bufType,
    MBOOL isInputPort)
{
    ssize_t keyIndex = -1;
    if(isInputPort)
    {
        keyIndex = request->vInputFrameInfo.indexOfKey(bufType);
        if(keyIndex>=0)
        {
            frame = request->vInputFrameInfo.valueAt(keyIndex);
            return MTRUE;
        }
        else
        {
            return MFALSE;
        }
    }
    else
    {
        keyIndex = request->vOutputFrameInfo.indexOfKey(bufType);
        if(keyIndex>=0)
        {
            frame = request->vOutputFrameInfo.valueAt(keyIndex);
            return MTRUE;
        }
        else
        {
            return MFALSE;
        }
    }
    return MFALSE;
}