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

#ifndef _MTKCAM_HWNODE_P2_PARAM_H_
#define _MTKCAM_HWNODE_P2_PARAM_H_

#include "P2_Header.h"
#include "P2_Common.h"
#include "P2_Info.h"

namespace P2
{

enum IO_DIR
{
    IO_DIR_UNKNOWN  = 0,
    IO_DIR_IN       = (1 << 0),
    IO_DIR_OUT      = (1 << 1),
};

enum IO_STATUS { IO_STATUS_INVALID, IO_STATUS_READY, IO_STATUS_OK, IO_STATUS_ERROR };
enum IO_FLAG
{
    IO_FLAG_DEFAULT = 0,
    IO_FLAG_INVALID = (1 << 0),
    IO_FLAG_COPY    = (1 << 1),
};

// Current support 4 sensor
#define DEF_ENUM_BY_SENSOR(function)   \
    function(1),             \
    function(2),             \
    function(3),             \
    function(4)

#define DEF_FUNC_BY_SENSOR(function)   \
    function(1)             \
    function(2)             \
    function(3)             \
    function(4)

#define DEF_SWITCH_BY_SENSOR(function)   \
    function(1);             \
    function(2);             \
    function(3);             \
    function(4)


#define ENUM_ID_META(idx)   \
    IN_APP_PHY_##idx,        \
    IN_P1_APP_##idx,         \
    IN_P1_HAL_##idx,         \
    OUT_APP_PHY_##idx

/* Becareful order must the same as ID_IMG main enum define*/
#define ENUM_ID_IMG(idx)    \
    IN_OPAQUE_##idx,         \
    IN_FULL_##idx,           \
    IN_RESIZED_##idx,        \
    IN_LCSO_##idx,           \
    IN_LCSHO_##idx,          \
    IN_RSSO_##idx,           \
    IN_FULL_YUV_##idx,       \
    IN_RESIZED_YUV1_##idx,   \
    IN_RESIZED_YUV2_##idx

#define ENUM_ID_STREAM(idx)    \
    IN_S_OPAQUE_##idx,         \
    IN_S_FULL_##idx,           \
    IN_S_RESIZED_##idx,        \
    IN_S_P1STT_##idx,          \
    IN_S_RSSO_##idx,           \
    IN_S_FULL_YUV_##idx,       \
    IN_S_RESIZED_YUV1_##idx,   \
    IN_S_RESIZED_YUV2_##idx

enum ID_META
{
    NO_META,
    IN_APP,
// === Main ===
    IN_MAIN_META_START,
    IN_APP_PHY,
    IN_P1_APP,
    IN_P1_HAL,
    OUT_APP_PHY,
// === Subs ====
    IN_SUB_1_META_START = IN_MAIN_META_START,
    DEF_ENUM_BY_SENSOR(ENUM_ID_META),

// == Subs end ====
    OUT_APP,
    OUT_HAL,
    ID_META_MAX,
    ID_META_INVALID,
};

enum ID_IMG
{
    NO_IMG,
    IN_REPROCESS,
    IN_REPROCESS2,
    IN_MAIN_IMG_START,
// === Main input ===
    IN_OPAQUE,
    IN_FULL,
    IN_RESIZED,
    IN_LCSO,
    IN_LCSHO,
    IN_RSSO,
    IN_FULL_YUV,
    IN_RESIZED_YUV1,
    IN_RESIZED_YUV2,
// === Sub input ===
    IN_SUB_1_IMG_START = IN_MAIN_IMG_START,
    DEF_ENUM_BY_SENSOR(ENUM_ID_IMG),
// === Sub input end ===
    OUT_FD,
    OUT_YUV,
    OUT_JPEG_YUV,
    OUT_THN_YUV,
    OUT_POSTVIEW,
    OUT_CLEAN,
    OUT_DEPTH,
    OUT_BOKEH,
    OUT_RAW,
    ID_IMG_MAX,
    ID_IMG_INVALID,
};

enum ID_STREAM
{
    NO_STREAM,
    IN_S_REPROCESS,
    IN_S_REPROCESS2,
// === Main input ===
    IN_S_MAIN_START,
    IN_S_OPAQUE,
    IN_S_FULL,
    IN_S_RESIZED,
    IN_S_P1STT,
    IN_S_RSSO,
    IN_S_FULL_YUV,
    IN_S_RESIZED_YUV1,
    IN_S_RESIZED_YUV2,
// === Sub input ===
    IN_S_SUB_1_START = IN_S_MAIN_START,
    DEF_ENUM_BY_SENSOR(ENUM_ID_STREAM),
// === Sub input end ===
    OUT_S_FD,
    OUT_S_YUV,
    OUT_S_JPEG_YUV,
    OUT_S_THN_YUV,
    OUT_S_POSTVIEW,
    OUT_S_CLEAN,
    OUT_S_DEPTH,
    OUT_S_BOKEH,
    OUT_S_RAW,
    ID_STREAM_MAX,
    ID_STREAM_INVALID,
};

static inline const char* ImgType2Name(const IMG_TYPE type)
{
    switch(type)
    {
        case IMG_TYPE_DISPLAY:  return "disp";
        case IMG_TYPE_RECORD:   return "rec";
        case IMG_TYPE_PHYSICAL: return "physical";
        case IMG_TYPE_FD:       return "fd";
        default :               return "extra";
    }
}

struct META_INFO
{
    ID_META id = NO_META;
    ID_META mirror = NO_META;
    IO_DIR dir = IO_DIR_UNKNOWN;
    std::string name;
    MUINT32 flag = IO_FLAG_DEFAULT;

    META_INFO();
    META_INFO(ID_META sID, ID_META sMirror, IO_DIR sDir, const std::string &sName, MUINT32 sFlag);
};

struct IMG_INFO
{
    ID_IMG id = NO_IMG;
    ID_IMG mirror = NO_IMG;
    IO_DIR dir = IO_DIR_UNKNOWN;
    std::string name;
    MUINT32 flag = IO_FLAG_DEFAULT;
    ID_STREAM sID = NO_STREAM;

    IMG_INFO();
    IMG_INFO(ID_IMG sID, ID_IMG sMirror, IO_DIR sDir, const std::string &sName, MUINT32 sFlag, ID_STREAM sId);
};

class P2InIDMap : virtual public android::RefBase
{
public:
    typedef ID_META (*metaFp)(ID_META);
    typedef ID_IMG (*imgFp)(ID_IMG);
    typedef ID_STREAM (*imgStreamFp)(ID_STREAM);

    static const std::vector<ID_IMG> InputImgIDs;
    static const std::vector<metaFp> MetaFs;
    static const std::vector<imgFp> ImgFs;
    static const std::vector<imgStreamFp> ImgStreamFs;

public:
    P2InIDMap(const MUINT32 mainSensorID, const std::vector<MUINT32> &subSensorIDList);
    virtual ~P2InIDMap() {}

    ID_META getMetaID(const MUINT32 sensorID, const ID_META inID) const;
    ID_IMG getImgID(const MUINT32 sensorID, const ID_IMG inID) const;
    ID_STREAM getImgStreamID(const MUINT32 sensorID, const ID_STREAM inID) const;
    metaFp getMetaMap(const MUINT32 sensorID) const;
    imgFp getImgMap(const MUINT32 sensorID) const;
    imgStreamFp getImgStreamMap(const MUINT32 sensorID) const;

public:
    const MUINT32 mMainSensorID;

private:
    std::unordered_map<MUINT32, metaFp> mSensor2MetaFp;
    std::unordered_map<MUINT32, imgFp> mSensor2ImgFp;
    std::unordered_map<MUINT32, imgStreamFp> mSensor2ImgStreamFp;

};


class P2MetaSet
{
public:
    P2MetaSet();
public:
    MBOOL mHasOutput;
    IMetadata mInApp;
    IMetadata mInHal;
    IMetadata mOutApp;
    IMetadata mOutHal;
};

class P2Meta : virtual public android::RefBase
{
public:
    static const std::unordered_map<ID_META, META_INFO> InfoMap;
    static const META_INFO& getMetaInfo(ID_META id);
    static const char* getName(ID_META id);

public:
    P2Meta(const ILog &log, const P2Pack &p2Pack, ID_META id);
    virtual ~P2Meta() {}
    ID_META getID() const;

public:
    virtual StreamId_T getStreamID() const = 0;
    virtual MBOOL isValid() const = 0;
    virtual IO_DIR getDir() const = 0;
    virtual MVOID updateResult(MBOOL result) = 0;
    virtual IMetadata* getIMetadataPtr() const = 0;
    virtual IMetadata::IEntry getEntry(MUINT32 tag) const = 0;
    virtual MBOOL setEntry(MUINT32 tag, const IMetadata::IEntry &entry) = 0;
    virtual MVOID detach() = 0;

    template<typename T> MBOOL tryGet(MUINT32 tag, T &val);
    template<typename T> MBOOL trySet(MUINT32 tag, const T &val);
    template<typename T> MVOID getVector(MUINT32 tag, std::vector<T> &v);

protected:
    ILog mLog;

private:
    P2Pack mP2Pack;
    ID_META mMetaID;
};

class P2ImgPlugin;
class P2Img : virtual public android::RefBase
{
public:
    static const std::unordered_map<ID_IMG, IMG_INFO> InfoMap;
    static const IMG_INFO& getImgInfo(ID_IMG id);
    static const IMG_INFO& getImgInfoFromStream(ID_STREAM id);
    static const char* getName(ID_IMG id);
    static const char* Fmt2Name(MINT fmt);

public:
    P2Img(const ILog &log, const P2Pack &p2Pack, ID_IMG id, MUINT32 debugIndex);
    virtual ~P2Img() {}
    ID_IMG getID() const;
    ID_IMG getMirrorID() const;
    const char* getHumanName() const;

public:
    virtual MBOOL isValid() const = 0;
    virtual IO_DIR getDir() const = 0;
    virtual MVOID registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin) = 0;
    virtual MVOID updateResult(MBOOL result) = 0;
    virtual MVOID updateVRTimestamp(MUINT32 count, const MINT64 &cam2FwTs) = 0;
    virtual IImageBuffer* getIImageBufferPtr() const = 0;
    virtual std::vector<IImageBuffer*> getIImageBufferPtrs() const = 0;
    virtual MUINT32 getIImageBufferPtrsCount() const = 0;
    virtual MUINT32 getTransform() const = 0;
    virtual MUINT32 getUsage() const = 0;
    virtual MBOOL isDisplay() const = 0;
    virtual MBOOL isRecord() const = 0;
    virtual MBOOL isCapture() const = 0;
    virtual MBOOL isPhysicalStream() const = 0;
    virtual MBOOL isFD() const = 0;
    virtual MVOID detach() = 0;
    virtual IMG_TYPE getImgType() const;
    MVOID setImgInfo(const MSize &size, const std::vector<MINT64> &timestamp);
    MVOID setImgSize(const MSize &size);
    MVOID setTimestamp(const std::vector<MINT64> &timestamp);
    MSize getImgSize() const;
    MSize getTransformSize() const;
    MVOID dumpBuffer() const;
    MVOID dumpNddBuffer() const;
    MINT32 getMagic3A() const;
    const P2SensorData& getSensorData() const;

private:
    MVOID dumpBuffer(IImageBuffer *buffer, unsigned index) const;

protected:
    ILog mLog;

private:
    P2Pack mP2Pack;
    ID_IMG mImgID;
    ID_IMG mImgMirrorID;
    MUINT32 mDebugIndex;
};

class P2ImgPlugin : virtual public android::RefBase
{
public:
    P2ImgPlugin() {}
    virtual ~P2ImgPlugin() {}
    virtual MBOOL onPlugin(const sp<P2Img> &img);
    virtual MBOOL onPlugin(const P2Img *img) = 0;
};


MBOOL isValid(const P2Meta *holder);
MBOOL isValid(const P2Img *holder);
MBOOL isValid(const sp<P2Meta> &holder);
MBOOL isValid(const sp<P2Img> &holder);

IMetadata* toIMetadataPtr(const sp<P2Meta> &meta);
IImageBuffer* toIImageBufferPtr(const sp<P2Img> &img);
std::vector<IImageBuffer*> toIImageBufferPtrs(const sp<P2Img> &img);
std::list<ID_IMG> getImgsFromStream(ID_STREAM id);
IO_DIR getDirFromStream(ID_STREAM id);

template<typename T>
MBOOL P2Meta::tryGet(MUINT32 tag, T &val)
{
    MBOOL ret = MFALSE;
    ret = ::P2::tryGet<T>(this->getIMetadataPtr(), tag, val);
    return ret;
}

template<typename T>
MVOID P2Meta::getVector(MUINT32 tag, std::vector<T> &v)
{
    ::P2::getVector<T>(this->getIMetadataPtr(), tag, v);
}

template<typename T>
MBOOL P2Meta::trySet(MUINT32 tag, const T &val)
{
    MBOOL ret = MFALSE;
    ret = ::P2::trySet<T>(this->getIMetadataPtr(), tag, val);
    return ret;
}

template<typename T>
MBOOL tryGet(const sp<P2Meta> &meta, MUINT32 tag, T &val)
{
    MBOOL ret = MFALSE;
    if( meta != NULL )
    {
        ret = meta->tryGet<T>(tag, val);
    }
    return ret;
}

template<typename T>
T getMeta(const sp<P2Meta> &meta, MUINT32 tag, T val)
{
    if( meta != NULL )
    {
        meta->tryGet<T>(tag, val);
    }
    return val;
}

template<typename T>
std::vector<T> getMetaVector(const sp<P2Meta> &meta, MUINT32 tag)
{
    std::vector<T> vector;
    if( meta != NULL )
    {
        meta->getVector<T>(tag, vector);
    }
    return vector;
}

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_PARAM_H_
