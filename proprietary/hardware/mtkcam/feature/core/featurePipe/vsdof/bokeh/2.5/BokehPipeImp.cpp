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
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "BokehPipe"
//
#include <PipeLog.h>
#include <core/include/CamPipe.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
#include <mtkcam/utils/std/Trace.h>
//
#include "BokehPipeNode.h"
#include "BokehPipeImp.h"
//
using namespace NSCam::NSCamFeature::NSFeaturePipe;
//************************************************************************
MBOOL
BokehPipeImp::
init(const char *name)
{
    CAM_TRACE_NAME("BokehPipe::init");
    TRACE_FUNC_ENTER();
    MY_LOGD("Init pipe");
    (void)name;
    MBOOL ret;
    ret = PARENT_PIPE::init();
    TRACE_FUNC_EXIT();
    return ret;
}
//************************************************************************
MBOOL
BokehPipeImp::
uninit(const char *name)
{
    CAM_TRACE_NAME("BokehPipe::uninit");
    TRACE_FUNC_ENTER();
    (void)name;
    MBOOL ret;
    ret = PARENT_PIPE::uninit();
    TRACE_FUNC_EXIT();
    return ret;
}
//************************************************************************
MBOOL
BokehPipeImp::
enque(EffectRequestPtr &effectPtr)
{
    CAM_TRACE_NAME("BokehPipe::enque");
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    MUINT32 reqID = effectPtr->getRequestNo();
    // autolock for request map
    {
        android::Mutex::Autolock lock(mReqMapLock);
        mvRequestPtrMap.add(reqID, effectPtr);
    }
    ret = PARENT_PIPE::enque(ID_ROOT_ENQUE, effectPtr);
    TRACE_FUNC_EXIT();
    return ret;
}
//************************************************************************

MVOID
BokehPipeImp::
flush()
{
    CAM_TRACE_NAME("BokehPipe::flush");
    TRACE_FUNC_ENTER();
    MY_LOGD("BokehPipe::flush +:  req size=%d", mvRequestPtrMap.size());
    MBOOL ret = MFALSE;
    PARENT_PIPE::flush();

    // relase all the effectRequest
    android::Mutex::Autolock lock(mReqMapLock);
    for(size_t index=0;index<mvRequestPtrMap.size();++index)
    {
        MUINT32 iFlushReqID = mvRequestPtrMap.keyAt(index);

        EffectRequestPtr pEffectReq = mvRequestPtrMap.valueAt(index);
        // callback to pipeline node with FLUSH KEY
        pEffectReq->mpOnRequestProcessed(pEffectReq->mpTag, String8( BOKEH_FLUSH_KEY ), pEffectReq);
    }
    // clear all request map
    mvRequestPtrMap.clear();
    TRACE_FUNC_EXIT();
    MY_LOGD("-");
}
//************************************************************************

MVOID
BokehPipeImp::
sync()
{
    CAM_TRACE_NAME("BokehPipe::sync");
    TRACE_FUNC_ENTER();
    MY_LOGD("SYNC");
    MBOOL ret = MFALSE;
    PARENT_PIPE::sync();
    TRACE_FUNC_EXIT();
}
//************************************************************************
BokehPipeImp::BokehPipeImp(MINT32 openSensorIndex, MINT32 runPath)
  : CamPipe<BokehPipeNode>("BokehPipe"),
    mP2BNode("P2BNode", &mCamGraph, openSensorIndex, runPath),
    mMDPNode("MDPNode", &mCamGraph, openSensorIndex),
    mVendorFilterNode("VendorFilterNode", &mCamGraph, openSensorIndex),
    miOpenId(openSensorIndex),
    mvRequestPtrMap()
{
    if(runPath == GENERAL)
    {
        MY_LOGD("GENERAL path");
        buildGeneralPath();
    }
    else if(runPath == HW_BOKEH)
    {
        MY_LOGD("HW_BOKEH");
        buildHWBokehPath();
    }
    else if(runPath == SW_BOKEH)
    {
        MY_LOGD("SW_BOKEH");
        buildSWBokehPath();
    }
    else if(runPath == VENDOR_BOKEH)
    {
        MY_LOGD("VENDOR_BOKEH");
        buildVendorBokehPath();
    }
    else
    {
        MY_LOGE("Path(%d) is not support.", runPath);
    }
    mMode = runPath;
    MY_LOGD("OpenId(%d) runPath(%d)", miOpenId, runPath);
}
//************************************************************************
BokehPipeImp::~BokehPipeImp()
{
    // must call dispose to free CamGraph
    this->dispose();
}
//************************************************************************
MBOOL BokehPipeImp::onInit()
{
    return MTRUE;
}
//************************************************************************
MVOID BokehPipeImp::onUninit()
{
}
//************************************************************************
MBOOL BokehPipeImp::onData(DataID id, EffectRequestPtr &data)
{
    android::String8 status;
    if(id == P2B_OUT)
    {
        status = "Done";
    }
    else if(id == VENDOR_OUT)
    {
        status = "Done";
    }
    else if(id == MDP_OUT)
    {
        status = "Done";
    }
    //
    if(data->mpOnRequestProcessed)
    {
        data->mpOnRequestProcessed(data->mpTag, status, data);
    }
    MUINT32 reqID = data->getRequestNo();
    // remove request
    {
        android::Mutex::Autolock lock(mReqMapLock);
        mvRequestPtrMap.removeItem(reqID);
    }
    return MTRUE;
}
//************************************************************************
MBOOL BokehPipeImp::onData(DataID id, FrameInfoPtr &data)
{
    MY_LOGE("Not implement.");
    return MTRUE;
}
//************************************************************************
MBOOL BokehPipeImp::onData(DataID id, SmartImageBuffer &data)
{
    MY_LOGE("Not implement.");
    return MTRUE;
}
//************************************************************************
void
BokehPipeImp::
buildGeneralPath()
{
    this->connectData(P2B_OUT, P2B_OUT, mP2BNode, this);
    this->connectData(P2B_MDP_REQUEST, P2B_MDP_REQUEST, mP2BNode, mMDPNode);
    this->connectData(MDP_OUT, MDP_OUT, mMDPNode, this);
    this->setRootNode(&mP2BNode);
}
//************************************************************************
void
BokehPipeImp::
buildHWBokehPath()
{
    this->connectData(P2B_OUT, P2B_OUT, mP2BNode, this);
    this->setRootNode(&mP2BNode);
}
//************************************************************************
void
BokehPipeImp::
buildSWBokehPath()
{
    this->connectData(P2B_MDP_REQUEST, P2B_MDP_REQUEST, mVendorFilterNode, mMDPNode);
    this->connectData(MDP_OUT, MDP_OUT, mMDPNode, this);
    this->setRootNode(&mVendorFilterNode);
}
//************************************************************************
void
BokehPipeImp::
buildVendorBokehPath()
{
    this->connectData(VENDOR_OUT, VENDOR_OUT, mVendorFilterNode, this);
    this->connectData(MDP_OUT, MDP_OUT, mMDPNode, this);
    this->connectData(VENDOR_TO_MDP, VENDOR_TO_MDP, mVendorFilterNode, mMDPNode);
    this->setRootNode(&mVendorFilterNode);
}
//************************************************************************
void
BokehPipeImp::
setStopMode(MBOOL flushOnStop)
{
    MY_LOGD("stopMode(%d)", flushOnStop);
    this->setFlushOnStop(flushOnStop);
}
//************************************************************************
MINT32
BokehPipeImp::
getPipeMode()
{
    return mMode;
}
