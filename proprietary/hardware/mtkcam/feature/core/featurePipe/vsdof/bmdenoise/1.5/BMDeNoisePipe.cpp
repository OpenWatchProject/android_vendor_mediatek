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
#include <EGL/egl.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>

// Module header file

// Local header file
#include "BMDeNoisePipe_Common.h"
#include "BMDeNoisePipe.h"


/*******************************************************************************
* Global Define
********************************************************************************/
#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "BMDeNoisePipe"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#define THREAD_NAME       ("Cam@BMDeNoisePipe")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

#define SCOPE_LOGGER          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}

#define SCOPE_TIMER(VAR, TEXT)  auto VAR = create_scope_timer(TEXT)
#include <memory>
#include <functional>
#include <utils/SystemClock.h> // elapsedRealtime(), uptimeMillis()
static std::unique_ptr<const char, std::function<void(const char*)> >
create_scope_timer(const char* text)
{
    auto t1 = android::elapsedRealtimeNano();
    return std::unique_ptr<const char, std::function<void(const char*)> >
        (
            text,
            [t1](const char* t)->void
            {
                auto t2 = android::elapsedRealtimeNano();
                CAM_LOGD("BMDeNoise_Profile %s --> duration(ms): %" PRId64 "", t, (t2 -t1)/1000000);
            }
        );
}

#define TAKE_PCITURE "BMDNPipe takePic"

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
BMDeNoisePipe::BMDeNoisePipe(MINT32 openSensorIndex)
  : CamPipe<BMDeNoisePipeNode>("BMDeNoisePipe")
  , miOpenId(openSensorIndex)
  , mRootNode("RootNode", &mCamGraph, openSensorIndex)
  , mPreProcessNode("PreProcess", &mCamGraph, openSensorIndex)
  , mDeNoiseNode("DeNoise", &mCamGraph, openSensorIndex)
  , mSWNRNode("SWNR", &mCamGraph, openSensorIndex)
  , mP2AFMNode("P2AFM", &mCamGraph, openSensorIndex)
  , mPostProcessNode("PostProcess", &mCamGraph, openSensorIndex)
  , mBMN3DNode("BMN3D", &mCamGraph, openSensorIndex)
  , mDPENode("DPE", &mCamGraph, openSensorIndex)
  , mBMHelperNode("BMHelper", &mCamGraph, openSensorIndex)
{
    MY_LOGD("OpenId(%d)", miOpenId);

    mbDebugLog = StereoSettingProvider::isLogEnabled(PERPERTY_BMDENOISE_NODE_LOG);

    if(::property_get_int32("vendor.debug.bmdenoise.quickThumb", 1) == 1){
        mbQuickThumb = MTRUE;
    }

    MY_LOGD("mbDebugLog(%d) mbQuickThumb(%d) ", mbDebugLog, mbQuickThumb);
}

