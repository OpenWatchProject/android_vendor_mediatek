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
#include <string.h>
#include "P2_MWData.h"
#include "P2_DebugControl.h"

#include <mtkcam/utils/imgbuf/IGraphicImageBufferHeap.h>
#include <mtkcam3/plugin/streaminfo/types.h>

using NSCam::IGraphicImageBufferHeap;
using SttInfo = NSCam::plugin::streaminfo::P1STT;
using QueryData = NSCam::v3::IStreamInfo::QueryPrivateData;

#include "P2_VRGralloc.h"

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWInfo
#define P2_TRACE        TRACE_MW_INFO
#include "P2_LogHeader.h"

#define MY_ASSERT(cond, f, arg...) \
    do {                        \
        if(!cond)               \
        {                       \
            MY_LOGF(f, ##arg);     \
        }                       \
    } while(0)


CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

MBOOL MWInfo::ID_STREAM_PAIR::isValidDir(IO_DIR dir)
{
    return (dir == IO_DIR_IN) || (dir == IO_DIR_OUT);
}

MVOID MWInfo::ID_STREAM_PAIR::set(IO_DIR dir, ID_STREAM sID)
{
    MY_ASSERT( isValidDir(dir) , "SID Pair only support pure in or out. dir(%d) sID(%d)", dir, sID);
    switch(dir)
    {
        case IO_DIR_IN : mIn = sID; break;
        case IO_DIR_OUT : mOut = sID; break;
        default:  break;
    }
}

ID_STREAM MWInfo::ID_STREAM_PAIR::get(IO_DIR dir) const
{
    MY_ASSERT( isValidDir(dir) , "SID Pair only support pure in or out. dir(%d)", dir);
    ID_STREAM sID = ID_STREAM_INVALID;
    switch(dir)
    {
        case IO_DIR_IN : sID = mIn; break;
        case IO_DIR_OUT : sID = mOut; break;
        default:  break;
    }
    return sID;
}

MWInfo::MWInfo(const NSCam::v3::P2StreamingNode::ConfigParams &param, const sp<P2InIDMap> &idMap)
{
    TRACE_FUNC_ENTER();
    if(idMap == NULL)
    {
        MY_LOGF("IDMap is Null in create P2S MWInfo !");
        return;
    }
    mDefaultSensorID = idMap->mMainSensorID;

    // not sensor specific meta / image
    initMetaInfo(OUT_APP, param.pOutAppMeta, mDefaultSensorID);
    initMetaInfo(OUT_HAL, param.pOutHalMeta, mDefaultSensorID);
    initImgStreamInfo(IN_S_REPROCESS, param.pInYuvImage, mDefaultSensorID);
    initImgStreamInfo(OUT_S_YUV, param.vOutImage, mDefaultSensorID);
    initImgStreamInfo(OUT_S_FD, param.pOutFDImage, mDefaultSensorID);

    // Main Meta & image
    initMetaInfo(IN_APP, param.pInAppMeta, mDefaultSensorID);
    initMetaInfo(IN_P1_APP, param.pInAppRetMeta, mDefaultSensorID);
    initMetaInfo(IN_P1_HAL, param.pInHalMeta, mDefaultSensorID);

    initImgStreamInfo(IN_S_OPAQUE, param.pvInOpaque, mDefaultSensorID);
    initImgStreamInfo(IN_S_FULL, param.pvInFullRaw, mDefaultSensorID);
    initImgStreamInfo(IN_S_RESIZED, param.pInResizedRaw, mDefaultSensorID);
    initImgStreamInfo(IN_S_P1STT, param.pInLcsoRaw, mDefaultSensorID);
    initImgStreamInfo(IN_S_RSSO, param.pInRssoRaw, mDefaultSensorID);
    initImgStreamInfo(IN_S_FULL_YUV, param.pInFullYuv, mDefaultSensorID);
    initImgStreamInfo(IN_S_RESIZED_YUV1, param.pInResizedYuv1, mDefaultSensorID);
    initImgStreamInfo(IN_S_RESIZED_YUV2, param.pInResizedYuv2, mDefaultSensorID);

    // Sub Meta & image
    for( const NSCam::v3::P2StreamingNode::ConfigParams::P1SubStreams &streamSet : param.vP1SubStreamsInfo )
    {
        P2InIDMap::metaFp metaMap = idMap->getMetaMap(streamSet.sensorId);
        if( metaMap != NULL )
        {
            initMetaInfo(metaMap(IN_P1_APP), streamSet.pInAppRetMeta_Sub, streamSet.sensorId);
            initMetaInfo(metaMap(IN_P1_HAL), streamSet.pInHalMeta_Sub, streamSet.sensorId);
        }
        else
        {
            MY_LOGF("sensorID(%d) metaMap is NULL in create MWInfo !!", streamSet.sensorId);
        }

        P2InIDMap::imgStreamFp imgMap = idMap->getImgStreamMap(streamSet.sensorId);
        if( imgMap != NULL )
        {
            initImgStreamInfo(imgMap(IN_S_FULL),        streamSet.pInFullRaw_Sub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_RESIZED),     streamSet.pInResizedRaw_Sub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_P1STT),        streamSet.pInLcsoRaw_Sub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_RSSO),        streamSet.pInRssoRaw_Sub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_FULL_YUV),    streamSet.pInFullYuv_Sub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_RESIZED_YUV1), streamSet.pInResizedYuv1_Sub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_RESIZED_YUV2), streamSet.pInResizedYuv2_Sub, streamSet.sensorId);
        }
        else
        {
            MY_LOGF("sensorID(%d) imgStreamMap is NULL in create MWInfo!!", streamSet.sensorId);
        }
    }

    // Physical Meta
    for( const NSCam::v3::P2StreamingNode::ConfigParams::PhysicalStream &streamSet : param.vPhysicalStreamsInfo )
    {
        P2InIDMap::metaFp metaMap = idMap->getMetaMap(streamSet.sensorId);
        if( metaMap != NULL )
        {
            initMetaInfo(metaMap(IN_APP_PHY), streamSet.pInAppPhysicalMeta, streamSet.sensorId);
            initMetaInfo(metaMap(OUT_APP_PHY), streamSet.pOutAppPhysicalMeta, streamSet.sensorId);
        }
        else
        {
            MY_LOGF("sensorID(%d) metaMap is NULL in create MWInfo !!", streamSet.sensorId);
        }
    }


    mBurstNum = param.burstNum;
    mCustomOption = param.customOption;
    TRACE_FUNC_EXIT();
}

MWInfo::MWInfo(const NSCam::v3::P2CaptureNode::ConfigParams &param, const sp<P2InIDMap> &idMap)
{
    TRACE_FUNC_ENTER();
    if(idMap == NULL)
    {
        MY_LOGF("IDMap is Null in create P2C MWInfo !");
        return;
    }
    mDefaultSensorID = idMap->mMainSensorID;
    initMetaInfo(IN_APP, param.pInAppMeta, mDefaultSensorID);
    initMetaInfo(IN_P1_APP, param.pInAppRetMeta, mDefaultSensorID);
    initMetaInfo(IN_P1_HAL, param.pInHalMeta, mDefaultSensorID);
    initMetaInfo(OUT_APP, param.pOutAppMeta, mDefaultSensorID);
    initMetaInfo(OUT_HAL, param.pOutHalMeta, mDefaultSensorID);
    initImgStreamInfo(IN_S_OPAQUE, param.pvInOpaque, mDefaultSensorID);
    initImgStreamInfo(IN_S_FULL, param.pvInFullRaw, mDefaultSensorID);
    initImgStreamInfo(IN_S_RESIZED, param.pInResizedRaw, mDefaultSensorID);
    initImgStreamInfo(IN_S_P1STT, param.pInLcsoRaw, mDefaultSensorID);
    initImgStreamInfo(IN_S_REPROCESS, param.pInFullYuv, mDefaultSensorID);
    initImgStreamInfo(IN_S_REPROCESS2, param.pInFullYuv2, mDefaultSensorID);
    initImgStreamInfo(OUT_S_YUV, param.pvOutImage, mDefaultSensorID);
    initImgStreamInfo(OUT_S_JPEG_YUV, param.pOutJpegYuv, mDefaultSensorID);
    initImgStreamInfo(OUT_S_THN_YUV, param.pOutThumbnailYuv, mDefaultSensorID);
    initImgStreamInfo(OUT_S_POSTVIEW, param.pOutPostView, mDefaultSensorID);
    initImgStreamInfo(OUT_S_CLEAN, param.pOutClean, mDefaultSensorID);
    initImgStreamInfo(OUT_S_DEPTH, param.pOutDepth, mDefaultSensorID);
    initImgStreamInfo(OUT_S_BOKEH, param.pOutBokeh, mDefaultSensorID);
    initImgStreamInfo(OUT_S_RAW, param.pvOutRaw, mDefaultSensorID);

    // Sub Meta & image
    for( const NSCam::v3::P2CaptureNode::ConfigParams::P1SubStreams &streamSet : param.vP1SubStreamsInfo )
    {
        P2InIDMap::metaFp metaMap = idMap->getMetaMap(streamSet.sensorId);
        if( metaMap != NULL )
        {
            initMetaInfo(metaMap(IN_P1_APP), streamSet.pInAppRetMetaSub, streamSet.sensorId);
            initMetaInfo(metaMap(IN_P1_HAL), streamSet.pInHalMetaSub, streamSet.sensorId);
        }
        else
        {
            MY_LOGF("sensorID(%d) metaMap is NULL in create MWInfo !!", streamSet.sensorId);
        }

        P2InIDMap::imgStreamFp imgMap = idMap->getImgStreamMap(streamSet.sensorId);
        if( imgMap != NULL )
        {
            initImgStreamInfo(imgMap(IN_S_FULL),        streamSet.pInFullRawSub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_RESIZED),     streamSet.pInResizedRawSub, streamSet.sensorId);
            initImgStreamInfo(imgMap(IN_S_P1STT),        streamSet.pInLcsoRawSub, streamSet.sensorId);
        }
        else
        {
            MY_LOGF("sensorID(%d) imgMap is NULL in create MWInfo!!", streamSet.sensorId);
        }
    }

    // Physical Meta
    for( const NSCam::v3::P2CaptureNode::ConfigParams::PhysicalStream &streamSet : param.vPhysicalStreamsInfo )
    {
        P2InIDMap::metaFp metaMap = idMap->getMetaMap(streamSet.sensorId);
        if( metaMap != NULL )
        {
            initMetaInfo(metaMap(IN_APP_PHY), streamSet.pInAppPhysicalMeta, streamSet.sensorId);
            initMetaInfo(metaMap(OUT_APP_PHY), streamSet.pOutAppPhysicalMeta, streamSet.sensorId);
        }
        else
        {
            MY_LOGF("sensorID(%d) metaMap is NULL in create MWInfo !!", streamSet.sensorId);
        }
    }

    mIsCapture = MTRUE;
    mCustomOption = param.uCustomOption;
    TRACE_FUNC_EXIT();
}
MWInfo::~MWInfo()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL MWInfo::isCapture() const
{
    return mIsCapture;
}

