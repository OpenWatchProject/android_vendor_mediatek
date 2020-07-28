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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_REQUEST_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_REQUEST_H_

#include "CaptureFeature_Common.h"

#include <time.h>

#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/BitSet.h>
#include <utils/Printer.h>
#include <utils/RWLock.h>

#include <mtkcam3/feature/featurePipe/ICaptureFeaturePipe.h>
#include <featurePipe/core/include/IIBuffer.h>

#include <featurePipe/core/include/WaitQueue.h>
#include <featurePipe/core/include/CamNodeULog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

#define MULTI_DEF_1(PREFIX) PREFIX
#define MULTI_DEF_2(PREFIX) MULTI_DEF_1(PREFIX) ,PREFIX##_R1
#define MULTI_DEF_3(PREFIX) MULTI_DEF_2(PREFIX) ,PREFIX##_R2
#define MULTI_DEF_4(PREFIX) MULTI_DEF_3(PREFIX) ,PREFIX##_R3
#define MULTI_DEF_5(PREFIX) MULTI_DEF_4(PREFIX) ,PREFIX##_R4
#define MULTI_DEF_6(PREFIX) MULTI_DEF_5(PREFIX) ,PREFIX##_R5
#define MULTI_DEFINE_STEP(PREFIX, count) MULTI_DEF_##count(PREFIX)
#define MULTI_ENUM_DEFINE(PREFIX,count) MULTI_DEFINE_STEP(PREFIX, count)

enum CaptureFeaturePathID {
    PID_ENQUE,
    PID_ROOT_TO_RAW,
    PID_ROOT_TO_P2A,
    PID_ROOT_TO_P2B,
    PID_ROOT_TO_MULTIRAW,
    PID_RAW_TO_P2A,
    PID_MULTIRAW_TO_P2A,
    PID_P2A_TO_DEPTH,
    PID_P2A_TO_FUSION,
    PID_P2A_TO_MULTIYUV,
    PID_P2A_TO_YUV,
    PID_P2A_TO_YUV2,
    PID_P2A_TO_MDP,
    PID_P2A_TO_FD,
    PID_P2B_TO_YUV,
    PID_P2B_TO_P2A,
    PID_FD_TO_DEPTH,
    PID_FD_TO_FUSION,
    PID_FD_TO_MULTIFRAME,
    PID_FD_TO_YUV,
    PID_FD_TO_YUV2,
    PID_MULTIFRAME_TO_YUV,
    PID_MULTIFRAME_TO_YUV2,
    PID_MULTIFRAME_TO_MDP_C,
    PID_MULTIFRAME_TO_BOKEH,
    PID_MULTIFRAME_TO_MDP,
    PID_FUSION_TO_YUV,
    PID_FUSION_TO_YUV2,
    PID_FUSION_TO_MDP,
    PID_DEPTH_TO_BOKEH,
    PID_DEPTH_TO_YUV2,
    PID_YUV_TO_MDP_C,
    PID_YUV_TO_BOKEH,
    PID_YUV_TO_YUV2,
    PID_YUV_TO_MDP,
    PID_BOKEH_TO_MDP_B,
    PID_BOKEH_TO_YUV2,
    PID_BOKEH_TO_MDP,
    PID_YUV2_TO_MDP,
    PID_DEQUE,
    NUM_OF_PATH,
    NULL_PATH = 0xFF,
};

enum CaptureFeatureNodeID {
    NID_ROOT,
    NID_P2B,
    NID_MULTIRAW,
    NID_RAW,
    NID_P2A,
    NID_FD,
    NID_DEPTH,
    NID_MULTIYUV,
    NID_FUSION,
    // Define multiple node: NID_YUV/NID_YUV_R1/NID_YUV_R2 ...
    MULTI_ENUM_DEFINE(NID_YUV, MAX_YUV_REPEAT_NUM),
    NID_MDP_C,          // Clean image
    NID_BOKEH,
    NID_MDP_B,          // Bokeh only
    // Define multiple node: NID_YUV2/NID_YUV2_R1/NID_YUV2_R2 ...
    MULTI_ENUM_DEFINE(NID_YUV2, MAX_YUV_REPEAT_NUM),
    NID_MDP,
    NUM_OF_NODE,
    NULL_NODE = 0xFF,
};

