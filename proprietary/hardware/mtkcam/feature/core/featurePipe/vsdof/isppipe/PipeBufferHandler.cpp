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

 /**
 * @file PipeBufferHandler.cpp
 * @brief Buffer handler for isp pipe
 */

 // Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "IspPipeRequest.h"
#include "PipeBufferHandler.h"
#include "IspPipe_Common.h"
// Logging header file
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "PipeBufferHandler"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PipeBufferHandler::
PipeBufferHandler(
    PipeBufferPoolMgr* pPoolMgr
)
: mpBufferPoolMgr(pPoolMgr)
{
    initEnqueBufferMap(pPoolMgr->getNodeCount());
}

PipeBufferHandler::
~PipeBufferHandler()
{
    for(size_t index=0;index<mEnqueBufferMap.size();++index)
    {
        BIDToSmartBufferMap map = mEnqueBufferMap.valueAt(index);
        map.clear();
    }
    for(size_t index=0;index<mEnqueGraphicBufferMap.size();++index)
    {
        BIDToGraphicBufferMap map = mEnqueGraphicBufferMap.valueAt(index);
        map.clear();
    }
    for(size_t index=0;index<mEnqueTuningBufferMap.size();++index)
    {
        BIDToTuningBufferMap map = mEnqueTuningBufferMap.valueAt(index);
        map.clear();
    }

    mEnqueBufferMap.clear();
    mEnqueGraphicBufferMap.clear();
    mEnqueTuningBufferMap.clear();
    mEnqueIndiviBufferMap.clear();
    mpIspPipeRequest = nullptr;
    mpBufferPoolMgr = nullptr;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferHandler Public Operations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID
PipeBufferHandler::
initEnqueBufferMap(MINT32 nodeCount)
{
    // lock all buffer map
    RWLock::AutoWLock _lI(mEnqueRWLock[eBUFFER_IMAGE]);
    RWLock::AutoWLock _lG(mEnqueRWLock[eBUFFER_GRAPHIC]);
    RWLock::AutoWLock _lT(mEnqueRWLock[eBUFFER_TUNING]);
    // clear all buffer map
    mEnqueBufferMap.clear();
    mEnqueGraphicBufferMap.clear();
    mEnqueTuningBufferMap.clear();
    // init maps
    // Daniel:
    // TODO: refactor
    for(int nodeId=0;nodeId<nodeCount;++nodeId)
    {
        IspPipeNodeID id = static_cast<IspPipeNodeID>(nodeId);
        // init the enque buffer map
        mEnqueBufferMap.add(id, BIDToSmartBufferMap());
        mEnqueIndiviBufferMap.add(id, BIDToSmartIndividualBufferMap());
        mEnqueGraphicBufferMap.add(id, BIDToGraphicBufferMap());
        mEnqueTuningBufferMap.add(id, BIDToTuningBufferMap());
    }
}

MBOOL
PipeBufferHandler::
configRequest(IspPipeRequest* pRequest)
{
    mpIspPipeRequest = pRequest;
    return MTRUE;
}

IImageBuffer*
PipeBufferHandler::
requestBuffer(
    IspPipeNodeID nodeID,
    IspPipeBufferID bufferID
)
{
    PIPE_LOGD("+  srcNodeID=%d bufferID=%d", nodeID, bufferID);

    BufferIOType ioType;
    if(mpIspPipeRequest->getBufferIOType(bufferID, ioType))
    {
        IImageBuffer* pOutImgBuf = nullptr;
        MBOOL bRet = mpIspPipeRequest->getRequestImageBuffer({.bufferID=bufferID, .ioType=ioType}, pOutImgBuf);
        if(!bRet)
            MY_LOGE("Cannot find buffer inside request!, srcNodeID=%d bufferID=%d", nodeID, bufferID);
        return pOutImgBuf;
    }
    else
    {
        PIPE_LOGD("request buffer mgr, srcNodeID=%d bufferID=%d", nodeID, bufferID);
        SmartPipeImgBuffer smImgBuf = mpBufferPoolMgr->request(bufferID);
        if(smImgBuf.get() != nullptr)
        {
            addEnquedBuffer(nodeID, bufferID, smImgBuf);
            return smImgBuf->mImageBuffer.get();
        }
        SmartImageBuffer smImgIndiviBuf = mpBufferPoolMgr->requestIB(bufferID);
        if(smImgIndiviBuf.get() != nullptr)
        {
            addEnquedBuffer(nodeID, bufferID, smImgIndiviBuf);
            return smImgIndiviBuf->mImageBuffer.get();
        }
        SmartGraphicBuffer smGraBuf = mpBufferPoolMgr->requestGB(bufferID);
        if(smGraBuf.get() != nullptr)
        {
            addEnquedBuffer(nodeID, bufferID, smGraBuf);
            return smGraBuf->mImageBuffer.get();
        }

        MY_LOGE("Failed to request buffer, src node:%d, buffer id:%d", nodeID, bufferID);
        return nullptr;
    }
}

IImageBuffer*
PipeBufferHandler::
requestWorkingBuffer(IspPipeBufferID bufferID)
{
    PIPE_LOGD("+ , bufferID:%d", bufferID);
    IImageBuffer* pOutImgBuf = nullptr;
    //
    SmartPipeImgBuffer smImgBuf = mpBufferPoolMgr->request(bufferID);
    if(smImgBuf != nullptr)
    {
        addEnquedWorkingBuffer(smImgBuf);
        pOutImgBuf = smImgBuf->mImageBuffer.get();
    }
    else
    {
        SmartGraphicBuffer smGraBuf = mpBufferPoolMgr->requestGB(bufferID);
        if(smGraBuf.get() != NULL)
        {
            addEnquedWorkingBuffer(smGraBuf);
            pOutImgBuf = smGraBuf->mImageBuffer.get();
        }
    }
    if(pOutImgBuf == nullptr)
        MY_LOGE("Failed to request working buffer, buffer id:%d", bufferID);

    PIPE_LOGD("- , bufferID:%d", bufferID);
    return pOutImgBuf;
}

MVOID*
PipeBufferHandler::
requestTuningBuf(
    IspPipeNodeID nodeID,
    IspPipeBufferID bufferID
)
{
    SmartTuningBuffer smTuningBuf = mpBufferPoolMgr->requestTB(bufferID);

    if(smTuningBuf.get() != nullptr)
    {
        addEnquedBuffer(nodeID, bufferID, smTuningBuf);
        return smTuningBuf->mpVA;
    }

    MY_LOGE("Failed to request tuning buffer!! bufferID:%d", bufferID);
    return nullptr;
}

MVOID*
PipeBufferHandler::
requestWorkingTuningBuf(
    IspPipeBufferID bufferID
)
{
    SmartTuningBuffer smTuningBuf = mpBufferPoolMgr->requestTB(bufferID);

    if(smTuningBuf.get() != nullptr)
    {
        addEnquedWorkingBuffer(smTuningBuf);
        return smTuningBuf->mpVA;
    }

    MY_LOGE("Failed to request tuning buffer!! bufferID:%d", bufferID);
    return nullptr;
}

IMetadata*
PipeBufferHandler::
requestMetadata(
    IspPipeNodeID nodeID,
    IspPipeBufferID bufferID
)
{
    IMetadata* pOutMetauf;

    BufferIOType ioType;
    if(mpIspPipeRequest->getBufferIOType(bufferID, ioType))
    {
        mpIspPipeRequest->getRequestMetadata({.bufferID=bufferID, .ioType=ioType}, pOutMetauf);
        return pOutMetauf;
    }
    else
    {
        MY_LOGE("Failed to request metadata !! nodeID:%d bufferID:%d", nodeID, bufferID);
        return nullptr;
    }
}

MBOOL
PipeBufferHandler::
onProcessDone(IspPipeNodeID nodeID)
{
    // local all buffer map
    RWLock::AutoWLock _lI(mEnqueRWLock[eBUFFER_IMAGE]);
    RWLock::AutoWLock _lG(mEnqueRWLock[eBUFFER_GRAPHIC]);
    RWLock::AutoWLock _lT(mEnqueRWLock[eBUFFER_TUNING]);
    // release the enque buffer
    mEnqueBufferMap.editValueFor(nodeID).clear();
    mEnqueIndiviBufferMap.editValueFor(nodeID).clear();
    mEnqueGraphicBufferMap.editValueFor(nodeID).clear();
    mEnqueTuningBufferMap.editValueFor(nodeID).clear();
    return MTRUE;
}

MBOOL
PipeBufferHandler::
configOutBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    IspPipeNodeID outNodeID
)
{
    return configInOutBuffer(srcNodeID, bufferID, outNodeID, bufferID);
}

