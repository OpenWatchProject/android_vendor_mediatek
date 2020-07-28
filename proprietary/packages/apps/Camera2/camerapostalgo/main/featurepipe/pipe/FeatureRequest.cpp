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



#include <pipe/FeatureRequest.h>

#include <utility>
#include <algorithm>

#define PIPE_CLASS_TAG "Request"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_DATA
#include <core/PipeLog.h>
#include <utils/std/LogTool.h>
//#include <mtkcam3/feature/DualCam/DualCam.Common.h>


#define __DEBUG


namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

//using namespace std;
using android::RWLock;

FeatureRequest::FeatureRequest()
: mEnableBoost(MFALSE)
{
    clear();
}

FeatureRequest::~FeatureRequest()
{
}

MVOID FeatureRequest::addBuffer(BufferID_T bufId, sp<BufferHandle> pBufHandle)
{
    BufferItem item;

#define BUFFER_DESC(buf, type, crossable)   \
    case buf:                               \
        MY_LOGD("add buffer id: %d", bufId); \
        item.mTypeId    = type;             \
        item.mCrossable = crossable;        \
        item.mCreated   = MTRUE;            \
        item.mpHandle   = pBufHandle;       \
        break;

    switch (bufId) {
        BUFFER_DESC(BID_MAN_IN_FULL,        TID_MAN_FULL_RAW,   MTRUE);
        BUFFER_DESC(BID_MAN_IN_YUV,         TID_MAN_FULL_YUV,   MTRUE);
        BUFFER_DESC(BID_MAN_IN_LCS,         TID_MAN_LCS,        MTRUE);
        BUFFER_DESC(BID_MAN_IN_RSZ,         TID_MAN_RSZ_RAW,    MTRUE);
        BUFFER_DESC(BID_MAN_OUT_YUV,        TID_OUT_YUV,        MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV0,       TID_OUT_YUV0,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV1,       TID_OUT_YUV1,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV2,       TID_OUT_YUV2,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV3,       TID_OUT_YUV3,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV4,       TID_OUT_YUV4,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV5,       TID_OUT_YUV5,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV6,       TID_OUT_YUV6,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_YUV7,       TID_OUT_YUV7,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_JPEG,       TID_JPEG,           MFALSE);
        BUFFER_DESC(BID_MAN_OUT_THUMBNAIL,  TID_THUMBNAIL,      MFALSE);
        BUFFER_DESC(BID_MAN_OUT_POSTVIEW,   TID_POSTVIEW,       MFALSE);
        BUFFER_DESC(BID_MAN_OUT_CLEAN,      TID_MAN_CLEAN,      MFALSE);
        BUFFER_DESC(BID_MAN_OUT_DEPTH,      TID_MAN_DEPTH,      MFALSE);
        BUFFER_DESC(BID_MAN_OUT_BOKEH,      TID_MAN_BOKEH,      MFALSE);
        BUFFER_DESC(BID_SUB_IN_FULL,        TID_SUB_FULL_RAW,   MTRUE);
        BUFFER_DESC(BID_SUB_IN_LCS,         TID_SUB_LCS,        MTRUE);
        BUFFER_DESC(BID_SUB_IN_RSZ,         TID_SUB_RSZ_RAW,    MTRUE);

        case BID_SUB_OUT_YUV00:
        case BID_SUB_OUT_YUV01:
            MY_LOGE("unsupport buffer id: %d", bufId);
            return;
        default:
            MY_LOGE("unknown buffer id: %d", bufId);
            return;
    }

#undef BUFFER_DESC

    mBufferItems.add(bufId, item);
}

sp<BufferHandle> FeatureRequest::getBuffer(BufferID_T bufId)
{
    sp<BufferHandle> pBuffer;

    // get input buffers from cross request
    auto pCrossRequest = mpCrossRequest.promote();
    if (pCrossRequest != NULL) {
        pCrossRequest->mBufferMutex.lock();

        auto& rBufferItems = pCrossRequest->mBufferItems;
        if (rBufferItems.indexOfKey(bufId) >= 0 &&
            rBufferItems.valueFor(bufId).mCrossable)
        {
            auto& rItem = rBufferItems.editValueFor(bufId);
            pBuffer = rItem.mpHandle;

            /** if buffer is not Acquired and not Cancel, Acquire it **/
            if (!rItem.mAcquired && (pBuffer != NULL)) {
                rItem.mAcquired = MTRUE;
                pCrossRequest->mBufferMutex.unlock();
                pBuffer->acquire();
            } else {
                pCrossRequest->mBufferMutex.unlock();
            }

            return pBuffer;
        } else {
            pCrossRequest->mBufferMutex.unlock();
        }
    }

    {
        mBufferMutex.lock();
        if (mBufferItems.indexOfKey(bufId) < 0) {
            mBufferMutex.unlock();
            return NULL;
        }

        auto& rItem = mBufferItems.editValueFor(bufId);
        pBuffer = rItem.mpHandle;

        /** if buffer is not Acquired and not Cancel, Acquire it **/
        if (!rItem.mAcquired && (pBuffer != NULL)) {
            rItem.mAcquired = MTRUE;
            mBufferMutex.unlock();
            pBuffer->acquire();
        } else {
            mBufferMutex.unlock();
        }
    }

    return pBuffer;
}

MVOID FeatureRequest::setCrossRequest(sp<FeatureRequest> pRequest)
{
    mpCrossRequest = pRequest;
}

sp<FeatureRequest> FeatureRequest::getCrossRequest()
{
    return mpCrossRequest.promote();
}

MVOID FeatureRequest::setDelayInference(std::function<void()>& func)
{
    Mutex::Autolock lock(mInferMutex);
    mInferFunction = move(func);
}

MBOOL FeatureRequest::hasDelayInference()
{
    Mutex::Autolock lock(mInferMutex);
    return mInferFunction != NULL;
}

MVOID FeatureRequest::startInference()
{
    if (hasDelayInference()) {
        Mutex::Autolock lock(mInferMutex);
        mInferFunction();
        mInferFunction = NULL;
        mInferCond.broadcast();
    }
}

MVOID FeatureRequest::waitInference()
{
    Mutex::Autolock lock(mInferMutex);
    if (mInferFunction != NULL) {
        MY_LOGI("wait+");
        mInferCond.wait(mInferMutex);
        MY_LOGI("wait-");
    }
}

MVOID FeatureRequest::addBufferItem(BufferID_T bufId, TypeID_T typeId, MSize& size, Format_T fmt, MSize& align)
{
    BufferItem item;
    item.mTypeId = typeId;
    item.mCreated = MFALSE;
    item.mSize = size;
    item.mFormat = fmt;
    item.mAlign  = align;
    mBufferItems.add(bufId, item);
}

MVOID FeatureRequest::addMetadata(MetadataID_T metaId, sp<MetadataHandle> pMetaHandle)
{
    MetadataItem item;

    switch (metaId) {
        case MID_MAN_IN_P1_DYNAMIC:
        case MID_MAN_IN_APP:
        case MID_MAN_IN_HAL:
        case MID_SUB_IN_P1_DYNAMIC:
        case MID_SUB_IN_HAL:
            item.mCrossable = MTRUE;
            break;

        case MID_MAN_OUT_APP:
        case MID_MAN_OUT_HAL:
        case MID_SUB_OUT_APP:
        case MID_SUB_OUT_HAL:
            break;

        default:
           MY_LOGE("unknown metadata id: %d", metaId);
           return;
    }

    item.mpHandle = pMetaHandle;
    mMetadataItems.add(metaId, item);
}

sp<MetadataHandle> FeatureRequest::getMetadata(MetadataID_T metaId)
{
    sp<MetadataHandle> pMetadata;

    auto pCrossRequest = mpCrossRequest.promote();
    if (pCrossRequest != NULL) {
        Mutex::Autolock lock(pCrossRequest->mMetadataMutex);

        auto& rMetadataItems = pCrossRequest->mMetadataItems;
        if (rMetadataItems.indexOfKey(metaId) >= 0 &&
            rMetadataItems.valueFor(metaId).mCrossable)
        {
            auto& rItem = rMetadataItems.editValueFor(metaId);
            pMetadata = rItem.mpHandle;

            if (!rItem.mAcquired) {
                pMetadata->acquire();
                rItem.mAcquired = MTRUE;
            }

            return pMetadata;
        }
    }

    {
        Mutex::Autolock lock(mMetadataMutex);
        if (mMetadataItems.indexOfKey(metaId) < 0)
            return NULL;

        auto& rItem = mMetadataItems.editValueFor(metaId);
        pMetadata = rItem.mpHandle;

        if (!rItem.mAcquired) {
            pMetadata->acquire();
            rItem.mAcquired = MTRUE;
        }
    }

    return pMetadata;
}

MVOID FeatureRequest::addParameter(ParameterID_T paramId, MINT32 value)
{
    auto pCrossRequest = mpCrossRequest.promote();
    if (pCrossRequest != NULL)
    {
        RWLock::AutoWLock lock(pCrossRequest->mParamRWLock);
        pCrossRequest->mParameter[paramId] = value;
    }
    else
    {
        RWLock::AutoWLock _l(mParamRWLock);
        mParameter[paramId] = value;
    }
}

MVOID FeatureRequest::deleteParameter(ParameterID_T paramId)
{
    auto pCrossRequest = mpCrossRequest.promote();
    if (pCrossRequest != NULL)
    {
        RWLock::AutoWLock lock(pCrossRequest->mParamRWLock);
        pCrossRequest->mParameter[paramId] = -1;
    }
    else
    {
        RWLock::AutoWLock _l(mParamRWLock);
        mParameter[paramId] = -1;
    }
}

MINT32 FeatureRequest::getParameter(ParameterID_T paramId)
{
    auto pCrossRequest = mpCrossRequest.promote();
    if (pCrossRequest != NULL)
    {
        RWLock::AutoRLock lock(pCrossRequest->mParamRWLock);
        return pCrossRequest->mParameter[paramId];
    }
    else
    {
        RWLock::AutoRLock _l(mParamRWLock);
        return mParameter[paramId];
    }
}

MBOOL FeatureRequest::hasParameter(ParameterID_T paramId)
{
    return getParameter(paramId) >= 0;
}

MINT32 FeatureRequest::getRequestNo()
{
    return getParameter(PID_REQUEST_NUM);
}

MINT32 FeatureRequest::getTimestamp()
{
    return getParameter(PID_TIMESTAMP);
}

MBOOL FeatureRequest::isSingleFrame()
{
    if (getParameter(PID_FRAME_COUNT) < 2)
        return MTRUE;
    else if(this->isUnderBSS() && this->getBSSFrameCount() < 2)
        return MTRUE;

    return MFALSE;
}

MBOOL FeatureRequest::isPipelineFirstFrame()
{
    return (getPipelineFrameIndex() == 0);
}

MBOOL FeatureRequest::isPipelineLastFrame()
{
    return (getPipelineFrameCount() == (getPipelineFrameIndex() + 1));
}

MINT32 FeatureRequest::getPipelineFrameCount()
{
    auto count = getParameter(PID_FRAME_COUNT);
    if(count< 2)
        return 1;
    return count;
}

MINT32 FeatureRequest::getPipelineFrameIndex()
{
    auto index = getParameter(PID_FRAME_INDEX);;
    if(index<0)
        return 0;
    return index;
}

MBOOL FeatureRequest::isUnderBSS()
{
    if(hasParameter(PID_BSS_REQ_STATE) &&
       getParameter(PID_BSS_REQ_STATE) != BSS_STATE_NOT_BSS_REQ
    )
        return MTRUE;
    else
        return MFALSE;
}

MBOOL FeatureRequest::isBypassBSS()
{
    if(hasParameter(PID_BSS_REQ_STATE) &&
       getParameter(PID_BSS_REQ_STATE) == BSS_STATE_BYPASS_BSS
    )
        return MTRUE;
    else
        return MFALSE;
}

MBOOL FeatureRequest::isBSSFirstFrame()
{
    return (getBSSFrameIndex() == 0);
}

MBOOL FeatureRequest::isBSSLastFrame()
{
    return (getBSSFrameCount() == (getBSSFrameIndex() + 1));
}

MINT32 FeatureRequest::getBSSFrameCount()
{
    auto frameCount = getParameter(PID_FRAME_COUNT);

    if (frameCount< 2)
        return 1;

    auto count = getParameter(PID_BSS_BYPASSED_COUNT);
    MINT32 bypassBSS = (count>0) ? count : 0;
    count = getParameter(PID_DROPPED_COUNT);
    MINT32 dropCount = (count>0) ? count : 0;

    return frameCount - bypassBSS - dropCount;
}

MINT32 FeatureRequest::getBSSFrameIndex()
{
    auto index = getParameter(PID_BSS_ORDER);
    if (index >= 0)
        return index;
    else
        return -1;
}

MBOOL FeatureRequest::isActiveFirstFrame()
{
    if(!this->isUnderBSS())
        return this->isPipelineFirstFrame();
    else
        return this->isBSSFirstFrame();
}

MBOOL FeatureRequest::isActiveLastFrame()
{
    if(!this->isUnderBSS())
        return this->isPipelineLastFrame();
    else
        return this->isBSSLastFrame();
}

MUINT32 FeatureRequest::getActiveFrameCount()
{
    if(!this->isUnderBSS())
        return this->getPipelineFrameCount();
    else
        return this->getBSSFrameCount();
}

MINT32 FeatureRequest::getActiveFrameIndex()
{
    if(!this->isUnderBSS())
        return this->getPipelineFrameIndex();
    else
        return this->getBSSFrameIndex();
}

MBOOL FeatureRequest::isMainFrame()
{
    if(this->getActiveFrameIndex() == 0)
        return MTRUE;
    else
        return MFALSE;
}


MBOOL FeatureRequest::isDropFrame()
{
    if(this->hasParameter(PID_DROPPED_FRAME) &&
        this->getParameter(PID_DROPPED_FRAME) == 1)
        return MTRUE;
    else
        return MFALSE;
}

MVOID FeatureRequest::addPath(PathID_T pathId)
{
    MY_LOGD("add Path=%d %s", pathId, PathID2Name(pathId));
    const NodeID_T* nodeId =GetPath(pathId);
    NodeID_T src = nodeId[0];
    NodeID_T dst = nodeId[1];

    MY_LOGD("addPath src=%s  dst=%s", NodeID2Name(src), NodeID2Name(dst));

    mNodePath[src][dst] = pathId;

    mPathToDo.markBit(pathId);
    mNodeToDo.markBit(dst);
}

MVOID FeatureRequest::lock() {
    mRequestMutex.lock();
}

MVOID FeatureRequest::unlock() {
    mRequestMutex.unlock();
}

MVOID FeatureRequest::finishPath_Locked(PathID_T pathId)
{
    mPathToDo.clearBit(pathId);
}

MVOID FeatureRequest::finishNode_Locked(NodeID_T nodeId)
{
    mNodeToDo.clearBit(nodeId);
}

MBOOL FeatureRequest::isFinished_Locked()
{
    return mPathToDo.isEmpty() && mNodeToDo.isEmpty();
}

MBOOL FeatureRequest::isReadyFor(NodeID_T nodeId)
{
    for (NodeID_T i = 0; i < NUM_OF_NODE; i++) {
        if (mNodePath[i][nodeId] == NULL_PATH)
            continue;

        PathID_T pathId = FindPath(i, nodeId);
        if (pathId == NULL_PATH)
            continue;

        if (mPathToDo.hasBit(pathId))
            return MFALSE;
    }

    return MTRUE;
}

MBOOL FeatureRequest::isCancelled()
{
    return (hasParameter(PID_ABORTED) && !hasParameter(PID_RESTORED)) ||
           hasParameter(PID_FAILURE) ||
           hasParameter(PID_DROPPED_FRAME);
}