BMDeNoisePipe::~BMDeNoisePipe()
{
    // must call dispose to free CamGraph
    MY_LOGD("dctor(0x%x)", this);
    this->dispose();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
BMDeNoisePipe::
init()
{
    CAM_TRACE_NAME("BMDeNoisePipe::init");
    SCOPE_LOGGER;
    // init EGL for graphic buffers
    eglInitialize(eglGetDisplay(EGL_DEFAULT_DISPLAY), NULL, NULL);
    return PARENT::init();
}

MBOOL
BMDeNoisePipe::
uninit()
{
    CAM_TRACE_NAME("BMDeNoisePipe::uninit");
    SCOPE_LOGGER;
    requestExit();

    if(!mDeNoiseNode.releaseALG()){
        MY_LOGE("failed releasing denoise alg");
        return MFALSE;
    }

    MBOOL ret = PARENT::uninit();
    // uninit EGL for graphic buffers
    eglTerminate(eglGetDisplay(EGL_DEFAULT_DISPLAY));
    return ret;
}

MVOID
BMDeNoisePipe::
sync()
{
    CAM_TRACE_NAME("BMDeNoisePipe::sync");
    SCOPE_LOGGER;

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
BMDeNoisePipe::
flush()
{
    CAM_TRACE_NAME("BMDeNoisePipe::flush");
    SCOPE_LOGGER;
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
BMDeNoisePipe::
enque(PipeRequestPtr& request)
{
    SCOPE_LOGGER;
    MUINT32 reqID = request->getRequestNo();
    // autolock for request map
    {
        android::Mutex::Autolock lock(mReqMapLock);
        mvRequestPtrMap.add(reqID, request);
        mvCaptureRequestsTime.add(reqID, chrono::system_clock::now());
        accumulatedReqCnt ++;
        MY_LOGD("request map add reqId=%d mvRequestPtrMap.size:%d accumulatedReqCnt:%d", reqID, mvRequestPtrMap.size(), accumulatedReqCnt);
    }
    if(mvRequestPtrMap.size()>6)
        MY_LOGW("The size of queued request inside BMDeNoisePipe is over 6.");

    request->setStopWatchCollection(new StopWatchCollection(PIPE_CLASS_TAG, reqID));
    request->getStopWatchCollection()->BeginStopWatch("total_time", (void*)request.get());

    return CamPipe::enque(ROOT_ENQUE, request);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
BMDeNoisePipe::
onInit()
{
    SCOPE_LOGGER;
    setupP2Operator();

    // node connection
    // BMDN pipe
    MY_LOGD("connectData...");
    // // RootNode
    this->connectData(ROOT_TO_PREPROCESS,           ROOT_TO_PREPROCESS,         mRootNode,            mPreProcessNode);
    this->connectData(ROOT_TO_P2AFM,                ROOT_TO_P2AFM,              mRootNode,            mP2AFMNode);
    this->connectData(ROOT_TO_DENOISE,              ROOT_TO_DENOISE,            mRootNode,            mDeNoiseNode);
    this->connectData(SHUTTER,                      SHUTTER,                    mRootNode,            this);
    this->connectData(ERROR_OCCUR_NOTIFY,           ERROR_OCCUR_NOTIFY,         mRootNode,            this);
    // PreProcessNode
    this->connectData(PREPROCESS_TO_DENOISE,        PREPROCESS_TO_DENOISE,      mPreProcessNode,      mDeNoiseNode);
    this->connectData(POSTVIEW_DONE,                POSTVIEW_DONE,              mPreProcessNode,      this);
    //P2AFM to N3D
    this->connectData(P2AFM_TO_N3D_FEFM_CCin,       P2AFM_TO_N3D_FEFM_CCin,     mP2AFMNode,           mBMN3DNode);
    //N3D to DPE
    this->connectData(WPE_TO_DPE_WARP_RESULT,       WPE_TO_DPE_WARP_RESULT,     mBMN3DNode,           mDPENode);
    //DPE to Denoise
    this->connectData(DPE_TO_DENOISE_DMP_RESULT,    DPE_TO_DENOISE_DMP_RESULT,  mDPENode,             mDeNoiseNode);
    // DeNoiseNode
    this->connectData(DENOISE_TO_POSTPROCESS,       DENOISE_TO_POSTPROCESS,     mDeNoiseNode,         mPostProcessNode);
    this->connectData(ENTER_HIGH_PERF,              ENTER_HIGH_PERF,            mDeNoiseNode,         mBMHelperNode);
    this->connectData(P2_DONE,                      P2_DONE,                    mDeNoiseNode,         this);
    // PostProcessNode
    this->connectData(BMDENOISE_RESULT_TO_SWNR,     BMDENOISE_RESULT_TO_SWNR,   mPostProcessNode,     mSWNRNode);
    // SWNRNode
    this->connectData(SWNR_TO_HELPER,               SWNR_TO_HELPER,             mSWNRNode,            mBMHelperNode);
    this->connectData(EXIT_HIGH_PERF,               EXIT_HIGH_PERF,             mSWNRNode,            mBMHelperNode);
    // HelperNode
    this->connectData(DENOISE_FINAL_RESULT_OUT,     DENOISE_FINAL_RESULT_OUT,   mBMHelperNode,        this);

    this->setRootNode(&mRootNode);

    MY_LOGD("push buffer producer nodes to list...");
    mvBufferProducerNodes.push_back(&mPreProcessNode);
    mvBufferProducerNodes.push_back(&mDeNoiseNode);
    mvBufferProducerNodes.push_back(&mPostProcessNode);
    mvBufferProducerNodes.push_back(&mP2AFMNode);
    mvBufferProducerNodes.push_back(&mBMN3DNode);
    mvBufferProducerNodes.push_back(&mDPENode);
    mvBufferProducerNodes.push_back(&mSWNRNode);

    return MTRUE;
}

MVOID
BMDeNoisePipe::
onUninit()
{
    SCOPE_LOGGER;
}

MBOOL
BMDeNoisePipe::
onData(DataID id, PipeRequestPtr &request)
{
    android::String8 status;
    status = BMDENOISE_COMPLETE_KEY;
    MUINT32 reqID = request->getRequestNo();

    MY_LOGD("recieve reqId:%d DataID:%d mvRequestPtrMap.size:%d +", reqID, id, mvRequestPtrMap.size());

    android::Mutex::Autolock lock(mReqMapLock);

    MY_LOGE_IF(id == ERROR_OCCUR_NOTIFY, "Watch out! There is an ERROR_OCCUR_NOTIFY!");

    // remove request
    if(id == DENOISE_FINAL_RESULT_OUT || id == ERROR_OCCUR_NOTIFY){
        mvRequestPtrMap.removeItem(reqID);

        request->getStopWatchCollection()->EndStopWatch((void*)request.get());
        request->getStopWatchCollection()->PrintLog();

        chrono::time_point<chrono::system_clock> captureStartTime = mvCaptureRequestsTime.valueFor(reqID);
        mvCaptureRequestsTime.removeItem(reqID);

        chrono::duration<double> elapsed_seconds = chrono::system_clock::now()-captureStartTime;
        MY_LOGD("[BMDN][Benchmark]: BMDNPipe total time(%lf ms) reqID=%d",
            elapsed_seconds.count() *1000,
            reqID
        );
    }

    sp<IBMDeNoisePipeCB> cb = request->mwpCB.promote();
    if(cb != nullptr){
        switch(id){
            case DENOISE_FINAL_RESULT_OUT:
                cb->onEvent(IBMDeNoisePipeCB::EventId::eCompleted, request);
                if(!mRequestDumpHelper.isDoPorter()){
                    if(shouldDoP2Callback(id, reqID) || mRequestDumpHelper.isDoDump(reqID)){
                        cb->onEvent(IBMDeNoisePipeCB::EventId::eP2Done, request);
                    }
                }
                break;
            case P2_DONE:
                if(!mRequestDumpHelper.isDoPorter()){
                    if(shouldDoP2Callback(id, reqID) && !mRequestDumpHelper.isDoDump(reqID)){
                        cb->onEvent(IBMDeNoisePipeCB::EventId::eP2Done, request);
                    }
                }
                break;
            case SHUTTER:
                cb->onEvent(IBMDeNoisePipeCB::EventId::eShutter, request);
                break;
            case ERROR_OCCUR_NOTIFY:
                cb->onEvent(IBMDeNoisePipeCB::EventId::eFailed, request);
                break;
            case POSTVIEW_DONE:
                if(mbQuickThumb){
                    chrono::time_point<chrono::system_clock> captureStartTime = mvCaptureRequestsTime.valueFor(reqID);
                    chrono::duration<double> elapsed_seconds = chrono::system_clock::now()-captureStartTime;
                    MY_LOGD("[BMDN][Benchmark]: BMDNPipe quickThumb time(%lf ms) reqID=%d",
                        elapsed_seconds.count() *1000,
                        reqID
                    );
                    cb->onEvent(IBMDeNoisePipeCB::EventId::ePostview, request);
                }
                break;
            default:
                MY_LOGE("unknown DataID:%d", id);
        }
    }else{
        if(id == DENOISE_FINAL_RESULT_OUT){
            request->mpOnRequestProcessed(request->mpTag, String8( status ), request);
        }
    }

    MY_LOGD("recieve reqId:%d DataID:%d mvRequestPtrMap.size:%d -", reqID, id, mvRequestPtrMap.size());
    return MTRUE;
}

MBOOL
BMDeNoisePipe::
onData(DataID id, EffectRequestPtr &data)
{
    MY_LOGE("Not implemented.");
    return UNKNOWN_ERROR;
}

MBOOL
BMDeNoisePipe::
onData(DataID id, FrameInfoPtr &data)
{
    MY_LOGE("Not implemented.");
    return MTRUE;
}

MBOOL
BMDeNoisePipe::
onData(DataID id, SmartImageBuffer &data)
{
    MY_LOGE("Not implemented.");
    return MTRUE;
}

MVOID
BMDeNoisePipe::
setupP2Operator()
{
    SCOPE_LOGGER;
    mspP2Op = new P2Operator(PIPE_LOG_TAG, miOpenId);

    mPreProcessNode.setP2Operator(mspP2Op);
    mDeNoiseNode.setP2Operator(mspP2Op);
    mP2AFMNode.setP2Operator(mspP2Op);
    mPostProcessNode.setP2Operator(mspP2Op);
}

MBOOL
BMDeNoisePipe::
shouldDoP2Callback(DataID id, MINT32 reqNo)
{
    MBOOL ret = MFALSE;

    MINT32 fastS2S = ::property_get_int32("vendor.debug.bmdenoise.fastS2S", 1);
    MINT32 maxCapCnt = 1;

    if(fastS2S == 1){
        maxCapCnt = getBMDNCapturePhaseCnt();
    }

   if(mvRequestPtrMap.size() < maxCapCnt && accumulatedP2DoneCnt <= accumulatedReqCnt){
        ret = MTRUE;
        accumulatedP2DoneCnt ++;
    }else{
        ret = MFALSE;
    }

    MY_LOGD("fastS2S:%d reqNo:%d dataId:%d on-going reqs:%d, max:%d ret:%d accP2Dones/accReqs:%d/%d",
        fastS2S,
        reqNo,
        id,
        mvRequestPtrMap.size(),
        maxCapCnt,
        ret,
        accumulatedP2DoneCnt,
        accumulatedReqCnt
    );
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
BMDeNoisePipe::
requestExit()
{
    mbThreadExit = MTRUE;
    Thread::requestExitAndWait();
}

bool
BMDeNoisePipe::
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
BMDeNoisePipe::
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