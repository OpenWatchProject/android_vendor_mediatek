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

#include "P2_MWData.h"
#include "P2_DebugControl.h"

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWInfo
#define P2_TRACE        TRACE_MW_INFO
#include "P2_LogHeader.h"

MWInfo::MWInfo(const NSCam::v3::P2FeatureNode::ConfigParams &param)
{
    TRACE_FUNC_ENTER();
    initMetaInfo(IN_APP, param.pInAppMeta);
    initMetaInfo(IN_P1_APP, param.pInAppRetMeta);
    initMetaInfo(IN_P1_HAL, param.pInHalMeta);
    initMetaInfo(OUT_APP, param.pOutAppMeta);
    initMetaInfo(OUT_HAL, param.pOutHalMeta);
    //initImgInfo(IN_OPAQUE, param.pvInOpaque);
    initImgInfo(IN_FULL, param.pvInFullRaw);
    initImgInfo(IN_RESIZED, param.pInResizedRaw);
    //initImgInfo(IN_REPROCESS, param.pInYuvImage);
    initImgInfo(IN_LCSO, param.pInLcsoRaw);
    initImgInfo(IN_RSSO, param.pInRssoRaw);
    initImgInfo(OUT_YUV, param.vOutImage);
    initImgInfo(OUT_FD, param.pOutFDImage);
    TRACE_FUNC_EXIT();
}

MWInfo::MWInfo(const NSCam::v3::P2Node::ConfigParams &param)
{
    TRACE_FUNC_ENTER();
    initMetaInfo(IN_APP, param.pInAppMeta);
    initMetaInfo(IN_P1_APP, param.pInAppRetMeta);
    initMetaInfo(IN_P1_HAL, param.pInHalMeta);
    initMetaInfo(OUT_APP, param.pOutAppMeta);
    initMetaInfo(OUT_HAL, param.pOutHalMeta);
    initImgInfo(IN_OPAQUE, param.pvInOpaque);
    initImgInfo(IN_FULL, param.pvInFullRaw);
    initImgInfo(IN_RESIZED, param.pInResizedRaw);
    initImgInfo(IN_REPROCESS, param.pInYuvImage);
    initImgInfo(IN_LCSO, param.pInLcsoRaw);
    initImgInfo(IN_RSSO, param.pInRssoRaw);
    initImgInfo(OUT_YUV, param.vOutImage);
    initImgInfo(OUT_FD, param.pOutFDImage);
    mBurstNum = param.burstNum;
    TRACE_FUNC_EXIT();
}

MWInfo::~MWInfo()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL MWInfo::isValid(const Logger &logger) const
{
    TRACE_S_FUNC_ENTER(logger);
    MBOOL ret = MTRUE;
    MBOOL hasFull = hasImg(IN_FULL);
    MBOOL hasResized = hasImg(IN_RESIZED);
    MBOOL hasFD = hasImg(OUT_FD);
    MBOOL hasYUV = hasImg(OUT_YUV);
    MBOOL hasInApp = hasMeta(IN_APP);
    MBOOL hasInHal = hasMeta(IN_P1_HAL);
    if( !(hasFull || hasResized) ||
        !(hasFD || hasYUV) ||
        !hasInApp || !hasInHal )
    {
        MY_S_LOGW(logger, "missing necessary stream: full(%d) resized(%d) fd(%d) yuv(%d) inApp(%d) inHal(%d)", hasFull, hasResized, hasFD, hasYUV, hasInApp, hasInHal);
        this->print(logger);
        ret = MFALSE;
    }
    TRACE_S_FUNC_EXIT(logger);
    return ret;
}

sp<IMetaStreamInfo> MWInfo::findMetaInfo(ID_META id) const
{
    TRACE_FUNC_ENTER();
    sp<IMetaStreamInfo> info;
    auto it = mMetaInfoMap.find(id);
    if( it != mMetaInfoMap.end() && it->second.size() )
    {
        info = it->second[0];
    }
    TRACE_FUNC_EXIT();
    return info;
}

sp<IImageStreamInfo> MWInfo::findImgInfo(ID_IMG id) const
{
    TRACE_FUNC_ENTER();
    sp<IImageStreamInfo> info;
    auto it = mImgInfoMap.find(id);
    if( it != mImgInfoMap.end() && it->second.size() )
    {
        info = it->second[0];
    }
    TRACE_FUNC_EXIT();
    return info;
}

ID_META MWInfo::toMetaID(StreamId_T sID) const
{
    ID_META id = ID_META_INVALID;
    auto it = mMetaIDMap.find(sID);
    if( it != mMetaIDMap.end() )
    {
        id = it->second;
    }
    return id;
}

ID_IMG MWInfo::toImgID(StreamId_T sID) const
{
    ID_IMG id = ID_IMG_INVALID;
    auto it = mImgIDMap.find(sID);
    if( it != mImgIDMap.end() )
    {
        id = it->second;
    }
    return id;
}

MBOOL MWInfo::isCaptureIn(StreamId_T sID) const
{
    MBOOL ret = MFALSE;
    auto it = mImgIDMap.find(sID);
    if( it != mImgIDMap.end() )
    {
        ret = (it->second == IN_OPAQUE) ||
              (it->second == IN_FULL) ||
              (it->second == IN_REPROCESS);
    }
    return ret;
}

MUINT32 MWInfo::getBurstNum() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mBurstNum;
}

MVOID MWInfo::print(const Logger &logger) const
{
    TRACE_S_FUNC_ENTER(logger);
    for( auto &&info : P2Meta::InfoMap )
    {
        std::vector<sp<IMetaStreamInfo>> mwInfo;
        auto it = mMetaInfoMap.find(info.second.id);
        if( it != mMetaInfoMap.end() )
        {
            mwInfo = it->second;
        }
        MUINT32 i = 0, size = mwInfo.size();
        do
        {
            sp<IMetaStreamInfo> meta = (i < size) ? mwInfo[i] : NULL;
            MY_S_LOGD(logger, "[meta] %s[%d]=%p", info.second.name, i, meta.get());
        }while( ++i < size );
    }
    for( auto &&info : P2Img::InfoMap )
    {
        std::vector<sp<IImageStreamInfo>> mwInfo;
        auto it = mImgInfoMap.find(info.second.id);
        if( it != mImgInfoMap.end() )
        {
            mwInfo = it->second;
        }
        MUINT32 i = 0, size = mwInfo.size();
        do
        {
            sp<IImageStreamInfo> img = (i < size) ? mwInfo[i] : NULL;
            MY_S_LOGD(logger, "[img] %s[%d] = %p", info.second.name, i, img.get());
        }while( ++i < size );
    }
    TRACE_S_FUNC_EXIT(logger);
}

MVOID MWInfo::initMetaInfo(ID_META id, const sp<IMetaStreamInfo> &info)
{
    TRACE_FUNC_ENTER();
    if( info != NULL )
    {
        mMetaInfoMap[id].push_back(info);
        mMetaIDMap[info->getStreamId()] = id;
    }
    TRACE_FUNC_EXIT();
}

