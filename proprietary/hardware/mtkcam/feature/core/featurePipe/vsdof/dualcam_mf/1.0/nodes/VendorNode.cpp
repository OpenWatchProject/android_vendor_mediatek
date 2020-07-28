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
#include "../util/vsdof_util.h"

#include "VendorNode.h"

#define PIPE_MODULE_TAG "DualCamMF"
#define PIPE_CLASS_TAG "VendorNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <PipeLog.h>

#include <mtkcam/feature/stereo/hal/stereo_common.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;
using namespace VSDOF::util;

#define DO_VENDOR_JOB "DO_VENDOR_JOB"

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

/*******************************************************************************
 *
 ********************************************************************************/
VendorNode::
VendorNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : DualCamMFPipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
VendorNode::
~VendorNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VendorNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MFALSE;
    Mutex::Autolock _l(mLock);
    switch(id)
    {
        case PREPROCESS_TO_VENDOR:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VendorNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_CALL();
    if(!DualCamMFPipeNode::onInit()){
        MY_LOGE("DualCamMFPipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VendorNode::
onUninit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    cleanUp();

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VendorNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mBufPool.uninit();
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VendorNode::
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
VendorNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VendorNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    // block until queue ready, or flush() breaks the blocking state too.
    if( !waitAllQueue() ){
        return MFALSE;
    }

    list<PipeRequestPtr> vToDoRequests;
    {
        Mutex::Autolock _l(mLock);

        PipeRequestPtr pipeRequest = nullptr;
        if( !mRequests.deque(pipeRequest) )
        {
            MY_LOGD("mRequests.deque() failed");
            return MFALSE;
        }

        this->incExtThreadDependency();

        addPendingRequests(pipeRequest);

        getReadyData(vToDoRequests);
    }

    CAM_TRACE_CALL();

    if(!vToDoRequests.empty()){
        MY_LOGD("data ready");
        doVendorJob(vToDoRequests);
    }else{
        MY_LOGD("data NOT ready");
    }

    this->decExtThreadDependency();

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VendorNode::
addPendingRequests(PipeRequestPtr request)
{
    mvPendingRequests.push_back(request);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VendorNode::
getReadyData(list<PipeRequestPtr>& rvToDoRequests)
{
    PipeRequestPtr request = mvPendingRequests.front();

    auto frameNum = request->getParam(DualCamMFParamID::PID_FRAME_NUM);

    if(mvPendingRequests.size() < frameNum){
        MY_LOGD("(%d < %d) data not ready, keep waiting", mvPendingRequests.size(), frameNum);
    }else{
        for(int i=0 ; i < frameNum ; i++){
            if(mvPendingRequests.empty()){
                MY_LOGE("mvPendingRequests is empty!");
                return;
            }

            PipeRequestPtr request = mvPendingRequests.front();
            mvPendingRequests.pop_front();

            rvToDoRequests.push_back(request);
        }
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VendorNode::
doVendorJob(list<PipeRequestPtr>& rvToDoRequests)
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MET_START(DO_VENDOR_JOB);

    // enter high perf, auto restore perf setting after leaving this scope
    // AutoScenCtrl autoScen(PIPE_LOG_TAG, miOpenId);

    // Warning:
    // Can't Do anything here because of BSS reordering,
    // the yuv buffer may be early released already,
    // any read/write to these buffer is not allowed!

    // return data
    while(!rvToDoRequests.empty()){
        PipeRequestPtr request = rvToDoRequests.front();
        rvToDoRequests.pop_front();
        handleFinish(request);
    }

    MET_END(DO_VENDOR_JOB);
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VendorNode::
handleFinish(
    PipeRequestPtr request)
{
    MINT32 reqNo = request->getRequestNo();
    CAM_TRACE_FMT_BEGIN("VendorNode::handleFinish req(%d)", reqNo);
    MY_LOGD("+, reqID=%d", reqNo);

    MINT32 isMainFrame = request->getParam(DualCamMFParamID::PID_IS_MAIN);
    if(isMainFrame == 1){
        handleData(NORMAL_POSTVIEW, request);
        handleData(P2_DONE, request);
    }

    handleData(FINAL_RESULT, request);

    CAM_TRACE_FMT_END();
    MY_LOGD("-, reqID=%d", reqNo);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
VendorNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    Vector<NSDCMF::BufferConfig> vBufConfig;



    // alloc working buffer if needed
    // int allocateSize = 1;
    // {
    //     NSBMDN::BufferConfig c = {
    //         "BID_XXX",
    //         BID_XXX,
    //         (MUINT32)some_width,
    //         (MUINT32)some_height,
    //         eImgFmt_XXX,
    //         ImageBufferPool::USAGE_HW,
    //         MTRUE,
    //         MFALSE,
    //         (MUINT32)allocateSize
    //     };
    //     vBufConfig.push_back(c);
    // }

    if(!mBufPool.init(vBufConfig)){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
VendorNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}
