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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_NODE_H_

#include <featurePipe/core/include/CamThreadNode.h>
#include <featurePipe/core/include/SeqUtil.h>
#include <featurePipe/core/include/IOUtil.h>
#include "StreamingFeature_Common.h"
#include "StreamingFeatureData.h"
#include "StreamingFeaturePipeUsage.h"
#include "MtkHeader.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
enum StreamingFeatureDataID {
    ID_INVALID,
    ID_ROOT_ENQUE,
    ID_ROOT_TO_P2A,
    ID_ROOT_TO_P2SM,
    ID_ROOT_TO_RSC,
    ID_ROOT_TO_DEPTH,
    ID_ROOT_TO_TOF,
    ID_ROOT_TO_EIS,
    ID_P2A_TO_P2NR,
    ID_P2A_TO_WARP_FULLIMG,
    ID_P2A_TO_EIS_P2DONE,
    ID_P2A_TO_PMDP,
    ID_P2A_TO_HELPER,
    ID_P2SM_TO_HELPER,
    ID_P2NR_TO_VNR,
    ID_VNR_TO_NEXT_FULLIMG,
    ID_PMDP_TO_HELPER,
    ID_BOKEH_TO_HELPER,
    ID_WARP_TO_HELPER,
    ID_ASYNC_TO_HELPER,
    ID_DISP_TO_HELPER,
    ID_HELPER_TO_ASYNC,
    ID_EIS_TO_WARP,
    ID_P2A_TO_VENDOR_FULLIMG,
    ID_BOKEH_TO_VENDOR_FULLIMG,
    ID_VENDOR_TO_NEXT,
    ID_VMDP_TO_NEXT_FULLIMG,
    ID_VMDP_TO_HELPER,
    ID_RSC_TO_HELPER,
    ID_RSC_TO_EIS,
    ID_DEPTH_TO_BOKEH,
    ID_DEPTH_TO_VENDOR,
    ID_TOF_TO_NEXT,
    ID_RSC_TO_P2A,
};

class StreamingReqInfo
{
public:
    const MUINT32 mFrameNo;
    const MUINT32 mFeatureMask;
    const MUINT32 mMasterID;
    const MUINT32 mSensorID;
    const MSize   mRRZOsize;
    MVOID makeDebugStr() {
        mStr = android::String8::format("No(%d), fmask(0x%08x), sID(%d), masterID(%d), mRRZOsize(%dx%d)",
            mFrameNo, mFeatureMask, mSensorID, mMasterID, mRRZOsize.w, mRRZOsize.h);
    }
    const char* dump() const { return mStr.c_str();}
    MBOOL isMaster() const {return mMasterID == mSensorID;}

    StreamingReqInfo(MUINT32 fno, MUINT32 mask, MUINT32 mID, MUINT32 sID, MSize rrzoSize)
    : mFrameNo(fno)
    , mFeatureMask(mask)
    , mMasterID(mID)
    , mSensorID(sID)
    , mRRZOsize(rrzoSize)
    {
        makeDebugStr();
    }
private:
    android::String8 mStr = android::String8("");
};

struct DumpFilter
{
    MUINT32 index = 0;
    const char *name = NULL;
    DumpFilter(MUINT32 _index, const char *_name)
    : index(_index)
    , name(_name)
    {}
    DumpFilter() {}
};

class NodeSignal : public virtual android::RefBase
{
public:
    enum Signal
    {
        SIGNAL_GPU_READY = 0x01 << 0,
    };

    enum Status
    {
        STATUS_IN_FLUSH = 0x01 << 0,
    };

    NodeSignal();
    virtual ~NodeSignal();
    MVOID setSignal(Signal signal);
    MVOID clearSignal(Signal signal);
    MBOOL getSignal(Signal signal);
    MVOID waitSignal(Signal signal);

    MVOID setStatus(Status status);
    MVOID clearStatus(Status status);
    MBOOL getStatus(Status status);

private:
    android::Mutex mMutex;
    android::Condition mCondition;
    MUINT32 mSignal;
    MUINT32 mStatus;
};

