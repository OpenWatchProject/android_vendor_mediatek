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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

// Android system/core header file
#include <utils/String8.h>
#include <utils/Vector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
// Module header file

// Local header file
#include "IspPipe.h"

// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "IspPipe"
#include <featurePipe/core/include/PipeLog.h>


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

IspPipeSetting::
IspPipeSetting(
    MUINT32 senIdx_main1,
    MUINT32 senIdx_main2,
    MSize szIMGO_Main1,
    MSize szIMGO_Main2,
    MSize szRRZO_Main1,
    MSize szRRZO_Main2,
    MSize szPreviewSize
)
: miSensorIdx_Main1(senIdx_main1)
, miSensorIdx_Main2(senIdx_main2)
, mszIMGO_Main1(szIMGO_Main1)
, mszIMGO_Main2(szIMGO_Main2)
, mszRRZO_Main1(szRRZO_Main1)
, mszRRZO_Main2(szRRZO_Main2)
, mszPreviewYUV(szPreviewSize)
{
    // decide the FD img size by ratio
    if(StereoSettingProvider::imageRatio() == eRatio_16_9)
        mszFDImg = MSize(640, 360);
    else
        mszFDImg = MSize(640, 480);

    MY_LOGD("mszIMGO_Main1=%dx%d, mszIMGO_Main2=%dx%d, mszRRZO_Main1=%dx%d"
            ", mszRRZO_Main2=%dx%d mszPreviewYUV=%dx%d",
            mszIMGO_Main1.w, mszIMGO_Main1.h, mszIMGO_Main2.w, mszIMGO_Main2.h,
            mszRRZO_Main1.w, mszRRZO_Main1.h, mszRRZO_Main2.w, mszRRZO_Main2.h,
            mszPreviewYUV.w, mszPreviewYUV.h);
}

IIspPipe*
IIspPipe::
createInstance(
    IspPipeSetting& setting,
    IspPipeOption& option,
    IspPipeFlowControlType type
)
{
#ifdef GTEST
    IspPipeLoggingSetup::mbProfileLog = MTRUE;
    IspPipeLoggingSetup::mbDebugLog = MTRUE;
#else
    IspPipeLoggingSetup::mbProfileLog =  property_get_int32("vendor.debug.isppipe.profile", 0);
    IspPipeLoggingSetup::mbDebugLog = property_get_int32("vendor.debug.isppipe.debug", 0) | IspPipeLoggingSetup::mbProfileLog;
#endif
    MY_LOGD("LoggingSetup: mbProfileLog =%d mbDebugLog=%d ",
            IspPipeLoggingSetup::mbProfileLog, IspPipeLoggingSetup::mbDebugLog);

    sp<IIspPipeFlowControler> pFlowControler = IIspPipeFlowControler::createInstance(type, setting, option);

    return new IspPipe(new IspPipeSetting(setting), new IspPipeOption(option), pFlowControler);
}

MBOOL
IIspPipe::destroyInstance()
{
    delete this;
    return MTRUE;
}

IspPipe::
IspPipe(
    sp<IspPipeSetting> setting,
    sp<IspPipeOption> option,
    sp<IIspPipeFlowControler> pFlowControler
)
: CamPipe<IspPipeNode>("IspPipe")
, mpPipeSetting(setting)
, mpPipeOption(option)
, mpIspPipeFlowControler(pFlowControler)
{
    mpNodeSignal = new NodeSignal();
    MY_LOGD("Isp Pipe Option: mbEnableLCE = %d mbEnableEIS1d2=%d", option->mbEnableLCE, option->mbEnableEIS1d2);
}

IspPipe::
~IspPipe()
{
    // must call dispose to free CamGraph
    this->dispose();
    // free all nodes
    for(size_t idx=0;idx<mNodeMap.size();++idx)
    {
        IspPipeNode *pNode = mNodeMap.valueAt(idx);
        delete pNode;
    }
    mNodeMap.clear();
}

MBOOL
IspPipe::
onInit()
{
    MY_LOGD("+");
    mpBufferPoolMgr = new PipeBufferPoolMgr(mpPipeSetting);

    // flow control
    {
        mpIspPipeFlowControler->BuildPipeBufferPool(*mpBufferPoolMgr.get());

        mpIspPipeFlowControler->BuildPipeNodes(*this, mNodeMap);
    }

    // set node signal
    for(size_t index=0;index<mNodeMap.size();++index)
    {
        mNodeMap.valueAt(index)->setNodeSignal(mpNodeSignal);
    }
    // default node graph - Error handling
    for(size_t index=0;index<mNodeMap.size();++index)
    {
        this->connectData(ERROR_OCCUR_NOTIFY, ERROR_OCCUR_NOTIFY, *mNodeMap.valueAt(index), this);
    }

    MY_LOGD("-");
    return MTRUE;
}

MVOID
IspPipe::
onUninit()
{
    MY_LOGD("+");
    mpBufferPoolMgr = nullptr;
    MY_LOGD("-");
}

MBOOL
IspPipe::
init()
{
    MBOOL bRet = CamPipe<IspPipeNode>::init();
    // Use flush on stop by default
    CamPipe<IspPipeNode>::setFlushOnStop(MTRUE);
    return bRet;
}

