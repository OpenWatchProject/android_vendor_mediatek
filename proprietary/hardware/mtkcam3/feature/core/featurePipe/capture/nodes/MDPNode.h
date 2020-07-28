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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_MDP_NODE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_MDP_NODE_H_

#include "CaptureFeatureNode.h"

#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/hw/IFDContainer.h>
#include <DpBlitStream.h>
#include <DpIspStream.h>
#include "DpDataType.h"

#include <vector>

#define NUM_MDP_BUFFER 3

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

class MDPNode : public CaptureFeatureNode
{
public:
    MDPNode(NodeID_T nid, const char *name, MINT32 policy = SCHED_NORMAL, MINT32 priority = DEFAULT_CAMTHREAD_PRIORITY);
    virtual ~MDPNode();


public:
    virtual MBOOL onData(DataID id, const RequestPtr& pRequest);

public:
    virtual MERROR evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInference);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:

    MBOOL onRequestProcess(NodeID_T nodeId, RequestPtr&);
    MVOID onRequestFinish(NodeID_T nodeId, RequestPtr&);

    struct BufferItem {
        IImageBuffer*                       mpImageBuffer   = NULL;
        MUINT32                             mTransform      = 0;
        MRect                               mCrop           = MRect(0,0);
        MSize                               mOriSize        = MSize(0, 0);
        MBOOL                               mIsCapture      = MFALSE;
        MBOOL                               mDumpWithMargin = MFALSE;
        MINT32                              mRound          = 0;
        IImageBuffer*                       mpSourceBuffer  = NULL;
        TypeID_T                            mTypeId         = -1;
    };

    Vector<BufferItem> mBufferItems;
    MINT32 getISPProfileForMDP(const RequestPtr& pRequest, NodeID_T nodeId);
    MVOID  updateSensorCropRegion(RequestPtr& pRequest, MRect cropRegion);
private:
    struct RequestPack {
        RequestPtr                          mpRequest;
        NodeID_T                            mNodeId = NULL_NODE;
    };

    WaitQueue<RequestPack>                  mRequestPacks;

    //IImageTransform*                        mpTransformer;

    sp<IFDContainer>                        mpFdReader;
    MDPSetting*                             mpMdpSetting;
    MBOOL                                   mDebugDump;
    MUINT32                                 mDebugDumpFilter;
    MBOOL                                   mDebugDumpMDP;
};

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_MDP_NODE_H_
