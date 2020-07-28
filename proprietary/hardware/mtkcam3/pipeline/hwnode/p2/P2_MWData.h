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

#ifndef _MTKCAM_HWNODE_P2_MW_DATA_H_
#define _MTKCAM_HWNODE_P2_MW_DATA_H_

#include <mtkcam3/pipeline/hwnode/P2StreamingNode.h>
#include <mtkcam3/pipeline/hwnode/P2CaptureNode.h>
#include "P2_Param.h"
#include "P2_MWFrame.h"

namespace P2
{

class MWInfo : virtual public android::RefBase
{
public:
    MWInfo(const NSCam::v3::P2StreamingNode::ConfigParams &param, const sp<P2InIDMap> &idMap);
    MWInfo(const NSCam::v3::P2CaptureNode::ConfigParams &param, const sp<P2InIDMap> &idMap);
    virtual ~MWInfo();
    MBOOL isValid(const ILog &log) const;
    sp<IMetaStreamInfo> findMetaInfo(ID_META id) const;
    ID_META toMetaID(StreamId_T sID) const;
    ID_STREAM toStreamID(StreamId_T sID, IO_DIR dir) const;
    std::list<ID_IMG> toImgIDs(StreamId_T sID, IO_DIR dir) const;
    MUINT32 toSensorID(StreamId_T sID) const;
    IMG_TYPE getImgType(StreamId_T sID) const;
    MBOOL isCaptureIn(StreamId_T sID) const;
    MUINT32 getBurstNum() const;
    MUINT32 getCustomOption() const;
    std::vector<P2AppStreamInfo> getAppStreamInfo() const;
    MBOOL supportPQ() const;
    MBOOL supportClearZoom() const;
    MBOOL supportDRE() const;
    MBOOL supportHFG() const;
    MVOID print(const ILog &log) const;
    MBOOL isCapture() const;

private:
    class ID_STREAM_PAIR
    {
    public:
        MVOID set(IO_DIR dir, ID_STREAM sID);
        ID_STREAM get(IO_DIR dir) const;
    private:
        static MBOOL isValidDir(IO_DIR dir);
        ID_STREAM mIn  = ID_STREAM_INVALID;
        ID_STREAM mOut = ID_STREAM_INVALID;
    };
    std::unordered_map<ID_META, std::vector<sp<IMetaStreamInfo>>> mMetaInfoMap;
    std::unordered_map<ID_STREAM, std::vector<sp<IImageStreamInfo>>> mImgStreamInfoMap;
    std::unordered_map<StreamId_T, ID_META> mMetaIDMap;
    std::unordered_map<StreamId_T, ID_STREAM_PAIR> mImgStreamIDMap;
    /* Reprocessing or output img/meta , the related sensor ID maybe undefined*/
    std::unordered_map<StreamId_T, MUINT32> mSensorIDMap;
    std::unordered_map<StreamId_T, P2AppStreamInfo> mAppStreamInfoMap;
    MUINT32 mBurstNum = 0;
    MUINT32 mCustomOption = 0;
    MUINT32 mDefaultSensorID = INVALID_SENSOR_ID;
    MBOOL mIsCapture = MFALSE;

private:
    MVOID initMetaInfo(ID_META id, const sp<IMetaStreamInfo> &info, MUINT32 sensorID);
    MVOID initMetaInfo(ID_META id, const Vector<sp<IMetaStreamInfo>> &infos, MUINT32 sensorID);
    MVOID initImgStreamInfo(ID_STREAM id, const sp<IImageStreamInfo> &info, MUINT32 sensorID);
    MVOID initImgStreamInfo(ID_STREAM id, const Vector<sp<IImageStreamInfo>> &infos, MUINT32 sensorID);
    std::vector<sp<IMetaStreamInfo>> findStreamInfo(ID_META id) const;
    std::vector<sp<IImageStreamInfo>> findStreamInfo(ID_STREAM id) const;
    MBOOL hasMeta(ID_META id) const;
    MBOOL hasImgStream(ID_STREAM id) const;

};

class MWMeta : virtual public P2Meta
{
public:
    MWMeta(const ILog &log, const P2Pack &p2Pack, const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const META_INFO &info);
    virtual ~MWMeta();
    virtual StreamId_T getStreamID() const;
    virtual MBOOL isValid() const;
    virtual IO_DIR getDir() const;
    virtual MVOID updateResult(MBOOL result);
    virtual IMetadata* getIMetadataPtr() const;
    virtual IMetadata::IEntry getEntry(MUINT32 tag) const;
    virtual MBOOL setEntry(MUINT32 tag, const IMetadata::IEntry &entry);
    virtual MVOID detach();

private:
    sp<MWFrame> mMWFrame;
    sp<MWFrame::ReleaseToken> mReleaseToken;
    StreamId_T mStreamID;
    IO_DIR mDir;
    IO_STATUS mStatus;
    sp<IMetaStreamBuffer> mStreamBuffer;
    IMetadata *mMetadata;
    IMetadata *mLockedMetadata;
    IMetadata *mMetadataCopy;
};

class MWStream : virtual public android::RefBase
{
public:
    MWStream(const ILog &log, const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const std::list<ID_IMG> &imgIDs);
    virtual ~MWStream();
    std::vector<sp<IImageBuffer>> acquireBuffers(ID_IMG id, ID_IMG mirrorID, MBOOL needSWRW);
    StreamId_T getStreamID() const;
    IO_DIR getDir() const;
    MVOID updateResult(MBOOL result);
    MUINT32 getTransform() const;
    MUINT32 getUsage() const;
    MINT32 getPhysicalID() const;
    MBOOL isStatusValid() const;
    sp<MWFrame::ReleaseToken> getReleaseToken() const;

    MVOID detach();

private:
    ILog mLog;
    sp<MWFrame> mMWFrame;
    sp<MWFrame::ReleaseToken> mReleaseToken;
    StreamId_T mStreamID;
    IO_DIR mDir;
    IO_STATUS mStatus;
    sp<IImageStreamBuffer> mStreamBuffer;
    sp<IImageBufferHeap> mHeap;
    MBOOL mIsComposedType = MFALSE;
    MUINT32 mTransform = 0;
    MUINT32 mUsage = 0;
    MINT32 mPhysicalID = -1;

};

class MWImg : virtual public P2Img
{
public:

    static std::list<sp<P2Img>> createP2Imgs(const ILog &log,const P2Pack &p2Pack, const sp<MWInfo> &mwInfo, const sp<MWFrame> &frame, const StreamId_T &streamID,
                                                IO_DIR dir, MUINT32 debugIndex, MBOOL needSWRW);

    MWImg(const ILog &log, const P2Pack &p2Pack, const sp<MWStream> &stream, const std::vector<sp<IImageBuffer>> &buffers, IMG_TYPE type, const IMG_INFO &info, MUINT32 debugIndex);
    virtual ~MWImg();
    virtual MBOOL isValid() const;
    virtual IO_DIR getDir() const;
    MVOID registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin);
    virtual MVOID updateResult(MBOOL result);
    virtual MVOID updateVRTimestamp(MUINT32 count, const MINT64 &cam2FwTs);
    virtual IImageBuffer* getIImageBufferPtr() const;
    virtual std::vector<IImageBuffer*> getIImageBufferPtrs() const;
    virtual MUINT32 getIImageBufferPtrsCount() const;
    virtual MUINT32 getTransform() const;
    virtual MUINT32 getUsage() const;
    virtual MBOOL isDisplay() const;
    virtual MBOOL isRecord() const;
    virtual MBOOL isCapture() const;
    virtual MBOOL isPhysicalStream() const;
    virtual MBOOL isFD() const;
    virtual MVOID detach();
    virtual IMG_TYPE getImgType() const;

private:
    MVOID processPlugin() const;
    MBOOL getBufferHandle(buffer_handle_t &handle) const;

private:
    sp<MWStream> mMWStream;
    std::vector<sp<IImageBuffer>> mImageBuffers;
    std::vector<IImageBuffer*> mImageBufferPtrs;
    IImageBuffer *mFirstImageBuffer = NULL;
    std::list<sp<P2ImgPlugin>> mPlugin;
    IMG_TYPE mType = IMG_TYPE_EXTRA;
    MUINT32 mTransform = 0;
    MUINT32 mUsage = 0;
    MINT32 mPhysicalID = -1;
    IO_DIR mDir = IO_DIR_UNKNOWN;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_MW_DATA_H_
