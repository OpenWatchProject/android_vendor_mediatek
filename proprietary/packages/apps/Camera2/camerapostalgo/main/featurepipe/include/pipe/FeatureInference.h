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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_INFERENCE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_INFERENCE_H_

#include <vector>
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/BitSet.h>
#include <memory>

#include <plugin/PipelinePlugin.h>
#include <plugin/PipelinePluginType.h>

#include <core/Timer.h>

#include <pipe/FeatureRequest.h>

#define NUM_OF_DATA    (32)

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

using namespace std;
using std::vector;
class FeatureNode;

enum eFeatureInferenceStrategy {
    STG_LONGEST_PATH,
    STG_PREDEFINED
};
typedef MUINT Format_T;

struct DataItem
{
    DataItem()
        : mNodeId(0)
        , mTypeId(NULL_TYPE)
        , mFormat(0)
        , mSizeId(NULL_SIZE)
        , mSize(0, 0)
        , mAlign(0, 0)
        , mRedirect(-1)
        , mBufferId(NULL_BUFFER)
        , mbAllFmtSupported(MFALSE)
    {
    }
    inline MVOID markReference(NodeID_T nodeId) {
        mReferences.markBit(nodeId);
    }

    inline MVOID markFeature(FeatureID_T featureId) {
        mFeatures.markBit(featureId);
    }
    inline void setAllFmtSupport(MBOOL bSupport) {
            mbAllFmtSupported = bSupport;
        }
    void initFormat(Format_T fmt);
    /**
     * @brief update the format request from reference node during inference
     * @param [in] iFormat request format from source item
     * @param [in] vSupportFormats supported format of source item
     */
    MBOOL updateFormatRequest(
            const MBOOL& bAllFmtSupport,
            const Format_T& iFormat,
            const vector<Format_T>& vSupportFormats);

    MBOOL _findIntersection(
            vector<Format_T>& vSrc1,
            vector<Format_T>& vSrc2,
            vector<Format_T>& vIntersection);

    NodeID_T    mNodeId;        // buffer owner
    TypeID_T    mTypeId;
    BitSet32    mReferences;    // node referred
    BitSet64    mFeatures;      // Feature applied
    Format_T    mFormat;        // Image Format
    SizeID_T    mSizeId;        // Image Size
    MSize       mSize;          // Specified Size
    MSize       mAlign;
    MINT        mRedirect;
    BufferID_T  mBufferId;
    vector<Format_T> mvSupportFormats;
    MBOOL       mbAllFmtSupported;
    static MINT32 mLogLevel;
};

/*
 * 1. Use source data to infer the maximum of data flow
 * 2. Map the targets into the flow
 * 3. Remove the data without any reference.
 *
 */
class FeatureInferenceData
{
public:

    struct SrcData
    {
        friend class FeatureInferenceData;
        SrcData()
            : mTypeId(0)
            , mSizeId(0)
            , mSize(MSize(0, 0))
            , mAlign(MSize(0, 0))
            , mFormat(0)
            , mbAllFmtSupported(MFALSE)
        {
        }

        inline void setFormat(Format_T fmt) {
            mFormat = fmt;
            mvSupportFormats.push_back(fmt);
        }
        inline void addSupportFormat(Format_T fmt) {
            mvSupportFormats.push_back(fmt);
        }
        template <typename Type>
        void addSupportFormats(std::vector<Type> vecfmt) {
            size_t index=0;
            if(mFormat == 0)
            {
                this->setFormat(vecfmt[0]);
                index = 1;
            }
            for(;index<vecfmt.size();++index)
            {
                if(vecfmt[index] != mFormat)
                    mvSupportFormats.push_back(vecfmt[index]);
            }
        }
        inline void setAllFmtSupport(MBOOL bSupport) {
            mbAllFmtSupported = bSupport;
        }
        inline MBOOL getAllFmtSupport() {
            return mbAllFmtSupported;
        }
        Format_T    getFormat() {return mFormat;}

        TypeID_T    mTypeId;
        SizeID_T    mSizeId;
        MSize       mSize;
        MSize       mAlign;
private:
        Format_T    mFormat;
        std::vector<Format_T> mvSupportFormats;
        MBOOL       mbAllFmtSupported;
    };

    struct DstData
    {
        friend class FeatureInferenceData;
        friend struct SrcData;
        DstData()
            : mTypeId(0)
            , mSizeId(0)
            , mSize(MSize(0, 0))
            , mInPlace(MFALSE)
            , mFormat(0)
            , mbAllFmtSupported(MFALSE)
        {
        }

        inline void setFormat(Format_T fmt) {
            mFormat = fmt;
            mvSupportFormats.push_back(fmt);
        }
        inline void addSupportFormat(Format_T fmt) {
            mvSupportFormats.push_back(fmt);
        }
        template <typename Type>
        void addSupportFormats(std::vector<Type> vecfmt) {
            size_t index=0;
            if(mFormat == 0)
            {
                this->setFormat(vecfmt[0]);
                index = 1;
            }
            for(;index<vecfmt.size();++index)
            {
                if(vecfmt[index] != mFormat)
                    mvSupportFormats.push_back(vecfmt[index]);
            }
        }
        inline void setAllFmtSupport(MBOOL bSupport) {
            mbAllFmtSupported = bSupport;
        }
        inline MBOOL getAllFmtSupport() {
            return mbAllFmtSupported;
        }
        inline Format_T    getFormat() {return mFormat;}
        void copyFormats(SrcData srcData) {
            mFormat = srcData.mFormat;
            mvSupportFormats = srcData.mvSupportFormats;
        }

        TypeID_T    mTypeId;
        SizeID_T    mSizeId;
        MSize       mSize;
        MBOOL       mInPlace;
private:
        Format_T    mFormat;
        std::vector<Format_T> mvSupportFormats;
        MBOOL       mbAllFmtSupported;
    };
    FeatureInferenceData(MINT32 iLogLevel = -1);

    /*
     * Step 1:
     */
    MVOID addSource(TypeID_T tid, BufferID_T bid, Format_T fmt, MSize size);
    /*
     * Step 2:
     * a node could have multiple IO, but the IO should have no duplicated type
     *
     */
    MBOOL addNodeIO(NodeID_T nid,
                    Vector<SrcData>& vSrcData,
                    Vector<DstData>& vDstData,
                    Vector<MetadataID_T>& vMetadata,
                    Vector<FeatureID_T>& vFeature,
                    MBOOL forced = MFALSE);

    /*
     * Step 3:
     *
     */
    MVOID addTarget(TypeID_T tid, BufferID_T bid);

    /*
     * Step 4:
     *
     */
    MVOID determine(sp<FeatureRequest> pRequest);

    MVOID dump();
    MVOID dumpAllDataItem();

    inline MBOOL hasType(TypeID_T tid) {
        return mInferredType.hasBit(tid);
    }

    inline MBOOL hasFeature(FeatureID_T fid) {
        return mFeatures.hasBit(fid);
    }

    inline MVOID markFeature(FeatureID_T fid) {
        mFeatures.markBit(fid);
    }

    inline MVOID clearFeature(FeatureID_T fid) {
        mFeatures.clearBit(fid);
    }

    inline MVOID markFaceData(NSCam::NSPipelinePlugin::FaceData fdData) {
        mFaceDateType.markBit(fdData);
    }

    inline MVOID appendBootType(MUINT8 boostType) {
        mBoostType |= boostType;
    }

    inline MVOID markThumbnailTiming(MUINT32 timing) {
        mThumbnailTiming.markBit(timing);
    }

    inline MVOID setThumbnailDelay(MUINT32 delay) {
        if (mThumbnailDelay < delay)
            mThumbnailDelay = delay;
    }

    MSize getSize(TypeID_T typeId);

    inline Vector<SrcData>& getSharedSrcData() {
        mTempSrcData.clear();
        return mTempSrcData;
    }

    inline Vector<DstData>& getSharedDstData() {
        mTempDstData.clear();
        return mTempDstData;
    }

    inline Vector<FeatureID_T>& getSharedFeatures() {
        mTempFeatures.clear();
        return mTempFeatures;
    }

    inline Vector<MetadataID_T>& getSharedMetadatas() {
        mTempMetadatas.clear();
        return mTempMetadatas;
    }

    inline MUINT8 getDroppedCount() {
        return mDroppedCount;
    }

    inline MUINT8 getBSSBypassCount() {
        return mBSSBypassCount;
    }

    inline MUINT8 getRequestCount() {
        return mRequestCount;
    }

    inline MINT32 getRequestNum() {
        return mRequestNum;
    }
    inline MINT8 getRequestIndex() {
        return mRequestIndex;
    }

    inline MBOOL isMainFrame() {
        return mIsMainFrame;
    }

    inline MBOOL isYUVProcess() {
        return mIsYUVProcess;
    }

    inline MBOOL isUnderBSS() {
        return mIsUnderBSS;
    }

    inline MBOOL isBypassBSS() {
        return mIsBypassBSS;
    }

    inline MBOOL isDropFrame() {
        return mIsDropFrame;
    }

private:
    MUINT8 addDataItem(NodeID_T nid, TypeID_T tid,
            BufferID_T bid = NULL_BUFFER,
            BitSet64 features = BitSet64(),
            MBOOL bSupportAllFmt = MFALSE);


public:

    // for reuse
    Vector<SrcData>         mTempSrcData;
    Vector<DstData>         mTempDstData;
    Vector<FeatureID_T>     mTempFeatures;
    Vector<MetadataID_T>    mTempMetadatas;


    shared_ptr<IMetadata>   mpIMetadataHal;
    shared_ptr<IMetadata>   mpIMetadataApp;
    shared_ptr<IMetadata>   mpIMetadataDynamic;

    DataItem                mDataItems[NUM_OF_DATA];
    MUINT8                  mDataCount;

    MBOOL                   mIsUnderBSS;
    MBOOL                   mIsMainFrame;
    MBOOL                   mIsYUVProcess;
    MBOOL                   mIsBypassBSS;
    MBOOL                   mIsDropFrame;
    MUINT8                  mDroppedCount;
    MUINT8                  mBSSBypassCount;
    MINT32                  mRequestNum;
    MINT8                   mRequestIndex;
    MUINT8                  mRequestCount;
    android::BitSet32       mInferredType;
    // map type id into data item
    MUINT8                  mInferredItems[NUM_OF_TYPE];

    android::BitSet64       mFeatures;

    android::BitSet32       mFaceDateType;
    android::BitSet32       mThumbnailTiming;
    MUINT32                 mThumbnailDelay;
    MUINT8                  mBoostType;
    // the value is index of data item
    MINT8                   mNodeInput[NUM_OF_NODE][NUM_OF_TYPE];
    MINT8                   mNodeOutput[NUM_OF_NODE][NUM_OF_TYPE];
    Vector<MetadataID_T>    mNodeMeta[NUM_OF_NODE];

    android::BitSet64       mPathUsed;
    android::BitSet32       mNodeUsed;
    MUINT8                  mPipeBufferCounter;

    // feature related
    MBOOL                   mRequiredFD;
    MBOOL                   mPerFrameFD;
    //
    MINT32                  mLogLevel;
};


class FeatureInference
{

public:
    FeatureInference();

    virtual MVOID addNode(NodeID_T nid, FeatureNode* pNode);

    virtual MERROR evaluate(sp<FeatureRequest> pRequest);

    virtual ~FeatureInference() {};

private:
    MVOID syncRequestStatus(const sp<FeatureRequest> pRequest);

private:
    KeyedVector<NodeID_T, FeatureNode*> mNodeMap;

    // sync thumbnail timing status for the same request
    MINT32 mRequestNo;
    MINT32 mThumbnailTiming;
    MINT32 mLogLevel;
};
} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_INFERENCE_H_