class StreamingFeatureDataHandler : virtual public CamNodeULogHandler
{
public:
    typedef StreamingFeatureDataID DataID;
public:
    virtual ~StreamingFeatureDataHandler();
    virtual MBOOL onData(DataID, const RequestPtr&)     { return MFALSE; }
    virtual MBOOL onData(DataID, const ImgBufferData&)  { return MFALSE; }
    virtual MBOOL onData(DataID, const EisConfigData&)  { return MFALSE; }
    virtual MBOOL onData(DataID, const FaceData&)       { return MFALSE; }
    virtual MBOOL onData(DataID, const CBMsgData&)      { return MFALSE; }
    virtual MBOOL onData(DataID, const HelperData&)     { return MFALSE; }
    virtual MBOOL onData(DataID, const RSCData&)        { return MFALSE; }
    virtual MBOOL onData(DataID, const DSDNData&)       { return MFALSE; }
    virtual MBOOL onData(DataID, const BasicImgData&)   { return MFALSE; }
    virtual MBOOL onData(DataID, const DepthImgData&)       { return MFALSE; }
    virtual MBOOL onData(DataID, const DualBasicImgData&)    { return MFALSE; }
    virtual MBOOL onData(DataID, const P2AMDPReqData&)  { return MFALSE; }
    virtual MBOOL onData(DataID, const TPIData&)        { return MFALSE; }
    virtual MBOOL onData(DataID, const VMDPReqData&)    { return MFALSE; }
    virtual MBOOL onData(DataID, const WarpControlData&) { return MFALSE; }

    static const char* ID2Name(DataID id);

    template<typename T>
    static unsigned getSeq(const T &data)
    {
        return data.mRequest->mRequestNo;
    }
    static unsigned getSeq(const RequestPtr &data)
    {
        return data->mRequestNo;
    }

    static const bool supportSeq = true;
};

class StreamingFeatureNode : public StreamingFeatureDataHandler, public CamThreadNode<StreamingFeatureDataHandler>
{
public:
    typedef CamGraph<StreamingFeatureNode> Graph_T;
    typedef StreamingFeatureDataHandler Handler_T;

public:
    StreamingFeatureNode(const char *name);
    virtual ~StreamingFeatureNode();
    MVOID setSensorIndex(MUINT32 sensorIndex);
    MVOID setPipeUsage(const StreamingFeaturePipeUsage &usage);
    MVOID setNodeSignal(const android::sp<NodeSignal> &nodeSignal);

    virtual IOPolicyType getIOPolicy(StreamType stream, const StreamingReqInfo &reqInfo) const;
    virtual MBOOL getInputBufferPool(const StreamingReqInfo &reqInfo, android::sp<IBufferPool>& pool, NextFullInfo &info);

protected:

    virtual MBOOL onInit();
    virtual MBOOL onUninit()         { return MTRUE; }
    virtual MBOOL onThreadStart()    { return MTRUE; }
    virtual MBOOL onThreadStop()     { return MTRUE; }
    virtual MBOOL onThreadLoop() = 0;
    typedef const char* (*MaskFunc)(MUINT32);
    MVOID enableDumpMask(MUINT32 defaultMask, const std::vector<DumpFilter> &vFilter, const char *postFix = NULL);
    MBOOL allowDump(MUINT8 maskIndex) const;


    static MBOOL dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, const BasicImg &img, TuningUtils::YUV_PORT portIndex, const char *pUserStr = NULL);
    static MBOOL dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, const ImgBuffer &img, TuningUtils::YUV_PORT portIndex, const char *pUserStr = NULL);
    static MBOOL dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, IImageBuffer *buffer, TuningUtils::YUV_PORT portIndex, const char *pUserStr = NULL);

    MVOID drawScanLine(IImageBuffer *buffer);
    static MBOOL syncAndDump(const RequestPtr &request, const BasicImg &img, const char *fmt, ...);
    static MBOOL syncAndDump(const RequestPtr &request, const ImgBuffer &img, const char *fmt, ...);
    static MBOOL dumpData(const RequestPtr &request, const ImgBuffer &buffer, const char *fmt, ...);
    static MBOOL dumpData(const RequestPtr &request, const BasicImg &buffer, const char *fmt, ...);
    static MBOOL dumpData(const RequestPtr &request, IImageBuffer *buffer, const char *fmt, ...);
    static MBOOL dumpNamedData(const RequestPtr &request, IImageBuffer *buffer, const char *name);
    static MUINT32 dumpData(const char *buffer, MUINT32 size, const char *filename);
    static MBOOL loadData(IImageBuffer *buffer, const char *filename);
    static MUINT32 loadData(char *buffer, size_t size, const char *filename);

protected:
    MUINT32 mSensorIndex;
    MINT32 mNodeDebugLV;
    StreamingFeaturePipeUsage mPipeUsage;
    android::sp<NodeSignal> mNodeSignal;
    DebugScanLine *mDebugScanLine;
    MUINT32 mDebugDumpMask = 0;
};
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_NODE_H_
