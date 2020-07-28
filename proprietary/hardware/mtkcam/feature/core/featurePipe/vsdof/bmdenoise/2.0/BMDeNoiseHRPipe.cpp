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


// Standard C header file
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>

// Module header file

// Local header file
#include "BMDeNoisePipe_Common.h"
#include "BMDeNoiseHRPipe.h"

/*******************************************************************************
* Global Define
********************************************************************************/
#define PIPE_MODULE_TAG "BMDeNoiseHR"
#define PIPE_CLASS_TAG "BMDeNoiseHRPipe"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#define THREAD_NAME       ("Cam@BMDeNoiseHRPipe")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

/*******************************************************************************
* External Function
********************************************************************************/

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BMDeNoiseHRPipe::BMDeNoiseHRPipe(MINT32 openSensorIndex)
  : CamPipe<BMDeNoisePipeNode>("BMDeNoiseHRPipe")
  , miOpenId(openSensorIndex)
  , mRootHRNode("RootHRNode", &mCamGraph, openSensorIndex)
  , mBayerPreProcessNode("BayerPreProcess", &mCamGraph, openSensorIndex)
  , mMonoPreProcessNode("MonoPreProcess", &mCamGraph, openSensorIndex)
  , mBSSNode("BSS", &mCamGraph, openSensorIndex)
  , mP2AFMNode("P2AFM", &mCamGraph, openSensorIndex)
  , mBMN3DNode("BMN3D", &mCamGraph, openSensorIndex)
  , mFusionNode("Fusion", &mCamGraph, openSensorIndex)
  , mMFNRNode("MFNR", &mCamGraph, openSensorIndex)
  , mSWNRNode("SWNR", &mCamGraph, openSensorIndex)
  , mBMHelperNode("BMHelper", &mCamGraph, openSensorIndex)
{
    MY_LOGD("OpenId(%d)", miOpenId);

    mbDebugLog = StereoSettingProvider::isLogEnabled(PERPERTY_BMDENOISE_NODE_LOG);
}