MBOOL MWInfo::isValid(const ILog &log) const
{
    TRACE_S_FUNC_ENTER_2(log);
    MBOOL ret = MTRUE;
    MBOOL hasFull = hasImgStream(IN_S_FULL);
    MBOOL hasResized = hasImgStream(IN_S_RESIZED);
    MBOOL hasReprYUVInput = hasImgStream(IN_S_REPROCESS) || hasImgStream(IN_S_REPROCESS2);
    MBOOL hasFD = hasImgStream(OUT_S_FD);
    MBOOL hasYUV = hasImgStream(OUT_S_YUV) || hasImgStream(OUT_S_JPEG_YUV);
    MBOOL hasOutRaw = hasImgStream(OUT_S_RAW);
    MBOOL hasInApp = hasMeta(IN_APP);
    MBOOL hasInHal = hasMeta(IN_P1_HAL);
    if( !(hasFull || hasResized || hasReprYUVInput) ||
        !(hasFD || hasYUV || hasOutRaw) ||
        !hasInApp || !hasInHal )
    {
        MY_S_LOGW(log, "missing necessary stream: full(%d) resized(%d) reproc(%d) fd(%d) yuv(%d) outRaw(%d) inApp(%d) inHal(%d)", hasFull, hasResized, hasReprYUVInput, hasFD, hasYUV, hasOutRaw, hasInApp, hasInHal);
        ret = MFALSE;
    }
    this->print(log);
    TRACE_S_FUNC_EXIT_2(log);
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

ID_STREAM MWInfo::toStreamID(StreamId_T sID, IO_DIR dir) const
{
    ID_STREAM id = ID_STREAM_INVALID;
    auto it = mImgStreamIDMap.find(sID);
    if( it != mImgStreamIDMap.end() )
    {
        id = it->second.get(dir);
    }
    return id;
}

std::list<ID_IMG> MWInfo::toImgIDs(StreamId_T sID, IO_DIR dir) const
{
    return getImgsFromStream(toStreamID(sID, dir));
}

MUINT32 MWInfo::toSensorID(StreamId_T sID) const
{
    MUINT32 id = mDefaultSensorID;
    auto it = mSensorIDMap.find(sID);
    if( it != mSensorIDMap.end() )
    {
        id = it->second;
    }
    else
    {
        MY_LOGE("Can not find SensorID in stream 0x%09" PRIx64 , sID);
    }
    return id;
}

IMG_TYPE MWInfo::getImgType(StreamId_T sID) const
{
    IMG_TYPE type = IMG_TYPE_UNKNOWN;
    auto it = mAppStreamInfoMap.find(sID);
    if( it != mAppStreamInfoMap.end() )
    {
        type = it->second.mImageType;
    }
    return type;
}

MBOOL MWInfo::isCaptureIn(StreamId_T sID) const
{
    MBOOL ret = MFALSE;
    auto it = mImgStreamIDMap.find(sID);
    if( it != mImgStreamIDMap.end() )
    {
        ID_STREAM s = it->second.get(IO_DIR_IN);
        ret = (s == IN_S_OPAQUE) ||
              (s == IN_S_FULL) ||
              (s == IN_S_REPROCESS);
    }
    return ret;
}

MUINT32 MWInfo::getBurstNum() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mBurstNum;
}

MUINT32 MWInfo::getCustomOption() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mCustomOption;
}

std::vector<P2AppStreamInfo> MWInfo::getAppStreamInfo() const
{
    std::vector<P2AppStreamInfo> streamInfo;
    streamInfo.reserve(mAppStreamInfoMap.size());
    for( const auto &info : mAppStreamInfoMap )
    {
        streamInfo.push_back(info.second);
    }
    return streamInfo;
}

MBOOL MWInfo::supportPQ() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mCustomOption & NSCam::v3::P2Common::CUSTOM_OPTION_PQ_SUPPORT;
}

MBOOL MWInfo::supportClearZoom() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mCustomOption & NSCam::v3::P2Common::CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT;
}

