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
#include "StreamingFeature_Common.h"
#include "StreamingFeatureData.h"
#include "StreamingFeaturePipeUsage.h"

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
    ID_ROOT_TO_RSC,
    ID_P2A_TO_FD_DSIMG,
    ID_P2A_TO_VFB_DSIMG,
    ID_P2A_TO_WARP_FULLIMG,
    ID_P2A_TO_P2B_FULLIMG,
    ID_P2A_TO_EIS_P2DONE,
    ID_P2A_TO_EIS_FM,
    ID_P2A_TO_HELPER,
    ID_WARP_TO_MDP_FULLIMG,
    ID_WARP_TO_HELPER,
    ID_MDP_TO_P2B_FULLIMG,
    ID_MDP_TO_HELPER,
    ID_FD_TO_VFB_FACE,
    ID_VFB_TO_P2B,
    ID_VFB_TO_WARP,
    ID_EIS_TO_VFB_WARP,
    ID_EIS_TO_WARP,
    ID_P2A_TO_VENDOR_FULLIMG,
    ID_VENDOR_TO_VMDP_FULLIMG,
    ID_VMDP_TO_NEXT_FULLIMG,
    ID_VMDP_TO_HELPER,
    ID_RSC_TO_HELPER,
    ID_RSC_TO_EIS,

    ID_P2A_TO_FOV_FEFM,
    ID_P2A_TO_FOV_FULLIMG,
    ID_P2A_TO_FOV_WARP,
    ID_FOV_TO_FOV_WARP,
    ID_FOV_TO_EIS_WARP,
    ID_FOV_WARP_TO_HELPER,
    ID_FOV_WARP_TO_VENDOR,
    ID_FOV_TO_EIS_FULLIMG,
    ID_P2A_TO_N3DP2,
    ID_N3DP2_TO_N3D,
    ID_N3D_TO_HELPER,
    ID_N3D_TO_VMDP,
    ID_P2A_TO_VFOV_FULLIMG,
    ID_VFOV_TO_VENDOR,
    ID_VFOV_TO_VMDP_FULLIMG,
    ID_RSC_TO_P2A,
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

class StreamingFeatureDataHandler
{
public:
    typedef StreamingFeatureDataID DataID;
public:
    virtual ~StreamingFeatureDataHandler();
    virtual MBOOL onData(DataID, const RequestPtr&)     { return MFALSE; }
    virtual MBOOL onData(DataID, const ImgBufferData&)  { return MFALSE; }
    virtual MBOOL onData(DataID, const EisConfigData&)  { return MFALSE; }
    virtual MBOOL onData(DataID, const FaceData&)       { return MFALSE; }
    virtual MBOOL onData(DataID, const VFBData&)        { return MFALSE; }
    virtual MBOOL onData(DataID, const FMData&)         { return MFALSE; }
    virtual MBOOL onData(DataID, const CBMsgData&)      { return MFALSE; }
    virtual MBOOL onData(DataID, const RSCData&)        { return MFALSE; }
    virtual MBOOL onData(DataID, const FOVP2AData&)     { return MFALSE; }
    virtual MBOOL onData(DataID, const FOVData&)        { return MFALSE; }
    virtual MBOOL onData(DataID, const BasicImgData&)   { return MFALSE; }
    virtual MBOOL onData(DataID, const BasicIOImgData&) { return MFALSE; }
    virtual MBOOL onData(DataID, const N3DData&)        { return MFALSE; }
    virtual MBOOL onData(DataID, const DualBasicIOImgData&)    { return MFALSE; }

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

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit()         { return MTRUE; }
    virtual MBOOL onThreadStart()    { return MTRUE; }
    virtual MBOOL onThreadStop()     { return MTRUE; }
    virtual MBOOL onThreadLoop() = 0;


    static MBOOL dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, IImageBuffer *buffer, MUINT32 portIndex);
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
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_STREAMING_FEATURE_NODE_H_
