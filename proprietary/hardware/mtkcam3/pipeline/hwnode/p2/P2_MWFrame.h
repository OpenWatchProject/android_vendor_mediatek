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
class MWFrame : virtual public android::RefBase, virtual public IP2Frame
{
public:
    class ReleaseToken final : public android::RefBase
    {
    public:
        enum StreamState
        {
            STATE_RELEASED = 0,
            STATE_RELEASING,
            STATE_USING,
            STATE_PRERELEASED
        };

    public:
        ReleaseToken(const ILog &log, const NodeName_T &nodeName, const IPipelineNode::NodeId_T &nodeID);
        ~ReleaseToken();

        MVOID registerStream(const StreamId_T &sID, const std::string &name);
        MVOID update(const StreamId_T &sID, const StreamState &status);
        MVOID refreshStreamStatus(LogString &str);

    public:
        const ILog                      mLog;
        const NodeName_T                mNodeName;
        const IPipelineNode::NodeId_T   mNodeID;

    private:
        class MWStreamInfo
        {
        public:
            std::string mName;
            MUINT8 mState = STATE_USING;

            MWStreamInfo(const std::string &name, StreamState state)
            : mName(name)
            , mState(state)
            {}

            MWStreamInfo()
            {}
        };

    private:
        MBOOL mIsPreRelease = MFALSE;
        std::map<StreamId_T, MWStreamInfo>  mMWStreamMap;
    };

public:
    MWFrame(const ILog &log, const IPipelineNode::NodeId_T &nodeID, const NodeName_T &nodeName, const sp<IPipelineFrame> &frame);
    virtual ~MWFrame();
    static MVOID dispatchFrame(const ILog &log, const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID);
    static MVOID releaseFrameStream(const ILog &log, const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID);
    static MVOID flushFrame(const ILog &log, const sp<IPipelineFrame> &frame, const IPipelineNode::NodeId_T &nodeID);

    static MVOID releaseMeta(const sp<ReleaseToken> &releaseToken, const sp<IMetaStreamBuffer> &stream, IMetadata *meta);
    static MVOID releaseMetaStream(const sp<ReleaseToken> &releaseToken, const sp<IMetaStreamBuffer> &stream, IO_DIR dir, IO_STATUS status);
    static MVOID releaseImage(const sp<ReleaseToken> &releaseToken, const std::vector<sp<IImageBuffer>> &images);
    static MVOID releaseImageHeap(const sp<ReleaseToken> &releaseToken, const sp<IImageStreamBuffer> &stream, const sp<IImageBufferHeap> &heap);
    static MVOID releaseImageStream(const sp<ReleaseToken> &releaseToken, const sp<IImageStreamBuffer> &stream, IO_DIR dir, IO_STATUS status);

    MUINT32 getMWFrameID() const;
    MUINT32 getMWFrameRequestID() const;
    MUINT32 getFrameID() const;
    MVOID notifyRelease();
    MVOID beginBatchRelease();
    MVOID endBatchRelease();
    MVOID notifyNextCapture(int requestCnt, MBOOL bSkipCheck);
    MVOID metaResultAvailable(IMetadata* partialMeta);
    MBOOL getInfoIOMapSet(IPipelineFrame::InfoIOMapSet &ioMap);
    sp<IMetaStreamBuffer> acquireMetaStream(const StreamId_T &sID);
    sp<IImageStreamBuffer> acquireImageStream(const StreamId_T &sID);
    IMetadata* acquireMeta(const sp<IMetaStreamBuffer> &stream, IO_DIR dir) const;
    sp<IImageBufferHeap> acquireImageHeap(const sp<IImageStreamBuffer> &stream, IO_DIR dir) const;
    std::vector<sp<IImageBuffer>> acquireImage(const sp<IImageStreamBuffer> &stream, const sp<IImageBufferHeap> &heap,
                                               const NSCam::ImageBufferInfo &blobInfo, IO_DIR dir, MBOOL needSWRW, MBOOL isOpaque) const;
    MVOID preReleaseMetaStream(const sp<IMetaStreamBuffer> &stream, IO_DIR dir);
    MVOID preReleaseImageStream(const sp<IImageStreamBuffer> &stream, IO_DIR dir);
    sp<ReleaseToken> getReleaseToken();

    static MVOID print(const ILog &log, const sp<IMetaStreamInfo> &info, StreamId_T id, unsigned s, unsigned i, const char* io);
    static MVOID print(const ILog &log, const sp<IImageStreamInfo> &info, StreamId_T id, unsigned s, unsigned i, const char* io);
    static MVOID print(const ILog &log, const IPipelineFrame::InfoIOMapSet &ioMap);

private:
    MVOID createOpaqueImages(const sp<IImageBufferHeap> &heap, std::vector<sp<IImageBuffer>> &images) const;
    MVOID createBlobImages(const sp<IImageBufferHeap> &heap, const NSCam::ImageBufferInfo &blobInfo, std::vector<sp<IImageBuffer>> &images) const;
    MVOID createNormalImages(const sp<IImageBufferHeap> &heap, std::vector<sp<IImageBuffer>> &images) const;
    MVOID doRelease();
    MVOID acquireFence(const sp<IStreamBuffer> &stream) const;
    MBOOL validateStream(const StreamId_T &sID, IStreamBufferSet &bufferSet, const sp<IStreamBuffer> &stream, MBOOL acquire = USE_ACQUIRE) const;

private:
    static MUINT32 toStreamBufferStatus(const ILog &log, IO_STATUS status);
    static MUINT32 toUserStatus(const ILog &log, IO_STATUS status);

private:
    android::Mutex mMutex;
    ILog mLog;
    IPipelineNode::NodeId_T mNodeID;
    const NodeName_T mNodeName;
    sp<IPipelineFrame> mFrame;
    MBOOL mDirty;
    MINT32 mBatchMode;
    android::String8 mTraceName;
    MBOOL mIsPreRelease = MFALSE;
    sp<ReleaseToken> mReleaseToken;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_MW_FRAME_H_
