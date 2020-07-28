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

#include <mtkcam/pipeline/hwnode/P2FeatureNode.h>
#include <mtkcam/pipeline/hwnode/P2Node.h>
#include "P2_Param.h"
#include "P2_MWFrame.h"

namespace P2
{

class MWInfo : virtual public android::RefBase
{
public:
    MWInfo(const NSCam::v3::P2FeatureNode::ConfigParams &param);
    MWInfo(const NSCam::v3::P2Node::ConfigParams &param);
    virtual ~MWInfo();
    MBOOL isValid(const Logger &logger = Logger()) const;
    sp<IMetaStreamInfo> findMetaInfo(ID_META id) const;
    sp<IImageStreamInfo> findImgInfo(ID_IMG id) const;
    ID_META toMetaID(StreamId_T sID) const;
    ID_IMG toImgID(StreamId_T sID) const;
    MBOOL isCaptureIn(StreamId_T sID) const;
    MUINT32 getBurstNum() const;
    MVOID print(const Logger &logger = Logger()) const;

private:
    std::unordered_map<ID_META, std::vector<sp<IMetaStreamInfo>>> mMetaInfoMap;
    std::unordered_map<ID_IMG, std::vector<sp<IImageStreamInfo>>> mImgInfoMap;
    std::unordered_map<StreamId_T, ID_META> mMetaIDMap;
    std::unordered_map<StreamId_T, ID_IMG> mImgIDMap;
    MUINT32 mBurstNum = 0;

private:
    MVOID initMetaInfo(ID_META id, const sp<IMetaStreamInfo> &info);
    MVOID initMetaInfo(ID_META id, const Vector<sp<IMetaStreamInfo>> &infos);
    MVOID initImgInfo(ID_IMG id, const sp<IImageStreamInfo> &info);
    MVOID initImgInfo(ID_IMG id, const Vector<sp<IImageStreamInfo>> &infos);
    MBOOL hasMeta(ID_META id) const;
    MBOOL hasImg(ID_IMG id) const;

};

class MWMeta : virtual public P2Meta, virtual public LoggerHolder
{
public:
    MWMeta(const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const META_INFO &info);
    virtual ~MWMeta();
    virtual StreamId_T getStreamID() const;
    virtual MBOOL isValid() const;
    virtual IO_DIR getDir() const;
    virtual MVOID updateResult(MBOOL result);
    virtual IMetadata* getIMetadataPtr() const;
    virtual IMetadata::IEntry getEntry(MUINT32 tag) const;
    virtual MBOOL setEntry(MUINT32 tag, const IMetadata::IEntry &entry);

private:
    sp<MWFrame> mMWFrame;
    StreamId_T mStreamID;
    IO_DIR mDir;
    IO_STATUS mStatus;
    sp<IMetaStreamBuffer> mStreamBuffer;
    IMetadata *mMetadata;
    IMetadata *mLockedMetadata;
    IMetadata mMetadataCopy;
};

class MWImg : virtual public P2Img, virtual public LoggerHolder
{
public:
    MWImg(const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const IMG_INFO &info, MUINT32 debugIndex);
    virtual ~MWImg();
    virtual StreamId_T getStreamID() const;
    virtual MBOOL isValid() const;
    virtual IO_DIR getDir() const;
    MVOID registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin);
    virtual MVOID updateResult(MBOOL result);
    virtual IImageBuffer* getIImageBufferPtr() const;
    virtual MUINT32 getTransform() const;
    virtual MUINT32 getUsage() const;
    virtual MBOOL isDisplay() const;
    virtual MBOOL isRecord() const;
    virtual MBOOL isCapture() const;

private:
    MVOID processPlugin() const;

private:
    sp<MWFrame> mMWFrame;
    StreamId_T mStreamID;
    IO_DIR mDir;
    IO_STATUS mStatus;
    sp<IImageStreamBuffer> mStreamBuffer;
    sp<IImageBuffer> mImageBuffer;
    MUINT32 mTransform;
    MUINT32 mUsage;
    std::list<sp<P2ImgPlugin>> mPlugin;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_MW_DATA_H_
