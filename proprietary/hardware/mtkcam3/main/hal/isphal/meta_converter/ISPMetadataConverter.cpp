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

#define LOG_TAG "MtkCam/ISPMetadataConverter"

#include "ISPMetadataConverter.h"
#include <mtkcam/utils/std/Log.h>
#include <log/log.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/mtk_metadata_types.h>
#include <unordered_map>
#include <vector>


namespace NSCam {
namespace ISPHal {

#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

using namespace NSCam;
using namespace android;

static std::unordered_map<uint32_t, uint32_t> gMtkTagMap = {
        {MTK_SCALER_CROP_REGION, (uint32_t)(MTK_TYPE_ENUM::TYPE_MRect)}
};

static int32_t bytecount_of_type(uint32_t type)
{
    switch(type)
    {
        case (uint32_t)MetaDataType::TYPE_BYTE:
            return 1;
        case (uint32_t)MetaDataType::TYPE_INT32:
        case (uint32_t)MetaDataType::TYPE_FLOAT:
            return 4;
        case (uint32_t)MetaDataType::TYPE_INT64:
            return 8;
        default:
            return 0;
    }
}

template<class T>
static void
AddToEntry(IMetadata::IEntry &entry, T* data_start)
{
    entry.push_back(*data_start, Type2Type< T >());
}

#define ARRAY_TO_ENTRY(_ENTRY_, _DATA_START_, _TYPE_) \
    if(_TYPE_ == (uint32_t)MetaDataType::TYPE_BYTE) \
        AddToEntry(_ENTRY_, (MUINT8*)_DATA_START_); \
    if (_TYPE_ == (uint32_t)MetaDataType::TYPE_INT32) \
        AddToEntry(_ENTRY_, (MINT32*)_DATA_START_); \
    if(_TYPE_ == (uint32_t)MetaDataType::TYPE_FLOAT) \
        AddToEntry(_ENTRY_, (MFLOAT*)_DATA_START_); \
    if (_TYPE_ == (uint32_t)MetaDataType::TYPE_INT64) \
        AddToEntry(_ENTRY_, (MINT64*)_DATA_START_);


/******************************************************************************
 *
 ******************************************************************************/
ISPMetadataConverter::
ISPMetadataConverter()
{
    MY_LOGI("create ISP meta converter");
    mMaptoIMetaType.emplace((uint32_t)(MetaDataType::TYPE_BYTE), (uint32_t)(MTK_TYPE_ENUM::TYPE_MUINT8));
    mMaptoIMetaType.emplace((uint32_t)(MetaDataType::TYPE_INT32), (uint32_t)(MTK_TYPE_ENUM::TYPE_MINT32));
    mMaptoIMetaType.emplace((uint32_t)(MetaDataType::TYPE_FLOAT), (uint32_t)(MTK_TYPE_ENUM::TYPE_MFLOAT));
    mMaptoIMetaType.emplace((uint32_t)(MetaDataType::TYPE_INT64), (uint32_t)(MTK_TYPE_ENUM::TYPE_MINT64));

    mMaptoISPType.emplace((uint32_t)(MTK_TYPE_ENUM::TYPE_MUINT8), (uint32_t)(MetaDataType::TYPE_BYTE));
    mMaptoISPType.emplace((uint32_t)(MTK_TYPE_ENUM::TYPE_MINT32), (uint32_t)(MetaDataType::TYPE_INT32));
    mMaptoISPType.emplace((uint32_t)(MTK_TYPE_ENUM::TYPE_MFLOAT), (uint32_t)(MetaDataType::TYPE_FLOAT));
    mMaptoISPType.emplace((uint32_t)(MTK_TYPE_ENUM::TYPE_MINT64), (uint32_t)(MetaDataType::TYPE_INT64));
}

ISPMetadataConverter::
~ISPMetadataConverter()
{
    MY_LOGI("destroy ISP meta converter");
}


auto ISPMetadataConverter::convertToIMetadata(const void* src, IMetadata& dst) -> MBOOL
{
    if (src == nullptr)
    {
        return false;
    }
    uintptr_t metaptr = (uintptr_t)src;
    mtkisp_metadata_t *meta = (mtkisp_metadata_t *)src;
    mtkisp_metadata_entry_t *isp_data = (mtkisp_metadata_entry_t *)(metaptr + meta->entry_offset);
    MY_LOGD("ISPMeta entry count : %d", meta->entry_count);
    for (uint32_t i = 0; i < meta->entry_count; i++)
    {
         MY_LOGD("tag : 0x%X", isp_data->tag);
        IMetadata::IEntry entry(isp_data->tag);
        int32_t bytecount = bytecount_of_type(isp_data->type);
        if (gMtkTagMap.find(isp_data->tag) != gMtkTagMap.end())
        {
            if (gMtkTagMap[isp_data->tag] == (uint32_t)(MTK_TYPE_ENUM::TYPE_MRect))
            {
                for(uint32_t j = 0; (j * bytecount) < isp_data->count; j+=4) {
                    MRect data(MPoint(isp_data->data.i32[j], isp_data->data.i32[j+1]), MSize(isp_data->data.i32[j+2], isp_data->data.i32[j+3]));
                    AddToEntry(entry, &data);
                }
            }
        }
        else
        {
            for (uint32_t j = 0; j < isp_data->count; j += bytecount)
            {
                ARRAY_TO_ENTRY(entry, &(isp_data->data.u8[j]), isp_data->type);
            }
        }
        dst.update(isp_data->tag, entry);
        isp_data++;
    }
    return true;
}

auto ISPMetadataConverter::convertToISPMetadata(const IMetadata& src, void*& dst, size_t* pDstSize) -> MBOOL
{

    mtkisp_metadata_t *meta = allocate_isp_metadata_byentrynum(src.count());
    MY_LOGD("IMetadata entry count : %d", src.count());
    uint32_t datasize = 0;
    for (uint32_t i = 0; i < src.count(); i++)
    {
        IMetadata::IEntry entry = src.entryAt(i);
        auto type = entry.type();
        MY_LOGD("IMetadata entry tag : 0x%X, type : %d", entry.tag(), type);
        auto addEntryToISPMeta = [&] (void *data, uint32_t count) -> int
        {
            auto isp_type = mMaptoISPType[type];
            auto err = add_isp_metadata_entry(meta, entry.tag(), isp_type, data,
                           count*bytecount_of_type(isp_type), &datasize);

            return err;
        };
        if (type == (uint32_t)MTK_TYPE_ENUM::TYPE_MUINT8 || type == (uint32_t)MTK_TYPE_ENUM::TYPE_MINT32
         || type == (uint32_t)MTK_TYPE_ENUM::TYPE_MFLOAT || type == (uint32_t)MTK_TYPE_ENUM::TYPE_MINT64)
        {
            auto err = addEntryToISPMeta((void *)(entry.data()), entry.count());
            if ( err )
            {
                MY_LOGD("tag[0x%X] is not support with err[%d], maybe data size is too large", entry.tag(), err);
                continue;
            }
        }
        else if (gMtkTagMap.find(entry.tag()) != gMtkTagMap.end() && entry.count() != 0)
        {
            if (gMtkTagMap[entry.tag()] == (uint32_t)(MTK_TYPE_ENUM::TYPE_MRect))
            {
                std::vector<MINT32> vec_data;
                for(uint32_t j = 0; j < entry.count(); j++) {
                    MRect src_rect = entry.itemAt(j, Type2Type< MRect >());
                    vec_data.push_back(src_rect.p.x);
                    vec_data.push_back(src_rect.p.y);
                    vec_data.push_back(src_rect.s.w);
                    vec_data.push_back(src_rect.s.h);
                }
                type = (uint32_t)MTK_TYPE_ENUM::TYPE_MINT32;
                auto err = addEntryToISPMeta((void *)(vec_data.data()), entry.count()*4);
                if ( err )
                {
                    MY_LOGI("tag[0x%X] is not support with err[%d], type is rect", entry.tag(), err);
                    continue;
                }
            }
        }
        else
        {
            MY_LOGI("tag[0x%X] has not support type[%d]", entry.tag(), type);
        }
    }
    dst = (void *)meta;
    if (pDstSize != nullptr)
    {
        *pDstSize = datasize;
    }
    return true;

}

auto ISPMetadataConverter::freeISPMetaBuffer(void* buf) -> MBOOL
{
    mtkisp_metadata_t *meta = (mtkisp_metadata_t *)buf;
    free_isp_metadata(meta);
    return true;
}



};
};


