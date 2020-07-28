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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTKCAM_HWNODE_P2_MW_FRAME_H_
#define _MTKCAM_HWNODE_P2_MW_FRAME_H_

#include "P2_Param.h"
#include "P2_Request.h"

namespace P2
{

typedef android::String8 NodeName_T;

#define USE_ACQUIRE MTRUE
class MWFrame : virtual public android::RefBase, virtual public IP2Frame, virtual public LoggerHolder
{
public:
    MWFrame(const IPipelineNode::NodeId_T &nodeID, const NodeName_T &nodeName, const sp<IPipelineFrame> &frame, const Logger &logger);
    virtual ~MWFrame();
    static MVOID dispatchFrame(const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID, const Logger &logger);
    static MVOID releaseFrameStream(const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID, const Logger &logger);
    static MVOID flushFrame(const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID, const Logger &logger);
    MUINT32 getMWFrameID() const;
    MUINT32 getMWFrameRequestID() const;
    MUINT32 getFrameID() const;
    MVOID notifyRelease();
    MVOID beginBatchRelease();
    MVOID endBatchRelease();
    MBOOL getInfoIOMapSet(IPipelineFrame::InfoIOMapSet &ioMap);
    sp<IMetaStreamBuffer> acquireMetaStream(const StreamId_T &sID) const;
    sp<IImageStreamBuffer> acquireImageStream(const StreamId_T &sID) const;
    IMetadata* acquireMeta(const sp<IMetaStreamBuffer> &stream, IO_DIR dir) const;
    sp<IImageBuffer> acquireImage(const sp<IImageStreamBuffer> &stream, IO_DIR dir) const;
    MVOID releaseMeta(const sp<IMetaStreamBuffer> &stream, IMetadata *meta) const;
    MVOID releaseMetaStream(const sp<IMetaStreamBuffer> &stream, IO_DIR dir, IO_STATUS state);
    MVOID releaseImage(const sp<IImageStreamBuffer> &stream, const sp<IImageBuffer> &image) const;
    MVOID releaseImageStream(const sp<IImageStreamBuffer> &stream, IO_DIR dir, IO_STATUS state);

private:
    MVOID doRelease();
    MVOID acquireFence(const sp<IStreamBuffer> &stream) const;
    MBOOL validateStream(const StreamId_T &sID, IStreamBufferSet &bufferSet, const sp<IStreamBuffer> &stream, MBOOL acquire = USE_ACQUIRE) const;
    MUINT32 toStreamBufferStatus(IO_STATUS status) const;
    MUINT32 toUserStatus(IO_STATUS status) const;

private:
    android::Mutex mMutex;
    IPipelineNode::NodeId_T mNodeID;
    const NodeName_T mNodeName;
    sp<IPipelineFrame> mFrame;
    MBOOL mDirty;
    MINT32 mBatchMode;
    android::String8 mTraceName;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_MW_FRAME_H_