MBOOL
IspPipe::
uninit()
{
    return CamPipe<IspPipeNode>::uninit();
}

MVOID
IspPipe::
sync()
{
    return CamPipe<IspPipeNode>::sync();
}

MVOID
IspPipe::
flush()
{
    PIPE_LOGD("IspPipe::flush + req size=%d", mvRequestDataMap.size());
    // set flush status
    mpNodeSignal->setStatus(NodeSignal::STATUS_IN_FLUSH);
    // lauch the default flush operations
    CamPipe::flush();
    // relase all the effectRequest
    android::Mutex::Autolock lock(mReqMapLock);
    {
        for(size_t index=0;index<mvRequestDataMap.size();++index)
        {
            MUINT32 iFlushReqID = mvRequestDataMap.keyAt(index);
            IspPipeRequestPtr pRequest = mvRequestDataMap.valueAt(index);
            PIPE_LOGD("flush ReqID=%d", iFlushReqID);
            // callback to pipeline node with FLUSH state
            pRequest->launchFinishCallback(eRESULT_FLUSH);
        }
        // clear all request map
        mvRequestDataMap.clear();
    }
    // clear flush status
    mpNodeSignal->clearStatus(NodeSignal::STATUS_IN_FLUSH);
    PIPE_LOGD("-");
}

MBOOL
IspPipe::
enque(sp<IIspPipeRequest>& pInputRequest)
{
    PIPE_LOGD("+");

    IspPipeRequestPtr pRequest = static_cast<IspPipeRequest*> (pInputRequest.get());
     MUINT32 reqID = pRequest->getRequestNo();
    // autolock for request map
    {
        android::Mutex::Autolock lock(mReqMapLock);
        mvRequestDataMap.add(reqID, pRequest);
        PIPE_LOGD("request map add reqId=%d, size=%zu", reqID, mvRequestDataMap.size());
    }
    // warning msg
    static const MINT32 maxCountForNotify = 100;
    if(mvRequestDataMap.size()>maxCountForNotify)
    {
        MY_LOGE("[Critical] Might occur memory leak, request inside IspPipe is too much, reqID=%d, size=%d/%d", reqID, mvRequestDataMap.size(), maxCountForNotify);
    }
    // create BufferHandler for each request
    sp<PipeBufferHandler> pBufferHandler = mpBufferPoolMgr->createBufferHandler();
    // call init inside request
    MBOOL bRet = pRequest->init(pBufferHandler, mpPipeOption);
    if(!bRet)
    {
        MY_LOGE("reqID=%d, Request init failed, stop enque!", reqID);
        return MFALSE;
    }
    // call parent class:FeaturePipe enque function
    MBOOL ret = CamPipe::enque(ROOT_ENQUE, pRequest);

    PIPE_LOGD("-");
    return ret;
}

MBOOL
IspPipe::
onData(
    DataID id,
    const IspPipeRequestPtr& pRequest
)
{
    MUINT32 iReqID = pRequest->getRequestNo();
    PIPE_LOGD("+ : reqID=%d DataID=%d", iReqID, id);

    MBOOL bRet;
    switch(id)
    {
        case ERROR_OCCUR_NOTIFY:
            bRet = onErrorNotify(pRequest);
            break;
        default:
            MBOOL bSuccess = mpIspPipeFlowControler->HandlePipeOnData(id, pRequest);
            bSuccess ? onDataSuccess(pRequest) : onErrorNotify(pRequest);
    }
    PIPE_LOGD("-");
    return MTRUE;
}

MBOOL
IspPipe::
onErrorNotify(const IspPipeRequestPtr& pRequest)
{
    android::Mutex::Autolock lock(mReqMapLock);
    MUINT32 reqID = pRequest->getRequestNo();
    if(mvRequestDataMap.indexOfKey(reqID)<0)
    {
        MY_LOGW("IspPipe: onErrorNotify non-existed reqID=%d, might already return.", reqID);
        return MTRUE;
    }
    MY_LOGE("IspPipe request occur error!! reqID=%d", reqID);
    pRequest->launchFinishCallback(eRESULT_FAILURE);
    // remove request
    mvRequestDataMap.removeItem(reqID);

    return MTRUE;
}

MBOOL
IspPipe::
onDataSuccess(const IspPipeRequestPtr& pRequest)
{
    MUINT32 reqID = pRequest->getRequestNo();
    PIPE_LOGD("+ reqID=%d", reqID);
    // autolock for request map
    android::Mutex::Autolock lock(mReqMapLock);
    // check exist
    ssize_t reqPos =  mvRequestDataMap.indexOfKey(reqID);
    if(reqPos < 0)
    {
        MY_LOGW("IspPipe: onDataSuccess non-existed reqID=%d, might already return.", reqID);
        return MTRUE;
    }
    // if ready callback to client
    if(pRequest->checkAllOutputReady())
    {
        PIPE_LOGD("Request data ready! req_id=%d", pRequest->getRequestNo());
        // remove request
        mvRequestDataMap.removeItemsAt(reqPos);
        // callback
        pRequest->launchFinishCallback(eRESULT_SUCCESS);
    }
    PIPE_LOGD("-");

    return MTRUE;
}



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
