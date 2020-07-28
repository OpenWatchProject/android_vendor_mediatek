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

#include <featurePipe/core/include/CamThreadNode.h>
#include <featurePipe/core/include/SeqUtil.h>
#include "CaptureFeature_Common.h"
#include "CaptureFeatureRequest.h"
#include "CaptureFeatureInference.h"
#include "buffer/CaptureBufferPool.h"
#include "thread/CaptureTaskQueue.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/BitSet.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

class CaptureFeatureDataHandler : virtual public CamNodeULogHandler
{
public:
    typedef PathID_T DataID;
public:
    virtual ~CaptureFeatureDataHandler();
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

class CaptureFeatureNode : public CaptureFeatureDataHandler, public CamThreadNode<CaptureFeatureDataHandler>
{
public:
    typedef CamGraph<CaptureFeatureNode> Graph_T;
    typedef CaptureFeatureDataHandler Handler_T;

public:
    CaptureFeatureNode(NodeID_T nid, const char *name, MUINT32 uLogLevel = 0, MINT32 policy = SCHED_NORMAL, MINT32 priority = DEFAULT_CAMTHREAD_PRIORITY);
    virtual ~CaptureFeatureNode();
    virtual MVOID setSensorIndex(MINT32 sensorIndex, MINT32 sensorIndex2 = -1);
    MVOID setSensorList(const std::vector<MINT32> sensorList);
    MVOID setCropCalculator(const android::sp<CropCalculator> &rCropCalculator);
    MVOID setFovCalculator(const android::sp<FovCalculator> &rFovCalculator);
    MVOID setLogLevel(MUINT32 uLogLevel);
    MVOID setUsageHint(const ICaptureFeaturePipe::UsageHint &rUsageHint);
    MVOID setTaskQueue(const android::sp<CaptureTaskQueue>& pTaskQueue);

    virtual MERROR evaluate(NodeID_T, CaptureFeatureInferenceData&) = 0;
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
    MBOOL needToDump(const RequestPtr pRequest);

protected:
    MINT32 mSensorIndex;
    MINT32 mSensorIndex2;
    std::vector<MINT32> mSensorList;
    NodeID_T mNodeId;
    MUINT32 mLogLevel;
    android::sp<CropCalculator>     mpCropCalculator;
    ICaptureFeaturePipe::UsageHint  mUsageHint;
    android::sp<CaptureTaskQueue>   mpTaskQueue;
    android::sp<FovCalculator>      mpFOVCalculator;
    MBOOL mbDumpCamPipe = MFALSE;
};

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_CAPTURE_FEATURE_NODE_H_
