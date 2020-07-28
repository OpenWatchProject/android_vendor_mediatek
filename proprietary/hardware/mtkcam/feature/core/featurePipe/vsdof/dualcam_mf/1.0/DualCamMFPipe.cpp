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
#include "DualCamMFPipe_Common.h"
#include "DualCamMFPipe.h"

/*******************************************************************************
* Global Define
********************************************************************************/
#define PIPE_MODULE_TAG "DualCamMF"
#define PIPE_CLASS_TAG "DualCamMFPipe"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <featurePipe/core/include/PipeLog.h>

#define THREAD_NAME       ("Cam@DualCamMFPipe")
#define THREAD_POLICY     (SCHED_OTHER)
#define THREAD_PRIORITY   (0)

#define SCOPE_LOGGER          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}
/*******************************************************************************
* External Function
********************************************************************************/

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSDCMF {
/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DualCamMFPipe::DualCamMFPipe(MINT32 openSensorIndex)
  : CamPipe<DualCamMFPipeNode>("DualCamMFPipe")
  , miOpenId(openSensorIndex)
  , mBssNode("Bss", &mCamGraph, openSensorIndex)
  , mPreProcessNode("PreProcess", &mCamGraph, openSensorIndex)
  , mVendorNode("Vendor", &mCamGraph, openSensorIndex)
{
    MY_LOGD("OpenId(%d)", miOpenId);

    mbDebugLog = StereoSettingProvider::isLogEnabled(PERPERTY_BMDENOISE_NODE_LOG);

    if(::property_get_int32(STRING_QUICK_POST, 1) == 1){
        mbQuickPostview = MTRUE;
    }

    MY_LOGD("mbDebugLog(%d) mbQuickPostview(%d) ", mbDebugLog, mbQuickPostview);
}

DualCamMFPipe::~DualCamMFPipe()
{
    // must call dispose to free CamGraph
    this->dispose();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
DualCamMFPipe::
init()
{
    CAM_TRACE_NAME("DualCamMFPipe::init");
    return PARENT::init();
}

MBOOL
DualCamMFPipe::
uninit()
{
    CAM_TRACE_NAME("DualCamMFPipe::uninit");
    SCOPE_LOGGER;
    requestExit();

    return PARENT::uninit();
}

MVOID
DualCamMFPipe::
sync()
{
    CAM_TRACE_NAME("DualCamMFPipe::sync");

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
DualCamMFPipe::
flush()
{
    CAM_TRACE_NAME("DualCamMFPipe::flush");
    // lauch the default flush operations
    CamPipe::flush();

    // relase all the effectRequest
    android::Mutex::Autolock lock(mReqMapLock);
    for(size_t index=0;index<mvRequestPtrMap.size();++index)
    {
        MUINT32 iFlushReqID = mvRequestPtrMap.keyAt(index);
        MY_LOGD("flush ReqID = %d", iFlushReqID);

        PipeRequestPtr pPipeReq = mvRequestPtrMap.valueAt(index);

        // callback to pipeline node with FLUSH KEY
        sp<IDualCamMFPipeCB> cb = pPipeReq->mwpCB.promote();
        if(cb != nullptr){
            cb->onEvent(IDualCamMFPipeCB::EventId::eFlush, pPipeReq);
        }
    }
    // clear all request map
    mvRequestPtrMap.clear();
}

MBOOL
DualCamMFPipe::
enque(PipeRequestPtr& request)
{
    SCOPE_LOGGER;
    MUINT32 reqID = request->getRequestNo();
    // autolock for request map
    {
        android::Mutex::Autolock lock(mReqMapLock);
        mvRequestPtrMap.add(reqID, request);

        if(request->getParam(DualCamMFParamID::PID_IS_MAIN) == 1){
            mvMainRequestPtrMap.add(reqID, request);
            mvCaptureRequestsTime.add(reqID, chrono::system_clock::now());
            mAccumulatedReqCnt ++;
            MY_LOGI("request map add reqId=%d (main frame) mvRequestPtrMap.size:(main)%d/(all)%d accMainReqCnts:%d",
                reqID,
                mvMainRequestPtrMap.size(),
                mvRequestPtrMap.size(),
                mAccumulatedReqCnt);
        }else{
            MY_LOGD("request map add reqId=%d mvRequestPtrMap.size:%d", reqID, mvRequestPtrMap.size());
        }
    }
    if(mvRequestPtrMap.size() > getDCMFCaptureCnt()*getDCMFCapturePhaseCnt()){
        MY_LOGW("The size of queued request inside DualCamMFPipe is over %d!", getDCMFCaptureCnt()*getDCMFCapturePhaseCnt());
    }

    request->setStopWatchCollection(new StopWatchCollection(PIPE_CLASS_TAG, reqID));
    request->getStopWatchCollection()->BeginStopWatch("total_time", (void*)request.get());

    return CamPipe::enque(ROOT_ENQUE, request);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
DualCamMFPipe::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    setupP2Operator();

    // node connection
    // DCMF pipe
    MY_LOGD("connectData...");

    this->connectData(SHUTTER,                          SHUTTER,                           mBssNode,               this);
    this->connectData(BSS_RESULT_ORIGIN,                BSS_RESULT_ORIGIN,                 mBssNode,               mPreProcessNode);
    this->connectData(BSS_RESULT_ORDERED,               BSS_RESULT_ORDERED,                mBssNode,               mPreProcessNode);

    this->connectData(QUICK_POSTVIEW,                   QUICK_POSTVIEW,                    mPreProcessNode,        this);
    this->connectData(PREPROCESS_YUV_DONE,              PREPROCESS_YUV_DONE,               mPreProcessNode,        this);
    this->connectData(PREPROCESS_TO_VENDOR,             PREPROCESS_TO_VENDOR,              mPreProcessNode,        mVendorNode);

    this->connectData(P2_DONE,                          P2_DONE,                           mVendorNode,            this);
    this->connectData(NORMAL_POSTVIEW,                  NORMAL_POSTVIEW,                   mVendorNode,            this);
    this->connectData(FINAL_RESULT,                     FINAL_RESULT,                      mVendorNode,            this);

    this->setRootNode(&mBssNode);

    MY_LOGD("push buffer producer nodes to list...");
    mvBufferProducerNodes.push_back(&mBssNode);
    mvBufferProducerNodes.push_back(&mPreProcessNode);
    mvBufferProducerNodes.push_back(&mVendorNode);
    return MTRUE;
}

MVOID
DualCamMFPipe::
onUninit()
{
}

MBOOL
DualCamMFPipe::
onData(DataID id, PipeRequestPtr &request)
{
    MUINT32 reqID = request->getRequestNo();

    MY_LOGD("recieve reqId:%d DataID:%d mvRequestPtrMap.size:%d +", reqID, id, mvRequestPtrMap.size());

    android::Mutex::Autolock lock(mReqMapLock);

    // remove request
    if(id == FINAL_RESULT){
        mvRequestPtrMap.removeItem(reqID);

        request->getStopWatchCollection()->EndStopWatch((void*)request.get());
        request->getStopWatchCollection()->PrintReport();

        if(request->getParam(DualCamMFParamID::PID_IS_MAIN) == 1){
            mvMainRequestPtrMap.removeItem(reqID);
            chrono::time_point<chrono::system_clock> captureStartTime = mvCaptureRequestsTime.valueFor(reqID);
            mvCaptureRequestsTime.removeItem(reqID);

            chrono::duration<double> elapsed_seconds = chrono::system_clock::now()-captureStartTime;
            MY_LOGI("[DCMF][Benchmark]: DCMFPipe total time(%lf ms) reqID=%d",
                elapsed_seconds.count() *1000,
                reqID
            );
        }
    }

    sp<IDualCamMFPipeCB> cb = request->mwpCB.promote();

    auto doPostviewCB = [&](){
        chrono::time_point<chrono::system_clock> captureStartTime = mvCaptureRequestsTime.valueFor(reqID);
        chrono::duration<double> elapsed_seconds = chrono::system_clock::now()-captureStartTime;
        MY_LOGD("[DCMF][Benchmark]: DCMFPipe quickPostview time(%lf ms) reqID=%d",
            elapsed_seconds.count() *1000,
            reqID
        );
        cb->onEvent(IDualCamMFPipeCB::EventId::ePostview, request);
    };

    switch(id){
        case FINAL_RESULT:
            cb->onEvent(IDualCamMFPipeCB::EventId::eCompleted, request);
            break;
        case P2_DONE:
            cb->onEvent(IDualCamMFPipeCB::EventId::eP2Done, request);
            break;
        case SHUTTER:
            cb->onEvent(IDualCamMFPipeCB::EventId::eShutter, request);
            break;
        case ERROR_OCCUR_NOTIFY:
            cb->onEvent(IDualCamMFPipeCB::EventId::eFailed, request);
            break;
        case NORMAL_POSTVIEW:
            if(!mbQuickPostview)doPostviewCB();
            break;
        case QUICK_POSTVIEW:
            if(mbQuickPostview)doPostviewCB();
            break;
        case PREPROCESS_YUV_DONE:
            cb->onEvent(IDualCamMFPipeCB::EventId::eYuvDone, request);
            break;
        default:
            MY_LOGE("unknown DataID:%d", id);
    }

    MY_LOGD("recieve reqId:%d DataID:%d mvRequestPtrMap.size:%d -", reqID, id, mvRequestPtrMap.size());
    return MTRUE;
}

MVOID
DualCamMFPipe::
setupP2Operator()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    mspP2Op = new P2Operator(PIPE_LOG_TAG, miOpenId);

    mPreProcessNode.setP2Operator(mspP2Op);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void
DualCamMFPipe::
requestExit()
{
    mbThreadExit = MTRUE;
    Thread::requestExitAndWait();
}

bool
DualCamMFPipe::
threadLoop(
)
{
    SCOPE_LOGGER;

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
DualCamMFPipe::
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

}; // NSDCMF
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam