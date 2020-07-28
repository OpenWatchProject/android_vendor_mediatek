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
#include "ExifWriter.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "ExifWriter"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#include <PipeLog.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace dbg_cam_reservec_param_0;
/*******************************************************************************
 *
 ********************************************************************************/
ExifWriter::
ExifWriter(
    const char *name)
    : mName(name)
{
    MY_LOGD("ctor(%p)", this);
    mEnable = ::property_get_int32("vendor.debug.bmdenoise.exif", 1);
}
/*******************************************************************************
 *
 ********************************************************************************/
ExifWriter::
~ExifWriter()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExifWriter::
doExifUpdate(PipeRequestPtr request, BMDN_EXIF_DATA_T exifData)
{
    return MFALSE;
}
/*******************************************************************************
 *
 ********************************************************************************/
uint32_t
ExifWriter::
doExifUpdate(   PipeRequestPtr request,
                BMDeNoiseFeatureType type,
                map<MINT32, MINT32>& data)
{
    MY_LOGD("regId:%d type:%d +", request->getRequestNo(), type);

    IMetadata* pMeta_in = request->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_out = request->getMetadata(BID_META_OUT_HAL);

    if(pMeta_in == nullptr){
        MY_LOGE("pMeta_in == nullptr! (BID_META_IN_HAL)");
        return -1;
    }

    if(pMeta_out == nullptr){
        MY_LOGE("pMeta_out == nullptr! (BID_META_OUT_HAL)");
        return -1;
    }

    data[RESERVEB_TAG_VERSION] = 0;

    uint32_t TS = Utils::TimeTool::getReadableTime();
    data[BM_TAG_UNIQUE_ID] = TS;

    // allocate memory of debug information
    IMetadata::Memory memory_dbgInfo;
    memory_dbgInfo.resize(sizeof(DEBUG_RESERVEB_INFO_T));
    std::fill(memory_dbgInfo.begin(), memory_dbgInfo.end(), 0);

    auto pTag = reinterpret_cast<debug_exif_field*>(memory_dbgInfo.editArray());

    // fill in the values
    for(MINT32 i = RESERVEB_TAG_VERSION ; i != RESERVEB_TAG_END ; i ++){
        pTag[i].u4FieldID = 0x10000000 | i; // 0x1000000 makes debug parser works

        if(data.count(i) == 1){
            pTag[i].u4FieldValue = data[i];
            MY_LOGD("tag:%d => %d", i, data[i]);
        }else{
            pTag[i].u4FieldValue = -1;
        }
    }

    IMetadata exifMeta;
    if( !tryGetMetadata<IMetadata>(pMeta_in, MTK_3A_EXIF_METADATA, exifMeta) ) {
        MY_LOGW("no tag: MTK_3A_EXIF_METADATA");
    }

    if(mEnable == 1){
        // update debug exif metadata
        updateEntry<MINT32>(&exifMeta, MTK_RESVB_EXIF_DBGINFO_KEY, DEBUG_EXIF_MID_CAM_RESERVE2);
        updateEntry<IMetadata::Memory>(&exifMeta, MTK_RESVB_EXIF_DBGINFO_DATA, memory_dbgInfo);
    }else{
        MY_LOGD("exifWriter disabled, use original exif data");
    }

    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());

    // push back to hal meta
    pMeta_out->update(entry_exif.tag(), entry_exif);

    MY_LOGD("regId:%d type:%d TS:%zu -", request->getRequestNo(), type, TS);
    return TS;
}
/*******************************************************************************
 *
 ********************************************************************************/
const char *
ExifWriter::
tagToName(int tag)
{
    #define MAKE_NAME_CASE(name) \
        case name: return #name;

    switch(tag)
    {
        MAKE_NAME_CASE(BM_TAG_DENOISE_VERSION);
        MAKE_NAME_CASE(BM_TAG_DENOISE_TYPE);
        MAKE_NAME_CASE(BM_TAG_DENOISE_COMPOSITION);
    }
    MY_LOGW("unknown tag:%d", tag);

    return "unknown";
    #undef MAKE_NAME_CASE
}