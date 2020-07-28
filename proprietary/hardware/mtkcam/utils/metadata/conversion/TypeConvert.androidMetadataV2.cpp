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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "Metadata2/ConvertA"
//
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/def/common.h>
#include "MetadataConverter.h"

#include <system/camera_metadata.h>
#include <MetadataConverter.h>
#include <mtkcam/utils/metadata/IMetadataTagSet.h>
#include <mtkcam/utils/metadata/mtk_metadata_types.h>

#ifdef MTKCAM_METADATA_V2

using namespace android;
using namespace NSCam;

/******************************************************************************
 *  IMetadata --> hidl_vec<uint8_t>
 ******************************************************************************/
MBOOL
MetadataConverter::
convertToHidl(const IMetadata& src, void* dst/*::android::hardware::hidl_vec<uint8_t>* */)
{
    auto pDst = static_cast<::android::hardware::hidl_vec<uint8_t>*>(dst);
    if ( pDst == nullptr ) {
        META_LOGE("bad destination:%p", pDst);
        return MFALSE;
    }

    camera_metadata* p_camera_metadata = nullptr;
    size_t size = 0;
    if ( ! convert(src, p_camera_metadata, &size) || ! p_camera_metadata || 0 == size ) {
        META_LOGE("fail: IMetadata -> camera_metadata(%p) size:%zu", p_camera_metadata, size);
        return MFALSE;
    }

    pDst->setToExternal((uint8_t *)p_camera_metadata, size);
    return MTRUE;
}


// IMetadata --> camera_metadata
MBOOL
MetadataConverter::
convert(const IMetadata &rMetadata, camera_metadata* &pDstMetadata, size_t* pDstSize) const
{
    META_LOGV("Convert from IMetadata to camera_metadata");

    //allocate
    size_t entryCount = rMetadata.count();

    // it should >> real data size due to flatten each item include headers(tag, size)
    size_t dataCount = rMetadata.flattenSize();
    META_LOGV("Allocating %zu entries, %zu extra bytes from HAL modules", entryCount, dataCount);

    pDstMetadata = ::allocate_camera_metadata(entryCount, dataCount);
    if ( !pDstMetadata )
    {
        META_LOGE(
            "Unable to allocate camera static info (%zu entries, %zu bytes extra data)\n",
            entryCount, dataCount
        );
        return  MFALSE;
    }

    MBOOL result = MTRUE;
    //start to convert
    for (uint32_t i = 0; i < entryCount; i++)
    {
        //(1) get tag by index, and skip it if it doesn't exist in android-metadata-tag list.
        auto &&entry = rMetadata.entryAt(i);
        auto mtk_tag = entry.tag();
        auto mtk_type = entry.type();
        auto count = entry.count();

        //(2) map to tag defined by android; return if undefined.
        #if (PLATFORM_SDK_VERSION >= 21)
        MUINT32 android_tag = getTagInfo()->getAndroidTag((MUINT32)mtk_tag);
        #else
        MUINT32 android_tag = 0;
        #endif
        if (android_tag == IMetadata::IEntry::BAD_TAG){
            META_LOGD("%s: Tag 0x%x not found in Android Metadata", __FUNCTION__, mtk_tag);
            continue;
        }

        //(3) get android tag's type
        int android_type = get_camera_metadata_tag_type(android_tag);

        META_LOGV("mtk (tag: 0x%x, name: %s, type: %d), android (tag: 0x%x, type: %d), data_count:%d",
                mtk_tag, getTagInfo()->getName(mtk_tag), mtk_type, android_tag, android_type, count);

        //handle empty data case. it should still pass this TAG to android layer
        if(count == 0) {
            mtk_type = android_type; // use android's type due to mtk_type not be assign when count = 0
        }

        //(4.1) types are equal ==> normal copy.
        if (android_type == mtk_type)
        {
            if (OK==update(pDstMetadata, android_tag, entry.data(), count))
                continue;
        }
        //(4.2) types are not equal
        else
        {
            if(mtk_type == TYPE_MRect && android_type == TYPE_INT32) {
                if (OK==update(pDstMetadata, android_tag, entry.data(), count*4))
                    continue;
            }
            else if(mtk_type == TYPE_MSize && android_type == TYPE_INT32) {
                if (OK==update(pDstMetadata, android_tag, entry.data(), count*2))
                    continue;
            }
            else if(mtk_type == TYPE_Memory &&  android_type == TYPE_BYTE) {
                auto _memory = entry.itemAt(0, Type2Type<IMetadata::Memory>());
                if (OK==update(pDstMetadata, android_tag, _memory.array(), _memory.size()))
                    continue;
            }
        }
        // this entry didn't be handled
        result = MFALSE;
        META_LOGE("Entry not be handled! mtk (tag: 0x%x, name: %s, type: %d), android (tag: 0x%x, type: %d), data_count:%d",
                mtk_tag, getTagInfo()->getName(mtk_tag), mtk_type, android_tag, android_type, count);
    }

    if  ( pDstSize != nullptr ) {
        *pDstSize = ::get_camera_metadata_size(pDstMetadata);
    }

    return result;
}
/******************************************************************************
 * Update
 ******************************************************************************/
inline status_t
MetadataConverter::
update(camera_metadata* &mBuffer, MUINT32 tag, const void *data, MUINT32 data_count)
{
    //MTK Metadata no duplicate tag, should direct add
    auto res = add_camera_metadata_entry(mBuffer, tag, data, data_count);

    if (res != OK) {
        META_LOGE("%s: Unable to update metadata entry %s.%s (%x): %s (%d)",
                __FUNCTION__, get_camera_metadata_section_name(tag),
                get_camera_metadata_tag_name(tag), tag, strerror(-res), res);
        return NO_MEMORY;
    }

    return OK;
}


#endif //MTKCAM_METADATA_V2
