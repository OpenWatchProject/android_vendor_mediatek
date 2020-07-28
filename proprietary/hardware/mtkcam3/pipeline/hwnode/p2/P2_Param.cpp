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

#include "P2_Param.h"

#define DUMP_PATH "/data/vendor/p2_dump"

using NSCam::TuningUtils::FILE_DUMP_NAMING_HINT;
using NSCam::TuningUtils::RAW_PORT_IMGO;
using NSCam::TuningUtils::RAW_PORT_RRZO;
using NSCam::TuningUtils::YUV_PORT_WDMAO;
using NSCam::TuningUtils::YUV_PORT_WROTO;
using NSCam::TuningUtils::YUV_PORT_UNDEFINED;

namespace P2
{

#define MAIN_META_INFO(id, dir, name, flag) \
    { id, META_INFO(id, id, dir, name, flag) }

#define SUB_META_INFO(index, id, dir, name, flag) \
    { id##_##index, META_INFO(id##_##index, id, dir, name "_" #index, flag) }

#define MAIN_IMG_INFO(id, dir, name, flag, sid) \
    { id, IMG_INFO(id, id, dir, name, flag, sid) }

#define SUB_IMG_INFO(index, id, dir, name, flag, sid) \
    { id##_##index, IMG_INFO(id##_##index, id, dir, name "_" #index, flag, sid##_##index) }

META_INFO::META_INFO()
{
}

META_INFO::META_INFO(ID_META sID, ID_META sMirror, IO_DIR sDir, const std::string &sName, MUINT32 sFlag)
    : id(sID)
    , mirror(sMirror)
    , dir(sDir)
    , name(sName)
    , flag(sFlag)
{
}

IMG_INFO::IMG_INFO()
{
}

IMG_INFO::IMG_INFO(ID_IMG sID, ID_IMG sMirror, IO_DIR sDir, const std::string &sName, MUINT32 sFlag, ID_STREAM sId)
    : id(sID)
    , mirror(sMirror)
    , dir(sDir)
    , name(sName)
    , flag(sFlag)
    , sID(sId)
{
}

#define DEF_META_INFO(index) \
    SUB_META_INFO(index, IN_APP_PHY, IO_DIR_IN,  "inAppPhy", IO_FLAG_DEFAULT ), \
    SUB_META_INFO(index, IN_P1_APP,  IO_DIR_IN,  "inP1App",  IO_FLAG_COPY    ), \
    SUB_META_INFO(index, IN_P1_HAL,  IO_DIR_IN,  "inP1Hal",  IO_FLAG_COPY    ), \
    SUB_META_INFO(index, OUT_APP_PHY,IO_DIR_OUT, "outAppPhy",IO_FLAG_DEFAULT )

const std::unordered_map<ID_META, META_INFO> P2Meta::InfoMap =
{
    MAIN_META_INFO(IN_APP,     IO_DIR_IN,  "inApp",    IO_FLAG_DEFAULT ),
    MAIN_META_INFO(OUT_APP,    IO_DIR_OUT, "outApp",   IO_FLAG_DEFAULT ),
    MAIN_META_INFO(OUT_HAL,    IO_DIR_OUT, "outHal",   IO_FLAG_DEFAULT ),

    DEF_ENUM_BY_SENSOR(DEF_META_INFO),
};

#define DEF_IMG_INFO(index) \
    SUB_IMG_INFO(index, IN_OPAQUE,           IO_DIR_IN,  "inOpaque",         IO_FLAG_DEFAULT, IN_S_OPAQUE),  \
    SUB_IMG_INFO(index, IN_FULL,             IO_DIR_IN,  "inFull",           IO_FLAG_DEFAULT, IN_S_FULL),  \
    SUB_IMG_INFO(index, IN_RESIZED,          IO_DIR_IN,  "inResized",        IO_FLAG_DEFAULT, IN_S_RESIZED),  \
    SUB_IMG_INFO(index, IN_LCSO,             IO_DIR_IN,  "inLCSO",           IO_FLAG_DEFAULT, IN_S_P1STT),  \
    SUB_IMG_INFO(index, IN_LCSHO,            IO_DIR_IN,  "inLCSHO",          IO_FLAG_DEFAULT, IN_S_P1STT),  \
    SUB_IMG_INFO(index, IN_RSSO,             IO_DIR_IN,  "inRSSO",           IO_FLAG_DEFAULT, IN_S_RSSO),  \
    SUB_IMG_INFO(index, IN_FULL_YUV,         IO_DIR_IN,  "inFullYuv",        IO_FLAG_DEFAULT, IN_S_FULL_YUV),  \
    SUB_IMG_INFO(index, IN_RESIZED_YUV1,     IO_DIR_IN,  "inResizedYuv1",    IO_FLAG_DEFAULT, IN_S_RESIZED_YUV1),  \
    SUB_IMG_INFO(index, IN_RESIZED_YUV2,     IO_DIR_IN,  "inResizedYuv2",    IO_FLAG_DEFAULT, IN_S_RESIZED_YUV2)

const std::unordered_map<ID_IMG, IMG_INFO> P2Img::InfoMap =
{
    MAIN_IMG_INFO(IN_REPROCESS,        IO_DIR_IN,  "inReprocess",      IO_FLAG_DEFAULT,     IN_S_REPROCESS),
    MAIN_IMG_INFO(IN_REPROCESS2,       IO_DIR_IN,  "inReprocess2",     IO_FLAG_DEFAULT,     IN_S_REPROCESS2),
    MAIN_IMG_INFO(OUT_FD,              IO_DIR_OUT, "outFD",            IO_FLAG_DEFAULT,     OUT_S_FD),
    MAIN_IMG_INFO(OUT_THN_YUV,         IO_DIR_OUT, "outThumbnailYUV",  IO_FLAG_DEFAULT,     OUT_S_THN_YUV),
    MAIN_IMG_INFO(OUT_JPEG_YUV,        IO_DIR_OUT, "outJpegYUV",       IO_FLAG_DEFAULT,     OUT_S_JPEG_YUV),
    MAIN_IMG_INFO(OUT_YUV,             IO_DIR_OUT, "outYUV",           IO_FLAG_DEFAULT,     OUT_S_YUV),
    MAIN_IMG_INFO(OUT_POSTVIEW,        IO_DIR_OUT, "outPostView",      IO_FLAG_DEFAULT,     OUT_S_POSTVIEW),
    MAIN_IMG_INFO(OUT_CLEAN,           IO_DIR_OUT, "outClean",         IO_FLAG_DEFAULT,     OUT_S_CLEAN),
    MAIN_IMG_INFO(OUT_DEPTH,           IO_DIR_OUT, "outDepth",         IO_FLAG_DEFAULT,     OUT_S_DEPTH),
    MAIN_IMG_INFO(OUT_BOKEH,           IO_DIR_OUT, "outBokeh",         IO_FLAG_DEFAULT,     OUT_S_BOKEH),
    MAIN_IMG_INFO(OUT_RAW,             IO_DIR_OUT, "outRaw",           IO_FLAG_DEFAULT,     OUT_S_RAW),
    DEF_ENUM_BY_SENSOR(DEF_IMG_INFO),
};

const std::vector<ID_IMG> P2InIDMap::InputImgIDs =
{
    IN_OPAQUE,
    IN_FULL,
    IN_RESIZED,
    IN_LCSO,
    IN_LCSHO,
    IN_RSSO,
    IN_FULL_YUV,
    IN_RESIZED_YUV1,
    IN_RESIZED_YUV2,
};

// ID Mapping Function (Meta)

#define MAKE_META_FUNC(index)                       \
static ID_META _getMetaID_##index(ID_META id)        \
{                                                       \
    switch(id)                                          \
    {                                                   \
        case IN_APP:        return IN_APP;              \
        case IN_P1_APP:     return IN_P1_APP_##index;   \
        case IN_P1_HAL:     return IN_P1_HAL_##index;   \
        case IN_APP_PHY:    return IN_APP_PHY_##index;  \
        case OUT_APP_PHY:   return OUT_APP_PHY_##index; \
        default:            return NO_META;             \
    }                                                   \
}

#define MAKE_META_FUNC_PTR(index)    \
    &_getMetaID_##index

DEF_FUNC_BY_SENSOR(MAKE_META_FUNC)

const std::vector<P2InIDMap::metaFp> P2InIDMap::MetaFs =
{
    DEF_ENUM_BY_SENSOR(MAKE_META_FUNC_PTR),
};

// ID Mapping Function (Image)

#define MAKE_IMG_FUNC(index)                                \
static ID_IMG _getImgID_##index(ID_IMG id)                   \
{                                                               \
    switch(id)                                                  \
    {                                                           \
        case IN_REPROCESS:      return IN_REPROCESS;            \
        case IN_REPROCESS2:     return IN_REPROCESS2;           \
        case IN_OPAQUE:         return IN_OPAQUE_##index;       \
        case IN_FULL:           return IN_FULL_##index;         \
        case IN_RESIZED:        return IN_RESIZED_##index;      \
        case IN_LCSO:           return IN_LCSO_##index;         \
        case IN_LCSHO:          return IN_LCSHO_##index;        \
        case IN_RSSO:           return IN_RSSO_##index;         \
        case IN_FULL_YUV:       return IN_FULL_YUV_##index;     \
        case IN_RESIZED_YUV1:   return IN_RESIZED_YUV1_##index; \
        case IN_RESIZED_YUV2:   return IN_RESIZED_YUV2_##index; \
        default:                return NO_IMG;                  \
    }                                                           \
}

#define MAKE_IMG_FUNC_PTR(index)    \
    &_getImgID_##index

DEF_FUNC_BY_SENSOR(MAKE_IMG_FUNC)

const std::vector<P2InIDMap::imgFp> P2InIDMap::ImgFs =
{
    DEF_ENUM_BY_SENSOR(MAKE_IMG_FUNC_PTR),
};

// ID Mapping Function (ImageStream)

#define MAKE_IMG_STREAM_FUNC(index)                             \
static ID_STREAM _getImgStreamID_##index(ID_STREAM id)           \
{                                                                   \
    switch(id)                                                      \
    {                                                               \
        case IN_S_REPROCESS:      return IN_S_REPROCESS;            \
        case IN_S_REPROCESS2:     return IN_S_REPROCESS2;           \
        case IN_S_OPAQUE:         return IN_S_OPAQUE_##index;       \
        case IN_S_FULL:           return IN_S_FULL_##index;         \
        case IN_S_RESIZED:        return IN_S_RESIZED_##index;      \
        case IN_S_P1STT:          return IN_S_P1STT_##index;        \
        case IN_S_RSSO:           return IN_S_RSSO_##index;         \
        case IN_S_FULL_YUV:       return IN_S_FULL_YUV_##index;     \
        case IN_S_RESIZED_YUV1:   return IN_S_RESIZED_YUV1_##index; \
        case IN_S_RESIZED_YUV2:   return IN_S_RESIZED_YUV2_##index; \
        default:                  return NO_STREAM;                 \
    }                                                               \
}

#define MAKE_IMG_STREAM_FUNC_PTR(index)    \
    &_getImgStreamID_##index

DEF_FUNC_BY_SENSOR(MAKE_IMG_STREAM_FUNC)

const std::vector<P2InIDMap::imgStreamFp> P2InIDMap::ImgStreamFs =
{
    DEF_ENUM_BY_SENSOR(MAKE_IMG_STREAM_FUNC_PTR),
};

#define DEF_STREAM_IMG_MAP(index) \
        case IN_S_OPAQUE##_##index:           return {IN_OPAQUE##_##index};                     \
        case IN_S_FULL##_##index:             return {IN_FULL##_##index};                       \
        case IN_S_RESIZED##_##index:          return {IN_RESIZED##_##index};                    \
        case IN_S_P1STT##_##index:            return {IN_LCSO##_##index/*, IN_LCSHO##_##index*/};   \
        case IN_S_RSSO##_##index:             return {IN_RSSO##_##index};                       \
        case IN_S_FULL_YUV##_##index:         return {IN_FULL_YUV##_##index};                   \
        case IN_S_RESIZED_YUV1##_##index:     return {IN_RESIZED_YUV1##_##index};               \
        case IN_S_RESIZED_YUV2##_##index:     return {IN_RESIZED_YUV2##_##index};

std::list<ID_IMG> getImgsFromStream(ID_STREAM id)
{
    switch(id)
    {
        case IN_S_REPROCESS:        return {IN_REPROCESS};
        case IN_S_REPROCESS2:       return {IN_REPROCESS2};
        case OUT_S_FD:              return {OUT_FD};
        case OUT_S_THN_YUV:         return {OUT_THN_YUV};
        case OUT_S_JPEG_YUV:        return {OUT_JPEG_YUV};
        case OUT_S_YUV:             return {OUT_YUV};
        case OUT_S_POSTVIEW:        return {OUT_POSTVIEW};
        case OUT_S_CLEAN:           return {OUT_CLEAN};
        case OUT_S_DEPTH:           return {OUT_DEPTH};
        case OUT_S_BOKEH:           return {OUT_BOKEH};
        case OUT_S_RAW:             return {OUT_RAW};
        DEF_SWITCH_BY_SENSOR(DEF_STREAM_IMG_MAP);
        default:                    return {};
    }
}

IO_DIR getDirFromStream(ID_STREAM id)
{
    std::list<ID_IMG> list = getImgsFromStream(id);
    IO_DIR dir = IO_DIR_UNKNOWN;
    if( !list.empty() )
    {
        dir = P2Img::getImgInfo(list.front()).dir;
    }
    return dir;
}



#include "P2_DebugControl.h"
#define P2_CLASS_TAG   P2InIDMap
#define P2_TRACE   TRACE_P2_INIDMAP
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

P2InIDMap::P2InIDMap(const MUINT32 mainSensorID, const std::vector<MUINT32> &subSensorIDList)
    : mMainSensorID(mainSensorID)
{
    size_t size = subSensorIDList.size();

    mSensor2MetaFp[mainSensorID] = MetaFs[0];
    mSensor2ImgFp[mainSensorID] = ImgFs[0];
    mSensor2ImgStreamFp[mainSensorID] = ImgStreamFs[0];
    size_t allowSize = size + 1;
    if( allowSize > MetaFs.size() || allowSize > ImgFs.size() || allowSize > ImgStreamFs.size())
    {
        MY_LOGF("ERROR sensor ID list(%zu) > metaFs(%zu) or imgFs(%zu) or imgStreamFs(%zu)", size, MetaFs.size(), ImgFs.size(), ImgStreamFs.size());
    }
    else
    {
        for( MUINT32 index = 0 ; index < size ; index++ )
        {
            MUINT32 sId = subSensorIDList[index];
            if(sId == mainSensorID)
            {
                MY_LOGF("ERROR main Sensor ID (%u) in subID List !!", mainSensorID);
            }
            MUINT32 mapInd = index + 1;
            mSensor2MetaFp[sId] = MetaFs[mapInd];
            mSensor2ImgFp[sId] = ImgFs[mapInd];
            mSensor2ImgStreamFp[sId] = ImgStreamFs[mapInd];
        }
    }

}

ID_META P2InIDMap::getMetaID(const MUINT32 sensorID, const ID_META inID) const
{
    auto it = mSensor2MetaFp.find(sensorID);
    return (it != mSensor2MetaFp.end()) ? it->second(inID) : NO_META;
}
ID_IMG P2InIDMap::getImgID(const MUINT32 sensorID, const ID_IMG inID) const
{
    auto it = mSensor2ImgFp.find(sensorID);
    return (it != mSensor2ImgFp.end()) ? it->second(inID) : NO_IMG;
}

ID_STREAM P2InIDMap::getImgStreamID(const MUINT32 sensorID, const ID_STREAM inID) const
{
    auto it = mSensor2ImgStreamFp.find(sensorID);
    return (it != mSensor2ImgStreamFp.end()) ? it->second(inID) : NO_STREAM;
}

P2InIDMap::metaFp P2InIDMap::getMetaMap(const MUINT32 sensorID) const
{
    auto it = mSensor2MetaFp.find(sensorID);
    return (it != mSensor2MetaFp.end()) ? it->second : NULL;
}

P2InIDMap::imgFp P2InIDMap::getImgMap(const MUINT32 sensorID) const
{
    auto it = mSensor2ImgFp.find(sensorID);
    return (it != mSensor2ImgFp.end()) ? it->second : NULL;
}

P2InIDMap::imgStreamFp P2InIDMap::getImgStreamMap(const MUINT32 sensorID) const
{
    auto it = mSensor2ImgStreamFp.find(sensorID);
    return (it != mSensor2ImgStreamFp.end()) ? it->second : NULL;
}

const META_INFO& P2Meta::getMetaInfo(ID_META id)
{
    static const META_INFO sInvalid =
        {ID_META_INVALID, ID_META_INVALID, IO_DIR_UNKNOWN, "invalid", IO_FLAG_INVALID};

    auto it = P2Meta::InfoMap.find(id);
    return (it != P2Meta::InfoMap.end()) ? it->second : sInvalid;
}

const IMG_INFO& P2Img::getImgInfo(ID_IMG id)
{
    static const IMG_INFO sInvalid =
        {ID_IMG_INVALID, ID_IMG_INVALID, IO_DIR_UNKNOWN, "invalid", IO_FLAG_INVALID, ID_STREAM_INVALID};

    auto it = P2Img::InfoMap.find(id);
    return (it != P2Img::InfoMap.end()) ? it->second : sInvalid;
}

const IMG_INFO& P2Img::getImgInfoFromStream(ID_STREAM id)
{
    static const IMG_INFO sInvalid =
        {ID_IMG_INVALID, ID_IMG_INVALID, IO_DIR_UNKNOWN, "invalid", IO_FLAG_INVALID, ID_STREAM_INVALID};
    const std::list<ID_IMG> ids = getImgsFromStream(id);
    return (!ids.empty()) ? P2Img::getImgInfo(ids.front()) : sInvalid;
}

const char* P2Meta::getName(ID_META id)
{
    const char* name = "unknown";
    auto it = P2Meta::InfoMap.find(id);
    if( it != P2Meta::InfoMap.end() )
    {
        name = it->second.name.c_str();
    }
    return name;
}

const char* P2Img::getName(ID_IMG id)
{
    const char* name = "unknown";
    auto it = P2Img::InfoMap.find(id);
    if( it != P2Img::InfoMap.end() )
    {
        name = it->second.name.c_str();
    }
    return name;
}

P2MetaSet::P2MetaSet()
    : mHasOutput(MFALSE)
{
}

P2Meta::P2Meta(const ILog &log, const P2Pack &p2Pack, ID_META id)
    : mLog(log)
    , mP2Pack(p2Pack)
    , mMetaID(id)
{
}

ID_META P2Meta::getID() const
{
    return mMetaID;
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG   P2Img
#define P2_TRACE   TRACE_P2_IMG
#include "P2_LogHeader.h"


P2Img::P2Img(const ILog &log, const P2Pack &p2Pack, ID_IMG id, MUINT32 debugIndex)
    : mLog(log)
    , mP2Pack(p2Pack)
    , mImgID(id)
    , mDebugIndex(debugIndex)
{
    IMG_INFO info = getImgInfo(mImgID);
    mImgMirrorID = info.mirror;
}

ID_IMG P2Img::getID() const
{
    return mImgID;
}

ID_IMG P2Img::getMirrorID() const
{
    return mImgMirrorID;
}

const char* P2Img::getHumanName() const
{
    const char* name = getName(mImgID);
    if( mImgID == OUT_FD )
    {
        name = "fd";
    }
    else if( mImgID == OUT_YUV )
    {
        name = isDisplay() ?  "display" :
               isRecord() ?   "record" :
                              "previewCB";
    }
    return name;
}

IMG_TYPE P2Img::getImgType() const
{
    return IMG_TYPE_EXTRA;
}

MVOID P2Img::setImgInfo(const MSize &size, const std::vector<MINT64> &timestamp)
{
    std::vector<IImageBuffer*> imgs = this->getIImageBufferPtrs();
    MUINT32 iSize = imgs.size(), tSize = timestamp.size();
    MBOOL setSize = (size.w && size.h);
    MINT64 ts = 0;
    for( MUINT32 i = 0; i < iSize; ++i )
    {
        if( i < tSize )
        {
            ts = timestamp[i];
        }
        if( imgs[i] )
        {
            imgs[i]->setTimestamp(ts);
            if( setSize )
            {
                imgs[i]->setExtParam(size);
            }
        }
    }
}

MVOID P2Img::setImgSize(const MSize &size)
{
    for( IImageBuffer *img : this->getIImageBufferPtrs() )
    {
        if( img )
        {
            img->setExtParam(size);
        }
    }
}

MVOID P2Img::setTimestamp(const std::vector<MINT64> &timestamp)
{
    std::vector<IImageBuffer*> imgs = this->getIImageBufferPtrs();
    MUINT32 iSize = imgs.size(), tSize = timestamp.size();
    MINT64 ts = 0;
    for( MUINT32 i = 0; i < iSize; ++i )
    {
        if( i < tSize )
        {
            ts = timestamp[i];
        }
        if( imgs[i] )
        {
            imgs[i]->setTimestamp(ts);
        }
    }
}

MSize P2Img::getImgSize() const
{
    MSize size(0, 0);
    if( isValid() )
    {
        IImageBuffer *img = getIImageBufferPtr();
        if( img )
        {
            size = img->getImgSize();
        }
    }
    return size;
}

MSize P2Img::getTransformSize() const
{
    MSize size(0, 0);
    if( isValid() )
    {
        IImageBuffer *img = getIImageBufferPtr();
        if( img )
        {
            size = img->getImgSize();
            if( getTransform() & eTransform_ROT_90 )
            {
                size = MSize(size.h, size.w);
            }
        }
    }
    return size;
}

const char * P2Img::Fmt2Name(MINT fmt)
{
    switch(fmt)
    {
    case NSCam::eImgFmt_RGBA8888:          return "rgba";
    case NSCam::eImgFmt_RGB888:            return "rgb";
    case NSCam::eImgFmt_RGB565:            return "rgb565";
    case NSCam::eImgFmt_STA_BYTE:          return "byte";
    case NSCam::eImgFmt_YVYU:              return "yvyu";
    case NSCam::eImgFmt_UYVY:              return "uyvy";
    case NSCam::eImgFmt_VYUY:              return "vyuy";
    case NSCam::eImgFmt_YUY2:              return "yuy2";
    case NSCam::eImgFmt_YV12:              return "yv12";
    case NSCam::eImgFmt_YV16:              return "yv16";
    case NSCam::eImgFmt_NV16:              return "nv16";
    case NSCam::eImgFmt_NV61:              return "nv61";
    case NSCam::eImgFmt_NV12:              return "nv12";
    case NSCam::eImgFmt_NV21:              return "nv21";
    case NSCam::eImgFmt_I420:              return "i420";
    case NSCam::eImgFmt_I422:              return "i422";
    case NSCam::eImgFmt_Y800:              return "y800";
    case NSCam::eImgFmt_BAYER8:            return "bayer8";
    case NSCam::eImgFmt_BAYER10:           return "bayer10";
    case NSCam::eImgFmt_BAYER12:           return "bayer12";
    case NSCam::eImgFmt_BAYER14:           return "bayer14";
    case NSCam::eImgFmt_FG_BAYER8:         return "fg_bayer8";
    case NSCam::eImgFmt_FG_BAYER10:        return "fg_bayer10";
    case NSCam::eImgFmt_FG_BAYER12:        return "fg_bayer12";
    case NSCam::eImgFmt_FG_BAYER14:        return "fg_bayer14";
    default:                               return "unknown";
    };

}

MVOID P2Img::dumpBuffer() const
{
    MUINT32 index = 0;
    for( IImageBuffer *buffer : this->getIImageBufferPtrs() )
    {
        dumpBuffer(buffer, index++);
    }
}

MVOID P2Img::dumpBuffer(IImageBuffer *buffer, unsigned index) const
{
    if( buffer )
    {
        MUINT32 bufWidth, bufHeight, imgWidth, imgHeight;
        MUINT32 strideB, sizeB, pbpp, ibpp;
        MINT format = buffer->getImgFormat();
        strideB = buffer->getBufStridesInBytes(0);
        sizeB = buffer->getBufSizeInBytes(0);
        pbpp = buffer->getPlaneBitsPerPixel(0);
        ibpp = buffer->getImgBitsPerPixel();
        pbpp = pbpp ? pbpp : 8;
        ibpp = ibpp ? ibpp : 8;
        bufWidth = strideB * 8 / pbpp;
        bufWidth = bufWidth ? bufWidth : 1;
        bufHeight = sizeB / bufWidth;
        if( buffer->getPlaneCount() == 1 )
        {
            bufHeight = bufHeight * 8 / ibpp;
        }
        imgWidth = buffer->getImgSize().w;
        imgHeight = buffer->getImgSize().h;

        char path[256];
        snprintf(path, sizeof(path), DUMP_PATH "/%04d_%04dm%02d_%02d_%s_%dx%d_%dx%d_%d_%d.%s",
            getMagic3A(), mLog.getLogFrameID(), index, mDebugIndex, getHumanName(), imgWidth, imgHeight, bufWidth, bufHeight, strideB, buffer->getColorArrangement(), Fmt2Name(format));
        buffer->saveToFile(path);
    }
}

MVOID P2Img::dumpNddBuffer() const
{
    IImageBuffer *buffer = this->getIImageBufferPtr();
    if( buffer )
    {
        char filename[256] = {0};

        IMG_INFO info = getImgInfo(mImgID);

        FILE_DUMP_NAMING_HINT hint;
        hint = mP2Pack.getSensorData().mNDDHint;
        extract(&hint, buffer);

        switch(info.mirror)
        {
            case IN_FULL:
                genFileName_RAW(filename, sizeof(filename), &hint, RAW_PORT_IMGO);
                break;

            case IN_RESIZED:
                genFileName_RAW(filename, sizeof(filename), &hint, RAW_PORT_RRZO);
                break;

           case IN_LCSO:
               genFileName_LCSO(filename, sizeof(filename), &hint);
               break;

           case OUT_YUV:
               {
                   if( this->isDisplay())
                   {
                       genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_WDMAO);
                   }
                   else if( this->isRecord())
                   {
                       genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_WROTO);
                   }
                   else
                   {
                       genFileName_YUV(filename, sizeof(filename), &hint,YUV_PORT_UNDEFINED);
                   }
               }
               break;
           default:
               break;

        };

        if( filename[0] )
        {
            MY_S_LOGD(mP2Pack.mLog, "dump to: %s", filename);
            buffer->saveToFile(filename);
        }
    }
}

MINT32 P2Img::getMagic3A() const
{
    return mP2Pack.getSensorData().mMagic3A;
}

const P2SensorData& P2Img::getSensorData() const
{
    return mP2Pack.getSensorData();
}

MBOOL P2ImgPlugin::onPlugin(const sp<P2Img> &img)
{
    return onPlugin(img.get());
}

MBOOL isValid(const P2Meta *meta)
{
    return (meta != NULL) && meta->isValid();
}

MBOOL isValid(const P2Img *img)
{
    return (img != NULL) && img->isValid();
}

MBOOL isValid(const sp<P2Meta> &meta)
{
    return (meta != NULL) && meta->isValid();
}

MBOOL isValid(const sp<P2Img> &img)
{
    return (img != NULL) && img->isValid();
}

IMetadata* toIMetadataPtr(const sp<P2Meta> &meta)
{
    return meta != NULL ? meta->getIMetadataPtr() : NULL;
}

IImageBuffer* toIImageBufferPtr(const sp<P2Img> &img)
{
    return img != NULL ? img->getIImageBufferPtr() : NULL;
}

std::vector<IImageBuffer*> toIImageBufferPtrs(const sp<P2Img> &img)
{
    return img != NULL ? img->getIImageBufferPtrs() : std::vector<IImageBuffer*>();
}

} // namespace P2