enum CaptureFeatureBufferTypeID {
    TID_RAW_START,
    TID_MAN_FULL_RAW                  = TID_RAW_START,
    TID_MAN_RSZ_RAW,
    TID_SUB_FULL_RAW,
    TID_SUB_RSZ_RAW,
    //
    TID_YUV_START,
    TID_MAN_FULL_YUV                  = TID_YUV_START,
    TID_MAN_FULL_PURE_YUV,
    TID_MAN_RSZ_YUV,
    TID_MAN_CROP1_YUV,  // Only for Output
    TID_MAN_CROP2_YUV,  // Only for Output
    TID_MAN_CROP3_YUV,  // Only for Output
    TID_MAN_SPEC_YUV,
    TID_MAN_FD_YUV,
    TID_MAN_CLEAN,
    TID_SUB_FULL_YUV,
    TID_SUB_RSZ_YUV,
    TID_POSTVIEW,
    TID_JPEG,
    TID_THUMBNAIL,
    //
    TID_OTHER,
    TID_MAN_DEPTH                      = TID_OTHER,
    TID_MAN_BOKEH,
    TID_MAN_LCS,
    TID_MAN_DCES,
    TID_MAN_FD,         // Support per-frame computinf FD
    TID_SUB_LCS,
    //
    NUM_OF_TYPE,
    NULL_TYPE = 0xFF,
};

// TODO: Should use EImageSize instead of SID
enum CaptureFeatureSizeID {
    SID_FULL,
    SID_RESIZED,
    SID_QUARTER,
    SID_SPECIFIED,
    SID_ARBITRARY,
    NUM_OF_SIZE,
    NULL_SIZE = 0xFF,
};

enum Direction {
    INPUT,
    OUTPUT,
};

enum BSSReqState {
    BSS_STATE_NOT_BSS_REQ,
    BSS_STATE_DO_BSS,
    BSS_STATE_BYPASS_BSS
};

typedef MUINT8 NodeID_T;
typedef MUINT8 TypeID_T;
typedef MUINT8 PathID_T;
typedef MUINT8 SizeID_T;
typedef MUINT32 Format_T;

#define PIPE_BUFFER_STARTER   (0x1 << 5)


class CaptureFeatureRequest;
class CaptureFeatureNodeRequest : public virtual android::RefBase
{
friend class CaptureFeatureRequest;
public:
    CaptureFeatureNodeRequest(CaptureFeatureRequest* pRequest)
        : mpRequest(pRequest)
    {}

    virtual BufferID_T              mapBufferID(TypeID_T, Direction);

    virtual MBOOL                   hasMetadata(MetadataID_T);

    //
    virtual IImageBuffer*           acquireBuffer(BufferID_T);
    virtual MVOID                   releaseBuffer(BufferID_T);
    virtual MUINT32                 getImageTransform(BufferID_T) const;
    virtual IMetadata*              acquireMetadata(MetadataID_T);
    virtual MVOID                   releaseMetadata(MetadataID_T);

    virtual ~CaptureFeatureNodeRequest() {}
private:

    CaptureFeatureRequest*              mpRequest;
    KeyedVector<TypeID_T, BufferID_T>   mITypeMap;
    KeyedVector<TypeID_T, BufferID_T>   mOTypeMap;
    KeyedVector<BufferID_T, TypeID_T>   mIBufferMap;
    KeyedVector<BufferID_T, TypeID_T>   mOBufferMap;
    BitSet32                            mITypeAcquired;
    BitSet32                            mOTypeAcquired;
    BitSet32                            mITypeReleased;
    BitSet32                            mOTypeReleased;
    BitSet32                            mMetadataSet;
};

enum CaptureFeaturePrivateParameter {
    PID_REQUEST_REPEAT      = NUM_OF_PARAMETER,
    PID_FD_CACHED_DATA,
    PID_MULTIFRAME_TYPE,
    PID_THUMBNAIL_TIMING,
    PID_THUMBNAIL_DELAY,
    PID_ABORTED,
    PID_RESTORED,
    PID_FAILURE,
    PID_DROPPED_COUNT,
    PID_DROPPED_FRAME,
    PID_FRAME_INDEX_FORCE_BSS, // for autotest
    PID_IGNORE_CROP,
    PID_REPROCESSING,          // use source buffer's coordinates for reprocessing
    PID_ENABLE_P2_CROP,
    PID_BSS_REQ_STATE,
    PID_BSS_ORDER,
    PID_BSS_BYPASSED_COUNT,
    PID_CROPPED_FSYUV,
    PID_NEED_NDD,
    NUM_OF_TOTAL_PARAMETER,
};