Vector<NodeID_T> FeatureRequest::getPreviousNodes(NodeID_T nodeId)
{
    Vector<NodeID_T> vec;
    for (int i = 0; i < NUM_OF_NODE; i++) {
        if (mNodePath[i][nodeId] != NULL_PATH) {
            vec.push_back(i);
        }
    }
    return vec;
}

Vector<NodeID_T> FeatureRequest::getNextNodes(NodeID_T nodeId)
{
    Vector<NodeID_T> vec;
    for (int i = 0; i < NUM_OF_NODE; i++) {
        if (mNodePath[nodeId][i] != NULL_PATH) {
            vec.push_back(i);
        }
    }
    return vec;
}

MVOID FeatureRequest::addFeature(FeatureID_T fid)
{
    ALOGD("add Feature %u", fid);
    mFeatures.markBit(fid);
}

MVOID FeatureRequest::setFeatures(MUINT64 features)
{
    mFeatures.value = features;
}

MBOOL FeatureRequest::hasFeature(FeatureID_T fid)
{
    return mFeatures.hasBit(fid);
}

MVOID FeatureRequest::clear()
{
    memset(mNodePath, NULL_PATH, sizeof(mNodePath));
    {
        RWLock::AutoWLock _l(mParamRWLock);
        std::fill_n(mParameter, NUM_OF_TOTAL_PARAMETER, -1);
    }
}

MVOID FeatureRequest::addNodeIO(
        NodeID_T nodeId, Vector<BufferID_T>& vInBufId, Vector<BufferID_T>& vOutBufId, Vector<MetadataID_T>& vMetaId)
{
    sp<FeatureNodeRequest> pNodeRequest = new FeatureNodeRequest(this);

    for (BufferID_T bufId : vInBufId) {
        if (mBufferItems.indexOfKey(bufId) < 0) {
            MY_LOGE("can not find buffer, id:%d", bufId);
            continue;
        }

        TypeID_T typeId = mBufferItems.valueFor(bufId).mTypeId;

        pNodeRequest->mITypeMap.add(typeId, bufId);
        mBufferItems.editValueFor(bufId).mReference++;
#ifdef __DEBUG
        MY_LOGI("[R:%d/Timestamp%d] In, nodeId(%d), typeId(%d), bufferId(%d), reference(%d)",
                  getRequestNo(), getTimestamp(), nodeId, typeId, bufId, mBufferItems.editValueFor(bufId).mReference);
#endif
    }

    for (BufferID_T bufId : vOutBufId) {
        if (mBufferItems.indexOfKey(bufId) < 0) {
            MY_LOGE("can not find buffer, id:%d", bufId);
            continue;
        }

        TypeID_T typeId = mBufferItems.valueFor(bufId).mTypeId;

        pNodeRequest->mOTypeMap.add(typeId, bufId);
        mBufferItems.editValueFor(bufId).mReference++;
#ifdef __DEBUG
        MY_LOGI("[R:%d/Timestamp:%d] Out nodeId(%d), typeId(%d), bufferId(%d), reference(%d)",
                  getRequestNo(), getTimestamp(), nodeId, typeId, bufId, mBufferItems.editValueFor(bufId).mReference);
#endif
    }

    for (MetadataID_T metaId : vMetaId) {
        if (mMetadataItems.indexOfKey(metaId) < 0)
            continue;

        pNodeRequest->mMetadataSet.markBit(metaId);
        mMetadataItems.editValueFor(metaId).mReference++;
#ifdef __DEBUG
        MY_LOGI("[R:%d/Timestamp:%d] nodeId(%d), metaId(%d), reference(%d)",
                  getRequestNo(), getTimestamp(), nodeId, metaId, mMetadataItems.editValueFor(metaId).mReference);
#endif
    }
    MY_LOGD("Add node requet, node=%s", NodeID2Name(nodeId));
    mNodeRequest.add(nodeId, pNodeRequest);
}

MVOID FeatureRequest::incBufferRef(BufferID_T bufId)
{
    Mutex::Autolock lock(mBufferMutex);
    if (mBufferItems.indexOfKey(bufId) >= 0) {
        MUINT32& ref = mBufferItems.editValueFor(bufId).mReference;

        if (ref <= 0) {
            MY_LOGD("Buffer was already released. timestamp:%d, buffer:%d, ref:%d",
                getTimestamp(), bufId, ref);
        } else {
            ref++;
        }
    }
}

