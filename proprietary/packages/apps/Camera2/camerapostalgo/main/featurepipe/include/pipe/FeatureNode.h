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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_NODE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_NODE_H_

#include <core/CamThreadNode.h>
#include <core/SeqUtil.h>
#include "Feature_Common.h"
#include <pipe/FeatureRequest.h>
#include "FeatureInference.h"
#include "pipe/FeatureTaskQueue.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/BitSet.h>

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


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

class FeatureDataHandler
{
public:
    typedef PathID_T DataID;
public:
    virtual ~FeatureDataHandler();
    virtual MBOOL onData(DataID, const RequestPtr&)     { return MFALSE; }
    virtual MBOOL onAbort(RequestPtr&)                  { return MTRUE; }

    static const char* ID2Name(DataID id);

    template<typename T>
    static unsigned getSeq(const T &data)
    {
        return data->getRequestNo();
    }
    static unsigned getSeq(RequestPtr &data)
    {
        return data->getRequestNo();
    }

    static const bool supportSeq = true;
};

class FeatureNode : public FeatureDataHandler, public CamThreadNode<FeatureDataHandler>
{
public:
    typedef CamGraph<FeatureNode> Graph_T;
    typedef FeatureDataHandler Handler_T;

public:
    FeatureNode(NodeID_T nid, const char *name, MUINT32 uLogLevel = 0, MINT32 policy = SCHED_NORMAL, MINT32 priority = DEFAULT_CAMTHREAD_PRIORITY);
    virtual ~FeatureNode();
    MVOID setNodeSignal(const android::sp<NodeSignal> &nodeSignal);
    MVOID setLogLevel(MUINT32 uLogLevel);
    MVOID setUsageHint(const IFeaturePipe::UsageHint &rUsageHint);
    MVOID setTaskQueue(const android::sp<FeatureTaskQueue>& pTaskQueue);

    virtual MERROR evaluate(NodeID_T, FeatureInferenceData&) = 0;
    NodeID_T getNodeID() { return mNodeId; };

    virtual MVOID dispatch(const RequestPtr& pRequest, NodeID_T nodeId = NULL_NODE);
    virtual std::string getStatus(std::string& strDispatch __unused) {return std::string();};

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit()         { return MTRUE; }
    virtual MBOOL onThreadStart()    { return MTRUE; }
    virtual MBOOL onThreadStop()     { return MTRUE; }
    virtual MBOOL onThreadLoop() = 0;


    static MBOOL dumpData(RequestPtr &request, IImageBuffer *buffer, const char *fmt, ...);
    static MBOOL dumpNamedData(RequestPtr &request, IImageBuffer *buffer, const char *name);
    static MUINT32 dumpData(const char *buffer, MUINT32 size, const char *filename);
    static MBOOL loadData(IImageBuffer *buffer, const char *filename);
    static MUINT32 loadData(char *buffer, size_t size, const char *filename);

protected:
    MINT32 mSensorIndex;
    MINT32 mSensorIndex2;
    NodeID_T mNodeId;
    MUINT32 mLogLevel;
    android::sp<NodeSignal>         mNodeSignal;
    IFeaturePipe::UsageHint  mUsageHint;
    android::sp<FeatureTaskQueue>   mpTaskQueue;
};

} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_NODE_H_