class CaptureFeatureRequest : public ICaptureFeatureRequest
{
friend class CaptureFeatureInference;
friend class CaptureFeatureInferenceData;
friend class CaptureFeaturePipe;

// Implementation of ICaptureFeatureRequest
public:
    CaptureFeatureRequest();

    virtual MVOID                   addBuffer(BufferID_T, sp<BufferHandle>);
    virtual MVOID                   addMetadata(MetadataID_T, sp<MetadataHandle>);

    // get the acquired buffer handle;
    virtual sp<BufferHandle>        getBuffer(BufferID_T);
    virtual sp<MetadataHandle>      getMetadata(MetadataID_T);

    virtual MVOID                   addFeature(FeatureID_T);
    virtual MVOID                   removeFeature(FeatureID_T);
    virtual MBOOL                   hasFeature(FeatureID_T);
    virtual MVOID                   setFeatures(MUINT64);
    virtual MVOID                   setSensorIndex(MINT32, MINT32);
    virtual MVOID                   setIsPhysicalStream(MBOOL);
    virtual MVOID                   addParameter(ParameterID_T, MINT32);
    virtual MVOID                   deleteParameter(ParameterID_T);

    virtual MINT32                  getParameter(ParameterID_T);
    virtual MBOOL                   hasParameter(ParameterID_T);
    virtual MINT32                  getRequestNo();
    virtual MINT32                  getFrameNo();
    virtual MBOOL                   detachResourceItems();
    virtual MVOID                   waitInference();
    virtual std::string             getStatus(std::string& strDispatch);

    static Utils::ULog::RequestTypeId getULogRequestTypeId() {
        return Utils::ULog::REQ_CAP_FPIPE_REQUEST;
    }

    Utils::ULog::RequestSerial      getULogRequestSerial() {
        return getFrameNo();
    }

public:

    virtual MVOID                   setCrossRequest(sp<CaptureFeatureRequest> pRequest);
    virtual sp<CaptureFeatureRequest>
                                    getCrossRequest();

    virtual MVOID                   setDelayInference(std::function<void()>&);
    virtual MBOOL                   hasDelayInference();
    virtual MVOID                   startInference();

    virtual MBOOL                   isSingleFrame();
    virtual MBOOL                   isPhysicalStream();
    virtual MBOOL                   isPipelineFirstFrame();
    virtual MBOOL                   isPipelineLastFrame();
    virtual MUINT32                 getPipelineFrameCount();
    virtual MINT32                  getPipelineFrameIndex();
    virtual MINT32                  getMainSensorIndex();
    virtual MINT32                  getSubSensorIndex();

    virtual MBOOL                   isUnderBSS();
    virtual MBOOL                   isBypassBSS();
    virtual MBOOL                   isDropFrame();
    virtual MBOOL                   isBSSFirstFrame();
    virtual MBOOL                   isBSSLastFrame();
    virtual MUINT32                 getBSSFrameCount();
    virtual MINT32                  getBSSFrameIndex();
    virtual MBOOL                   isMainFrame();
    // the active-prefix function will check the BSS status to query the corresponding funciton
    virtual MBOOL                   isActiveFirstFrame();
    virtual MBOOL                   isActiveLastFrame();
    virtual MUINT32                 getActiveFrameCount();
    virtual MINT32                  getActiveFrameIndex();

    virtual MVOID                   addBufferItem(BufferID_T, TypeID_T, MSize&, Format_T, MSize&);
    virtual MVOID                   addPath(PathID_T);
    virtual MVOID                   lock();
    virtual MVOID                   unlock();
    virtual MVOID                   finishPath_Locked(PathID_T);
    virtual MVOID                   finishNode_Locked(NodeID_T);
    virtual MBOOL                   isFinished_Locked();
    virtual MBOOL                   isReadyFor(NodeID_T);
    virtual MBOOL                   isCancelled();
    virtual MVOID                   addNodeIO(NodeID_T, Vector<BufferID_T>&, Vector<BufferID_T>&, Vector<MetadataID_T>&);