MVOID FeatureRequest::decBufferRef(BufferID_T bufId)
{
    auto pCrossRequest = mpCrossRequest.promote();
    if (pCrossRequest != NULL) {

        bool bCrossBuffer;
        {
            Mutex::Autolock lock(pCrossRequest->mBufferMutex);
            auto& rBufferItems = pCrossRequest->mBufferItems;
            bCrossBuffer = rBufferItems.indexOfKey(bufId) >= 0 && rBufferItems.valueFor(bufId).mCrossable;
        }

        if (bCrossBuffer) {
            MUINT32 crossRef;
            {
                Mutex::Autolock lock(mBufferMutex);
                if (mBufferItems.indexOfKey(bufId) < 0) {
                    MY_LOGE("Local buffer(%d) is not existed.", bufId);
                    return;
                }

                // The cross request is to exchange the buffer handle, not to exchange reference count
                // Minus to the local reference count
                auto& rLocalItem = mBufferItems.editValueFor(bufId);
                MUINT32& ref = rLocalItem.mReference;
                crossRef = --ref;
            }

            if (crossRef <= 0)
            {
                Mutex::Autolock lock(pCrossRequest->mBufferMutex);
                auto& rBufferItems = pCrossRequest->mBufferItems;
                auto& rCrossItem = rBufferItems.editValueFor(bufId);
                if (rCrossItem.mpHandle != NULL) {
                    sp<BufferHandle> pBuffer = rCrossItem.mpHandle;
                    pBuffer->release();
                    rCrossItem.mpHandle = NULL;
#ifdef __DEBUG
                    MY_LOGI("The cross buffer(%d) has been released.", bufId);
#endif
                } else {
                    MY_LOGW("The buffer(%d) had been released.", bufId);
                }
            }
    #ifdef __DEBUG
                MY_LOGI("[R: %d/timestamp:%d] decRef(cross) bufId(%d), reference(%d)",
                          getRequestNo(), getTimestamp(), bufId, crossRef);
    #endif
            return;
        }
    }

    {
        Mutex::Autolock lock(mBufferMutex);
        if (mBufferItems.indexOfKey(bufId) >= 0) {
            auto& rLocalItem = mBufferItems.editValueFor(bufId);

            MUINT32& ref = rLocalItem.mReference;
            if (--ref <= 0) {
                // BSS: no allocated buffer if dropped
                if (rLocalItem.mpHandle != NULL) {
                    sp<BufferHandle> pBuffer = rLocalItem.mpHandle;
                    pBuffer->release();
                    rLocalItem.mpHandle = NULL;
#ifdef __DEBUG
                    MY_LOGI("The buffer(%d) has been released.", bufId);
#endif
                }
            }
#ifdef __DEBUG
            MY_LOGI("[R:%d/Timestamp:%d] decRef bufId(%d), reference(%d)",
                      getRequestNo(), getTimestamp(), bufId, rLocalItem.mReference);
#endif
        }
    }
}

MVOID FeatureRequest::decMetadataRef(MetadataID_T metaId)
{
    auto pCrossRequest = mpCrossRequest.promote();
    if (pCrossRequest != NULL) {
        bool bCrossMeta;
        {
            Mutex::Autolock lock(pCrossRequest->mMetadataMutex);
            auto& rMetadataItems = pCrossRequest->mMetadataItems;
            bCrossMeta = rMetadataItems.indexOfKey(metaId) >= 0 && rMetadataItems.valueFor(metaId).mCrossable;
        }
        if (bCrossMeta) {
            MUINT32 crossRef;
            {
                Mutex::Autolock lock(mMetadataMutex);
                if (mMetadataItems.indexOfKey(metaId) < 0) {
                    MY_LOGE("Local metadata(%d) is not existed.", metaId);
                    return;
                }

                // The cross request is to exchange the metadata handle, not to exchange reference count
                // Minus to the local reference count
                auto& rLocalItem = mMetadataItems.editValueFor(metaId);
                MUINT32& ref =  rLocalItem.mReference;
                crossRef = --ref;
            }
            if (crossRef <= 0)
            {
                Mutex::Autolock lock(pCrossRequest->mMetadataMutex);
                auto& rMetadataItems = pCrossRequest->mMetadataItems;
                auto& rCrossItem = rMetadataItems.editValueFor(metaId);
                if (rCrossItem.mpHandle != NULL) {
                    sp<MetadataHandle> pHandle = rCrossItem.mpHandle;
                    pHandle->release();
                    rCrossItem.mpHandle = NULL;
                } else {
                    MY_LOGW("The metadata(%d) had been released.", metaId);
                }
            }
    #ifdef __DEBUG
                MY_LOGI("[R:%d/Timestamp:%d] decRef(cross) metaId(%d), reference(%d)",
                        getRequestNo(), getTimestamp(), metaId, crossRef);
    #endif
            return;
        }
    }

    {
        Mutex::Autolock lock(mMetadataMutex);
        if (mMetadataItems.indexOfKey(metaId) >= 0) {
            auto& rLocalItem = mMetadataItems.editValueFor(metaId);

            MUINT32& ref =  rLocalItem.mReference;
            if (--ref <= 0) {
                // BSS: no allocated buffer if dropped
                if (rLocalItem.mpHandle != NULL) {
                    sp<MetadataHandle> pHandle = rLocalItem.mpHandle;
                    pHandle->release();
                    rLocalItem.mpHandle = NULL;
                }
            }
#ifdef __DEBUG
            MY_LOGI("[R:%d/Timestamp:%d] decRef metaId(%d), reference(%d)",
                    getRequestNo(), getTimestamp(), metaId, rLocalItem.mReference);
#endif
        }
    }
}

sp<FeatureNodeRequest> FeatureRequest::getNodeRequest(NodeID_T nodeId)
{
    if (mNodeRequest.indexOfKey(nodeId) >= 0)
        return mNodeRequest.valueFor(nodeId);

    return NULL;
}

