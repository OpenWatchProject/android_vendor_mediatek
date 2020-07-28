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
#include <time.h>
#include <string.h>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file
#include <featurePipe/core/include/DebugUtil.h>
// Local header file
#include "IspPipeNode.h"
// Logging module
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "IspPipeNode"
#include <featurePipe/core/include/PipeLog.h>
// TODO: refacotr here for the different flow - 01
#include "./flowControl/default/DefaultIspPipeFlow_Common.h"
#include "./IspPipe_Common.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* NodeSignal Definition
********************************************************************************/

NodeSignal::NodeSignal()
: mSignal(0)
, mStatus(0)
{
}

NodeSignal::~NodeSignal()
{
}

MVOID NodeSignal::setSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal |= signal;
    mCondition.broadcast();
}

MVOID NodeSignal::clearSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal &= ~signal;
}

MBOOL NodeSignal::getSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    return (mSignal & signal);
}

MVOID NodeSignal::waitSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    while( !(mSignal & signal) )
    {
        mCondition.wait(mMutex);
    }
}

MVOID NodeSignal::setStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus |= status;
}

MVOID NodeSignal::clearStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus &= ~status;
}

MBOOL NodeSignal::getStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    return (mStatus & status);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeDataHandler Definition
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char*
IspPipeDataHandler::
ID2Name(DataID id)
{
#define MAKE_NAME_CASE(name) \
  case name: return #name;
  // TODO: refacotr here for the different flow - 01
  switch(id)
  {
    MAKE_NAME_CASE(ID_INVALID);
    MAKE_NAME_CASE(ROOT_ENQUE);
    MAKE_NAME_CASE(P2A_TO_TP_YUV_DATA);
    MAKE_NAME_CASE(TP_TO_MDP_BOKEHYUV);
    MAKE_NAME_CASE(TP_TO_MDP_PVYUV);
    MAKE_NAME_CASE(TP_OUT_DEPTH_BOKEH);
    MAKE_NAME_CASE(MDP_OUT_THUMBNAIL);
    MAKE_NAME_CASE(MDP_OUT_YUVS);
    MAKE_NAME_CASE(TO_DUMP_RAWS);
  };
  MY_LOGW(" unknown id:%d", id);
  return "UNKNOWN";
#undef MAKE_NAME_CASE
}

IspPipeDataHandler::
IspPipeDataHandler()
{}

IspPipeDataHandler::
~IspPipeDataHandler()
{}
/*******************************************************************************
* IspPipeNode Definition
********************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeDataHandler Initialization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspPipeNode::
IspPipeNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config
)
: IspPipeNode(name, nodeId, config, SCHED_NORMAL, DEFAULT_CAMTHREAD_PRIORITY)
{

}

IspPipeNode::
IspPipeNode(
    const char *name,
    IspPipeNodeID nodeId,
    const PipeNodeConfigs& config,
    int policy,
    int priority)
: CamThreadNode(name, policy, priority)
, mNodeId(nodeId)
, mpPipeSetting(config.mpPipeSetting)
, mpPipeOption(config.mpPipeOption)
, mDataIDToBIDMapProvider(new EmptyDataIDToBIDMapProvider())
, mIspPipeConverter(new EmptyIspPipeConverter())
{
    miDumpBufSize = ::property_get_int32("vendor.debug.isp.pipe.dump.size", 0);
    miDumpStartIdx = ::property_get_int32("vendor.debug.isp.pipe.dump.start", 0);

    MY_LOGD("mbDebugLog=%d mbProfileLog=%d",
            IspPipeLoggingSetup::mbDebugLog, IspPipeLoggingSetup::mbProfileLog);
}

IspPipeNode::
~IspPipeNode()
{}

MVOID
IspPipeNode::
setDataIDToBIDMapProvider(sp<IDataIDToBIDMapProvider> provider)
{
    sp<IDataIDToBIDMapProvider> old = mDataIDToBIDMapProvider;
    mDataIDToBIDMapProvider = provider;
    MY_LOGD("Set data id to buffer ids provider, old:%s, new::%s", old->getName().c_str(), provider->getName().c_str());
}

MVOID
IspPipeNode::
setIspPipeConverter(sp<IIspPipeConverter> conveter)
{
    sp<IIspPipeConverter> old = mIspPipeConverter;
    mIspPipeConverter = conveter;
    MY_LOGD("Set isp pipe convert, old:%s, new::%s", old->getName().c_str(), conveter->getName().c_str());
}

MBOOL
IspPipeNode::
handleDump(
    DataID id,
    const IspPipeRequestPtr& request,
    DumpConfig* config
)
{
    return this->onDump(id, request, config);
}

MBOOL
IspPipeNode::
handleDataAndDump(DataID id, const IspPipeRequestPtr& request)
{
    // dump first and then handle data
    MBOOL bRet = this->onDump(id, request);
    bRet &= this->handleData(id, request);
    return bRet;
}

MBOOL
IspPipeNode::
onDump(
    DataID id,
    const IspPipeRequestPtr& pRequest,
    DumpConfig* config
)
{
    if(!checkToDump(id))
    {
        MY_LOGD("onDump reqID=%d dataid=%d, checkDump failed!", pRequest->getRequestNo(), id);
        return MFALSE;
    }
    MY_LOGD("%s onDump reqID=%d dataid=%d", getName(), pRequest->getRequestNo(), id);

    char* fileName = (config != NULL) ? config->fileName : NULL;
    char* postfix = (config != NULL) ? config->postfix : NULL;
    MBOOL bStridePostfix = (config != NULL) ? config->bStridePostfix : MFALSE;

    MUINT iReqIdx = pRequest->getRequestNo();
    // check dump index
    if(iReqIdx < miDumpStartIdx || iReqIdx >= miDumpStartIdx + miDumpBufSize)
        return MTRUE;

    if(mDataIDToBIDMapProvider->getMap().indexOfKey(id)<0)
    {
        MY_LOGD("%s onDump: reqID=%d, cannot find BID map of the data id:%d! Chk BaseBufferConfig.cpp",
                getName(), pRequest->getRequestNo(), id);
        return MFALSE;
    }
    // generate file path
    char filepath[1024];
    snprintf(filepath, 1024, "/sdcard/isppipe/%d/%s", iReqIdx, getName());

    // make path
    MY_LOGD("makePath: %s", filepath);
    makePath(filepath, 0660);
    // get the buffer id array for dumping
    const Vector<IspPipeBufferID>& vDumpBufferID = mDataIDToBIDMapProvider->getMap().valueFor(id);
    sp<PipeBufferHandler> pBufferHandler = pRequest->getBufferHandler();
    char writepath[1024];
    char strideStr[100];

    MY_LOGD("dataID:%d buffer id size=%d", id, vDumpBufferID.size());
    for(size_t i=0;i<vDumpBufferID.size();++i)
    {
        const IspPipeBufferID& BID = vDumpBufferID.itemAt(i);
        IImageBuffer* pImgBuf;
        MBOOL bRet = pBufferHandler->getEnqueBuffer(this->getNodeId(), BID, pImgBuf);
        if(!bRet)
        {
            PIPE_LOGD("Failed to get enqued buffer, id: %d", BID);
            continue;
        }
        // stride string
        if(bStridePostfix)
            snprintf(strideStr, 100, "_%d", pImgBuf->getBufStridesInBytes(0));
        else
            snprintf(strideStr, 100, "");

        const char* writeFileName = (fileName != NULL) ? fileName : onDumpBIDToName(BID);
        const char* postfixName = (postfix != NULL) ? postfix : "";

        if(strchr(postfixName, '.') != NULL)
            snprintf(writepath, 1024, "%s/%s_%dx%d%s%s", filepath, writeFileName,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, strideStr, postfixName);
        else
        {
            snprintf(writepath, 1024, "%s/%s_%dx%d%s%s.yuv", filepath, writeFileName,
                pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, strideStr, postfixName);
        }
        MY_LOGD("saveToFile: %s", writepath);
        pImgBuf->saveToFile(writepath);
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspPipeDataHandler Protected Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
IspPipeNode::
checkToDump(DataID id)
{
    MINT32 iNodeDump =  getPropValue();

    if(getPropValue() == 0)
    {
        PIPE_LOGD("node check failed!node: %s dataID: %d", getName(), id);
        return MFALSE;
    }

    if(getPropValue(id) == 0)
    {
        PIPE_LOGD("dataID check failed!dataID: %d", id);
        return MFALSE;
    }

    return MTRUE;
}

const char*
IspPipeNode::
onDumpBIDToName(IspPipeBufferID BID)
{
    return mIspPipeConverter->bufferIdtoString(BID).c_str();
}




}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
