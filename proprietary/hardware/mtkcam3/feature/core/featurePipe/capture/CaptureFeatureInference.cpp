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

#include <sstream>

#include "CaptureFeatureInference.h"
#include "CaptureFeatureNode.h"
#include "CaptureFeature_Common.h"

#include <utility>
#include <algorithm>
#include <bitset>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#define PIPE_CLASS_TAG "Inference"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_INFER
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_CAPTURE);

using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::Utils::Format;

//#define __DEBUG

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

auto fnLoggingFmtCont = [](auto containerMap) -> std::string{
    std::string sTemp="";
    for(auto index=0;index<containerMap.size();++index)
        sTemp += ImgFmt2Name(containerMap[index]) + std::string("|");
    return sTemp;
    };

#define LOG_FMT_CONTAINER(container) fnLoggingFmtCont(container).c_str()

MINT32 DataItem::mLogLevel;

int FormatComp(const Format_T* lhs, const Format_T* rhs)
{
    int result = *lhs > *rhs;
    return result;
}

void DataItem::initFormat(Format_T fmt)
{
    if(fmt != 0)
    {
        this->mFormat = fmt;
        this->mvSupportFormats.push_back(fmt);
    }
}

void DataItem::initFormat(Format_T fmt, vector<Format_T> vSupportFormats)
{
    if(fmt != 0)
    {
        this->mFormat = fmt;
        this->mvSupportFormats.reserve(this->mvSupportFormats.size()+vSupportFormats.size());
        this->mvSupportFormats.insert(this->mvSupportFormats.end(), vSupportFormats.begin(), vSupportFormats.end());
    }
}

MBOOL
DataItem::
_findIntersection(
    vector<Format_T>& vSrc1,
    vector<Format_T>& vSrc2,
    vector<Format_T>& vIntersection
)
{
    std::sort(vSrc1.begin(), vSrc1.end());
    std::sort(vSrc2.begin(), vSrc2.end());
    vector<Format_T> intersection;
    // find intersection
    std::set_intersection(vSrc1.begin(), vSrc1.end(),
                            vSrc2.begin(), vSrc2.end(),
                            std::back_inserter(vIntersection));
    return MTRUE;
}

MBOOL
DataItem::
updateFormatRequest(
    const MBOOL& bAllFmtSupport,
    const Format_T& iFormat,
    const vector<Format_T>& vSupportFormats
)
{
    MY_LOGD_IF(mLogLevel, "DataItem: NodeID=%s TypeID=%s fmt=%s sup_fmts=%s support_all=%d, src[fmt:%s sup_fmt:%s sup_all:%d]",
            NodeID2Name(this->mNodeId), TypeID2Name(this->mTypeId), ImgFmt2Name(this->mFormat), LOG_FMT_CONTAINER(this->mvSupportFormats), this->mbAllFmtSupported, ImgFmt2Name(iFormat), LOG_FMT_CONTAINER(vSupportFormats), bAllFmtSupport);

    MBOOL bSuccess = MTRUE;
    if(this->mbAllFmtSupported && bAllFmtSupport)
    {
        if(this->mFormat == 0)
        {
            MY_LOGD_IF(mLogLevel, "DataItem & src request support all fmt update to src");
            this->mFormat = iFormat;
            this->mvSupportFormats = vSupportFormats;
        }
        else
        {
            // use intersection format better
            vector<Format_T> srcSupportFormats = vSupportFormats;
            vector<Format_T> intersection;
            this->_findIntersection(srcSupportFormats, this->mvSupportFormats, intersection);
            if(intersection.size() > 0)
                this->mFormat = intersection[0];
            else {
                this->mFormat = iFormat;
            }
           // when support all, treat this as preferable fmts
            this->mvSupportFormats.insert(this->mvSupportFormats.end(), vSupportFormats.begin(), vSupportFormats.end());
            //
            MY_LOGD_IF(mLogLevel, "DataItem & src request support all fmt, do intersect(%d) fmt=%s all_fmts=%s",
                            intersection.size() > 0, ImgFmt2Name(this->mFormat), LOG_FMT_CONTAINER(this->mvSupportFormats));
        }
    }
    else if(this->mbAllFmtSupported && !bAllFmtSupport)
    {
        MY_LOGD_IF(mLogLevel, "DataItem support all fmt, update to src");
        this->mFormat = iFormat;
        this->mvSupportFormats = vSupportFormats;
        // disable the all format after update
        this->mbAllFmtSupported = MFALSE;
    }
    else if(!this->mbAllFmtSupported && bAllFmtSupport)
    {
        MY_LOGD_IF(mLogLevel, "DataItem NOT support all fmt, but src support all");
        vector<Format_T> srcSupportFormats = vSupportFormats;
        vector<Format_T> intersection;
        this->_findIntersection(srcSupportFormats, this->mvSupportFormats, intersection);
        if(intersection.size() > 0)
        {
            this->mFormat = intersection[0];
            MY_LOGD_IF(mLogLevel, "DataItem NOT support all fmt, but src support all, intersect fmt: fmt:%s all_fmts=%s",
                                ImgFmt2Name(this->mFormat), LOG_FMT_CONTAINER(this->mvSupportFormats));
        }
        else
        {
            MY_LOGD_IF(mLogLevel, "DataItem NOT support all fmt, but src support all, no intersection-> keep DataItem fmts");
        }
    }
    else
    {
        MY_LOGD_IF(mLogLevel, "DataItem & src NOT support all fmt");
        vector<Format_T> srcSupportFormats = vSupportFormats;
        vector<Format_T> intersection;
        this->_findIntersection(srcSupportFormats, this->mvSupportFormats, intersection);
        if(intersection.size() > 0)
        {
            this->mFormat = intersection[0];
            this->mvSupportFormats = intersection;
            MY_LOGD_IF(mLogLevel, "DataItem & src NOT support all fmt, do the intersection: DataItem fmt:%s all_fmts:%s",
                        ImgFmt2Name(this->mFormat), LOG_FMT_CONTAINER(this->mvSupportFormats));
        }
        else
        {
            MY_LOGE("Current DataItem fmt does not match, DataItem: NodeID=%s TypeID=%s fmt=%s sup_fmts=%s, src[fmt:%s sup_fmt:%s sup_all:%d]",
            NodeID2Name(this->mNodeId), TypeID2Name(this->mTypeId), ImgFmt2Name(this->mFormat), LOG_FMT_CONTAINER(this->mvSupportFormats), ImgFmt2Name(iFormat), LOG_FMT_CONTAINER(vSupportFormats), bAllFmtSupport);
            // not support for new format
            bSuccess = MFALSE;
        }
    }

    return bSuccess;
}

CaptureFeatureInferenceData::CaptureFeatureInferenceData(MINT32 iLogLevel)
    : mDataCount(0)
    , mIsUnderBSS(MFALSE)
    , mIsMainFrame(MFALSE)
    , mIsYUVProcess(MFALSE)
    , mIsBypassBSS(MFALSE)
    , mIsDropFrame(MFALSE)
    , mHasRawOut(MFALSE)
    , mDroppedCount(0)
    , mRequestNum(0)
    , mSensorIndex(-1)
    , mSensorIndex2(-1)
    , mBSSBypassCount(0)
    , mRequestIndex(0)
    , mRequestCount(1)
    , mFaceDateType(0)
    , mThumbnailDelay(0)
    , mBoostType(0)
    , mPipeBufferCounter(0)
    , mRequiredFD(MFALSE)
    , mPerFrameFD(MFALSE)
    , mIsPhysical(MFALSE)
    , mLogLevel(iLogLevel)
{
    memset(&mInferredItems, NULL_TYPE, sizeof(mInferredItems));
    memset(&mNodeInput, -1, sizeof(mNodeInput));
    memset(&mNodeOutput, -1, sizeof(mNodeOutput));
}

MBOOL CaptureFeatureInferenceData::addNodeIO(NodeID_T nodeId,
        Vector<SrcData>& vSrcData,
        Vector<DstData>& vDstData,
        Vector<MetadataID_T>& vMetadata,
        Vector<FeatureID_T>& vFeature,
        MBOOL forced)
{
    MUINT8 index;
    BitSet64 bitFeatures;
    for (auto src : vSrcData)
    {
        if (mInferredItems[src.mTypeId] == NULL_TYPE)
            continue;

        index = mInferredItems[src.mTypeId];

        MY_LOGD_IF(mLogLevel, "SrcData typeId=%s format=%s formats=%s  support_all=%d index=%d", TypeID2Name(src.mTypeId), ImgFmt2Name(src.mFormat), LOG_FMT_CONTAINER(src.mvSupportFormats), src.mbAllFmtSupported, index);

        DataItem* pAnchor = &mDataItems[index];
        MUINT8 sourceIdx = index;
        // find non-inplace data item
        while(pAnchor->mRedirect >= 0)
        {
            MY_LOGD_IF(mLogLevel, "redirect item from:%d to %d nodeID:%s", sourceIdx, pAnchor->mRedirect, NodeID2Name(mDataItems[pAnchor->mRedirect].mNodeId));
            sourceIdx = pAnchor->mRedirect;
            pAnchor = &mDataItems[pAnchor->mRedirect];
        }
        // real dataitem
        DataItem& item = *pAnchor;
//        MBOOL bValid = MTRUE;
        // update request fmt
        if (src.mFormat != 0) {
            if(!item.updateFormatRequest(src.mbAllFmtSupported, src.mFormat, src.mvSupportFormats)) {
                MY_LOGE("Inference: Failed to update srcData request! Fmt unconsistent! NodeID=%d(%s) DataItem:TypeID=%s Format=%s Formats=%s, srcData: TypeId=%s Format=%s Formats=%s",
                nodeId, NodeID2Name(nodeId), TypeID2Name(item.mTypeId), ImgFmt2Name(item.mFormat), LOG_FMT_CONTAINER(item.mvSupportFormats), TypeID2Name(src.mTypeId), ImgFmt2Name(src.mFormat), LOG_FMT_CONTAINER(src.mvSupportFormats));
                return MFALSE;
            }
        }
        item.mSizeId = src.mSizeId;
        if (src.mSize != MSize(0,0))
            item.mSize = src.mSize;
        if (src.mAlign != MSize(0,0))
            item.mAlign = src.mAlign;
        // mark reference of original data item
        mDataItems[index].markReference(nodeId);
        bitFeatures.value |= mDataItems[index].mFeatures.value;
    }

    for (FeatureID_T featId : vFeature)
    {
        MY_LOGD_IF(mLogLevel, "add feature=%s", FeatID2Name(featId));
        bitFeatures.markBit(featId);
    }

    mNodeMeta[nodeId] = vMetadata;

    for (auto dst : vDstData) {
        MINT redirect = -1;
        if (dst.mInPlace) {
            redirect = mInferredItems[dst.mTypeId];
            // Recursive
            DataItem& item = mDataItems[redirect];
            if (item.mRedirect >= 0)
                redirect = item.mRedirect;
        }
        index = addDataItem(nodeId, dst.mTypeId, NULL_BUFFER, bitFeatures, dst.mbAllFmtSupported);
        DataItem& item = mDataItems[index];
        item.initFormat(dst.mFormat, dst.mvSupportFormats);
        item.mSize = dst.mSize;
        item.mSizeId = dst.mSizeId;

        if (dst.mInPlace)
            item.mRedirect = redirect;

        MY_LOGD_IF(mLogLevel, "DstData add item nodeID=%s typeId=%s format=%s formats=%s redir=%d index=%d",
                    NodeID2Name(item.mNodeId), TypeID2Name(item.mTypeId), ImgFmt2Name(item.mFormat),
                    LOG_FMT_CONTAINER(item.mvSupportFormats), item.mRedirect, index);
    }

    if (forced)
        mNodeUsed.markBit(nodeId);
    return MTRUE;
}

MSize CaptureFeatureInferenceData::getSize(TypeID_T typeId)
{
    MUINT8 index = mInferredItems[typeId];
    DataItem& item = mDataItems[index];

    return item.mSize;
}

MVOID CaptureFeatureInferenceData::addSource(TypeID_T typeId, BufferID_T bufId, Format_T fmt, MSize size)
{
    MUINT8 index = addDataItem(NID_ROOT, typeId, bufId);
    DataItem& item = mDataItems[index];
    item.initFormat(fmt);
    item.mSize = size;
    MY_LOGD_IF(mLogLevel, "addSource: TypeID=%s bufID=%d fmt=%s index=%d", TypeID2Name(typeId), bufId, ImgFmt2Name(fmt), index);
}

MVOID CaptureFeatureInferenceData::addTarget(TypeID_T typeId, BufferID_T bufId)
{
    int index = -1;
    size_t featureSize = 0;
    for (int i = 0; i < mDataCount ; i++) {
        DataItem& item = mDataItems[i];
        if (item.mTypeId != typeId)
            continue;

        // Depth could be a working buffer or external buffer from pipeline model
        if (typeId != TID_MAN_DEPTH && typeId != TID_MAN_FULL_RAW && item.mReferences.value != 0)
            continue;

        if (index == -1 || item.mFeatures.count() > featureSize || mDataItems[index].mNodeId == NID_ROOT) {
            featureSize = item.mFeatures.count();
            index = i;
        }
    }

    if (index != -1) {
        MY_LOGD_IF(mLogLevel, "addTarget: index=%d", index);
        DataItem& item = mDataItems[index];
        // Output Buffer
        item.markReference(NID_ROOT);
        item.mSizeId = SID_ARBITRARY;
        item.mBufferId = bufId;
    }
}

MUINT8 CaptureFeatureInferenceData::addDataItem(
        NodeID_T nodeId, TypeID_T typeId, BufferID_T bufId, BitSet64 features, MBOOL bSupportAllFmt)
{
    MY_LOGD_IF(mLogLevel, "addDataItem nodeID=%s typeID=%s bufferID=%d, supportAll=%d ",
                NodeID2Name(nodeId), TypeID2Name(typeId), bufId, bSupportAllFmt);
    if (mDataCount >= NUM_OF_DATA - 1) {
        MY_LOGE("Over the maximun of data item. count:%d", mDataCount);
        return mDataCount;
    }

    DataItem& item = mDataItems[mDataCount];
    item.mNodeId = nodeId;
    item.mTypeId = typeId;
    item.mFeatures.value |= features.value;
    item.mBufferId = bufId;
    item.mbAllFmtSupported = bSupportAllFmt;

    mInferredType.markBit(typeId);
    // update the latest reference
    mInferredItems[typeId] = mDataCount;
    return mDataCount++;
}

MVOID CaptureFeatureInferenceData::dumpAllDataItem()
{
    String8 strFeature;
    String8 strReference;
    String8 strInput;
    String8 strOutput;
    for (int i = 0; i < mDataCount; i++)
    {
        DataItem& item = mDataItems[i];
        MY_LOGD("item index:%d nodeID=%s typeID=%s format=%s support formats=%s", i, NodeID2Name(item.mNodeId), TypeID2Name(item.mTypeId), ImgFmt2Name(item.mFormat), LOG_FMT_CONTAINER(item.mvSupportFormats));
    }
}

MVOID CaptureFeatureInferenceData::dump()
{
    String8 strFeature;
    String8 strReference;
    String8 strInput;
    String8 strOutput;
    for (int i = 0; i < mDataCount; i++)
    {
        DataItem& item = mDataItems[i];

#ifndef __DEBUG
        if (item.mReferences.isEmpty())
            continue;
#endif
        for (NodeID_T nodeId = 0; nodeId < NUM_OF_NODE; nodeId++) {
            if (item.mReferences.hasBit(nodeId)) {
                if (strReference.length() > 0) {
                    strReference += ",";
                }
                strReference += NodeID2Name(nodeId);
            }
        }

        for (FeatureID_T featId = 0; featId < NUM_OF_FEATURE; featId++) {
            if (item.mFeatures.hasBit(featId)) {
                if (strFeature.length() > 0) {
                    strFeature += ",";
                }
                strFeature += FeatID2Name(featId);
            }
        }

        MY_LOGD(" item:[%02d] buffer:[%03d] node:[%s] type:[%s] feature:[%s] referred:[%s] size:[%s%s] format:[%s] support_format:[%s]%s",
                i,
                item.mBufferId,
                NodeID2Name(item.mNodeId),
                TypeID2Name(item.mTypeId),
                strFeature.string(),
                strReference.string(),
                SizeID2Name(item.mSizeId),
                (item.mSize != MSize(0,0))
                    ? String8::format("(%dx%d)", item.mSize.w, item.mSize.h).string() : "",
                (item.mFormat)
                    ? ImgFmt2Name(item.mFormat) : "",
                (item.mvSupportFormats.size()>0)
                    ? LOG_FMT_CONTAINER(item.mvSupportFormats) : "",
                (item.mRedirect >= 0)
                    ? String8::format(" redirect:[%d]", item.mRedirect) : "");
        strFeature.clear();
        strReference.clear();
    }
#ifdef __DEBUG
    for (PathID_T pathId = 0; pathId < NUM_OF_PATH; pathId++) {
        if (mPathUsed.hasBit(pathId))
            MY_LOGD(" path: %s", PathID2Name(pathId));
    }

    for (NodeID_T nodeId = NID_ROOT + 1; nodeId < NUM_OF_NODE; nodeId++) {
        if (!mNodeUsed.hasBit(nodeId))
            continue;

        strInput.clear();
        strOutput.clear();
        for (TypeID_T typeId = 0; typeId < NUM_OF_TYPE; typeId++) {
            if (mNodeInput[nodeId][typeId] >= 0) {
                if (strInput.length() > 0) {
                    strInput += ",";
                }
                strInput += String8::format("%d", mNodeInput[nodeId][typeId]);
            }
        }

        for (TypeID_T typeId = 0; typeId < NUM_OF_TYPE; typeId++) {
            if (mNodeOutput[nodeId][typeId] >= 0) {
                if (strOutput.length() > 0) {
                    strOutput += ",";
                }
                strOutput += String8::format("%d", mNodeOutput[nodeId][typeId]);
            }
        }

        MY_LOGD(" node:[%s] input items:[%s] output items:[%s]",
                NodeID2Name(nodeId),
                strInput.string(),
                strOutput.string());
    }
#endif
}

MVOID CaptureFeatureInferenceData::determine(sp<CaptureFeatureRequest> pRequest)
{
    PathID_T pathId;
    NodeID_T nodeId;
    TypeID_T typeId;
    BitSet32 tmp;

    CaptureFeatureRequest& req = *(pRequest.get());

    // Root node represents all buffers from external
    mNodeUsed.markBit(NID_ROOT);

    // 1. Decide the required nodes using backward inference
    for (ssize_t i = mDataCount - 1; i >= 0; i--) {
        DataItem& item = mDataItems[i];
        if (item.mReferences.isEmpty())
            continue;

        if (item.mReferences.value & mNodeUsed.value) {
            mNodeUsed.markBit(item.mNodeId);
            MY_LOGD_IF(mLogLevel, "node use=%s", NodeID2Name(item.mNodeId));
        }
    }

    // 2. Decide the required buffers based on the reference
    //    is referred by an used node
    for (size_t i = 0; i < mDataCount; i++) {
        DataItem& item = mDataItems[i];
        if (!(item.mReferences.value & mNodeUsed.value))
            continue;
        MY_LOGD_IF(mLogLevel, "=============add output start================");
        MY_LOGD_IF(mLogLevel, "add output, data item  nodeid=%s typeId=%s bufferid=%d index=%zu",
                NodeID2Name(item.mNodeId), TypeID2Name(item.mTypeId), item.mBufferId, i);
        // The node output maps to a data item
        mNodeOutput[item.mNodeId][item.mTypeId] = i;
        // Create a working buffer if no buffer handle
        if (item.mBufferId == NULL_BUFFER) {
            // In-place processing
            if (item.mRedirect >= 0) {
                DataItem& redirect = mDataItems[item.mRedirect];
                item.mBufferId = redirect.mBufferId;
            } else {
                item.mBufferId = PIPE_BUFFER_STARTER | (mPipeBufferCounter++);
                item.mAlign = HwStrideAlignment::queryFormatAlignment(item.mFormat, item.mAlign);
                MY_LOGD_IF(mLogLevel, "bufferID:%d typeID=%s buffer alignment=(%d, %d) size=%dx%d",
                                item.mBufferId, TypeID2Name(item.mTypeId), item.mAlign.w, item.mAlign.h, item.mSize.w, item.mSize.h);
                req.addBufferItem(
                        item.mBufferId,
                        item.mTypeId,
                        item.mSize,
                        item.mFormat,
                        item.mAlign);
            }
        }
        MY_LOGD_IF(mLogLevel, "=============add output ends================");
        MY_LOGD_IF(mLogLevel, "=============find path start================");

        // Add the input buffer to who refer the item
        tmp.value = item.mReferences.value & mNodeUsed.value;
        do
        {
            nodeId = tmp.firstMarkedBit();
            tmp.clearFirstMarkedBit();

            MY_LOGD_IF(mLogLevel, "ref node id=%s", NodeID2Name(nodeId));

            mNodeInput[nodeId][item.mTypeId] = i;
            // there is no path for repeating request
            auto revertRepeatNode = [](NodeID_T nodeId) -> NodeID_T {
                if (nodeId > NID_YUV && nodeId <  NID_YUV + MAX_YUV_REPEAT_NUM)
                    return NID_YUV;
                if (nodeId > NID_YUV2 && nodeId <  NID_YUV2 + MAX_YUV_REPEAT_NUM)
                    return NID_YUV2;
                return nodeId;
            };
            NodeID_T itemFrom = revertRepeatNode(item.mNodeId);
            NodeID_T itemTo = revertRepeatNode(nodeId);
            MY_LOGD_IF(mLogLevel, "find path src=%s to=%s", NodeID2Name(itemFrom), NodeID2Name(itemTo));
            pathId = FindPath(itemFrom, itemTo);
            if (pathId != NULL_PATH)
            {
                MY_LOGD_IF(mLogLevel, "find path src=%s to=%s  FOUND! pid=%s", NodeID2Name(itemFrom), NodeID2Name(itemTo), PathID2Name(pathId));
                mPathUsed.markBit(pathId);
            }

        } while (!tmp.isEmpty());
    }
    MY_LOGD_IF(mLogLevel, "=============find path end================");

    NodeID_T srcNodeId;
    NodeID_T dstNodeId;
    MBOOL ret;
    MY_LOGD_IF(mLogLevel, "=============add path start================");
    // 3. Add the used paths to request
    for (pathId = 0; pathId < NUM_OF_PATH; pathId++) {
        if (mPathUsed.hasBit(pathId)) {
            ret = GetPath(pathId, srcNodeId, dstNodeId);
            if (!ret) {
                MY_LOGD("Can not find the path: %d", pathId);
                return;
            }
            if (mNodeUsed.hasBit(srcNodeId) && mNodeUsed.hasBit(dstNodeId))
                req.addPath(pathId);
            else
                mPathUsed.clearBit(pathId);
        }
    }
    MY_LOGD_IF(mLogLevel, "=============add path end================");

    // 4. Add the node's input/output buffer to request
    Vector<BufferID_T> vInBufIDs;
    Vector<BufferID_T> vOutBufIDs;
    int index;
    for (nodeId = NID_ROOT + 1; nodeId < NUM_OF_NODE; nodeId++) {
        if (!mNodeUsed.hasBit(nodeId))
            continue;

        vInBufIDs.clear();
        vOutBufIDs.clear();

        for (typeId = 0; typeId < NUM_OF_TYPE; typeId++) {
            index = mNodeInput[nodeId][typeId];
            if (index >= 0) {
                DataItem& item = mDataItems[index];
                vInBufIDs.push_back(item.mBufferId);
            }
        }

        for (typeId = 0; typeId < NUM_OF_TYPE; typeId++) {
            index = mNodeOutput[nodeId][typeId];
            if (index >= 0) {
                DataItem& item = mDataItems[index];
                vOutBufIDs.push_back(item.mBufferId);
            }
        }
        req.addNodeIO(nodeId, vInBufIDs, vOutBufIDs, mNodeMeta[nodeId]);
    }
}


CaptureFeatureInference::CaptureFeatureInference()
: mRequestNo(-1)
, mThumbnailTiming(-1)
, mIsCroppedFSYUV(MFALSE)
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.capture.log", 0);
    DataItem::mLogLevel = mLogLevel;
    MY_LOGD("mLogLevel=%d  DataItem logLevel=%d", mLogLevel, DataItem::mLogLevel);
    mDebugFDCache = property_get_int32("vendor.debug.camera.p2c.fdcahe", 0) > 0;
}

MVOID CaptureFeatureInference::addNode(NodeID_T nodeId, CaptureFeatureNode* pNode)
{
    mNodeMap.add(nodeId, pNode);
}

MERROR CaptureFeatureInference::evaluate(sp<CaptureFeatureRequest> pRequest)
{
    Timer timer;
    timer.start();

    CaptureFeatureRequest &rRequest = *(pRequest.get());

    auto GetMetadataPtr = [&](MetadataID_T metaId) -> shared_ptr<IMetadata>
    {
        auto pHandle = rRequest.getMetadata(metaId);
        if (pHandle == NULL)
            return NULL;

        IMetadata* pMetatada = pHandle->native();
        if (pMetatada == NULL)
            return NULL;

        return std::make_shared<IMetadata>(*pMetatada);
    };

    CaptureFeatureInferenceData data(mLogLevel);
    data.mpIMetadataHal = GetMetadataPtr(MID_MAN_IN_HAL);
    data.mpIMetadataApp = GetMetadataPtr(MID_MAN_IN_APP);
    data.mpIMetadataDynamic = GetMetadataPtr(MID_MAN_IN_P1_DYNAMIC);
    GetMetadataPtr(MID_MAN_OUT_APP);
    GetMetadataPtr(MID_MAN_OUT_HAL);
    GetMetadataPtr(MID_MAN_OUT_APP_PHY);
    GetMetadataPtr(MID_SUB_IN_P1_DYNAMIC);
    GetMetadataPtr(MID_SUB_IN_HAL);
    GetMetadataPtr(MID_SUB_OUT_APP);
    GetMetadataPtr(MID_SUB_OUT_HAL);
    GetMetadataPtr(MID_SUB_OUT_APP_PHY);


    data.mFeatures = rRequest.mFeatures;
    data.mRequestNum = rRequest.getRequestNo();
    data.mRequestIndex = rRequest.getActiveFrameIndex();
    data.mRequestCount = rRequest.getActiveFrameCount();
    data.mIsMainFrame = rRequest.isMainFrame();
    data.mDroppedCount = (pRequest->hasParameter(PID_DROPPED_COUNT)) ?
                            pRequest->getParameter(PID_DROPPED_COUNT) : 0;
    data.mBSSBypassCount = (pRequest->hasParameter(PID_BSS_BYPASSED_COUNT)) ?
                            pRequest->getParameter(PID_BSS_BYPASSED_COUNT) : 0;
    data.mIsYUVProcess = MFALSE;
    MUINT8 yuvProcess = MTK_FEATURE_CAP_YUV_PROCESSING_NOT_NEEDED;
    if(tryGetMetadata<MUINT8>(data.mpIMetadataHal.get(), MTK_FEATURE_CAP_YUV_PROCESSING, yuvProcess))
    {
        data.mIsYUVProcess = (yuvProcess == MTK_FEATURE_CAP_YUV_PROCESSING_NEEDED)
                                ? MTRUE : MFALSE;
    }
    data.mIsUnderBSS = rRequest.isUnderBSS();
    data.mIsBypassBSS = rRequest.isBypassBSS();
    data.mIsDropFrame = rRequest.isDropFrame();
    data.mSensorIndex = rRequest.getMainSensorIndex();
    data.mSensorIndex2 = rRequest.getSubSensorIndex();
    data.mIsPhysical = rRequest.isPhysicalStream();
    data.mHasRawOut = (rRequest.mBufferItems.indexOfKey(BID_MAN_OUT_RAW) >= 0);

    MY_LOGD("Inference data R/F:%d/%d I/C:%d/%d IsUnderBSS=%d isMainFrame=%d DropCount=%d BSSBypassCount=%d isYUVProcess=%d isCross=%d",
            rRequest.getRequestNo(), rRequest.getFrameNo(), rRequest.getActiveFrameIndex(), rRequest.getActiveFrameCount(),
            data.mIsUnderBSS, data.mIsMainFrame, data.mDroppedCount, data.mBSSBypassCount, data.mIsYUVProcess, rRequest.isCross());

    // from input/output buffer type to check whether doing NDD or not (NDD only when RAW to YUV)
    MBOOL bcheckNDDInput = MFALSE;
    auto addSource = [&](BufferID_T bufId, TypeID_T typeId)
    {
        if (rRequest.mBufferItems.indexOfKey(bufId) >= 0) {
            auto pBufHandle = rRequest.getBuffer(bufId);
            IImageBuffer* pImgBuf = pBufHandle->native();
            data.addSource(typeId, bufId,
                pImgBuf->getImgFormat(),
                pImgBuf->getImgSize());
            if(!bcheckNDDInput && typeId == TID_MAN_FULL_RAW){
                bcheckNDDInput = MTRUE;
            }
        }
    };

    auto addTarget = [&](BufferID_T bufId, TypeID_T typeId)
    {
        if (rRequest.mBufferItems.indexOfKey(bufId) >= 0)
        {
            MY_LOGD_IF(mLogLevel, "addTarget bufId=%d typeid=%s inn", bufId, TypeID2Name(typeId));
            rRequest.getBuffer(bufId);
            data.addTarget(typeId, bufId);
            auto& item = rRequest.mBufferItems.editValueFor(bufId);
            item.mIsTarget = MTRUE;
            // leverage legacy platfrom for NDD behavior
            if(bcheckNDDInput && !pRequest->hasParameter(PID_NEED_NDD) && (typeId >= TID_YUV_START && typeId < TID_OTHER)){
                pRequest->addParameter(PID_NEED_NDD, 1);
                MY_LOGD_IF(mLogLevel, "PID_NEED_NDD is added, out_TID: %s", TypeID2Name(typeId));
            }
        }
    };

    // 1. add all given input buffers
    addSource(BID_MAN_IN_YUV,  TID_MAN_FULL_YUV);
    addSource(BID_MAN_IN_FULL, TID_MAN_FULL_RAW);
    addSource(BID_MAN_IN_RSZ,  TID_MAN_RSZ_RAW);
    addSource(BID_MAN_IN_LCS,  TID_MAN_LCS);
    addSource(BID_SUB_IN_YUV,  TID_SUB_FULL_YUV);
    addSource(BID_SUB_IN_FULL, TID_SUB_FULL_RAW);
    addSource(BID_SUB_IN_RSZ,  TID_SUB_RSZ_RAW);
    addSource(BID_SUB_IN_LCS,  TID_SUB_LCS);

    // 1-1.
    MBOOL isValidCroppedFSYUVSize = MFALSE;
    auto pOutJpedHnd = pRequest->getBuffer(BID_MAN_OUT_JPEG);
    if (pOutJpedHnd != NULL) {
        IImageBuffer* pInOutJpegBuf = pOutJpedHnd->native();
        if (pInOutJpegBuf != NULL) {
            const MUINT32 trans = pOutJpedHnd->getTransform();
            const MSize outJpegSize = pInOutJpegBuf->getImgSize();
            if ((outJpegSize.w > 0) && (outJpegSize.h > 0)) {
                data.mCroppedYUVSize = (trans & eTransform_ROT_90) ? MSize(outJpegSize.h, outJpegSize.w) : outJpegSize;
                MY_LOGD("valid cropped FSYUV Size, old:(%d, %d), new:(%d, %d), trans:%d",
                    outJpegSize.w, outJpegSize.h,
                    data.mCroppedYUVSize.w, data.mCroppedYUVSize.h, trans);
                isValidCroppedFSYUVSize = MTRUE;
            }
        }
    }

    // 1-2. update inference data of subframes
    const MINT32 curRequestNo = pRequest->getRequestNo();
    const MINT32 curFrameIndex = pRequest->getActiveFrameIndex();
    if (curFrameIndex < 0) {
        MY_LOGW("invaild frame index, requestNo:%d, curRequestNo:%d, curFrameIndex:%d",
            mRequestNo, curRequestNo, curFrameIndex);
    } else if (isValidCroppedFSYUVSize && (curFrameIndex == 0)) { // main-frame
        MY_LOGD("update cropped size, oldRequestNo:%d, newtRequestNo:%d, cropped size (%d, %d)",
            mRequestNo, curRequestNo, data.mCroppedYUVSize.w, data.mCroppedYUVSize.h);
        mCroppedYUVSize = data.mCroppedYUVSize;
    } else if ((mCroppedYUVSize.w > 0) && (mCroppedYUVSize.h > 0)) { // sub-frame
        if (curRequestNo != mRequestNo) {
            MY_LOGW("invaild request number, requestNo:%d, curRequestNo:%d, curFrameIndex:%d",
                mRequestNo, curRequestNo, curFrameIndex);
        } else {
            MY_LOGD("update cropped size, curRequestNo:%d, curFrameIndex:%d, cropped size (%d, %d)",
                curRequestNo, curFrameIndex, mCroppedYUVSize.w, mCroppedYUVSize.h);
            data.mCroppedYUVSize = mCroppedYUVSize;
            isValidCroppedFSYUVSize = MTRUE;
        }
    }

    // 2. inference all possible outputs
    //    - Root node do not involve to inference. It has no specific node IO.
    MBOOL bSuccess = MTRUE;
    for (NodeID_T nodeId = NID_ROOT + 1; nodeId < NUM_OF_NODE && bSuccess ; nodeId++) {
        if (mNodeMap.indexOfKey(nodeId) >= 0) {
            MY_LOGD_IF(mLogLevel, "NodeID: %s do evaluate.", NodeID2Name(nodeId));
            MERROR bRet = mNodeMap.valueFor(nodeId)->evaluate(nodeId, data);
            if(bRet!=OK)
            {
                MY_LOGE("NodeId:%s evaluate failed!", NodeID2Name(nodeId));
                bSuccess = MFALSE;
                goto lbStop;
            }
        }
    }
lbStop:
    if(!bSuccess)
    {
        data.dumpAllDataItem();
        return BAD_VALUE;
    }
    // 2-1. Refine the request's feature, probably be ignored by plugin negotiation
    rRequest.mFeatures = data.mFeatures;

    // 2-2. add face data type
    // note: when flag eFD_Current exist, there is FDNode inference result, and no need cache FD
    const MBOOL isNeededCachedFD = (!data.mFaceDateType.hasBit(eFD_Current) && data.mFaceDateType.hasBit(eFD_Cache));
    pRequest->addParameter(PID_FD_CACHED_DATA, isNeededCachedFD);

    // 2-2.1 add face data type from customer when no plugin
    if (mDebugFDCache) {
        pRequest->addParameter(PID_FD_CACHED_DATA, 1);
    }

    // 2-3. feature boost
    // only work when force disable capture boost
    // 2-3. boost
    const MBOOL isBoost = ((data.mBoostType & eBoost_CPU) != 0);
    if ( !pRequest->isBoostEnable() && isBoost ) {
        std::stringstream ss;
        ss << "req#:" << pRequest->getRequestNo();
        pRequest->setBooster(IBooster::createInstance(ss.str()));
        pRequest->enableBoost();
    }

    if (!data.mThumbnailTiming.isEmpty()) {
        pRequest->addParameter(PID_THUMBNAIL_TIMING, data.mThumbnailTiming.lastMarkedBit());
    }

    if (data.mThumbnailDelay > 0) {
        pRequest->addParameter(PID_THUMBNAIL_DELAY, data.mThumbnailDelay);
    }

    if(isValidCroppedFSYUVSize && !data.mSupportCroppedFSYUV.isEmpty()){
        pRequest->addParameter(PID_CROPPED_FSYUV, 1);
        MY_LOGD("croped FSYUV, outputJPEGSize:(%d, %d)", data.mCroppedYUVSize.w, data.mCroppedYUVSize.h);
    } else {
        pRequest->addParameter(PID_CROPPED_FSYUV, 0);
        MY_LOGD("no need croped FSYUV, isValidCroppedFSYUVSize:%d", isValidCroppedFSYUVSize);
    }

    syncRequestStatus(pRequest);

    MY_LOGD("R/F Num: %d/%d, thumbnailTiming:%d, mThumbnailDelay:%d, mSupportCroppedFSYUV:%d",
            pRequest->getRequestNo(), pRequest->getFrameNo(),
            (pRequest->hasParameter(PID_THUMBNAIL_TIMING) ? pRequest->getParameter(PID_THUMBNAIL_TIMING) : 0),
            (pRequest->hasParameter(PID_THUMBNAIL_DELAY) ? pRequest->getParameter(PID_THUMBNAIL_DELAY) : 0),
            (pRequest->hasParameter(PID_CROPPED_FSYUV) ? pRequest->getParameter(PID_CROPPED_FSYUV) : 0));

    // 3. add output buffers
    addTarget(BID_MAN_OUT_JPEG,      TID_JPEG);
    addTarget(BID_MAN_OUT_THUMBNAIL, TID_THUMBNAIL);
    addTarget(BID_MAN_OUT_POSTVIEW,  TID_POSTVIEW);
    addTarget(BID_MAN_OUT_CLEAN,     TID_MAN_CLEAN);
    addTarget(BID_MAN_OUT_DEPTH,     TID_MAN_DEPTH);
    addTarget(BID_MAN_OUT_BOKEH,     TID_MAN_BOKEH);
    addTarget(BID_MAN_OUT_RAW,       TID_MAN_FULL_RAW);
    addTarget(BID_MAN_OUT_YUV00,     TID_MAN_CROP1_YUV);
    addTarget(BID_MAN_OUT_YUV01,     TID_MAN_CROP2_YUV);
    addTarget(BID_MAN_OUT_YUV02,     TID_MAN_CROP3_YUV);

    // YUV processing --> add target
    if(data.mIsYUVProcess)
    {
        addTarget(NULL_BUFFER, TID_MAN_FULL_YUV);
    }

    // 4. determin final pathes, which contain all node's input and output
    data.determine(pRequest);

    timer.stop();
    MY_LOGI("R/F Num: %d/%d, timeconsuming: %d ms",
            pRequest->getRequestNo(),
            pRequest->getFrameNo(),
            timer.getElapsed());

    data.dump();

    return OK;
}

MVOID CaptureFeatureInference::syncRequestStatus(const sp<CaptureFeatureRequest> pRequest)
{
    // apply main-frame (PID_FRAME_INDEX == 0) thumbnail timing to sub-frame (PID_FRAME_INDEX > 0)
    // NOTICE: PID_FRAME_INDEX must be set before
    const MINT32 curRequestNo = pRequest->getRequestNo();
    const MINT32 curFrameIndex = pRequest->getActiveFrameIndex();
    if (curFrameIndex < 0) {
        MY_LOGW("invaild frame index, requestNo:%d, curRequestNo:%d, curFrameIndex:%d",
            mRequestNo, curRequestNo, curFrameIndex);
    } else if (curFrameIndex == 0) { // main-frame
        const MINT32 curThumbnailTiming = pRequest->getParameter(PID_THUMBNAIL_TIMING);
        const MBOOL  curIsCroppedFSYUV = pRequest->getParameter(PID_CROPPED_FSYUV);
        MY_LOGD("update sync status, oldRequestNo:%d, newtRequestNo:%d, thumbnailTiming:%d, croppedFSYUV:%d",
            mRequestNo, curRequestNo, curThumbnailTiming, curIsCroppedFSYUV);
            mRequestNo = curRequestNo;
            mThumbnailTiming = curThumbnailTiming;
            mIsCroppedFSYUV = curIsCroppedFSYUV;
    }
    else { // sub-frame
        if (curRequestNo != mRequestNo) {
            MY_LOGW("invaild request number, requestNo:%d, curRequestNo:%d, curFrameIndex:%d",
                mRequestNo, curRequestNo, curFrameIndex);
        } else {
            if (mThumbnailTiming >= 0) {
                MY_LOGD("sync status, curRequestNo:%d, curFrameIndex:%d, thumbnailTiming:%d",
                    curRequestNo, curFrameIndex, mThumbnailTiming);
                pRequest->addParameter(PID_THUMBNAIL_TIMING, mThumbnailTiming);
            }
            pRequest->addParameter(PID_CROPPED_FSYUV, mIsCroppedFSYUV);
        }
    }
}


} // NSCapture
} // NSFeaturePipe
} // NSCamFeature
} // NSCam