MVOID FeatureRequest::decNodeReference(NodeID_T nodeId)
{
    if (mNodeRequest.indexOfKey(nodeId) < 0)
        return;
    sp<FeatureNodeRequest> pNodeReq = mNodeRequest.valueFor(nodeId);
    auto& rITypeMap = pNodeReq->mITypeMap;
    auto& rOTypeMap = pNodeReq->mOTypeMap;
    auto& rITypeReleased = pNodeReq->mITypeReleased;
    auto& rOTypeReleased = pNodeReq->mOTypeReleased;

    for (size_t i = 0; i < rITypeMap.size(); i++) {
        TypeID_T typeId = rITypeMap.keyAt(i);
        BufferID_T bufId = rITypeMap.valueAt(i);
        if (!rITypeReleased.hasBit(typeId)) {
            rITypeReleased.markBit(typeId);
            decBufferRef(bufId);
        }
    }

    for (size_t i = 0; i < rOTypeMap.size(); i++) {
        BufferID_T bufId = rOTypeMap.valueAt(i);
        TypeID_T typeId = rOTypeMap.keyAt(i);
        if (!rOTypeReleased.hasBit(typeId)) {
            rOTypeReleased.markBit(typeId);
            decBufferRef(bufId);
        }
    }

    for (MetadataID_T i = 0; i < NUM_OF_METADATA; i++) {
        if (pNodeReq->hasMetadata(i))
            decMetadataRef(i);
    }
    return;
}

MVOID FeatureRequest::dump()
{
    android::LogPrinter logPrinter(LOG_TAG, ANDROID_LOG_DEBUG, "[dump]");
    dump(logPrinter);
}

MVOID FeatureRequest::dump(android::Printer& printer)
{
    String8 strFeature;
    for (FeatureID_T featId = 0; featId < NUM_OF_FEATURE; featId++) {
        if (mFeatures.hasBit(featId)) {
            if (strFeature.length() > 0) {
                strFeature += "/";
            }
            strFeature += FeatID2Name(featId);
        }
    }

    if (hasParameter(PID_FRAME_INDEX) && hasParameter(PID_FRAME_COUNT)) {
        printer.printFormatLine("  [R:%d/Timestamp:%d] [Multi-Frame Index/Count/BSSIndex/BSSCount: %d/%d/%d/%d Features: %s]",
                getRequestNo(),
                getTimestamp(),
                getPipelineFrameIndex(),
                getPipelineFrameCount(),
                getBSSFrameIndex(),
                getBSSFrameCount(),
                strFeature.string());
    } else {
        printer.printFormatLine("  [R:%d/Timestamp:%d] [Features: %s]",
                getRequestNo(),
                getTimestamp(),
                strFeature.string());
    }

    auto pTool = LogTool::get();
    for (size_t i = 0; i < mNodeRequest.size(); i++)
    {
        NodeID_T nodeId = mNodeRequest.keyAt(i);
        const sp<FeatureNodeRequest> pNodeReq = mNodeRequest.valueAt(i);

        timespec& rBegin = mNodeBegin[nodeId];
        timespec& rEnd = mNodeEnd[nodeId];
        printer.printFormatLine("      NODE %-10s [%s ~ %s]",
                String8::format("[%s]", NodeID2Name(nodeId)).string(),
                (rBegin.tv_sec) ? pTool->convertToFormattedLogTime(&rBegin).c_str() : "",
                (rEnd.tv_sec)   ? pTool->convertToFormattedLogTime(&rEnd).c_str()   : "");


        auto& rITypeMap = pNodeReq->mITypeMap;
        for (size_t j = 0; j < rITypeMap.size(); j++) {

            TypeID_T typeId = rITypeMap.keyAt(j);
            BufferID_T bufId = rITypeMap.valueAt(j);
            const sp<BufferHandle> pBuffer = getBuffer(bufId);

            if (pBuffer == NULL) {
                printer.printFormatLine("          INPUT-%zu     %-18s Released",
                        j, TypeID2Name(typeId));
            } else {
                printer.printFormatLine("          INPUT-%zu     %-18s Handle:0x%p",
                        j, TypeID2Name(typeId), pBuffer.get());
            }
        }

        auto& rOTypeMap = pNodeReq->mOTypeMap;
        for (size_t j = 0; j < rOTypeMap.size(); j++) {

            TypeID_T typeId = rOTypeMap.keyAt(j);
            BufferID_T bufId = rOTypeMap.valueAt(j);
            const sp<BufferHandle> pBuffer = getBuffer(bufId);
            if (pBuffer == NULL) {
                printer.printFormatLine("          OUTPUT-%zu    %-18s Released",
                        j, TypeID2Name(typeId));
            } else {
                printer.printFormatLine("          OUTPUT-%zu    %-18s Handle:0x%p",
                        j, TypeID2Name(typeId), pBuffer.get());
            }
        }

    }
}

MVOID FeatureRequest::dump(const char* name, NodeID_T nodeId, TypeID_T typeId, Direction dir)
{
    sp<FeatureNodeRequest> nodeRequestPtr = getNodeRequest(nodeId);
    const BufferID_T bufId = nodeRequestPtr->mapBufferID(typeId, dir);
    if ( bufId == NULL_BUFFER )
    {
        MY_LOGW("failed to get bufId, nodeId:%d(%s), typeId:%d(%s), dir:%d, name:%s",
            nodeId, NodeID2Name(nodeId), typeId, TypeID2Name(typeId), dir, name);
    }
    else
    {
        sp<BufferHandle> bufferHandle = getBuffer(bufId);
        IImageBuffer* pImgBuf = bufferHandle->native();
        dump(name, nodeId, pImgBuf);
    }
}

MVOID FeatureRequest::dump(const char* name, NodeID_T nodeId, IImageBuffer* pImgBuf)
{
    if( (name == NULL) || (pImgBuf == NULL) )
    {
        MY_LOGW("invalid arguments, name:%p, pImgBuf:%p", name, pImgBuf);
        return;
    }

    const MINT32 reqNum = getRequestNo();
    const MINT32 timestamp = getTimestamp();
    const String8 relatedFilePath = String8::format("%08d_%16d/%s", reqNum, timestamp, NodeID2Name(nodeId));
    //
    const MSize imgSize = pImgBuf->getImgSize();
    const size_t imgStrides = pImgBuf->getBufStridesInBytes(0);
    const MINT imgFmt = pImgBuf->getImgFormat();
    MY_LOGD("dump image, relatedFilePath:%s, name:%s, addr:%p, size:(%dx%d), fmt:%#09x, stride:%zu",
        relatedFilePath.string(), name, pImgBuf, imgSize.w, imgSize.h, imgFmt, imgStrides);

    // TODO: get image format to file extension
    const char* extensionName = "yuv";

    static const MINT32 writePermission = 0660;
    static const std::string directoryName = "/sdcard/capturePipe";
    const String8 directoryPath = String8::format("%s/%s", directoryName.c_str(), relatedFilePath.string());
    const MBOOL isSuccessCreateDirectory = NSCam::Utils::makePath(directoryPath.string(), writePermission);
    if ( !isSuccessCreateDirectory )
    {
        MY_LOGW("failed to create directory, fullFilePath:%s", directoryPath.string());
        return;
    }
    //
    const String8 fullFilePath = String8::format("%s/%s_%dx%d_%zu_%s.%s",
        directoryPath.string(), name, imgSize.w, imgSize.h, imgStrides, ImgFmt2Name(imgFmt), extensionName);
    pImgBuf->saveToFile(fullFilePath.string());
}

std::string FeatureRequest::getStatus(std::string& strDispatch)
{
    String8 ret;
    String8 strFeature;
    String8 strNode;
    for (FeatureID_T featId = 0; featId < NUM_OF_FEATURE; featId++) {
        if (mFeatures.hasBit(featId)) {
            if (strFeature.length() > 0) {
                strFeature += "/";
            }
            strFeature += FeatID2Name(featId);
        }
    }

    if (hasParameter(PID_FRAME_INDEX) && hasParameter(PID_FRAME_COUNT)) {
        ret = String8::format("[R:%d/Timestamp:%d]  [Multi-Frame Index/Count/BSSIndex/BSSCount: %d/%d/%d/%d Features: %s]",
                              getRequestNo(),
                              getTimestamp(),
                              getPipelineFrameIndex(),
                              getPipelineFrameCount(),
                              getBSSFrameIndex(),
                              getBSSFrameCount(),
                              strFeature.string());
    } else {
        ret = String8::format("  [R:%d/Timestamp:%d]  [Features: %s]",
                              getRequestNo(),
                              getTimestamp(),
                              strFeature.string());
    }
    MY_LOGI("%s", ret.string());

    auto pTool = LogTool::get();

    ret += String8::format(" [NOT Finish Node:");
    for (size_t i = 0; i < mNodeRequest.size(); i++)
    {
        NodeID_T nodeId = mNodeRequest.keyAt(i);
        const sp<FeatureNodeRequest> pNodeReq = mNodeRequest.valueAt(i);

        timespec& rBegin = mNodeBegin[nodeId];
        timespec& rEnd = mNodeEnd[nodeId];

        MY_LOGI("      NODE %-10s [%s ~ %s]",
                String8::format("[%s]", NodeID2Name(nodeId)).string(),
                (rBegin.tv_sec) ? pTool->convertToFormattedLogTime(&rBegin).c_str() : "",
                (rEnd.tv_sec)   ? pTool->convertToFormattedLogTime(&rEnd).c_str()   : "");

        auto& rITypeMap = pNodeReq->mITypeMap;
        for (size_t j = 0; j < rITypeMap.size(); j++) {

            TypeID_T typeId = rITypeMap.keyAt(j);
            BufferID_T bufId = rITypeMap.valueAt(j);
            const sp<BufferHandle> pBuffer = getBuffer(bufId);

            if (pBuffer == NULL) {
                MY_LOGI("          INPUT-%zu     %-18s Released",
                        j, TypeID2Name(typeId));
            } else {
                MY_LOGI("          INPUT-%zu     %-18s Handle:0x%p",
                        j, TypeID2Name(typeId), pBuffer.get());
            }
        }

        auto& rOTypeMap = pNodeReq->mOTypeMap;
        for (size_t j = 0; j < rOTypeMap.size(); j++) {

            TypeID_T typeId = rOTypeMap.keyAt(j);
            BufferID_T bufId = rOTypeMap.valueAt(j);
            const sp<BufferHandle> pBuffer = getBuffer(bufId);
            if (pBuffer == NULL) {
                MY_LOGI("          OUTPUT-%zu    %-18s Released",
                        j, TypeID2Name(typeId));
            } else {
                MY_LOGI("          OUTPUT-%zu    %-18s Handle:0x%p",
                        j, TypeID2Name(typeId), pBuffer.get());
            }
        }

        if (!rEnd.tv_sec) {
            if (strNode.length() > 0) {
                strNode += "/";
            }

            strNode += String8::format("%s", NodeID2Name(nodeId));
            if (strDispatch.size() == 0) {
                String8 strTmp = String8::format(" NOT Finish Node: %s", NodeID2Name(nodeId));
                strDispatch = strTmp.string();
            }
        }
    }
    ret += String8::format("%s]", strNode.c_str());

    return ret.string();
}

BufferID_T FeatureNodeRequest::mapBufferID(TypeID_T typeId, Direction dir)
{
    BufferID_T bufId = NULL_BUFFER;
    if (dir == INPUT) {
        if (mITypeMap.indexOfKey(typeId) >= 0) {
            bufId = mITypeMap.valueFor(typeId);
            mIBufferMap.add(bufId, typeId);
        }
    } else {
        if (mOTypeMap.indexOfKey(typeId) >= 0) {
            bufId = mOTypeMap.valueFor(typeId);
            mOBufferMap.add(bufId, typeId);
        }
    }

    return bufId;
}

MBOOL FeatureNodeRequest::hasMetadata(MetadataID_T metaId)
{
    return mMetadataSet.hasBit(metaId);
}

IImageBuffer* FeatureNodeRequest::acquireBuffer(BufferID_T bufId)
{
    if (bufId == NULL_BUFFER)
        return NULL;

    if (mIBufferMap.indexOfKey(bufId) >= 0)
    {
        TypeID_T typeId = mIBufferMap.valueFor(bufId);
        if (mITypeAcquired.hasBit(typeId))
        {
            MY_LOGW("Input:%s already acquired", TypeID2Name(typeId));
        }
        mITypeAcquired.markBit(typeId);

    }
    else if (mOBufferMap.indexOfKey(bufId) >= 0)
    {
        TypeID_T typeId = mOBufferMap.valueFor(bufId);
        if (mOTypeAcquired.hasBit(typeId))
        {
            MY_LOGW("Output:%s already acquired", TypeID2Name(typeId));
        }
        mOTypeAcquired.markBit(typeId);
    }

    auto pBufferHandle = mpRequest->getBuffer(bufId);

    if (pBufferHandle == NULL)
        return NULL;

    return pBufferHandle->native();
}

MVOID FeatureNodeRequest::releaseBuffer(BufferID_T bufId)
{
    if (mIBufferMap.indexOfKey(bufId) >= 0)
    {
        TypeID_T typeId = mIBufferMap.valueFor(bufId);
        mITypeReleased.markBit(typeId);
    }
    else if (mOBufferMap.indexOfKey(bufId) >= 0)
    {
        TypeID_T typeId = mOBufferMap.valueFor(bufId);
        mOTypeReleased.markBit(typeId);
    }

    mpRequest->decBufferRef(bufId);
}

IMetadata* FeatureNodeRequest::acquireMetadata(MetadataID_T metaId)
{
    if (metaId == NULL_METADATA)
        return NULL;

    if (!hasMetadata(metaId))
        return NULL;

    sp<MetadataHandle> pMetaHandle = mpRequest->getMetadata(metaId);
    if (pMetaHandle == NULL)
        return NULL;

    return pMetaHandle->native();
}

MVOID FeatureNodeRequest::releaseMetadata(MetadataID_T metaId)
{
    if (!hasMetadata(metaId))
        return;

    mpRequest->decMetadataRef(metaId);
}

MUINT32 FeatureNodeRequest::getImageTransform(BufferID_T bufId) const
{
    auto pBuffer = mpRequest->getBuffer(bufId);
    if (pBuffer == NULL)
        return 0;

    return pBuffer->getTransform();
}

MINT FeatureRequest::getImageFormat(BufferID_T bufId)
{
    if (mBufferItems.indexOfKey(bufId) < 0) {
        MY_LOGE("can not find buffer ID:%d", bufId);
        return 0;
    }

    if (bufId & PIPE_BUFFER_STARTER) {
        return  mBufferItems.valueFor(bufId).mFormat;
    } else {
        auto pBufferHandle = this->getBuffer(bufId);
        if (pBufferHandle == NULL)
            return 0;
        return pBufferHandle->native()->getImgFormat();
    }
}

MSize FeatureRequest::getImageSize(BufferID_T bufId)
{
    if (mBufferItems.indexOfKey(bufId) < 0) {
        MY_LOGE("can not find buffer ID:%d", bufId);
        return MSize(0, 0);
    }

    if (bufId & PIPE_BUFFER_STARTER) {
        return mBufferItems.valueFor(bufId).mSize;
    } else {
        auto pBufferHandle = this->getBuffer(bufId);
        if (pBufferHandle == NULL)
            return MSize(0, 0);
        return pBufferHandle->native()->getImgSize();
    }
}

MVOID FeatureRequest::startTimer(NodeID_T nodeId)
{
    if (auto pTool = LogTool::get()) {
        pTool->getCurrentLogTime(&mNodeBegin[nodeId]);
    }
}

MVOID FeatureRequest::stopTimer(NodeID_T nodeId)
{
    if (auto pTool = LogTool::get()) {
        pTool->getCurrentLogTime(&mNodeEnd[nodeId]);
    }
}

#if 0
MVOID FeatureRequest::setBooster(IBoosterPtr boosterPtr)
{
    mBoosterPtr = std::move(boosterPtr);
    if( (mBoosterPtr != NULL) && mEnableBoost ) {
        mBoosterPtr->enable();
    }
}

MVOID FeatureRequest::enableBoost()
{
    mEnableBoost = MTRUE;
    if( mBoosterPtr != NULL) {
        mBoosterPtr->enable();
    }
}

MVOID FeatureRequest::disableBoost()
{
    mEnableBoost = MFALSE;
    if( mBoosterPtr != NULL){
        mBoosterPtr->disable();
    }
}
#endif

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