MBOOL MWInfo::supportDRE() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mCustomOption & NSCam::v3::P2Common::CUSTOM_OPTION_DRE_SUPPORT;
}

MBOOL MWInfo::supportHFG() const
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return mCustomOption & NSCam::v3::P2Common::CUSTOM_OPTION_HFG_SUPPORT;
}

MVOID MWInfo::print(const ILog &log) const
{
    TRACE_S_FUNC_ENTER(log);
    LogString dumpStr(2048);
    dumpStr.append("%s",log.getLogStr());
    for( auto &info : P2Img::InfoMap )
    {
        std::vector<sp<IImageStreamInfo>> mwInfo = findStreamInfo(info.second.sID);
        for( unsigned i = 0, size = mwInfo.size(); i < size; ++i )
        {
            sp<IImageStreamInfo> img = mwInfo[i];
            const char *name = "NA";
            StreamId_T id = 0;
            MUINT64 allocator = 0, consumer = 0;
            MINT imgFmt = 0;
            MSize imgSize;
            IMG_TYPE type = IMG_TYPE_EXTRA;
            MINT physicalId = -1;
            if( img != NULL )
            {
                //name = img->getStreamName();
                id = img->getStreamId();
                allocator = img->getUsageForAllocator();
                consumer = img->getUsageForConsumer();
                imgFmt = img->getImgFormat();
                imgSize = img->getImgSize();
                if(img->getPhysicalCameraId() >= 0)
                {
                    physicalId = img->getPhysicalCameraId();
                }
                if(mAppStreamInfoMap.count(id) != 0)
                {
                    type = mAppStreamInfoMap.at(id).mImageType;
                }
            }
            dumpStr.append(" StreamInfo: [img:0x%09" PRIx64 "] (A/C:0x%09" PRIx64 "/0x%09" PRIx64 ") %s[%d]/%s streamSize(%dx%d) (fmt:0x%08x) type(%s) phy(%d)",
                    id, allocator, consumer, info.second.name.c_str(), i, name, imgSize.w, imgSize.h, imgFmt, ImgType2Name(type), physicalId);
        }
    }
    for( auto &info : P2Meta::InfoMap )
    {
        std::vector<sp<IMetaStreamInfo>> mwInfo = findStreamInfo(info.first);
        for( unsigned i = 0, size = mwInfo.size(); i < size; ++i )
        {
            sp<IMetaStreamInfo> meta = mwInfo[i];
            const char *name = "NA";
            StreamId_T id = 0;
            if( meta != NULL )
            {
                name = meta->getStreamName();
                id = meta->getStreamId();
            }
            dumpStr.append(" StreamInfo: [meta:0x%09" PRIx64 "] %s[%d]/%s", id, info.second.name.c_str(), i, name);
        }
    }
    MY_SAFE_LOGD(dumpStr.c_str(), dumpStr.length());
    TRACE_S_FUNC_EXIT(log);
}

MVOID MWInfo::initMetaInfo(ID_META id, const sp<IMetaStreamInfo> &info, MUINT32 sensorID)
{
    TRACE_FUNC_ENTER();
    if( info != NULL )
    {
        mMetaInfoMap[id].push_back(info);
        mMetaIDMap[info->getStreamId()] = id;
        mSensorIDMap[info->getStreamId()] = sensorID;
    }
    TRACE_FUNC_EXIT();
}

