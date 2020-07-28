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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#define PIPE_CLASS_TAG "Exif"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_NODE
#include <cutils/properties.h>
#include <featurePipe/core/include/PipeLog.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_CAPTURE);

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSCapture;

Mutex&                                       ExifWriter::gLock = *new Mutex();
KeyedVector< MINT32, map<MUINT32, MUINT32> > ExifWriter::gvCollectedData;
KeyedVector< MINT32, set<MINT32> >           ExifWriter::gvReqMapping;

/*******************************************************************************
 *
 ********************************************************************************/
ExifWriter::
ExifWriter(
    const char */*name*/)
//    : mName(name)
{
    MY_LOGD("ctor(%p)", this);
    mEnable = ::property_get_int32("debug.dualcamMF.exif", 1);
}
/*******************************************************************************
 *
 ********************************************************************************/
ExifWriter::
~ExifWriter()
{
    MY_LOGD("dctor(%p)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
uint32_t
ExifWriter::
doExifUpdate(   RequestPtr request,
                map<MUINT32, MUINT32>& data)
{

    Mutex::Autolock _l(gLock);
    auto pInMetaHal = request->getMetadata(MID_MAN_IN_HAL);
    auto pOutMetaHal = request->getMetadata(MID_MAN_OUT_HAL);
    IMetadata* pMeta_In = (pInMetaHal != nullptr) ? pInMetaHal->native() : nullptr;
    IMetadata* pMeta_Out = (pOutMetaHal != nullptr) ? pOutMetaHal->native() : nullptr;

    if(pMeta_In == nullptr||pMeta_Out == nullptr){
        MY_LOGE("pMeta_In or pMeta_Out is nullptr!! (MID_MAN_IN/OUT_HAL)");
        return -1;
    }

    IMetadata exifMeta;
    {
        IMetadata::IEntry entry = pMeta_In->entryFor(MTK_3A_EXIF_METADATA);
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

    // get exsited bss data and add to data
    IMetadata::Memory oldDebugInfoSet;
    MBOOL haveOldDebugInfo = IMetadata::getEntry<IMetadata::Memory>(&exifMeta, MTK_MF_EXIF_DBGINFO_MF_DATA, oldDebugInfoSet);
    MY_LOGD("exifMeta haveOldDebugInfo(%d)", haveOldDebugInfo);

    /* update debug Exif metadata */
    IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
    entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
    pMeta_Out->update(entry_exif.tag(), entry_exif);

    return 0;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExifWriter::
sendData(MINT32 reqId, MINT32 tag, MINT32 value)
{
    MY_LOGD_IF(0, "regId:%d tag:%d value:%d", reqId, tag, value);
    Mutex::Autolock _l(gLock);

    if(gvCollectedData.indexOfKey(reqId) < 0){
        map<MUINT32, MUINT32> data;
        gvCollectedData.add(reqId, data);
    }

    map<MUINT32, MUINT32>& data = gvCollectedData.editValueFor(reqId);

    data[(MUINT32)tag] = (MUINT32)value;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExifWriter::
makeExifFromCollectedData(RequestPtr request)
{

    map<MUINT32, MUINT32> data;
    {
        Mutex::Autolock _l(gLock);

        MUINT32 reqId = request->getRequestNo();

        if(gvCollectedData.indexOfKey(reqId) < 0){
            MY_LOGE("collectedData for reqId:%d not exist!", reqId);
            return MFALSE;
        }

        data = gvCollectedData.editValueFor(reqId);

        gvCollectedData.removeItem(reqId);
    }

    doExifUpdate(request, data);

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExifWriter::
addReqMapping(MINT32 mainReqId, set<MINT32>& vSubReqId)
{

    {
        Mutex::Autolock _l(gLock);

        MY_LOGD("mainReqId(%d)", mainReqId);

        for(auto &e : vSubReqId){
            MY_LOGD("belonging ReqId(%d)", e);
        }

        if(gvReqMapping.indexOfKey(mainReqId) < 0){
            gvReqMapping.add(mainReqId, vSubReqId);
        }else{
            MY_LOGD("overrites privious mapping");
        }
    }

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExifWriter::
makeExifFromCollectedData_multiframe_mapping(RequestPtr request)
{

    MUINT32 reqId_this = request->getRequestNo();
    MUINT32 reqId_main = 0;
    MBOOL found = MFALSE;

    map<MUINT32, MUINT32> data;
    {
        Mutex::Autolock _l(gLock);

        // get mapping request
        for(auto i=0 ; i<gvReqMapping.size() ; i++){
            set<MINT32>& theSet = gvReqMapping.editValueAt(i);
            if( theSet.count(reqId_this) > 0){
                reqId_main = gvReqMapping.keyAt(i);
                found = MTRUE;
                break;
            }
        }

        if( !found ){
            MY_LOGW("req(%d) not belong to any set!", reqId_this);
            return MFALSE;
        }


        if(gvCollectedData.indexOfKey(reqId_main) < 0){
            MY_LOGE("collectedData for reqId:%d not exist!", reqId_main);
            return MFALSE;
        }

        data = gvCollectedData.editValueFor(reqId_main);

        // dont erase data from gvCollectedData for multiple usage
        // gvCollectedData.removeItem(reqId);
    }

    doExifUpdate(request, data);

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
const char *
ExifWriter::
tagToName(int /*tag*/)
{
    return "";
}
