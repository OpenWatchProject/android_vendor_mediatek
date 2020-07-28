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

namespace P2
{

#define MAKE_INFO(id, dir, name, flag) \
    { id, { id, dir, name, flag }}

const std::unordered_map<ID_META, META_INFO> P2Meta::InfoMap =
{
    MAKE_INFO(IN_APP,     IO_DIR_IN,  "inApp",    IO_FLAG_DEFAULT ),
    MAKE_INFO(IN_P1_APP,  IO_DIR_IN,  "inP1App",  IO_FLAG_COPY    ),
    MAKE_INFO(IN_P1_HAL,  IO_DIR_IN,  "inP1Hal",  IO_FLAG_COPY    ),
    MAKE_INFO(OUT_APP,    IO_DIR_OUT, "outApp",   IO_FLAG_DEFAULT ),
    MAKE_INFO(OUT_HAL,    IO_DIR_OUT, "outHal",   IO_FLAG_DEFAULT ),
};

const std::unordered_map<ID_IMG, IMG_INFO> P2Img::InfoMap =
{
    MAKE_INFO(IN_OPAQUE,    IO_DIR_IN,  "inOpaque",     IO_FLAG_DEFAULT),
    MAKE_INFO(IN_FULL,      IO_DIR_IN,  "inFull",       IO_FLAG_DEFAULT),
    MAKE_INFO(IN_RESIZED,   IO_DIR_IN,  "inResized",    IO_FLAG_DEFAULT),
    MAKE_INFO(IN_REPROCESS, IO_DIR_IN,  "inReprocess",  IO_FLAG_DEFAULT),
    MAKE_INFO(IN_LCSO,      IO_DIR_IN,  "inLCSO",       IO_FLAG_DEFAULT),
    MAKE_INFO(IN_RSSO,      IO_DIR_IN,  "inRSSO",       IO_FLAG_DEFAULT),
    MAKE_INFO(OUT_FD,       IO_DIR_OUT, "outFD",        IO_FLAG_DEFAULT),
    MAKE_INFO(OUT_YUV,      IO_DIR_OUT, "outYUV",       IO_FLAG_DEFAULT),
};

const META_INFO& P2Meta::getMetaInfo(ID_META id)
{
    static const META_INFO sInvalid =
        {ID_META_INVALID, IO_DIR_UNKNOWN, "invalid", IO_FLAG_INVALID};

    auto it = P2Meta::InfoMap.find(id);
    return (it != P2Meta::InfoMap.end()) ? it->second : sInvalid;
}

const IMG_INFO& P2Img::getImgInfo(ID_IMG id)
{
    static const IMG_INFO sInvalid =
        {ID_IMG_INVALID, IO_DIR_UNKNOWN, "invalid", IO_FLAG_INVALID};

    auto it = P2Img::InfoMap.find(id);
    return (it != P2Img::InfoMap.end()) ? it->second : sInvalid;
}

const char* P2Meta::getName(ID_META id)
{
    const char* name = "unknown";
    auto it = P2Meta::InfoMap.find(id);
    if( it != P2Meta::InfoMap.end() )
    {
        name = it->second.name;
    }
    return name;
}

const char* P2Img::getName(ID_IMG id)
{
    const char* name = "unknown";
    auto it = P2Img::InfoMap.find(id);
    if( it != P2Img::InfoMap.end() )
    {
        name = it->second.name;
    }
    return name;
}

P2MetaSet::P2MetaSet()
    : mHasOutput(MFALSE)
{
}

P2Meta::P2Meta(ID_META id)
    : mMetaID(id)
{
}

ID_META P2Meta::getID() const
{
    return mMetaID;
}

P2Img::P2Img(ID_IMG id, MUINT32 debugIndex)
    : mImgID(id)
    , mDebugIndex(debugIndex)
{
}

ID_IMG P2Img::getID() const
{
    return mImgID;
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

MVOID P2Img::dump(const char *dirPath) const
{
    IImageBuffer *buffer = this->getIImageBufferPtr();
    if( buffer )
    {
        MUINT32 stride, pbpp, ibpp, width, height, size;
        stride = buffer->getBufStridesInBytes(0);
        pbpp = buffer->getPlaneBitsPerPixel(0);
        ibpp = buffer->getImgBitsPerPixel();
        size = buffer->getBufSizeInBytes(0);
        pbpp = pbpp ? pbpp : 8;
        width = stride * 8 / pbpp;
        width = width ? width : 1;
        ibpp = ibpp ? ibpp : 8;
        height = size / width;
        if( buffer->getPlaneCount() == 1 )
        {
            height = height * 8 / ibpp;
        }

        char path[256];
        snprintf(path, sizeof(path), "/%s/%04d_%02d_%s_%dx%d.bin", dirPath, this->getLogFrameID(), mDebugIndex, getName(mImgID), width, height);
        buffer->saveToFile(path);
    }
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

} // namespace P2