MVOID MWInfo::initMetaInfo(ID_META id, const Vector<sp<IMetaStreamInfo>> &infos, MUINT32 sensorID)
{
    TRACE_FUNC_ENTER();
    for( auto &&info : infos )
    {
        if( info != NULL )
        {
            mMetaInfoMap[id].push_back(info);
            mMetaIDMap[info->getStreamId()] = id;
            mSensorIDMap[info->getStreamId()] = sensorID;
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID MWInfo::initImgStreamInfo(ID_STREAM id, const sp<IImageStreamInfo> &info, MUINT32 sensorID)
{
    TRACE_FUNC_ENTER();
    if( info != NULL )
    {
        IO_DIR dir = getDirFromStream(id);
        mImgStreamInfoMap[id].push_back(info);
        mImgStreamIDMap[info->getStreamId()].set(dir, id);
        mSensorIDMap[info->getStreamId()] = sensorID;
        if(id == OUT_S_FD)
        {
            mAppStreamInfoMap[info->getStreamId()].mImageType = IMG_TYPE_FD;
        }
    }
    TRACE_FUNC_EXIT();
}

MVOID MWInfo::initImgStreamInfo(ID_STREAM id, const Vector<sp<IImageStreamInfo>> &infos, MUINT32 sensorID)
{
    TRACE_FUNC_ENTER();
    std::list<std::pair<StreamId_T, MSize> > hwCompIds;
    std::list<std::pair<StreamId_T, MSize> > hwTextureIds;
    std::list<std::pair<StreamId_T, MSize> > hwEncodeIds;
    IO_DIR dir = getDirFromStream(id);
    for( auto &&info : infos )
    {
        if( info != NULL )
        {
            MUINT32 usage = info->getUsageForAllocator();
            StreamId_T sId = info->getStreamId();
            MSize streamSize = info->getImgSize();

            mImgStreamInfoMap[id].push_back(info);
            mImgStreamIDMap[sId].set(dir, id);
            mSensorIDMap[sId] = sensorID;
            if(info->getPhysicalCameraId() >= 0)
            {
                mAppStreamInfoMap[sId].mImageType = IMG_TYPE_PHYSICAL;
                mAppStreamInfoMap[sId].mImageSize = streamSize;
            }
            if(usage & GRALLOC_USAGE_HW_COMPOSER) hwCompIds.push_back(std::make_pair(sId, streamSize));
            if(usage & GRALLOC_USAGE_HW_TEXTURE) hwTextureIds.push_back(std::make_pair(sId, streamSize));
            if(usage & GRALLOC_USAGE_HW_VIDEO_ENCODER) hwEncodeIds.push_back(std::make_pair(sId, streamSize));
        }
    }

    auto getUnOccupiedID = [&]
        (
            std::list<std::pair<StreamId_T, MSize> > &idList,
            StreamId_T &out,
            MSize &size
        )
        -> bool
        {
            for(auto && id : idList)
            {
                if(mAppStreamInfoMap.count(id.first) == 0)
                {
                    out = id.first;
                    size = id.second;
                    return true;
                }
            }
            return false;
        };

    // Record -> Display -> Unknown
    StreamId_T sID = (StreamId_T)(-1);
    MSize streamSize;
    if(getUnOccupiedID(hwEncodeIds, sID, streamSize))
    {
        mAppStreamInfoMap[sID].mImageType = IMG_TYPE_RECORD;
        mAppStreamInfoMap[sID].mImageSize = streamSize;
    }

    if(getUnOccupiedID(hwCompIds, sID, streamSize))
    {
        mAppStreamInfoMap[sID].mImageType = IMG_TYPE_DISPLAY;
        mAppStreamInfoMap[sID].mImageSize = streamSize;
    }
    else if(getUnOccupiedID(hwTextureIds, sID, streamSize))
    {
        mAppStreamInfoMap[sID].mImageType = IMG_TYPE_DISPLAY;
        mAppStreamInfoMap[sID].mImageSize = streamSize;
    }

    TRACE_FUNC_EXIT();
}

std::vector<sp<IMetaStreamInfo>> MWInfo::findStreamInfo(ID_META id) const
{
    auto it = mMetaInfoMap.find(id);
    return (it != mMetaInfoMap.end()) ?
           it->second : std::vector<sp<IMetaStreamInfo>>();
}

std::vector<sp<IImageStreamInfo>> MWInfo::findStreamInfo(ID_STREAM id) const
{
    auto it = mImgStreamInfoMap.find(id);
    return (it != mImgStreamInfoMap.end()) ?
           it->second : std::vector<sp<IImageStreamInfo>>();
}

MBOOL MWInfo::hasMeta(ID_META id) const
{
    auto &&it = mMetaInfoMap.find(id);
    return it != mMetaInfoMap.end() && it->second.size();
}

MBOOL MWInfo::hasImgStream(ID_STREAM id) const
{
    auto &&it = mImgStreamInfoMap.find(id);
    return it != mImgStreamInfoMap.end() && it->second.size();
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWMeta
#define P2_TRACE        TRACE_MW_META
#include "P2_LogHeader.h"

MWMeta::MWMeta(const ILog &log, const P2Pack &p2Pack, const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const META_INFO &info)
    : P2Meta(log, p2Pack, info.id)
    , mMWFrame(frame)
    , mReleaseToken(frame != NULL ? frame->getReleaseToken() : NULL)
    , mStreamID(streamID)
    , mDir(dir)
    , mStatus(IO_STATUS_INVALID)
    , mMetadata(NULL)
    , mLockedMetadata(NULL)
    , mMetadataCopy(NULL)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "acquireMeta");
    mStreamBuffer = mMWFrame->acquireMetaStream(mStreamID);
    if( mStreamBuffer != NULL )
    {
        mLockedMetadata = mMWFrame->acquireMeta(mStreamBuffer, mDir);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( mLockedMetadata != NULL)
    {
        if( dir == IO_DIR_IN &&
            (info.flag & IO_FLAG_COPY) )
        {
            if( dir == IO_DIR_IN &&
                (info.flag & IO_FLAG_COPY) )
            {
                mMetadataCopy = new IMetadata();
                *mMetadataCopy = *mLockedMetadata;
                mMetadata = mMetadataCopy;
            }
            else
            {
                mMetadata = mLockedMetadata;
            }
            mStatus = IO_STATUS_READY;
            TRACE_S_FUNC(mLog, "meta=%p count= %d", mMetadata, mMetadata->count());
        }
        else
        {
            mMetadata = mLockedMetadata;
        }
        mStatus = IO_STATUS_READY;
        TRACE_S_FUNC(mLog, "meta=%p count= %d", mMetadata, mMetadata->count());
    }
    TRACE_S_FUNC_EXIT_2(mLog);
}

MWMeta::~MWMeta()
{
    TRACE_S_FUNC_ENTER_2(mLog, "name(%s), count(%d)",
                       (mStreamBuffer != NULL) ? mStreamBuffer->getName() : "??",
                       (mLockedMetadata != NULL) ? mLockedMetadata->count() : -1);
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "~MWMeta->releaseMeta");
    MWFrame::releaseMeta(mReleaseToken, mStreamBuffer, mLockedMetadata);
    mMetadata = mLockedMetadata = NULL;
    MWFrame::releaseMetaStream(mReleaseToken, mStreamBuffer, mDir, mStatus);
    mStreamBuffer = NULL;
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( mMWFrame != NULL )
    {
        mMWFrame->notifyRelease();
        mMWFrame.clear();
    }

    if(mMetadataCopy != NULL)
    {
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "~MWMetag->freeCopyMeta");
        delete mMetadataCopy;
        mMetadataCopy = NULL;
        P2_CAM_TRACE_END(TRACE_ADVANCED);
    }
    TRACE_S_FUNC_EXIT_2(mLog);
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
    TRACE_S_FUNC_ENTER_2(mLog);
    if( (mDir & IO_DIR_OUT) &&
        mStatus != IO_STATUS_INVALID )
    {
        mStatus = result ? IO_STATUS_OK : IO_STATUS_ERROR;
    }
    TRACE_S_FUNC_EXIT_2(mLog);
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

MVOID MWMeta::detach()
{
    TRACE_S_FUNC_ENTER(mLog, "name(%s)", (mStreamBuffer != NULL) ? mStreamBuffer->getName() : "??");
    if( mMWFrame != NULL )
    {
        mMWFrame->preReleaseMetaStream(mStreamBuffer, mDir);
        mMWFrame->notifyRelease();
        mMWFrame.clear();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWImg
#define P2_TRACE        TRACE_MW_IMG
#include "P2_LogHeader.h"

std::list<sp<P2Img>> MWImg::createP2Imgs(const ILog &log,const P2Pack &p2Pack, const sp<MWInfo> &mwInfo, const sp<MWFrame> &frame, const StreamId_T &streamID,
                                                IO_DIR dir, MUINT32 debugIndex, MBOOL needSWRW)
{
    const std::list<ID_IMG> imgIDs = mwInfo->toImgIDs(streamID, dir);
    sp<MWStream> stream = new MWStream(log, frame, streamID, dir, imgIDs);
    MUINT32 sensorID = mwInfo->toSensorID(streamID);
    MBOOL needPackUpdate = (dir & IO_DIR_IN) && (sensorID != p2Pack.getSensorData().mSensorID);
    TRACE_S_FUNC(log, "sID=0x%09" PRIx64 ", sensorID=%d, needUpdate=%d", streamID, sensorID, needPackUpdate);
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "createP2Imgs");
    IMG_TYPE type = mwInfo->getImgType(streamID);
    std::list<sp<P2Img>> imgs;

    for( ID_IMG id : imgIDs)
    {
        const IMG_INFO &info = P2Img::getImgInfo(id);
        if( !(info.dir & dir) )
        {
            MY_S_LOGW(log, "Invalid img info(0x%09" PRIx64 "/%d:%s) dir=wanted(%d)/listed(%d)", streamID, id, info.name.c_str(), dir, info.dir);
        }
        else if( info.flag & IO_FLAG_INVALID )
        {
            MY_S_LOGW(log, "Invalid img info(0x%09" PRIx64 "/%d:%s) Invalid IO_INFO: flag(%d)", streamID, id, info.name.c_str(), info.flag);
        }
        else
        {
            std::vector<sp<IImageBuffer>> imgBufs = stream->acquireBuffers(id, info.mirror, needSWRW);
            if( !imgBufs.empty() )
            {
                sp<P2Img> img = new MWImg(log, (needPackUpdate ? p2Pack.getP2Pack(log, sensorID) : p2Pack), stream, imgBufs, type, info, debugIndex);
                imgs.push_back(img);
            }
        }
    }
    MY_S_LOGI_IF(log.getLogLevel(), log, "acquire Img StreamID=0x%09" PRIx64 ", type(%s), imgIDCnt(%zu), finalImgsCnt(%zu)", streamID, ImgType2Name(type), imgIDs.size(), imgs.size());
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    return imgs;
}

MWImg::MWImg(const ILog &log, const P2Pack &p2Pack, const sp<MWStream> &stream, const std::vector<sp<IImageBuffer>> &buffers, IMG_TYPE type, const IMG_INFO &info, MUINT32 debugIndex)
    : P2Img(log, p2Pack, info.id, debugIndex)
    , mMWStream(stream)
    , mImageBuffers(buffers)
    , mType(type)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    if( mMWStream != NULL && mImageBuffers.size() )
    {
        size_t size = mImageBuffers.size();
        mImageBufferPtrs.resize(size, NULL);
        for( size_t i = 0; i < size; ++i )
        {
            mImageBufferPtrs[i] = mImageBuffers[i].get();
        }
        mFirstImageBuffer = mImageBufferPtrs[0];

        mTransform = mMWStream->getTransform();
        mUsage = mMWStream->getUsage();
        mDir = mMWStream->getDir();
        mPhysicalID = mMWStream->getPhysicalID();
    }
    TRACE_S_FUNC(log, "sID=0x%09" PRIx64 ", id/name(%d/%s), #Buf(%zu), trans(%d), dir(%d)",
                (mMWStream != NULL) ? mMWStream->getStreamID() : 0xfffffff,
                info.id, info.name.c_str(),
                mImageBuffers.size(), mTransform, mDir);
    TRACE_S_FUNC_EXIT_2(mLog);
}

MWImg::~MWImg()
{
    TRACE_S_FUNC_ENTER_2(mLog);
    processPlugin();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "~MWImg->releaseImage");
    MY_S_LOGI_IF(mLog.getLogLevel(), mLog, "release Img 0x%09" PRIx64 ", type(%s), name(%s), time(%" PRId64 ")",
            mMWStream->getStreamID(), ImgType2Name(mType), getHumanName(), (mFirstImageBuffer != NULL) ? mFirstImageBuffer->getTimestamp() : -1);
    MWFrame::releaseImage(mMWStream->getReleaseToken(), mImageBuffers);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    mFirstImageBuffer = NULL;
    mImageBufferPtrs.clear();
    mImageBuffers.clear();
    mMWStream = NULL;
    TRACE_S_FUNC_EXIT_2(mLog);
}

MBOOL MWImg::isValid() const
{
    return (mFirstImageBuffer != NULL);
}

IO_DIR MWImg::getDir() const
{
    return mDir;
}

MVOID MWImg::registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    mPlugin = plugin;
    TRACE_S_FUNC_EXIT_2(mLog);
}

MVOID MWImg::updateResult(MBOOL result)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    if( mMWStream != NULL )
    {
        mMWStream->updateResult(result);
    }
    TRACE_S_FUNC_EXIT_2(mLog);
}

MVOID MWImg::updateVRTimestamp(MUINT32 count, const MINT64 &cam2FwTs)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    buffer_handle_t handle;
    if( getBufferHandle(handle) )
    {
        updateGralloc_VRInfo(mLog, handle, mImageBufferPtrs, count, cam2FwTs);
    }
    TRACE_S_FUNC_EXIT_2(mLog);
}

IImageBuffer* MWImg::getIImageBufferPtr() const
{
    return mFirstImageBuffer;
}

std::vector<IImageBuffer*> MWImg::getIImageBufferPtrs() const
{
    return mImageBufferPtrs;
}

MUINT32 MWImg::getIImageBufferPtrsCount() const
{
    return mImageBufferPtrs.size();
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
    //return (mUsage & (GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE));
    return mType == IMG_TYPE_DISPLAY;
}

MBOOL MWImg::isRecord() const
{
    //return mUsage & GRALLOC_USAGE_HW_VIDEO_ENCODER;
    return mType == IMG_TYPE_RECORD;
}

MBOOL MWImg::isPhysicalStream() const
{
    return mType == IMG_TYPE_PHYSICAL;
}