    virtual MVOID                   decNodeReference(NodeID_T);
    virtual MVOID                   incBufferRef(BufferID_T);
    virtual MVOID                   decBufferRef(BufferID_T);
    virtual MVOID                   decMetadataRef(MetadataID_T);
    virtual Vector<NodeID_T>        getPreviousNodes(NodeID_T);
    virtual Vector<NodeID_T>        getNextNodes(NodeID_T);

    virtual sp<CaptureFeatureNodeRequest>
                                    getNodeRequest(NodeID_T);
    virtual MVOID                   clear();
    virtual MVOID                   dump();
    virtual MVOID                   dump(android::Printer& printer);
    virtual MVOID                   dump(const char* name, NodeID_T nodeId, TypeID_T typeId, Direction dir);
    virtual MVOID                   dump(const char* name, NodeID_T nodeId, IImageBuffer* pImgBuf);
    virtual                         ~CaptureFeatureRequest();

    // Query Buffer Info
    virtual MINT                    getImageFormat(BufferID_T);
    virtual MSize                   getImageSize(BufferID_T);
    virtual MBOOL                   isTargetBuffer(BufferID_T);

    // Timer
    virtual MVOID                   startTimer(NodeID_T);
    virtual MVOID                   stopTimer(NodeID_T);

    MBOOL isCross() {return (mpCrossRequest.promote() != nullptr); }

    android::sp<RequestCallback>    mpCallback;
// boost operators
public:
    MVOID                           setBooster(IBoosterPtr boosterPtr);
    MVOID                           enableBoost();
    MVOID                           disableBoost();
    MBOOL                           isBoostEnable();

private:
    struct BufferItem {
        BufferItem()
            : mAcquired(MFALSE)
            , mCreated(MFALSE)
            , mCrossable(MFALSE)
            , mIsTarget(MFALSE)
            , mReference(0)
            , mTypeId(NULL_TYPE)
            , mSize(MSize(0, 0))
            , mAlign(MSize(0, 0))
            , mFormat(0)
        { }

        MBOOL       mAcquired;
        MBOOL       mCreated;
        MBOOL       mCrossable;
        MBOOL       mIsTarget;
        MUINT32     mReference;
        TypeID_T    mTypeId;
        MSize       mSize;
        MSize       mAlign;
        Format_T    mFormat;
        sp<BufferHandle>
                    mpHandle;
    };


    struct MetadataItem {
        MetadataItem()
            : mAcquired(MFALSE)
            , mCrossable(MFALSE)
            , mReference(0)
            , mpHandle(NULL)
        { }

        MBOOL       mAcquired;
        MBOOL       mCrossable;
        MUINT32     mReference;
        sp<MetadataHandle>
                    mpHandle;
    };

private:
    wp<CaptureFeatureRequest>                   mpCrossRequest = nullptr;
    KeyedVector<NodeID_T, sp<CaptureFeatureNodeRequest>>
                                                mNodeRequest;

    Mutex                                       mRequestMutex;  // mutex for mPathToDo and mNodeToDo
    Mutex                                       mBufferMutex;
    KeyedVector<BufferID_T, BufferItem>         mBufferItems;
    Mutex                                       mMetadataMutex;
    KeyedVector<MetadataID_T, MetadataItem>     mMetadataItems;
    android::BitSet64                           mFeatures;
    android::RWLock                             mParamRWLock;
    MINT32                                      mParameter[NUM_OF_TOTAL_PARAMETER];

    // The first index is source node ID, and the second index is destination node ID.
    // The value is path ID.
    MUINT8                                      mNodePath[NUM_OF_NODE][NUM_OF_NODE];

    // Timer
    timespec                                    mNodeBegin[NUM_OF_NODE] = {};
    timespec                                    mNodeEnd[NUM_OF_NODE]   = {};

    // Record all pathes to run
    BitSet64                                    mPathToDo;
    BitSet32                                    mNodeToDo;
    //
    //
    MBOOL                                       mEnableBoost;
    IBoosterPtr                                 mBoosterPtr;

    Mutex                                       mInferMutex;
    Condition                                   mInferCond;
    function<void()>                            mInferFunction;

    MINT32                                      mSensorIndex;
    MINT32                                      mSensorIndex2;

    // physical
    MBOOL                                       mIsPhysical;
};

typedef android::sp<CaptureFeatureRequest> RequestPtr;

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_REQUEST_H_
