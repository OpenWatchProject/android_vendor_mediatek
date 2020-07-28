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
#include <mtkcam/utils/exif/DebugExifUtils.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace dbg_cam_reservec_param_0;

Mutex                                      ExifWriter::gLock;
KeyedVector< MINT32, map<MINT32, MINT32> > ExifWriter::gvCollectedData;

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
    MY_LOGE("no implementation");
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

    Mutex::Autolock _l(gLock);

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
MBOOL
ExifWriter::
doExifUpdate_MFNR(  PipeRequestPtr request,
                    const std::map<unsigned int, uint32_t>& data)
{
    MY_LOGD("regId:%d type: MFNR +", request->getRequestNo());

    Mutex::Autolock _l(gLock);

    IMetadata* pMeta_out = request->getMetadata(BID_META_OUT_HAL);

    if(pMeta_out == nullptr){
        MY_LOGE("pMeta_out == nullptr! (BID_META_OUT_HAL)");
        return MFALSE;
    }

    IMetadata exifMeta;
    {
        IMetadata::IEntry entry = pMeta_out->entryFor(MTK_3A_EXIF_METADATA);
        if (entry.isEmpty()) {
            MY_LOGW("%s: no MTK_3A_EXIF_METADATA can be used", __FUNCTION__);
            return MFALSE;
        }

        exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
    }

    /* set debug information into debug Exif metadata */
    DebugExifUtils::setDebugExif(
            DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
            static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
            data,
            &exifMeta);

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    pMeta_out->update(entry_exif.tag(), entry_exif);

    MY_LOGD("regId:%d type: MFNR -", request->getRequestNo());
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExifWriter::
sendData(MINT32 reqId, MINT32 tag, MINT32 value)
{
    MY_LOGD("regId:%d tag:%d value:%d", reqId, tag, value);
    Mutex::Autolock _l(gLock);

    if(gvCollectedData.indexOfKey(reqId) < 0){
        map<MINT32, MINT32> data;
        gvCollectedData.add(reqId, data);
    }

    map<MINT32, MINT32>& data = gvCollectedData.editValueFor(reqId);

    data[tag] = value;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExifWriter::
makeExifFromCollectedData(PipeRequestPtr request, BMDeNoiseFeatureType type)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    map<MINT32, MINT32> data;
    {
        Mutex::Autolock _l(gLock);

        MINT32 reqId = request->getRequestNo();

        if(gvCollectedData.indexOfKey(reqId) < 0){
            MY_LOGE("collectedData for reqId:%d not exist!", reqId);
            return MFALSE;
        }

        data = gvCollectedData.editValueFor(reqId);

        gvCollectedData.removeItem(reqId);
    }

    doExifUpdate(request, type, data);

    return MTRUE;
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