MBOOL MWImg::isFD() const
{
    return mType == IMG_TYPE_FD;
}

MVOID MWImg::detach()
{
    TRACE_S_FUNC_ENTER(mLog, "name=%s", getHumanName());
    mMWStream->detach();
    TRACE_S_FUNC_EXIT(mLog);
}

IMG_TYPE MWImg::getImgType() const
{
    return mType;
}

MBOOL MWImg::isCapture() const
{
    return !(mUsage & (GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_HW_VIDEO_ENCODER));
}

MVOID MWImg::processPlugin() const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    if( mMWStream->isStatusValid() )
    {
        for( auto plugin : mPlugin )
        {
            plugin->onPlugin(this);
        }
    }
    TRACE_S_FUNC_EXIT_2(mLog);
}

MBOOL MWImg::getBufferHandle(buffer_handle_t &handle) const
{
    TRACE_S_FUNC_ENTER_2(mLog);
    MBOOL ret = MTRUE;

    IImageBufferHeap *heap = NULL;
    IGraphicImageBufferHeap *graphicHeap = NULL;
    buffer_handle_t *ptr = NULL;
    if( mFirstImageBuffer )
    {
        heap = mFirstImageBuffer->getImageBufferHeap();
    }
    if( heap )
    {
        graphicHeap = IGraphicImageBufferHeap::castFrom(heap);
    }
    if( graphicHeap )
    {
        ptr = graphicHeap->getBufferHandlePtr();
    }

    if( ptr == NULL )
    {
        MY_S_LOGE(mLog, "Fail to cast GraphicBufferHeap: buf=%p heap=%p graphicHeap=%p handle=%p", mFirstImageBuffer, heap, graphicHeap, ptr);
        ret = MFALSE;
    }
    else
    {
        handle = *ptr;
    }

    TRACE_S_FUNC_EXIT_2(mLog);
    return ret;
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWStream
#define P2_TRACE        TRACE_MW_STREAM
#include "P2_LogHeader.h"

MWStream::MWStream(const ILog &log, const sp<MWFrame> &frame, const StreamId_T &streamID, IO_DIR dir, const std::list<ID_IMG> &imgIDs)
    : mLog(log)
    , mMWFrame(frame)
    , mReleaseToken(frame != NULL ? frame->getReleaseToken() : NULL)
    , mStreamID(streamID)
    , mDir(dir)
    , mStatus(IO_STATUS_INVALID)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "acquireImageStreamHeap");
    MY_S_LOGI_IF(mLog.getLogLevel(), mLog, "acquire ImgStream 0x%09" PRIx64 , mStreamID);
    mStreamBuffer = mMWFrame->acquireImageStream(mStreamID);
    mHeap = mMWFrame->acquireImageHeap(mStreamBuffer, dir);
    if( mStreamBuffer != NULL && mHeap != NULL)
    {
        mIsComposedType = (imgIDs.size() > 1);
        mPhysicalID = mStreamBuffer->getStreamInfo()->getPhysicalCameraId();
        const IImageStreamInfo *sInfo = mStreamBuffer->getStreamInfo();
        if( sInfo != NULL )
        {
            //if( mIsComposedType ) //TODO need check transform & usage if buffer come from plugin data
            //{
                mTransform = sInfo->getTransform();
                mUsage = sInfo->getUsageForAllocator();
            //}
            mStatus = IO_STATUS_READY;
        }
        else
        {
            MY_S_LOGD(log,"mStreamBuffer->getStreamInfo return null ");
        }
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_S_FUNC_EXIT_2(mLog);
}