MVOID MWInfo::initMetaInfo(ID_META id, const Vector<sp<IMetaStreamInfo>> &infos)
{
    TRACE_FUNC_ENTER();
    for( auto &&info : infos )
    {
        if( info != NULL )
        {
            mMetaInfoMap[id].push_back(info);
            mMetaIDMap[info->getStreamId()] = id;
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID MWInfo::initImgInfo(ID_IMG id, const sp<IImageStreamInfo> &info)
{
    TRACE_FUNC_ENTER();
    if( info != NULL )
    {
        mImgInfoMap[id].push_back(info);
        mImgIDMap[info->getStreamId()] = id;
    }
    TRACE_FUNC_EXIT();
}

MVOID MWInfo::initImgInfo(ID_IMG id, const Vector<sp<IImageStreamInfo>> &infos)
{
    TRACE_FUNC_ENTER();
    for( auto &&info : infos )
    {
        if( info != NULL )
        {
            mImgInfoMap[id].push_back(info);
            mImgIDMap[info->getStreamId()] = id;
        }
    }
    TRACE_FUNC_EXIT();
}

MBOOL MWInfo::hasMeta(ID_META id) const
{
    auto &&it = mMetaInfoMap.find(id);
    return it != mMetaInfoMap.end() && it->second.size();
}

MBOOL MWInfo::hasImg(ID_IMG id) const
{
    auto &&it = mImgInfoMap.find(id);
    return it != mImgInfoMap.end() && it->second.size();
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWMeta
#define P2_TRACE        TRACE_MW_META
#include "P2_LogHeader.h"

MWMeta::MWMeta(const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const META_INFO &info)
    : P2Meta(info.id)
    , LoggerHolder(frame)
    , mMWFrame(frame)
    , mStreamID(streamID)
    , mDir(dir)
    , mStatus(IO_STATUS_INVALID)
    , mMetadata(NULL)
    , mLockedMetadata(NULL)
{
    TRACE_S_FUNC_ENTER(mLogger);
    mStreamBuffer = mMWFrame->acquireMetaStream(mStreamID);
    if( mStreamBuffer != NULL )
    {
        mLockedMetadata = mMWFrame->acquireMeta(mStreamBuffer, mDir);
        if( mLockedMetadata != NULL)
        {
            if( dir == IO_DIR_IN &&
                (info.flag & IO_FLAG_COPY) )
            {
                mMetadataCopy = *mLockedMetadata;
                mMetadata = &mMetadataCopy;
            }
            else
            {
                mMetadata = mLockedMetadata;
            }
            mStatus = IO_STATUS_READY;
            TRACE_S_FUNC(mLogger, "meta=%p count= %d", mMetadata, mMetadata->count());
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MWMeta::~MWMeta()
{
    TRACE_S_FUNC_ENTER(mLogger);
    mMWFrame->releaseMeta(mStreamBuffer, mLockedMetadata);
    mMetadata = mLockedMetadata = NULL;
    mMWFrame->releaseMetaStream(mStreamBuffer, mDir, mStatus);
    mStreamBuffer = NULL;
    mMWFrame->notifyRelease();
    mMWFrame = NULL;
    TRACE_S_FUNC_EXIT(mLogger);
}

StreamId_T MWMeta::getStreamID() const
{
    return mStreamID;
}

MBOOL MWMeta::isValid() const
{
    return (mMetadata != NULL);
}

IO_DIR MWMeta::getDir() const
{
    return mDir;
}

MVOID MWMeta::updateResult(MBOOL result)
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( (mDir & IO_DIR_OUT) &&
        mStatus != IO_STATUS_INVALID )
    {
        mStatus = result ? IO_STATUS_OK : IO_STATUS_ERROR;
    }
    TRACE_S_FUNC_EXIT(mLogger);
}


IMetadata* MWMeta::getIMetadataPtr() const
{
    return mMetadata;
}

IMetadata::IEntry MWMeta::getEntry(MUINT32 tag) const
{
    IMetadata::IEntry entry;
    if( mMetadata )
    {
        entry = mMetadata->entryFor(tag);
    }
    return entry;
}

MBOOL MWMeta::setEntry(MUINT32 tag, const IMetadata::IEntry &entry)
{
    MBOOL ret = MFALSE;
    if( mMetadata )
    {
        ret = (mMetadata->update(tag, entry) == OK);
    }
    return ret;
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWImg
#define P2_TRACE        TRACE_MW_IMG
#include "P2_LogHeader.h"

MWImg::MWImg(const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const IMG_INFO &info, MUINT32 debugIndex)
    : P2Img(info.id, debugIndex)
    , LoggerHolder(frame)
    , mMWFrame(frame)
    , mStreamID(streamID)
    , mDir(dir)
    , mStatus(IO_STATUS_INVALID)
    , mTransform(0)
    , mUsage(0)
{
    TRACE_S_FUNC_ENTER(mLogger);
    mStreamBuffer = mMWFrame->acquireImageStream(mStreamID);
    if( mStreamBuffer != NULL )
    {
        mImageBuffer = mMWFrame->acquireImage(mStreamBuffer, dir);
        if( mImageBuffer != NULL )
        {
            mTransform = mStreamBuffer->getStreamInfo()->getTransform();
            mUsage = mStreamBuffer->getStreamInfo()->getUsageForAllocator();
            mStatus = IO_STATUS_READY;
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MWImg::~MWImg()
{
    TRACE_S_FUNC_ENTER(mLogger);
    processPlugin();
    mMWFrame->releaseImage(mStreamBuffer, mImageBuffer);
    mImageBuffer = NULL;
    mMWFrame->releaseImageStream(mStreamBuffer, mDir, mStatus);
    mStreamBuffer = NULL;
    mMWFrame->notifyRelease();
    mMWFrame = NULL;
    TRACE_S_FUNC_EXIT(mLogger);
}

StreamId_T MWImg::getStreamID() const
{
    return mStreamID;
}

MBOOL MWImg::isValid() const
{
    return (mImageBuffer != NULL);
}

IO_DIR MWImg::getDir() const
{
    return mDir;
}

MVOID MWImg::registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin)
{
    TRACE_S_FUNC_ENTER(mLogger);
    mPlugin = plugin;
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWImg::updateResult(MBOOL result)
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( (mDir & IO_DIR_OUT) &&
        mStatus != IO_STATUS_INVALID )
    {
        mStatus = result ? IO_STATUS_OK : IO_STATUS_ERROR;
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

IImageBuffer* MWImg::getIImageBufferPtr() const
{
    return mImageBuffer.get();
}

MUINT32 MWImg::getTransform() const
{
    return mTransform;
}

MUINT32 MWImg::getUsage() const
{
    return mUsage;
}

MBOOL MWImg::isDisplay() const
{
    return (mUsage & (GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE));
}

MBOOL MWImg::isRecord() const
{
    return mUsage & GRALLOC_USAGE_HW_VIDEO_ENCODER;
}

MBOOL MWImg::isCapture() const
{
    return !(mUsage & (GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_HW_VIDEO_ENCODER));
}

MVOID MWImg::processPlugin() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( mStatus != IO_STATUS_ERROR )
    {
        for( auto plugin : mPlugin )
        {
            plugin->onPlugin(this);
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

} // namespace P2