MBOOL
PipeBufferHandler::
configExternalOutBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    IspPipeNodeID outNodeID,
    SmartPipeImgBuffer smImgBuf
)
{
    // add src node buffer
    MBOOL bRet = addOutputBuffer(srcNodeID, bufferID, smImgBuf);
    bRet &= addOutputBuffer(outNodeID, bufferID, smImgBuf);
    return bRet;
}

MBOOL
PipeBufferHandler::
configInOutBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID srcBufferID,
    IspPipeNodeID outNodeID,
    IspPipeBufferID outBufferID
)
{
    PIPE_LOGD("srcNodeID =%d srcBufferID=%d outNodeID=%d outBufferID=%d" , srcNodeID, srcBufferID, outNodeID, outBufferID);

    // only working buffer needs to config out buffer
    if(mpIspPipeRequest->isRequestBuffer(srcBufferID))
        return MTRUE;

    IspBufferType bufferType = mpBufferPoolMgr->queryBufferType(srcBufferID);

    if(bufferType == eBUFFER_IMAGE)
    {
        SmartPipeImgBuffer smBuf;
        BIDToSmartBufferMap queuedBufferMap;
        // read the buffer out
        {
            RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
            queuedBufferMap = mEnqueBufferMap.valueFor(srcNodeID);
            ssize_t bufIndex = queuedBufferMap.indexOfKey(srcBufferID);

            if(bufIndex<0)
            {
                PIPE_LOGD("Cannot find the buffer to config, id:%d", srcBufferID);
                return MFALSE;
            }
            smBuf = queuedBufferMap.valueAt(bufIndex);
        }

        this->addOutputBuffer(outNodeID, outBufferID, smBuf);
        return MTRUE;
    }
    else if(bufferType == eBUFFER_GRAPHIC)
    {
        SmartGraphicBuffer smBuf;
        BIDToGraphicBufferMap queuedBufferMap;
        // read the buffer out
        {
            RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_GRAPHIC]);
            queuedBufferMap = mEnqueGraphicBufferMap.valueFor(srcNodeID);
            ssize_t bufIndex = queuedBufferMap.indexOfKey(srcBufferID);

            if(bufIndex<0)
            {
                PIPE_LOGD("Cannot find the graphic buffer to config, id:%d", srcBufferID);
                return MFALSE;
            }
            smBuf = queuedBufferMap.valueAt(bufIndex);
        }

        this->addOutputBuffer(outNodeID, outBufferID, smBuf);
        return MTRUE;
    }
    else if(bufferType == eBUFFER_TUNING)
    {
        SmartTuningBuffer smBuf;
        BIDToTuningBufferMap queuedBufferMap;
        // read the buffer out
        {
            RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_TUNING]);
            queuedBufferMap = mEnqueTuningBufferMap.valueFor(srcNodeID);
            ssize_t bufIndex = queuedBufferMap.indexOfKey(srcBufferID);
            if(bufIndex<0)
            {
                PIPE_LOGD("Cannot find the tuning buffer to config, id:%d", srcBufferID);
                return MFALSE;
            }
            smBuf = queuedBufferMap.valueAt(bufIndex);
        }
        this->addOutputBuffer(outNodeID, outBufferID, smBuf);
        return MTRUE;
    }

    // not buffer inside buffer pool mgr
    PIPE_LOGD("No need to config, buffer not exist, srcBufferID=%d", srcBufferID);
    return MFALSE;
}


MBOOL
PipeBufferHandler::
getEnqueBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    IImageBuffer*& rpImgBuf)
{
    BufferIOType ioType;
    // if request buffer, just return
    if(mpIspPipeRequest->getBufferIOType(bufferID, ioType) &&
        mpIspPipeRequest->getRequestImageBuffer({.bufferID=bufferID, .ioType=ioType}, rpImgBuf))
    {
        PIPE_LOGD("getEnqueBuffer: request_buffer=%x bufferID=%d", rpImgBuf, bufferID);
        return MTRUE;
    }
    // get buffer from enqued vector
    IspBufferType bufferType = mpBufferPoolMgr->queryBufferType(bufferID);
    if(bufferType == eBUFFER_IMAGE)
    {
        RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
        BIDToSmartBufferMap bufferMap = mEnqueBufferMap.valueFor(srcNodeID);
        ssize_t index=bufferMap.indexOfKey(bufferID);
        if(index<0)
        {
           PIPE_LOGD("Cannot find enqued buffer, buffer id=%d", bufferID);
           return MFALSE;
        }

        SmartPipeImgBuffer smBuf = bufferMap.valueAt(index);
        rpImgBuf = smBuf->mImageBuffer.get();
        return MTRUE;
    }
    else if(bufferType == eBUFFER_GRAPHIC)
    {
        RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_GRAPHIC]);
        BIDToGraphicBufferMap bufferMap = mEnqueGraphicBufferMap.valueFor(srcNodeID);
        ssize_t index=bufferMap.indexOfKey(bufferID);
        if(index<0)
        {
            PIPE_LOGD("Cannot find enqued buffer, buffer id=%d", bufferID);
            return MFALSE;
        }

       SmartGraphicBuffer smBuf = bufferMap.valueAt(index);
       rpImgBuf = smBuf->mImageBuffer.get();
       return MTRUE;
    }

    return MFALSE;
}

MBOOL
PipeBufferHandler::
getEnquedSmartBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    SmartPipeImgBuffer& pSmImgBuf
)
{
    RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
    ssize_t index;
    const BIDToSmartBufferMap& bidToBufMap = mEnqueBufferMap.valueFor(srcNodeID);
    if((index=bidToBufMap.indexOfKey(bufferID))>=0)
    {
        pSmImgBuf = bidToBufMap.valueAt(index);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
PipeBufferHandler::
getEnquedSmartBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    SmartGraphicBuffer& pSmGraBuf
)
{
    RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_GRAPHIC]);
    ssize_t index;
    const BIDToGraphicBufferMap& bidToBufMap = mEnqueGraphicBufferMap.valueFor(srcNodeID);
    if((index=bidToBufMap.indexOfKey(bufferID))>=0)
    {
        pSmGraBuf = bidToBufMap.valueAt(index);
        return MTRUE;
    }
    return MFALSE;
}

MBOOL
PipeBufferHandler::
getEnquedSmartBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    SmartTuningBuffer& pSmTuningBuf
)
{
    RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_TUNING]);
    ssize_t index;
    const BIDToTuningBufferMap& bidToBufMap = mEnqueTuningBufferMap.valueFor(srcNodeID);
    if((index=bidToBufMap.indexOfKey(bufferID))>=0)
    {
        pSmTuningBuf = bidToBufMap.valueAt(index);
        return MTRUE;
    }
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PipeBufferHandler Private Operations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL
PipeBufferHandler::
addEnquedBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    SmartPipeImgBuffer smImgBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
    BIDToSmartBufferMap& bidBufMap = mEnqueBufferMap.editValueFor(srcNodeID);
    bidBufMap.add(bufferID, smImgBuf);

    return MTRUE;
}

MBOOL
PipeBufferHandler::
addEnquedBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    SmartImageBuffer smImgBuf
)
{
    // share the same lock with image
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
    BIDToSmartIndividualBufferMap& bidBufMap = mEnqueIndiviBufferMap.editValueFor(srcNodeID);
    bidBufMap.add(bufferID, smImgBuf);

    return MTRUE;
}

MBOOL
PipeBufferHandler::
addEnquedBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    SmartGraphicBuffer smGraBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_GRAPHIC]);
    BIDToGraphicBufferMap& bidGraBufMap = mEnqueGraphicBufferMap.editValueFor(srcNodeID);
    bidGraBufMap.add(bufferID, smGraBuf);

    return MTRUE;
}

MBOOL
PipeBufferHandler::
addEnquedBuffer(
    IspPipeNodeID srcNodeID,
    IspPipeBufferID bufferID,
    SmartTuningBuffer smTuningBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_TUNING]);
    BIDToTuningBufferMap& bidTuningBufMap = mEnqueTuningBufferMap.editValueFor(srcNodeID);
    bidTuningBufMap.add(bufferID, smTuningBuf);

    return MTRUE;
}

MBOOL
PipeBufferHandler::
addEnquedWorkingBuffer(
    SmartPipeImgBuffer smImgBuf
)
{
    RWLock::AutoWLock _l(mWorkingBufRWLock[eBUFFER_IMAGE]);
    mvWorkingSmartImgBuffer.push_back(smImgBuf);
    return MTRUE;
}

MBOOL
PipeBufferHandler::
addEnquedWorkingBuffer(
    SmartGraphicBuffer smGraBuf
)
{
    RWLock::AutoWLock _l(mWorkingBufRWLock[eBUFFER_GRAPHIC]);
    mvWorkingSmartGraImgBuffer.push_back(smGraBuf);
    return MTRUE;
}

MBOOL
PipeBufferHandler::
addEnquedWorkingBuffer(
    SmartTuningBuffer smTuningBuf
)
{
    RWLock::AutoWLock _l(mWorkingBufRWLock[eBUFFER_TUNING]);
    mvWorkingSmartTuningBuffer.push_back(smTuningBuf);
    return MTRUE;
}

MBOOL
PipeBufferHandler::
addOutputBuffer(
    IspPipeNodeID outNodeID,
    IspPipeBufferID bufferID,
    SmartPipeImgBuffer smImgBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
    BIDToSmartBufferMap& bidBufMap = mEnqueBufferMap.editValueFor(outNodeID);
    bidBufMap.add(bufferID, smImgBuf);

    return MTRUE;
}

MBOOL
PipeBufferHandler::
addOutputBuffer(
    IspPipeNodeID outNodeID,
    IspPipeBufferID bufferID,
    SmartGraphicBuffer smGraBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_GRAPHIC]);
    BIDToGraphicBufferMap& bidGraBufMap = mEnqueGraphicBufferMap.editValueFor(outNodeID);
    bidGraBufMap.add(bufferID, smGraBuf);

    return MTRUE;
}

MBOOL
PipeBufferHandler::
addOutputBuffer(
    IspPipeNodeID outNodeID,
    IspPipeBufferID bufferID,
    SmartTuningBuffer smTuningBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_TUNING]);
    BIDToTuningBufferMap& bidTuningBufMap = mEnqueTuningBufferMap.editValueFor(outNodeID);
    bidTuningBufMap.add(bufferID, smTuningBuf);
    return MTRUE;
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam
