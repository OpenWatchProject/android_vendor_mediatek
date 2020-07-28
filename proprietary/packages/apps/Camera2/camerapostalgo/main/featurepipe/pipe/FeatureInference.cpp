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

#include <utility>
#include <algorithm>
#include <bitset>
#include <utils/std/Format.h>
//#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <pipe/FeatureInference.h>
#include <pipe/FeatureNode.h>
#include <pipe/Feature_Common.h>
#include <plugin/PipelinePluginType.h>


#define PIPE_CLASS_TAG "Inference"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_INFER
#include <core/PipeLog.h>


using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::Utils::Format;

#define DEFAULT_CAPTURE_REQUEST_BOOST MTRUE

//#define __DEBUG

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

auto fnLoggingFmtCont = [](auto containerMap) -> std::string{
    std::string sTemp="";
    for(auto index=0;index<containerMap.size();++index)
        sTemp += ImgFmt2Name(containerMap[index]) + std::string("|");
    return sTemp;
    };

#define LOG_FMT_CONTAINER(container) fnLoggingFmtCont(container).c_str()

MINT32 DataItem::mLogLevel;

static MBOOL IS_ENABLE_BOOST_CPU = MTRUE;

static MBOOL getIsDefaultBoost()
{
    static MBOOL ret = []()
    {
        const char* propName = "vendor.debug.postalgo.featurepipe.enable.boost";
        const MBOOL propVal = property_get_int32(propName, DEFAULT_CAPTURE_REQUEST_BOOST);
        const MBOOL isSetProp = (propVal != DEFAULT_CAPTURE_REQUEST_BOOST);
        MY_LOGD("[getIsDefaultBoost] isSetProp:%d, isDefaultBoost:%d,", isSetProp, propVal);
        return propVal;
    }();
    return ret;
}

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
    MY_LOGD_IF(mLogLevel, "DataItem: NodeID=%s TypeID=%s fmt=%s sup_fmts=%s, src[fmt:%s sup_fmt:%s sup_all:%d]",
            NodeID2Name(this->mNodeId), TypeID2Name(this->mTypeId), ImgFmt2Name(this->mFormat), LOG_FMT_CONTAINER(this->mvSupportFormats), ImgFmt2Name(iFormat), LOG_FMT_CONTAINER(vSupportFormats), bAllFmtSupport);

    MBOOL bSuccess = MTRUE;
    if(this->mbAllFmtSupported && bAllFmtSupport)
    {
        if(this->mFormat == 0)
        {
            MY_LOGD_IF(mLogLevel, "Current DataItem & src request support all fmt update to src");
            this->mFormat = iFormat;
            this->mvSupportFormats = vSupportFormats;
        }
        else
        {
            // merge two prefer fmts, might exist duplicates -> it's ok.
            if(vSupportFormats.size()>0)
                this->mvSupportFormats.insert(this->mvSupportFormats.end(), vSupportFormats.begin(), vSupportFormats.end());
            MY_LOGD_IF(mLogLevel, "Current DataItem & src request support all fmt, merge src support fmts. after=%s", LOG_FMT_CONTAINER(this->mvSupportFormats));
        }
    }
    else if(this->mbAllFmtSupported && !bAllFmtSupport)
    {
        MY_LOGD_IF(mLogLevel, "Current DataItem support all fmt, update to src");
        this->mFormat = iFormat;
        this->mvSupportFormats = vSupportFormats;
        // disable the all format after update
        this->mbAllFmtSupported = MFALSE;
    }
    else if(this->mFormat == 0)
    {
        MY_LOGD_IF(mLogLevel, "Current DataItem has no fmt, update to src");
        this->mFormat = iFormat;
        this->mvSupportFormats = vSupportFormats;
    }
    else if(this->mFormat != iFormat)
    {
        MY_LOGD_IF(mLogLevel, "Current DataItem fmt does not match, search fitted-fmt in the all support fmt.");

        vector<Format_T> srcSupportFormats = vSupportFormats;
        vector<Format_T> intersection;
        this->_findIntersection(srcSupportFormats, this->mvSupportFormats, intersection);
        if(intersection.size() > 0)
        {
            this->mFormat = intersection[0];
            MY_LOGD_IF(mLogLevel, "Current DataItem fmt does not match, found the next match fmt:%d(%s)",
                                this->mFormat, ImgFmt2Name(this->mFormat));
            //record back to DataItem
            this->mvSupportFormats = intersection;
        }
        else
        {
            // if src data support all fmt, keep the original data fmt
            if(bAllFmtSupport)
            {
                MY_LOGW("[Need extra transform]DataItem not match to src data, but srcata support all fmt >keep the ori-fmt. NID=%s TID=%s", NodeID2Name(this->mNodeId), TypeID2Name(this->mTypeId));
            }
            else
            {
                MY_LOGE("Current DataItem fmt does not match, DataItem: NodeID=%s TypeID=%s fmt=%s sup_fmts=%s, src[fmt:%s sup_fmt:%s sup_all:%d]",
                NodeID2Name(this->mNodeId), TypeID2Name(this->mTypeId), ImgFmt2Name(this->mFormat), LOG_FMT_CONTAINER(this->mvSupportFormats), ImgFmt2Name(iFormat), LOG_FMT_CONTAINER(vSupportFormats), bAllFmtSupport);
                // not support for new format
                bSuccess = MFALSE;
            }
        }
    }
    else
    {
        vector<Format_T> srcSupportFormats = vSupportFormats;
        vector<Format_T> intersection;
        this->_findIntersection(srcSupportFormats, this->mvSupportFormats, intersection);
        //record back to DataItem
        this->mvSupportFormats = intersection;
        MY_LOGD_IF(mLogLevel, "Current DataItem fmt is same with update src fmt!Just do the format intersection:%s", LOG_FMT_CONTAINER(this->mvSupportFormats));
    }
    return bSuccess;
}

FeatureInferenceData::FeatureInferenceData(MINT32 iLogLevel)
    : mDataCount(0)
    , mDroppedCount(0)
    , mRequestIndex(0)
    , mRequestCount(1)
    , mFaceDateType(0)
    , mThumbnailDelay(0)
    , mBoostType(0)
    , mPipeBufferCounter(0)
    , mRequiredFD(MFALSE)
    , mPerFrameFD(MFALSE)
    , mLogLevel(iLogLevel)
{
    memset(&mInferredItems, NULL_TYPE, sizeof(mInferredItems));
    memset(&mNodeInput, -1, sizeof(mNodeInput));
    memset(&mNodeOutput, -1, sizeof(mNodeOutput));
}

MBOOL FeatureInferenceData::addNodeIO(NodeID_T nodeId,
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

        MY_LOGD_IF(mLogLevel, "SrcData typeId=%s format=%s formats=%s index=%d", TypeID2Name(src.mTypeId), ImgFmt2Name(src.mFormat), LOG_FMT_CONTAINER(src.mvSupportFormats), index);

        DataItem* pAnchor = &mDataItems[index];
        MUINT8 sourceIdx = index;
        // find non-inplace data item
        while(pAnchor->mRedirect >= 0 && !pAnchor->mbAllFmtSupported)
        {
            MY_LOGD_IF(mLogLevel, "redirect item from:%d to %d nodeID:%s", sourceIdx, pAnchor->mRedirect, NodeID2Name(mDataItems[pAnchor->mRedirect].mNodeId));
            sourceIdx = pAnchor->mRedirect;
            pAnchor = &mDataItems[pAnchor->mRedirect];
        }
        // real dataitem
        DataItem& item = *pAnchor;
        MBOOL bValid = MTRUE;
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
        bitFeatures.value |= item.mFeatures.value;
    }

    for (FeatureID_T featId : vFeature)
        bitFeatures.markBit(featId);

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
        item.initFormat(dst.mFormat);
        item.mSize = dst.mSize;
        item.mSizeId = dst.mSizeId;

        if (dst.mInPlace)
            item.mRedirect = redirect;

        MY_LOGD_IF(mLogLevel, "DstData add item nodeID=%s typeId=%s format=%s formats=%s redir=%d",
                    NodeID2Name(item.mNodeId), TypeID2Name(item.mTypeId), ImgFmt2Name(item.mFormat),
                    LOG_FMT_CONTAINER(item.mvSupportFormats), item.mRedirect);
    }

    if (forced)
        mNodeUsed.markBit(nodeId);

    return MTRUE;
}

MSize FeatureInferenceData::getSize(TypeID_T typeId)
{
    MUINT8 index = mInferredItems[typeId];
    DataItem& item = mDataItems[index];

    return item.mSize;
}

MVOID FeatureInferenceData::addSource(TypeID_T typeId, BufferID_T bufId, Format_T fmt, MSize size)
{
    MY_LOGD_IF(mLogLevel, "addSource: TypeID=%s bufID=%d fmt=%s", TypeID2Name(typeId), bufId, ImgFmt2Name(fmt));
    MUINT8 index = addDataItem(NID_ROOT, typeId, bufId);
    DataItem& item = mDataItems[index];
    item.initFormat(fmt);
    item.mSize = size;
}

MVOID FeatureInferenceData::addTarget(TypeID_T typeId, BufferID_T bufId)
{
    int index = -1;
    size_t featureSize = 0;
    for (int i = 0; i < mDataCount ; i++) {
        DataItem& item = mDataItems[i];
        if (item.mTypeId != typeId)
            continue;

        // Depth could be a working buffer or external buffer from pipeline model
        if (typeId != TID_MAN_DEPTH && item.mReferences.value != 0)
            continue;

        if (index == -1 || item.mFeatures.count() > featureSize) {
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

MUINT8 FeatureInferenceData::addDataItem(
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

MVOID FeatureInferenceData::dumpAllDataItem()
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

MVOID FeatureInferenceData::dump()
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

MVOID FeatureInferenceData::determine(sp<FeatureRequest> pRequest)
{

    PathID_T pathId;
    NodeID_T nodeId;
    TypeID_T typeId;
    BitSet32 tmp;

    FeatureRequest& req = *(pRequest.get());

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
        MY_LOGD_IF(mLogLevel, "i[%zu],mDataCount[%d]",i,mDataCount);
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

    MY_LOGD_IF(mLogLevel, "=============add path start================");
    // 3. Add the used paths to request
    for (pathId = 0; pathId < NUM_OF_PATH; pathId++) {
        if (mPathUsed.hasBit(pathId))
            req.addPath(pathId);
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


FeatureInference::FeatureInference()
: mRequestNo(-1)
, mThumbnailTiming(-1)
{
    mLogLevel = ::property_get_int32("vendor.debug.postalgo.featurepipe.inference.log", 1);
    //DataItem::mLogLevel = mLogLevel;
    //MY_LOGD("mLogLevel=%d  DataItem logLevel=%d", mLogLevel, DataItem::mLogLevel);
}

MVOID FeatureInference::addNode(NodeID_T nodeId, FeatureNode* pNode)
{
    mNodeMap.add(nodeId, pNode);
}

MERROR FeatureInference::evaluate(sp<FeatureRequest> pRequest)
{
    Timer timer;
    timer.start();

    FeatureRequest &rRequest = *(pRequest.get());

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

    FeatureInferenceData data(mLogLevel);
/*
    data.mpIMetadataHal = GetMetadataPtr(MID_MAN_IN_HAL);
    data.mpIMetadataDynamic = GetMetadataPtr(MID_MAN_IN_P1_DYNAMIC);
    GetMetadataPtr(MID_MAN_OUT_APP);
    GetMetadataPtr(MID_MAN_OUT_HAL);
    GetMetadataPtr(MID_SUB_IN_P1_DYNAMIC);
    GetMetadataPtr(MID_SUB_IN_HAL);
    GetMetadataPtr(MID_SUB_OUT_APP);
    GetMetadataPtr(MID_SUB_OUT_HAL);
*/
    data.mpIMetadataApp = GetMetadataPtr(MID_MAN_IN_APP);
    GetMetadataPtr(MID_MAN_OUT_APP);

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
    data.mIsUnderBSS = rRequest.isUnderBSS();
    data.mIsBypassBSS = rRequest.isBypassBSS();
    data.mIsDropFrame = rRequest.isDropFrame();
    MY_LOGD("Inference data R:%d/T:%d I/C:%d/%d IsUnderBSS=%d isMainFrame=%d DropCount=%d BSSBypassCount=%d isYUVProcess=%d isCross=%d",
            rRequest.getRequestNo(), rRequest.getTimestamp(), rRequest.getPipelineFrameIndex(), rRequest.getActiveFrameCount(),
            data.mIsUnderBSS, data.mIsMainFrame, data.mDroppedCount, data.mBSSBypassCount, data.mIsYUVProcess, rRequest.isCross());

    auto addSource = [&](BufferID_T bufId, TypeID_T typeId)
    {
        if (rRequest.mBufferItems.indexOfKey(bufId) >= 0) {
            auto pBufHandle = rRequest.getBuffer(bufId);
            IImageBuffer* pImgBuf = pBufHandle->native();
            data.addSource(typeId, bufId,pImgBuf->getImgFormat(),pImgBuf->getImgSize());
        }
    };

    auto addTarget = [&](BufferID_T bufId, TypeID_T typeId)
    {
        if (rRequest.mBufferItems.indexOfKey(bufId) >= 0)
        {
            rRequest.getBuffer(bufId);
            MY_LOGD_IF(mLogLevel, "addTarget bufId=%d typeid=%s inn", bufId, TypeID2Name(typeId));
            data.addTarget(typeId, bufId);
        }
    };

    // 1. add all given input buffers
    addSource(BID_MAN_IN_YUV,  TID_MAN_FULL_YUV);

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

    // 2-3. boost
    /*
    const MBOOL isBoost = ((data.mBoostType & eBoost_CPU) != 0);
    if ( getIsDefaultBoost() || isBoost ) {
        std::stringstream ss;
        ss << "req#:" << pRequest->getRequestNo();
        pRequest->setBooster(IBooster::createInstance(ss.str()));
    }
    */

    if (!data.mThumbnailTiming.isEmpty()) {
        pRequest->addParameter(PID_THUMBNAIL_TIMING, data.mThumbnailTiming.lastMarkedBit());
    }

    if (data.mThumbnailDelay > 0) {
        pRequest->addParameter(PID_THUMBNAIL_DELAY, data.mThumbnailDelay);
    }

    syncRequestStatus(pRequest);

    addTarget(BID_MAN_OUT_YUV,       TID_OUT_YUV);
    addTarget(BID_MAN_OUT_YUV0,      TID_OUT_YUV0);
    addTarget(BID_MAN_OUT_YUV1,      TID_OUT_YUV1);
    addTarget(BID_MAN_OUT_YUV2,      TID_OUT_YUV2);
    addTarget(BID_MAN_OUT_YUV3,      TID_OUT_YUV3);
    addTarget(BID_MAN_OUT_YUV4,      TID_OUT_YUV4);
    addTarget(BID_MAN_OUT_YUV5,      TID_OUT_YUV5);
    addTarget(BID_MAN_OUT_YUV6,      TID_OUT_YUV6);
    addTarget(BID_MAN_OUT_YUV7,      TID_OUT_YUV7);

    // 4. determin final pathes, which contain all node's input and output
    data.determine(pRequest);

    timer.stop();
    MY_LOGI("R: %d/Timestamp:%d, timeconsuming: %d ms",
            pRequest->getRequestNo(),
            pRequest->getTimestamp(),
            timer.getElapsed());

    data.dump();
    return OK;
}

MVOID FeatureInference::syncRequestStatus(const sp<FeatureRequest> pRequest)
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
        MY_LOGD("update sync status, oldRequestNo:%d, newtRequestNo:%d, thumbnailTiming:%d",
            mRequestNo, curRequestNo, curThumbnailTiming);
            mRequestNo = curRequestNo;
            mThumbnailTiming = curThumbnailTiming;
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
        }
    }
}

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

