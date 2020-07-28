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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
 * @file NodeBufferHandler.cpp
 * @brief VSDOF buffer handler
 */

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "NodeBufferHandler.h"
#include "../DepthMapPipeNode.h"
// Logging header file
#undef PIPE_CLASS_TAG
#define PIPE_CLASS_TAG "NodeBufferHandler"
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_SFPIPE_DEPTH);


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

#define BID2Name DepthMapPipeNode::onDumpBIDToName

/*******************************************************************************
* Global Define
********************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NodeBufferHandler::
NodeBufferHandler(
    sp<BaseBufferPoolMgr> pPoopMgr
)
: BaseBufferHandler(pPoopMgr)
{
    initEnqueBufferMap();
}

NodeBufferHandler::
~NodeBufferHandler()
{
    VSDOF_LOGD2("[Destructo] +");
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

    // clear working buffer
    for(size_t index = 0; index < mWorkingImageBufferMap.size(); ++index)
    {
        SmartImgBufferVector vBufs = mWorkingImageBufferMap.valueAt(index);
        vBufs.clear();
    }
    for(size_t index = 0; index < mWorkingGraphicBufferMap.size(); ++index)
    {
        SmartGraImgBufferVector vBufs = mWorkingGraphicBufferMap.valueAt(index);
        vBufs.clear();
    }
    for(size_t index = 0; index < mWorkingTuningBufferMap.size(); ++index)
    {
        SmartTuningBufferVector vBufs = mWorkingTuningBufferMap.valueAt(index);
        vBufs.clear();
    }
    mWorkingImageBufferMap.clear();
    mWorkingGraphicBufferMap.clear();
    mWorkingTuningBufferMap.clear();

    mpDepthMapEffectReq = NULL;
    mpBufferPoolMgr = NULL;
    VSDOF_LOGD2("[Destructo] -");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
NodeBufferHandler::
configRequest(
    DepthMapEffectRequest* pDepthReq
)
{
    mpDepthMapEffectReq = pDepthReq;
    mReqAttr = pDepthReq->getRequestAttr();
    return MTRUE;
}

IImageBuffer*
NodeBufferHandler::
requestBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID
)
{
    VSDOF_LOGD2("+  reqID=%d srcNodeID=%d bufferID=%s",
                 mpDepthMapEffectReq->getRequestNo(), srcNodeID, BID2Name(bufferID));
    IImageBuffer* pOutImgBuf = NULL;
    BufferIOType ioType;
    // check buffer is inside the request
    if(mpDepthMapEffectReq->getBufferIOType(bufferID, ioType))
    {
        VSDOF_LOGD2("inside request, srcNodeID=%d bufferID=%d %s", srcNodeID, bufferID, BID2Name(bufferID));
        MBOOL bRet = mpDepthMapEffectReq->getRequestImageBuffer({.bufferID=bufferID, .ioType=ioType}, pOutImgBuf);
        if(!bRet)
            MY_LOGW("Cannot find buffer inside request!, srcNodeID=%d bufferID=%s", srcNodeID, BID2Name(bufferID));
    }
    else
    {
        VSDOF_LOGD2("request buffer mgr, srcNodeID=%d bufferID=%s", srcNodeID, BID2Name(bufferID));
        SmartFatImageBuffer smImgBuf = mpBufferPoolMgr->request(bufferID, mReqAttr.bufferScenario);
        if(smImgBuf.get() != NULL)
        {
            addEnquedBuffer(srcNodeID, bufferID, smImgBuf);
            pOutImgBuf = smImgBuf->mImageBuffer.get();
        }
        else
        {
            VSDOF_LOGD2("request GB buffer mgr, srcNodeID=%d bufferID=%s", srcNodeID, BID2Name(bufferID));
            SmartGraphicBuffer smGraBuf = mpBufferPoolMgr->requestGB(bufferID, mReqAttr.bufferScenario);
            if(smGraBuf.get() != NULL)
            {
                addEnquedBuffer(srcNodeID, bufferID, smGraBuf);
                pOutImgBuf = smGraBuf->mImageBuffer.get();
            }
        }
        if(pOutImgBuf == NULL)
        {
            MY_LOGE("Failed to request buffer, src node:%d, buffer id:%s, scenario:%d",
                                srcNodeID, BID2Name(bufferID), mReqAttr.bufferScenario);
        }
    }

    VSDOF_LOGD2("-  srcNodeID=%d bufferID=%d", srcNodeID, bufferID);
    return pOutImgBuf;
}

IImageBuffer*
NodeBufferHandler::
requestWorkingBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID
)
{
    VSDOF_LOGD2("+  bufferID=%s", BID2Name(bufferID));
    IImageBuffer* pOutImgBuf = NULL;
    {
        SmartFatImageBuffer smImgBuf = mpBufferPoolMgr->request(bufferID, mReqAttr.bufferScenario);
        if(smImgBuf.get() != NULL)
        {
            addEnquedWorkingBuffer(srcNodeID, smImgBuf);
            pOutImgBuf = smImgBuf->mImageBuffer.get();
        }
        else
        {
            SmartGraphicBuffer smGraBuf = mpBufferPoolMgr->requestGB(bufferID, mReqAttr.bufferScenario);
            if(smGraBuf.get() != NULL)
            {
                addEnquedWorkingBuffer(srcNodeID, smGraBuf);
                pOutImgBuf = smGraBuf->mImageBuffer.get();
            }
        }
        if(pOutImgBuf == NULL)
        {
            MY_LOGE("Failed to request working buffer, buffer id:%s, scenario:%d", BID2Name(bufferID), mReqAttr.bufferScenario);
        }
    }

    VSDOF_LOGD2("-  bufferID=%d", bufferID);
    return pOutImgBuf;
}

MBOOL
NodeBufferHandler::
addEnquedBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    SmartFatImageBuffer smImgBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
    BIDToSmartBufferMap& bidBufMap = mEnqueBufferMap.editValueFor(srcNodeID);
    if(bidBufMap.indexOfKey(bufferID) >= 0)
    {
        MY_LOGE("BufferID :%s alread exist in the bufMap, srcNode=%d", BID2Name(bufferID), srcNodeID);
        return MFALSE;
    }
    bidBufMap.add(bufferID, smImgBuf);

    return MTRUE;
}

MBOOL
NodeBufferHandler::
addEnquedBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    SmartGraphicBuffer smGraBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_GRAPHIC]);
    BIDToGraphicBufferMap& bidGraBufMap = mEnqueGraphicBufferMap.editValueFor(srcNodeID);
    if(bidGraBufMap.indexOfKey(bufferID) >= 0)
    {
        MY_LOGE("BufferID :%s alread exist in the bidGraBufMap, srcNode=%d", BID2Name(bufferID), srcNodeID);
        return MFALSE;
    }
    bidGraBufMap.add(bufferID, smGraBuf);

    return MTRUE;
}

MBOOL
NodeBufferHandler::
addEnquedBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    SmartTuningBuffer smTuningBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_TUNING]);
    BIDToTuningBufferMap& bidTuningBufMap = mEnqueTuningBufferMap.editValueFor(srcNodeID);
    if(bidTuningBufMap.indexOfKey(bufferID) >= 0)
    {
        MY_LOGE("BufferID :%s alread exist in the bidTuningBufMap, srcNode=%d", BID2Name(bufferID), srcNodeID);
        return MFALSE;
    }
    bidTuningBufMap.add(bufferID, smTuningBuf);

    return MTRUE;
}

MBOOL
NodeBufferHandler::
addEnquedWorkingBuffer(
    DepthMapPipeNodeID srcNodeID,
    SmartFatImageBuffer smImgBuf
)
{
    RWLock::AutoWLock _l(mWorkingBufRWLock[eBUFFER_IMAGE]);
    SmartImgBufferVector& vWokringImgBuffer = mWorkingImageBufferMap.editValueFor(srcNodeID);
    vWokringImgBuffer.push_back(smImgBuf);
    return MTRUE;
}

MBOOL
NodeBufferHandler::
addEnquedWorkingBuffer(
    DepthMapPipeNodeID srcNodeID,
    SmartGraphicBuffer smGraBuf
)
{
    RWLock::AutoWLock _l(mWorkingBufRWLock[eBUFFER_GRAPHIC]);
    SmartGraImgBufferVector& vWokringGraphicBuffer = mWorkingGraphicBufferMap.editValueFor(srcNodeID);
    vWokringGraphicBuffer.push_back(smGraBuf);
    return MTRUE;
}

MBOOL
NodeBufferHandler::
addEnquedWorkingBuffer(
    DepthMapPipeNodeID srcNodeID,
    SmartTuningBuffer smTuningBuf
)
{
    RWLock::AutoWLock _l(mWorkingBufRWLock[eBUFFER_TUNING]);
    SmartTuningBufferVector& vWokringTuningBuffer = mWorkingTuningBufferMap.editValueFor(srcNodeID);
    vWokringTuningBuffer.push_back(smTuningBuf);
    return MTRUE;
}

IMetadata*
NodeBufferHandler::
requestMetadata(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID)
{
    IMetadata* pOutMeta = nullptr;
    // check buffer is inside the request
    BufferIOType ioType;
    if(mpDepthMapEffectReq->getBufferIOType(bufferID, ioType))
    {
        mpDepthMapEffectReq->getRequestMetadata({.bufferID=bufferID, .ioType=ioType}, pOutMeta);
        return pOutMeta;
    }
    else
    {
        MY_LOGE("Failed to request metadata !! bufferID:%s scenario:%d", BID2Name(bufferID), mReqAttr.bufferScenario);
        return NULL;
    }
}

MVOID*
NodeBufferHandler::
requestTuningBuf(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID
)
{
    SmartTuningBuffer smTuningBuf = mpBufferPoolMgr->requestTB(bufferID, mReqAttr.bufferScenario);

    if(smTuningBuf.get() != NULL)
    {
        addEnquedBuffer(srcNodeID, bufferID, smTuningBuf);
        return smTuningBuf->mpVA;
    }

    MY_LOGE("Failed to request tuning buffer!! bufferID:%s scenario:%d", BID2Name(bufferID), mReqAttr.bufferScenario);
    return NULL;
}

MVOID*
NodeBufferHandler::
requestWorkingTuningBuf(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID
)
{
    VSDOF_LOGD2("bufferID = %s", BID2Name(bufferID));
    SmartTuningBuffer smTuningBuf = mpBufferPoolMgr->requestTB(bufferID, mReqAttr.bufferScenario);

    if(smTuningBuf.get() != NULL)
    {
        addEnquedWorkingBuffer(srcNodeID, smTuningBuf);
        return smTuningBuf->mpVA;
    }

    MY_LOGE("Failed to request tuning buffer!! bufferID:%s scenario:%d", BID2Name(bufferID), mReqAttr.bufferScenario);
    return NULL;
}
MBOOL
NodeBufferHandler::
onProcessDone(DepthMapPipeNodeID nodeID)
{
    // release enqued buffer
    {
        // local all buffer map
        RWLock::AutoWLock _lI(mEnqueRWLock[eBUFFER_IMAGE]);
        RWLock::AutoWLock _lG(mEnqueRWLock[eBUFFER_GRAPHIC]);
        RWLock::AutoWLock _lT(mEnqueRWLock[eBUFFER_TUNING]);
        // release the enque buffer
        mEnqueBufferMap.editValueFor(nodeID).clear();
        mEnqueGraphicBufferMap.editValueFor(nodeID).clear();
        mEnqueTuningBufferMap.editValueFor(nodeID).clear();
    }
    // release working buffer
    {
        RWLock::AutoWLock _lI(mWorkingBufRWLock[eBUFFER_IMAGE]);
        RWLock::AutoWLock _lG(mWorkingBufRWLock[eBUFFER_GRAPHIC]);
        RWLock::AutoWLock _lT(mWorkingBufRWLock[eBUFFER_TUNING]);
        // release the enque buffer
        mWorkingImageBufferMap.editValueFor(nodeID).clear();
        mWorkingGraphicBufferMap.editValueFor(nodeID).clear();
        mWorkingTuningBufferMap.editValueFor(nodeID).clear();
    }

    return MTRUE;
}


MBOOL
NodeBufferHandler::
configOutBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    DepthMapPipeNodeID outNodeID
)
{
    return configInOutBuffer(srcNodeID, bufferID, outNodeID, bufferID);
}

MBOOL
NodeBufferHandler::
configInOutBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID srcBufferID,
    DepthMapPipeNodeID outNodeID,
    DepthMapBufferID outBufferID
)
{
    VSDOF_LOGD2("srcNodeID =%d srcBufferID=%s outNodeID=%d outBufferID=%s" , srcNodeID, BID2Name(srcBufferID), outNodeID, BID2Name(outBufferID));
    DepthBufferType bufferType;
    // check the need to config
    MBOOL bNeed = mpBufferPoolMgr->queryBufferType(srcBufferID, mReqAttr.bufferScenario, bufferType);
    if(!bNeed)
    {
        return MTRUE;
    }

    if(bufferType == eBUFFER_IMAGE)
    {
        SmartFatImageBuffer smBuf;
        BIDToSmartBufferMap queuedBufferMap;
        // read the buffer out
        {
            RWLock::AutoRLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
            queuedBufferMap = mEnqueBufferMap.valueFor(srcNodeID);
            ssize_t bufIndex = queuedBufferMap.indexOfKey(srcBufferID);

            if(bufIndex<0)
            {
                MY_LOGE("Cannot find the buffer to config, buffer id:%s", BID2Name(srcBufferID));
                return MFALSE;
            }
            smBuf = queuedBufferMap.valueAt(bufIndex);
            // remove the queue buffer
            queuedBufferMap.removeItemsAt(bufIndex);
        }
        // insert into output buffer
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
                MY_LOGE("Cannot find the graphic buffer to config, buffer id:%d", BID2Name(srcBufferID));
                return MFALSE;
            }
            smBuf = queuedBufferMap.valueAt(bufIndex);
            // remove the queue buffer
            queuedBufferMap.removeItemsAt(bufIndex);
        }
        // insert into output buffer
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
                MY_LOGE("Cannot find the tuning buffer to config, buffer id:%d", BID2Name(srcBufferID));
                return MFALSE;
            }
            smBuf = queuedBufferMap.valueAt(bufIndex);
            // remove the queue buffer
            queuedBufferMap.removeItemsAt(bufIndex);
        }
        // insert into output buffer
        this->addOutputBuffer(outNodeID, outBufferID, smBuf);
        return MTRUE;
    }

    MY_LOGE("Undefined bufferType:%d", bufferType);
    return MFALSE;
}

MBOOL
NodeBufferHandler::
configExternalOutBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    DepthMapPipeNodeID outNodeID,
    SmartFatImageBuffer smImgBuf
)
{
    VSDOF_LOGD2("configExternalOutBuffer +");
    // add src node buffer
    MBOOL bRet = addOutputBuffer(srcNodeID, bufferID, smImgBuf);
    bRet &= addOutputBuffer(outNodeID, bufferID, smImgBuf);
    VSDOF_LOGD2("configExternalOutBuffer -");
    return bRet;
}

MBOOL
NodeBufferHandler::
configExternalOutBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    DepthMapPipeNodeID outNodeID,
    SmartGraphicBuffer smGraImgBuf
)
{
    VSDOF_LOGD2("configExternalOutBuffer +");
    // add src node buffer
    MBOOL bRet = addOutputBuffer(srcNodeID, bufferID, smGraImgBuf);
    bRet &= addOutputBuffer(outNodeID, bufferID, smGraImgBuf);
    VSDOF_LOGD2("configExternalOutBuffer -");
    return bRet;
}

MBOOL
NodeBufferHandler::
addOutputBuffer(
    DepthMapPipeNodeID outNodeID,
    DepthMapBufferID bufferID,
    SmartFatImageBuffer smImgBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_IMAGE]);
    BIDToSmartBufferMap& bidBufMap = mEnqueBufferMap.editValueFor(outNodeID);
    bidBufMap.add(bufferID, smImgBuf);

    return MTRUE;
}

MBOOL
NodeBufferHandler::
addOutputBuffer(
    DepthMapPipeNodeID outNodeID,
    DepthMapBufferID bufferID,
    SmartGraphicBuffer smGraBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_GRAPHIC]);
    BIDToGraphicBufferMap& bidGraBufMap = mEnqueGraphicBufferMap.editValueFor(outNodeID);
    bidGraBufMap.add(bufferID, smGraBuf);

    return MTRUE;
}

MBOOL
NodeBufferHandler::
addOutputBuffer(
    DepthMapPipeNodeID outNodeID,
    DepthMapBufferID bufferID,
    SmartTuningBuffer smTuningBuf
)
{
    RWLock::AutoWLock _l(mEnqueRWLock[eBUFFER_TUNING]);
    BIDToTuningBufferMap& bidTuningBufMap = mEnqueTuningBufferMap.editValueFor(outNodeID);
    bidTuningBufMap.add(bufferID, smTuningBuf);

    return MTRUE;
}

MBOOL
NodeBufferHandler::
getEnqueBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    IImageBuffer*& pImgBuf
)
{
    // check buffer is inside the request
    BufferIOType ioType;
    if(mpDepthMapEffectReq->getBufferIOType(bufferID, ioType))
    {
        MBOOL bRet = mpDepthMapEffectReq->getRequestImageBuffer({.bufferID=bufferID, .ioType=ioType}, pImgBuf);
        if(!bRet)
            MY_LOGE("Cannot get enqued buffer in request, node id:%d, buffer id:%d  %s", srcNodeID, bufferID, BID2Name(bufferID));
        return bRet;
    }
    else
    {
        int index = 0;
        const BIDToSmartBufferMap& bidToBufMap = mEnqueBufferMap.valueFor(srcNodeID);
        if((index=bidToBufMap.indexOfKey(bufferID))>=0)
        {
            SmartFatImageBuffer smImgBuf = bidToBufMap.valueAt(index);
            pImgBuf = (smImgBuf == nullptr) ? nullptr : smImgBuf->mImageBuffer.get();
            return MTRUE;
        }
        const BIDToGraphicBufferMap& bidToGraBufMap = mEnqueGraphicBufferMap.valueFor(srcNodeID);
        if((index=bidToGraBufMap.indexOfKey(bufferID))>=0)
        {
            SmartGraphicBuffer smGraBuf = mEnqueGraphicBufferMap.valueFor(srcNodeID).valueAt(index);
            pImgBuf = (smGraBuf == nullptr) ? nullptr : smGraBuf->mImageBuffer.get();
            return MTRUE;
        }
        VSDOF_LOGD2("Cannot get enqued buffer, node id:%d, buffer id: %d %s", srcNodeID, bufferID, BID2Name(bufferID));
        return MFALSE;
    }
}

MBOOL
NodeBufferHandler::
getEnquedSmartBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    SmartFatImageBuffer& pSmImgBuf
)
{
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
NodeBufferHandler::
getEnquedSmartBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    SmartGraphicBuffer& pSmGraBuf
)
{
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
NodeBufferHandler::
getEnquedSmartBuffer(
    DepthMapPipeNodeID srcNodeID,
    DepthMapBufferID bufferID,
    SmartTuningBuffer& pSmTuningBuf
)
{
    ssize_t index;
    const BIDToTuningBufferMap& bidToBufMap = mEnqueTuningBufferMap.valueFor(srcNodeID);
    if((index=bidToBufMap.indexOfKey(bufferID))>=0)
    {
        pSmTuningBuf = bidToBufMap.valueAt(index);
        return MTRUE;
    }
    return MFALSE;
}

MVOID
NodeBufferHandler::
initEnqueBufferMap()
{
    // init enque buffer map
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
        for(int nodeId=0;nodeId<eDPETHMAP_PIPE_NODE_SIZE;++nodeId)
        {
            DepthMapPipeNodeID id = static_cast<DepthMapPipeNodeID>(nodeId);
            // init the enque buffer map
            mEnqueBufferMap.add(id, BIDToSmartBufferMap());
            mEnqueGraphicBufferMap.add(id, BIDToGraphicBufferMap());
            mEnqueTuningBufferMap.add(id, BIDToTuningBufferMap());
        }
    }
    
    // init working buffer map
    {
        // lock all working buffer map
        RWLock::AutoWLock _lI(mWorkingBufRWLock[eBUFFER_IMAGE]);
        RWLock::AutoWLock _lG(mWorkingBufRWLock[eBUFFER_GRAPHIC]);
        RWLock::AutoWLock _lT(mWorkingBufRWLock[eBUFFER_TUNING]);
        // clear all buffer map
        mWorkingImageBufferMap.clear();
        mWorkingGraphicBufferMap.clear();
        mWorkingTuningBufferMap.clear();
        // init maps
        for(int nodeId = 0; nodeId < eDPETHMAP_PIPE_NODE_SIZE; ++nodeId)
        {
            DepthMapPipeNodeID id = static_cast<DepthMapPipeNodeID>(nodeId);
            // init the working buffer map
            mWorkingImageBufferMap.add(id, SmartImgBufferVector());
            mWorkingGraphicBufferMap.add(id, SmartGraImgBufferVector());
            mWorkingTuningBufferMap.add(id, SmartTuningBufferVector());
        }
    }

}

MBOOL
NodeBufferHandler::
flush()
{
    VSDOF_LOGD2("NodeBufferHandler flush redID=%d +", mpDepthMapEffectReq->getRequestNo());
    initEnqueBufferMap();
    VSDOF_LOGD2("-");
    return MTRUE;
}



}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