BMDeNoiseHRPipe::~BMDeNoiseHRPipe()
{
    // must call dispose to free CamGraph
    this->dispose();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
BMDeNoiseHRPipe::
init()
{
    CAM_TRACE_NAME("BMDeNoiseHRPipe::init");
    return PARENT::init();
}

MBOOL
BMDeNoiseHRPipe::
uninit()
{
    CAM_TRACE_NAME("BMDeNoiseHRPipe::uninit");
    requestExit();
    return PARENT::uninit();
}

MVOID
BMDeNoiseHRPipe::
sync()
{
    CAM_TRACE_NAME("BMDeNoiseHRPipe::sync");

    // must sync here to wait for all nodes started
    MY_LOGD("sync +");
    PARENT::sync();
    MY_LOGD("sync -");

    // start buffer allocator thread
    if(run(PIPE_LOG_TAG) != OK){
        MY_LOGE("thread run failed!");
        return;
    }
}

MVOID
BMDeNoiseHRPipe::
flush()
{
    CAM_TRACE_NAME("BMDeNoiseHRPipe::flush");
    // lauch the default flush operations
    CamPipe::flush();

    // relase all the effectRequest
    android::Mutex::Autolock lock(mReqMapLock);
    for(size_t index=0;index<mvRequestPtrMap.size();++index)
    {
        MUINT32 iFlushReqID = mvRequestPtrMap.keyAt(index);
        VSDOF_LOGD("flush ReqID = %d", iFlushReqID);

        PipeRequestPtr pPipeReq = mvRequestPtrMap.valueAt(index);

        // callback to pipeline node with FLUSH KEY
        sp<IBMDeNoisePipeCB> cb = pPipeReq->mwpCB.promote();
        if(cb != nullptr){
            cb->onEvent(IBMDeNoisePipeCB::EventId::eFlush, pPipeReq);
        }else{
            pPipeReq->mpOnRequestProcessed(pPipeReq->mpTag, String8( BMDENOISE_FLUSH_KEY ), pPipeReq);
        }
    }
    // clear all request map
    mvRequestPtrMap.clear();
}

MBOOL
BMDeNoiseHRPipe::
enque(PipeRequestPtr& request)
{
    CAM_TRACE_NAME("BMDeNoiseHRPipe::enque");
    MUINT32 reqID = request->getRequestNo();
    // autolock for request map
    {
        VSDOF_PRFLOG("request map add reqId=%d ", reqID);
        android::Mutex::Autolock lock(mReqMapLock);
        mvRequestPtrMap.add(reqID, request);
    }
    if(mvRequestPtrMap.size()>6)
        MY_LOGW("The size of queued request inside BMDeNoiseHRPipe is over 6.");
    return CamPipe::enque(ROOT_ENQUE, request);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
BMDeNoiseHRPipe::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    setupP2Operator();

    // node connection
    // MFHR pipe
    MY_LOGD("connectData...");
    // Root Node
    this->connectData(ROOT_HR_TO_BAYER_PREPROCESS,      ROOT_HR_TO_BAYER_PREPROCESS,       mRootHRNode,                 mBayerPreProcessNode);
    this->connectData(ROOT_HR_TO_FUSION,                ROOT_HR_TO_FUSION,                 mRootHRNode,                 mFusionNode);
    this->connectData(ROOT_HR_TO_BSS,                   ROOT_HR_TO_BSS,                    mRootHRNode,                 mBSSNode);
    // BSS Node
    this->connectData(BSS_TO_P2AFM,                     BSS_TO_P2AFM,                      mBSSNode,                    mP2AFMNode);
    this->connectData(BSS_RESULT,                       BSS_RESULT,                        mBSSNode,                    mMFNRNode);
    this->connectData(SHUTTER,                          SHUTTER,                           mBSSNode,                    this);
    // P2AFM Ndoe
    this->connectData(P2AFM_TO_N3D_FEFM_CCin,           P2AFM_TO_N3D_FEFM_CCin,            mP2AFMNode,                  mBMN3DNode);
    // BMN3D Node
    this->connectData(WPE_TO_DPE_WARP_RESULT,           WPE_TO_DPE_WARP_RESULT,            mBMN3DNode,                  mFusionNode);
    // Bayer PreProcess
    this->connectData(BAYER_TO_MONO_PREPROCESS,         BAYER_TO_MONO_PREPROCESS,          mBayerPreProcessNode,        mMonoPreProcessNode);
    // Mono PreProcess
    this->connectData(MONO_PREPROCESS_TO_FUSION,        MONO_PREPROCESS_TO_FUSION,         mMonoPreProcessNode,         mFusionNode);
    // Fusion Node
    this->connectData(FUSION_TO_MFNR,                   FUSION_TO_MFNR,                    mFusionNode,                 mMFNRNode);
    this->connectData(P2_DONE,                          P2_DONE,                           mFusionNode,                 this);
    // MFNR Node
    this->connectData(BMDENOISE_RESULT_TO_SWNR,         BMDENOISE_RESULT_TO_SWNR,          mMFNRNode,                   mSWNRNode);
    // SWNRNode
    this->connectData(SWNR_TO_HELPER,                   SWNR_TO_HELPER,                    mSWNRNode,                   mBMHelperNode);
    this->connectData(EXIT_HIGH_PERF,                   EXIT_HIGH_PERF,                    mSWNRNode,                   mBMHelperNode);
    // HelperNode
    this->connectData(DENOISE_FINAL_RESULT_OUT,         DENOISE_FINAL_RESULT_OUT,          mBMHelperNode,               this);
    // P2_DONE is called by helper node when debug dump is enabled
    this->connectData(P2_DONE,                          P2_DONE,                           mBMHelperNode,               this);

    this->setRootNode(&mRootHRNode);

    MY_LOGD("push buffer producer nodes to list...");
    mvBufferProducerNodes.push_back(&mP2AFMNode);
    mvBufferProducerNodes.push_back(&mBMN3DNode);
    mvBufferProducerNodes.push_back(&mBayerPreProcessNode);
    mvBufferProducerNodes.push_back(&mMonoPreProcessNode);
    mvBufferProducerNodes.push_back(&mFusionNode);
    mvBufferProducerNodes.push_back(&mMFNRNode);

    return MTRUE;
}

MVOID
BMDeNoiseHRPipe::
onUninit()
{
}

MBOOL
BMDeNoiseHRPipe::
onData(DataID id, PipeRequestPtr &request)
{
    android::String8 status;
    status = BMDENOISE_COMPLETE_KEY;
    MUINT32 reqID = request->getRequestNo();

    MY_LOGD("recieve reqId:%d DataID:%d mvRequestPtrMap.size:%d +", reqID, id, mvRequestPtrMap.size());

    sp<IBMDeNoisePipeCB> cb = request->mwpCB.promote();
    if(cb != nullptr){
        switch(id){
            case DENOISE_FINAL_RESULT_OUT:
                cb->onEvent(IBMDeNoisePipeCB::EventId::eCompleted, request);
                break;
            case P2_DONE:
                cb->onEvent(IBMDeNoisePipeCB::EventId::eP2Done, request);
                break;
            case SHUTTER:
                cb->onEvent(IBMDeNoisePipeCB::EventId::eShutter, request);
                break;
            case ERROR_OCCUR_NOTIFY:
                cb->onEvent(IBMDeNoisePipeCB::EventId::eFailed, request);
                break;
            default:
                MY_LOGE("unknown DataID:%d", id);
        }
    }else{
        if(id == DENOISE_FINAL_RESULT_OUT){
            request->mpOnRequestProcessed(request->mpTag, String8( status ), request);
        }
    }

    // remove request
    if(id == DENOISE_FINAL_RESULT_OUT){
        android::Mutex::Autolock lock(mReqMapLock);
        mvRequestPtrMap.removeItem(reqID);
    }

    MY_LOGD("recieve reqId:%d DataID:%d mvRequestPtrMap.size:%d -", reqID, id, mvRequestPtrMap.size());
    return MTRUE;
}

MBOOL
BMDeNoiseHRPipe::
onData(DataID id, EffectRequestPtr &data)
{
    MY_LOGE("Not implemented.");
    return UNKNOWN_ERROR;
}

MBOOL
BMDeNoiseHRPipe::
onData(DataID id, FrameInfoPtr &data)
{
    MY_LOGE("Not implemented.");
    return MTRUE;
}

MBOOL
BMDeNoiseHRPipe::
onData(DataID id, SmartImageBuffer &data)
{
    MY_LOGE("Not implemented.");
    return MTRUE;
}

MVOID
BMDeNoiseHRPipe::
setupP2Operator()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    mspP2Op = new P2Operator(PIPE_LOG_TAG, miOpenId);

    mBayerPreProcessNode.setP2Operator(mspP2Op);
    mMonoPreProcessNode.setP2Operator(mspP2Op);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
BMDeNoiseHRPipe::
requestExit()
{
    mbThreadExit = MTRUE;
    Thread::requestExitAndWait();
}

bool
BMDeNoiseHRPipe::
threadLoop(
)
{
    FUNC_START;

    MBOOL needLoop = MFALSE;

    // loop-through all producer nodes
    for(auto e:mvBufferProducerNodes){
        MBOOL ret = e->doBufferPoolAllocation(1);

        MY_LOGD("[%s] ret:%d", e->getName(), ret);

        if(ret){
            needLoop = MTRUE;
        }
    }

    MY_LOGD("needLoop(%d) threadExit(%d)", needLoop, mbThreadExit);
    return needLoop;
}

status_t
BMDeNoiseHRPipe::
readyToRun(
)
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)THREAD_NAME, 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, THREAD_PRIORITY);
    //
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "tid(%d) policy(%d) priority(%d)"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;
}

}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam