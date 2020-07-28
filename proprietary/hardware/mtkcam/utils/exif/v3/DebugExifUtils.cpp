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

#define LOG_TAG "MtkCam/DebugExifUtils"

#include <vector>
//
#include <mtkcam/custom/ExifFactory.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
//
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
static auto
getDebugExif()
{
    static auto const inst = MAKE_DebugExif();
    return inst;
}

static auto
getBufInfo_cam()
{
    static auto const inst = ((NULL!=getDebugExif()) ? getDebugExif()->getBufInfo(DEBUG_EXIF_KEYID_CAM) : NULL);
    return inst;
}


// ---------------------------------------------------------------------------

template <typename T>
static inline MVOID updateEntry(
        IMetadata* metadata, const MUINT32 tag, const T& val)
{
    if (metadata == NULL)
    {
        CAM_LOGW("pMetadata is NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    metadata->update(tag, entry);
}

template <class T>
static MBOOL tryGetMetaData(IMetadata *pMetadata, MUINT32 const tag, T &rVal)
{
    if (pMetadata == NULL) {
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }

    return MFALSE;
}

static bool setDebugExifMF(
        const MUINT32 tagKey, const MUINT32 tagData,
        const std::map<MUINT32, MUINT32>& debugInfoList,
        IMetadata* exifMetadata)
{
    auto it = getBufInfo_cam()->body_layout.find(DEBUG_EXIF_MID_CAM_MF);
    if ( it == getBufInfo_cam()->body_layout.end() ) {
        CAM_LOGE("cannot find the layout: DEBUG_EXIF_MID_CAM_MF");
        return false;
    }

    auto const& info = it->second;

    // allocate memory of debug information
    IMetadata::Memory debugInfoSet;
    debugInfoSet.resize(info.size);
    ::memset(debugInfoSet.editArray(), 0, debugInfoSet.size());

    // add debug information
    {
        auto const tagId_MF_TAG_VERSION = getDebugExif()->getTagId_MF_TAG_VERSION();
        auto pTag = reinterpret_cast<debug_exif_field*>(debugInfoSet.editArray());
        pTag[tagId_MF_TAG_VERSION].u4FieldID    = (0x1000000 | tagId_MF_TAG_VERSION);
        pTag[tagId_MF_TAG_VERSION].u4FieldValue = info.version;

        for (const auto& item : debugInfoList)
        {
            const MUINT32 index = item.first;
            pTag[index].u4FieldID    = (0x1000000 | index);
            pTag[index].u4FieldValue = item.second;
        }
    }

    // update debug exif metadata
    updateEntry<MINT32>(exifMetadata, tagKey, DEBUG_EXIF_MID_CAM_MF);
    updateEntry<IMetadata::Memory>(exifMetadata, tagData, debugInfoSet);

    return true;
}
static bool setDebugExifRESERVE3(
        const MUINT32 tagKey, const MUINT32 tagData,
        const MUINT32 size,
        const void* debugInfoList,
        IMetadata* exifMetadata)
{
    auto it = getBufInfo_cam()->body_layout.find(DEBUG_EXIF_MID_CAM_RESERVE3); //MDP
    if ( it == getBufInfo_cam()->body_layout.end() ) {
        CAM_LOGE("cannot find the layout: DEBUG_EXIF_MID_CAM_RESERVE3");
        return false;
    }

    auto const& info = it->second;

    // allocate memory of debug information
    IMetadata::Memory debugInfoValue;
    debugInfoValue.resize(info.size);
    ::memset(debugInfoValue.editArray(), 0, debugInfoValue.size());

    // add debug information
    {
        auto pTag = reinterpret_cast<MUINT32*>(debugInfoValue.editArray());
        if (info.size < size) {
            CAM_LOGE("dst size (%d) is smaller than src size (%d)", info.size, size);
            return false;
        }
        ::memcpy(pTag, debugInfoList, size);
    }

    // update debug exif metadata
    updateEntry<MINT32>(exifMetadata, tagKey, DEBUG_EXIF_MID_CAM_RESERVE3);
    updateEntry<IMetadata::Memory>(exifMetadata, tagData, debugInfoValue);

    return true;
}

static bool setDebugExifCAM(
        const MUINT32 tagKey, const MUINT32 tagData,
        const std::map<MUINT32, MUINT32>& debugInfoList,
        IMetadata* exifMetadata)
{
    auto it = getBufInfo_cam()->body_layout.find(DEBUG_EXIF_MID_CAM_CMN);
    if ( it == getBufInfo_cam()->body_layout.end() ) {
        CAM_LOGE("cannot find the layout: DEBUG_EXIF_MID_CAM_CMN");
        return false;
    }

    IMetadata::Memory debugInfoSet;
    if(!tryGetMetaData(exifMetadata, tagData, debugInfoSet))
    {
        //CAM_LOGD("[debug mode] debugInfoList size: %d", debugInfoList.size());
        // allocate memory of debug information
        auto const& info = it->second;
        debugInfoSet.resize(info.size);
        ::memset(debugInfoSet.editArray(), 0, debugInfoSet.size());
    }
    // allocate memory of debug information
    auto pTag = reinterpret_cast<debug_exif_field*>(debugInfoSet.editArray());
    for (const auto& item : debugInfoList)
    {
       const MUINT32 index = item.first;
       //CAM_LOGD("[debug mode] item: %d value: %d", item.first, item.second);
       pTag[index].u4FieldID    = (0x1000000 | index);
       pTag[index].u4FieldValue = item.second;
    }
    // update debug exif metadata
    updateEntry<MINT32>(exifMetadata, tagKey, DEBUG_EXIF_MID_CAM_CMN);
    updateEntry<IMetadata::Memory>(exifMetadata, tagData, debugInfoSet);

    return true;
}

// ---------------------------------------------------------------------------

IMetadata* DebugExifUtils::setDebugExif(
        const DebugExifType type,
        const MUINT32 tagKey, const MUINT32 tagData,
        const std::map<MUINT32, MUINT32>& debugInfoList,
        IMetadata* exifMetadata)
{
    if (exifMetadata == NULL)
    {
        CAM_LOGW("invalid metadata(%p)", exifMetadata);
        return nullptr;
    }

    if ( ! getDebugExif() ) {
        CAM_LOGE("bad getDebugExif()");
        return nullptr;
    }

    if ( ! getBufInfo_cam() ) {
        CAM_LOGE("bad getBufInfo_cam()");
        return nullptr;
    }

    bool ret = [=, &type, &debugInfoList](IMetadata* metadata) -> bool {
        switch (type)
        {
            case DebugExifType::DEBUG_EXIF_MF:
                return setDebugExifMF(tagKey, tagData, debugInfoList, metadata);
            case DebugExifType::DEBUG_EXIF_CAM:
                return setDebugExifCAM(tagKey, tagData, debugInfoList, metadata);
            default:
                CAM_LOGW("invalid debug exif type, do nothing");
                return false;
        }
    } (exifMetadata);

    return (ret == true) ? exifMetadata : nullptr;

    return nullptr;
}
// ---------------------------------------------------------------------------

IMetadata* DebugExifUtils::setDebugExif(
        const DebugExifType type,
        const MUINT32 tagKey, const MUINT32 tagData,
        const MUINT32 size,
        const void* debugInfoList,
        IMetadata* exifMetadata)
{
    if (exifMetadata == NULL)
    {
        CAM_LOGW("invalid metadata(%p)", exifMetadata);
        return nullptr;
    }

    if ( ! getDebugExif() ) {
        CAM_LOGE("bad getDebugExif()");
        return nullptr;
    }

    if ( ! getBufInfo_cam() ) {
        CAM_LOGE("bad getBufInfo_cam()");
        return nullptr;
    }

    bool ret = [=, &type, &debugInfoList](IMetadata* metadata) -> bool {
        switch (type)
        {
            case DebugExifType::DEBUG_EXIF_RESERVE3:
                return setDebugExifRESERVE3(tagKey, tagData, size, debugInfoList, metadata);
            default:
                CAM_LOGW("invalid debug exif type, do nothing");
                return false;
        }
    } (exifMetadata);

    return (ret == true) ? exifMetadata : nullptr;

    return nullptr;
}
