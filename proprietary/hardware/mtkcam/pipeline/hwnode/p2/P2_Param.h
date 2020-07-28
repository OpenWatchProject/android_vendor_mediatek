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

#define INVALID_SENSOR_ID ((MUINT32)-1)

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

enum ID_META
{
    IN_APP,
    IN_P1_APP,
    IN_P1_HAL,
    OUT_APP,
    OUT_HAL,
    ID_META_MAX,
    ID_META_INVALID,
};

enum ID_IMG
{
    IN_OPAQUE,
    IN_FULL,
    IN_RESIZED,
    IN_REPROCESS,
    IN_LCSO,
    IN_RSSO,
    OUT_FD,
    OUT_YUV,
    ID_IMG_MAX,
    ID_IMG_INVALID,
};

struct META_INFO
{
    ID_META id;
    IO_DIR dir;
    const char* name;
    MUINT32 flag;
};

struct IMG_INFO
{
    ID_IMG id;
    IO_DIR dir;
    const char* name;
    MUINT32 flag;
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

class P2Meta : virtual public android::RefBase, virtual public ILogger
{
public:
    static const std::unordered_map<ID_META, META_INFO> InfoMap;
    static const META_INFO& getMetaInfo(ID_META id);
    static const char* getName(ID_META id);

public:
    P2Meta(ID_META id);
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

    template<typename T> MBOOL tryGet(MUINT32 tag, T &val);
    template<typename T> MBOOL trySet(MUINT32 tag, const T &val);

private:
    ID_META mMetaID;
};

class P2ImgPlugin;
class P2Img : virtual public android::RefBase, virtual public ILogger
{
public:
    static const std::unordered_map<ID_IMG, IMG_INFO> InfoMap;
    static const IMG_INFO& getImgInfo(ID_IMG id);
    static const char* getName(ID_IMG id);

public:
    P2Img(ID_IMG id, MUINT32 debugIndex);
    virtual ~P2Img() {}
    ID_IMG getID() const;

public:
    virtual StreamId_T getStreamID() const = 0;
    virtual MBOOL isValid() const = 0;
    virtual IO_DIR getDir() const = 0;
    virtual MVOID registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin) = 0;
    virtual MVOID updateResult(MBOOL result) = 0;
    virtual IImageBuffer* getIImageBufferPtr() const = 0;
    virtual MUINT32 getTransform() const = 0;
    virtual MUINT32 getUsage() const = 0;
    virtual MBOOL isDisplay() const = 0;
    virtual MBOOL isRecord() const = 0;
    virtual MBOOL isCapture() const = 0;
    MSize getImgSize() const;
    MSize getTransformSize() const;
    MVOID dump(const char *dirPath) const;

private:
    ID_IMG mImgID;
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

template<typename T>
MBOOL P2Meta::tryGet(MUINT32 tag, T &val)
{
    MBOOL ret = MFALSE;
    ret = ::P2::tryGet<T>(this->getIMetadataPtr(), tag, val);
    return ret;
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

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_PARAM_H_
