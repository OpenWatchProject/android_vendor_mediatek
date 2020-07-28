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
#include <utils/Log.h>
#include <processor/ParamConvert.h>
#include <processor/ConvertMap.h>

#include <utils/metadata/mtk_metadata_types.h>

#define LOG_TAG "ParamConvert"

using com::mediatek::campostalgo::FeatureParam;
namespace com {
namespace mediatek {
namespace campostalgo {

const ParamConvert* ParamConvert::getInstance() {
    static ParamConvert pc;
    return &pc;
}

ParamConvert::ParamConvert() {

#define _ADD_TAGMAP_(_feature_param_, _meta_tag_) \
    addTagMap(_feature_param_, _meta_tag_);

    ADD_ALL_MEMBERS
    ;

#undef _ADD_TAGMAP_

}

void ParamConvert::addTagMap(std::string paramKey, MUINT32 metaTag) {
    param_to_meta.add(paramKey, metaTag);
    meta_to_param.add(metaTag, paramKey);
}

MUINT32 ParamConvert::getMetaTag(std::string paramKey) const {
    return param_to_meta.valueFor(paramKey);
}

std::string ParamConvert::getParamKey(MUINT32 metaTag) const {
    return meta_to_param.valueFor(metaTag);
}

void ParamConvert::convertFromParam(const FeatureParam& param,
        IMetadata & dst) const {
    std::vector<std::string> keys;
    param.getParamKeys(keys);
    size_t size = keys.size();
    ALOGD("%s : %zu", __FUNCTION__, size);
    for (auto key : keys) {
        MUINT32 tag = getMetaTag(key);
        IMetadata::IEntry entry(tag);
        FeatureParam::ParamType type = param.getType(key);
        switch (type) {
        case FeatureParam::VAL_INTEGER: {
            int value = param.getInt(key);
            entry.push_back((MINT32) value, Type2Type<MINT32>());
            dst.update(tag, entry);
        }
            break;
        case FeatureParam::VAL_INTARRAY: {
            int length = param.getArrayLength(key);
            int *array = new int[length];
            param.getIntArray(key, array, length);
            if(length == 4) {
                MRect rect(MPoint(array[0], array[1]), MPoint(array[2], array[3]));
                entry.push_back(rect, Type2Type<MRect>());
                dst.update(tag, entry);
            } else if (length == 2) {
            	MSize size(array[0],array[1]);
            	entry.push_back(size, Type2Type<MSize>());
            	dst.update(tag, entry);
            } else {
            	ALOGE("%s :Not support array length: %d", __FUNCTION__, length);
            }
            delete[] array;
        }
            break;
        case FeatureParam::VAL_MAP:
            break;
        default:
            ALOGE("%s : Unkown data type : %d", __FUNCTION__, type);
            break;
        }
    }
}

void ParamConvert::convertFromTag(const IMetadata& meta,
        FeatureParam& param) const {
    MUINT count = meta.count();
    ALOGD("%s : %u", __FUNCTION__, count);
    IMetadata::IEntry entry;
    for (int i = 0; i < count; i++) {
        entry = meta.entryAt(i);
        MINT32 type = entry.type();
        std::string key = getParamKey(entry.tag());
        switch (type) {
        case MTK_TYPE_ENUM::TYPE_MINT32: {
            MINT32 value = entry.itemAt(0, Type2Type<MINT32>());
            param.appendInt32(key, value);
        }
            break;
        case MTK_TYPE_ENUM::TYPE_MRect: {
            MRect rect = entry.itemAt(0, Type2Type<MRect>());
            MINT32 value[4];
            value[0] = rect.leftTop().x;
            value[1] = rect.leftTop().y;
            value[2] = rect.rightBottom().x;
            value[3] = rect.rightBottom().y;
            param.appendIntArray(key, value, 4);
        }
            break;
        default:
            ALOGE("%s : Unkown data type : %d", __FUNCTION__, type);
            break;
        }
    }
}
}
}
}