MWStream::~MWStream()
{
    TRACE_S_FUNC_ENTER_2(mLog);
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "~MWImg->releaseStreamHeap");
    MWFrame::releaseImageStream(mReleaseToken, mStreamBuffer, mDir, mStatus);
    MWFrame::releaseImageHeap(mReleaseToken, mStreamBuffer, mHeap);
    mStreamBuffer = NULL;
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    if( mMWFrame != NULL )
    {
        mMWFrame->notifyRelease();
        mMWFrame.clear();
    }
    TRACE_S_FUNC_EXIT_2(mLog);
}

std::vector<sp<IImageBuffer>> MWStream::acquireBuffers(ID_IMG id, ID_IMG mirrorID, MBOOL needSWRW)
{
    std::vector<sp<IImageBuffer>> imgBufs;
    if( mStreamBuffer != NULL )
    {
        if( mIsComposedType )
        {
            const QueryData data = mStreamBuffer->getStreamInfo()->queryPrivateData();
            const SttInfo *pStt = NSCam::plugin::streaminfo::get_data_if<SttInfo>(&data.privData);
            if( pStt )
            {
                if( pStt->useLcso() && mirrorID == IN_LCSO && pStt->mLcsoInfo )
                {
                    imgBufs = mMWFrame->acquireImage(mStreamBuffer, mHeap, *(pStt->mLcsoInfo), mDir, needSWRW, MFALSE);
                }
                else if( pStt->useLcsho() && mirrorID == IN_LCSHO && pStt->mLcshoInfo )
                {
                    imgBufs = mMWFrame->acquireImage(mStreamBuffer, mHeap, *(pStt->mLcshoInfo), mDir, needSWRW, MFALSE);
                }
            }
        }
        else
        {
            MBOOL isOpaque = (id == IN_OPAQUE);
            const NSCam::ImageBufferInfo &blobInfo = mStreamBuffer->getStreamInfo()->getImageBufferInfo();
            imgBufs = mMWFrame->acquireImage(mStreamBuffer, mHeap, blobInfo, mDir, needSWRW, isOpaque);
        }
    }
    return imgBufs;
}

StreamId_T MWStream::getStreamID() const
{
    return mStreamID;
}

IO_DIR MWStream::getDir() const
{
    return mDir;
}

MUINT32 MWStream::getTransform() const
{
    return mTransform;
}

MUINT32 MWStream::getUsage() const
{
    return mUsage;
}

MINT32 MWStream::getPhysicalID() const
{
    return mPhysicalID;
}

MBOOL MWStream::isStatusValid() const
{
    return (mStatus != IO_STATUS_ERROR && mStatus != IO_STATUS_INVALID);
}

sp<MWFrame::ReleaseToken> MWStream::getReleaseToken() const
{
    return mReleaseToken;
}

MVOID MWStream::updateResult(MBOOL result)
{
    TRACE_S_FUNC_ENTER_2(mLog);
    if( !mIsComposedType && (mDir & IO_DIR_OUT) && mStatus != IO_STATUS_INVALID )
    {
        mStatus = result ? IO_STATUS_OK : IO_STATUS_ERROR;
    }
    TRACE_S_FUNC_EXIT_2(mLog);
}

MVOID MWStream::detach()
{
    TRACE_S_FUNC_ENTER(mLog, "name(%s)", (mStreamBuffer != NULL) ? mStreamBuffer->getName() : "??");
    if( mMWFrame != NULL )
    {
        mMWFrame->preReleaseImageStream(mStreamBuffer, mDir);
        mMWFrame->notifyRelease();
        mMWFrame.clear();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

} // namespace P2
