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

#define LOG_TAG "MtkCam/TemplateRequest"
//
#include "MyUtils.h"
#include <hardware/camera3.h>
//
#include <dlfcn.h>

// converter
#include <mtkcam/utils/metadata/client/TagMap.h>
#include <mtkcam/utils/metadata/IMetadataTagSet.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <system/camera_metadata.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/mtk_metadata_types.h>
#include <mtkcam/drv/IHalSensor.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#pragma GCC diagnostic pop

//
/******************************************************************************
 *
 ******************************************************************************/
#if 0
#define FUNC_PROFILE_START(_name_)  NSCam::Utils::CamProfile profile(__FUNCTION__, LOG_TAG)
#define FUNC_PROFILE_END()          profile.print("")
#else
#define FUNC_PROFILE_START(_name_)
#define FUNC_PROFILE_END()
#endif

/******************************************************************************
 *
 ******************************************************************************/
status_t
TemplateRequest::
impConstructRequestMetadata_by_SymbolName(
    String8 const&      s8Symbol,
    IMetadata&          metadata,
    int const           requestType
)
{
    FUNC_PROFILE_START(__FUNCTION__);
typedef status_t (*PFN_T)(
        IMetadata &         metadata,
        int const           requestType,
        Info const&         info
    );
    //
    PFN_T pfn = (PFN_T)::dlsym(RTLD_DEFAULT, s8Symbol.string());
    if  ( ! pfn ) {
        META_LOGW("%s not found", s8Symbol.string());
        FUNC_PROFILE_END();
        return  NAME_NOT_FOUND;
    }
    //
    status_t const status = pfn(metadata, requestType, mInfo);
    META_LOGW_IF(OK != status, "%s returns status[%s(%d)]", s8Symbol.string(), ::strerror(-status), -status);
    FUNC_PROFILE_END();
    //
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
TemplateRequest::
impConstructRequestMetadata(
    IMetadata&      metadata,
    int const       requestType
)
{
    status_t status = OK;
    FUNC_PROFILE_START(__FUNCTION__);
#ifdef COMMON_HIERARCHY
    //
    int32_t result = 0;
    {
        String8 const s8Symbol_Common = String8::format("%s_COMMON_%s", PREFIX_FUNCTION_REQUEST_METADATA, "COMMON");
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Common, metadata, requestType);
        if ( !status ) result|=COMMON_COMMON;

        String8 const s8Symbol_Sensor = String8::format("%s_COMMON_%s", PREFIX_FUNCTION_REQUEST_METADATA, mInfo.getSensorDrvName());
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Sensor, metadata, requestType);
        if ( !status ) result|=COMMON_SENSOR;
    }
    //
    {
        String8 const s8Symbol_Common = String8::format("%s_DEVICE_%s", PREFIX_FUNCTION_REQUEST_METADATA, "DEFAULT");
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Common, metadata, requestType);
        if ( !status ) result|=PLATFORM_COMMON;

        String8 const s8Symbol_Sensor = String8::format("%s_DEVICE_%s", PREFIX_FUNCTION_REQUEST_METADATA, mInfo.getSensorDrvName());
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Sensor, metadata, requestType);
        if ( !status ) result|=PLATFORM_SENSOR;
    }
    //
    {
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s", PREFIX_FUNCTION_REQUEST_METADATA, mInfo.getSensorDrvName());
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Sensor, metadata, requestType);
        if ( !status ) result|=PROJECT_SENSOR;
    }
    META_LOGD("%s: load status(0x%x)", "TemplateRequest", result);
    //
#else
    //
    {
#if 1
        String8 const s8Symbol_Sensor = String8::format("%s_DEVICE_%s", PREFIX_FUNCTION_REQUEST_METADATA, mInfo.getSensorDrvName());
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Sensor, metadata, requestType);
        if  ( OK == status ) {
                goto lbDevice;
        }
#endif
        String8 const s8Symbol_Common = String8::format("%s_DEVICE_%s", PREFIX_FUNCTION_REQUEST_METADATA, "COMMON");
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Common, metadata, requestType);
        if  ( OK == status ) {
                goto lbDevice;
        }
    }
    //
lbDevice:
    {
#if 1
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s", PREFIX_FUNCTION_REQUEST_METADATA, mInfo.getSensorDrvName());
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Sensor, metadata, requestType);
        if  ( OK == status ) {
        return  OK;
    }
#endif
    //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_%s", PREFIX_FUNCTION_REQUEST_METADATA, "COMMON");
        status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Common, metadata, requestType);
        if  ( OK == status ) {
            return  OK;
        }
    }
#endif
    FUNC_PROFILE_END();
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
static bool setTagInfo(IMetadataTagSet &rtagInfo)
{
    #define _IMP_SECTION_INFO_(...)
    #undef  _IMP_TAG_INFO_
    #define _IMP_TAG_INFO_(_tag_, _type_, _name_) \
        rtagInfo.addTag(_tag_, _name_, Type2TypeEnum<_type_>::typeEnum);
    #include <mtkcam/utils/metadata/client/mtk_metadata_tag_info.inl>
    #include <custom_metadata/custom_metadata_tag_info.inl>
    #undef  _IMP_TAG_INFO_

    #undef _IMP_TAGCONVERT_
    #define _IMP_TAGCONVERT_(_android_tag_, _mtk_tag_) \
        rtagInfo.addTagMap(_android_tag_, _mtk_tag_);
        #if (PLATFORM_SDK_VERSION >= 21)
        ADD_ALL_MEMBERS;
        #else
            #warning "no tag info"
        #endif

    #undef _IMP_TAGCONVERT_

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
TemplateRequest::
constructRequestMetadata(
    int const           requestType,
    camera_metadata*&   rpMetadata,
    IMetadata&          rMtkMetadata
)
{
    META_LOGD("constructRequestMetadata");

    status_t status = OK;

    //-----(1)-----//
    //get static informtation from customization (with camera_metadata format)
    //calculate its entry count and data count
    if  ( OK != (status = impConstructRequestMetadata(rMtkMetadata, requestType)) ) {
        META_LOGE("Unable evaluate the size for camera static info - status[%s(%d)]\n", ::strerror(-status), -status);
        return  status;
    }
    META_LOGD("Allocating %d entries from customization", rMtkMetadata.count());

    //calculate its entry count and data count
    // init converter
    IMetadataTagSet tagInfo;
    setTagInfo(tagInfo);
    sp<IMetadataConverter> pConverter = IMetadataConverter::createInstance(tagInfo);
    if (pConverter == nullptr) {
        META_LOGW("IMetadata converter is null!");
        return BAD_VALUE;
    }
#ifdef MTKCAM_METADATA_V1
    // METADATA_V2 remove get_data_count due to performance bad, and useless
    // this case only use to print count on log

    size_t entryCount = 0;
    size_t dataCount = 0;
    MBOOL ret = pConverter->get_data_count(rMtkMetadata, entryCount, dataCount);
    if ( ret != OK )
    {
        META_LOGE("get Imetadata count error\n");
        return UNKNOWN_ERROR;
    }
    META_LOGD("Allocating %zu entries, %zu extra bytes from HAL modules", entryCount, dataCount);
#endif

    //-----(2)-----//
    // overwrite
    updateData(requestType, rMtkMetadata);

    //-----(3)-----//
    // convert to android metadata
    pConverter->convert(rMtkMetadata, rpMetadata);
    ::sort_camera_metadata(rpMetadata);

    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
void
TemplateRequest::
updateData(int const requestType, IMetadata &rMetadata)
{

    IMetadataProvider* obj = NSMetadataProviderManager::valueFor(mInfo.getDeviceId());
    if(obj == NULL) {
        obj = IMetadataProvider::create(mInfo.getDeviceId());
        NSMetadataProviderManager::add(mInfo.getDeviceId(), obj);
    }

    IMetadata static_meta = obj->getMtkStaticCharacteristics();

    {
        MRect cropRegion;
        IMetadata::IEntry active_array_entry = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        if( !active_array_entry.isEmpty() ) {
            cropRegion = active_array_entry.itemAt(0, Type2Type<MRect>());
            cropRegion.p.x=0;
            cropRegion.p.y=0;
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(cropRegion, Type2Type< MRect >());
            rMetadata.update(MTK_SCALER_CROP_REGION, entry);
         }
    }

    {   // update focal lens from static metadata, widest one as default value
        auto focalLength_static = static_meta.entryFor(MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS);
        if ( !focalLength_static.isEmpty() ) {
            IMetadata::IEntry entry(MTK_LENS_FOCAL_LENGTH);
            entry.push_back(focalLength_static.itemAt(0, Type2Type<MFLOAT>()), Type2Type<MFLOAT>());
            rMetadata.update(MTK_LENS_FOCAL_LENGTH, entry);
        }
    }

    {   // update default af regions if more than one af mode (OFF) supported
        auto afregions_static = static_meta.entryFor(MTK_CONTROL_AF_AVAILABLE_MODES);
        auto minFocusDistance = static_meta.entryFor(MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE);
        if ( !afregions_static.isEmpty() && afregions_static.count()==1 &&
             afregions_static.itemAt(0, Type2Type<MUINT8>())==MTK_CONTROL_AF_MODE_OFF &&
             !minFocusDistance.isEmpty() &&
             minFocusDistance.itemAt(0, Type2Type<MFLOAT>())==0.0f ) {
            // META_LOGD("fixed focus");
        } else {
            auto template_afRegions = rMetadata.entryFor(MTK_CONTROL_AF_REGIONS);
            if ( template_afRegions.isEmpty() ) {
                IMetadata::IEntry entry(MTK_CONTROL_AF_REGIONS);
                entry.push_back(0, Type2Type<MINT32>());
                entry.push_back(0, Type2Type<MINT32>());
                entry.push_back(0, Type2Type<MINT32>());
                entry.push_back(0, Type2Type<MINT32>());
                entry.push_back(0, Type2Type<MINT32>());
                rMetadata.update(MTK_CONTROL_AF_REGIONS, entry);
            } else {
                META_LOGD("user defines default value");
            }
        }
    }

    { // === 3DNR ===
        IMetadata::IEntry nr3d_avail_entry = static_meta.entryFor(MTK_NR_FEATURE_AVAILABLE_3DNR_MODES);
        IMetadata::IEntry nr3d_entry = rMetadata.entryFor(MTK_NR_FEATURE_3DNR_MODE);
        MBOOL support3DNR = (IMetadata::IEntry::indexOf(nr3d_avail_entry, (MINT32)MTK_NR_FEATURE_3DNR_MODE_ON) != -1);

        if (nr3d_entry.isEmpty()){
            // tag not set in custom, use default on
            if(support3DNR){
                IMetadata::IEntry nr3d_entry_new(MTK_NR_FEATURE_3DNR_MODE);
                nr3d_entry_new.push_back(MTK_NR_FEATURE_3DNR_MODE_ON, Type2Type<MINT32>());
                rMetadata.update(MTK_NR_FEATURE_3DNR_MODE, nr3d_entry_new);
            }
        } else {
            // tag set in custom, confirm 3DNR support or not (just protect mechanism)
            if( ! support3DNR){
                nr3d_entry.clear();
                nr3d_entry.push_back(MTK_NR_FEATURE_3DNR_MODE_OFF, Type2Type<MINT32>());
                rMetadata.update(MTK_NR_FEATURE_3DNR_MODE, nr3d_entry);
            }
        }
    }

#ifndef EIS_SUPPORTED
    {
        IMetadata::IEntry eis_entry = rMetadata.entryFor(MTK_CONTROL_VIDEO_STABILIZATION_MODE);
        if (!eis_entry.isEmpty())
        {
            eis_entry.clear();
            eis_entry.push_back(MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF, Type2Type<MUINT8>());
            rMetadata.update(MTK_CONTROL_VIDEO_STABILIZATION_MODE, eis_entry);
        }
        IMetadata::IEntry adveis_entry = rMetadata.entryFor(MTK_EIS_FEATURE_EIS_MODE);
        if (!adveis_entry.isEmpty())
        {
            adveis_entry.clear();
            adveis_entry.push_back(MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF, Type2Type<MINT32>());
            rMetadata.update(MTK_EIS_FEATURE_EIS_MODE, adveis_entry);
        }
    }
#endif

    // capabilities
    {
        updateByCapabilities(requestType, static_meta, rMetadata);
    }

    // zsl
    {
        // Do not parsing static metadata to check if aosp enable_zsl exists or not.
        // The search for enable_zsl costs much time in availablerequestkey or availableresultkey.

        IMetadata::IEntry enableZsl = rMetadata.entryFor(MTK_CONTROL_ENABLE_ZSL);
        if ( enableZsl.isEmpty() ) {
            IMetadata::IEntry entry(MTK_CONTROL_ENABLE_ZSL);
            entry.push_back(MTK_CONTROL_ENABLE_ZSL_FALSE, Type2Type<MUINT8>());
            rMetadata.update(entry.tag(), entry);
        } else {
            META_LOGI("user defines enableZsl in custom files");
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
TemplateRequest::
updateByCapabilities(
    int const           requestType      __unused,
    IMetadata&          rStaticMetadata  __unused,
    IMetadata&          rMetadata        __unused
)
{
    bool bRaw16 = false;
    bool bOpaqueReproc = false;
    bool bYuvReproc = false;
    IMetadata::IEntry availableCapabilities = rStaticMetadata.entryFor(MTK_REQUEST_AVAILABLE_CAPABILITIES);
    for (int i = 0; i < availableCapabilities.count(); i++) {
        switch ( availableCapabilities.itemAt(i, Type2Type<MUINT8>() ) ) {
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_RAW: {
                bRaw16 = true;
                break;
            }
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_PRIVATE_REPROCESSING: {
                bOpaqueReproc = true;
                break;
            }
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_YUV_REPROCESSING: {
                bYuvReproc = true;
                break;
            }
            default: {
                break;
            }
        }
    }

    if ( bRaw16 )
    {
        // post raw sensitivity boost
        auto postRawSensitivityBoost = rMetadata.entryFor(MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST);
        if ( postRawSensitivityBoost.isEmpty() ) {
            IMetadata::IEntry entry(MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST);
            entry.push_back(100, Type2Type<MINT32>());
            rMetadata.update(entry.tag(), entry);
        } else {
            META_LOGD("user defines postRawSensitivityBoost: %d", postRawSensitivityBoost.itemAt(0, Type2Type<MINT32>()));
        }

        // lens shading map mode
        if ( requestType==CAMERA3_TEMPLATE_STILL_CAPTURE ) {
            auto lensShadingMapMode = rMetadata.entryFor(MTK_STATISTICS_LENS_SHADING_MAP_MODE);
            if ( lensShadingMapMode.isEmpty() ) {
                IMetadata::IEntry entry(MTK_STATISTICS_LENS_SHADING_MAP_MODE);
                entry.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type<MUINT8>());
                rMetadata.update(entry.tag(), entry);
            } else {
                if ( lensShadingMapMode.count()==1 &&
                     MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF == lensShadingMapMode.itemAt(0, Type2Type<MUINT8>() ) ) {
                    META_LOGD("replace lensShadingMapMode from OFF to ON (raw capability)");
                    lensShadingMapMode.replaceItemAt(0, MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type<MUINT8>());
                    rMetadata.update(lensShadingMapMode.tag(), lensShadingMapMode);
                } else {
                    META_LOGE("invalid lensShadingMapMode setting -> count(%u)", lensShadingMapMode.count());
                }
            }
        }
    }

    if ( bOpaqueReproc || bYuvReproc ) {
        if ( requestType==CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG ) {
            // update MTK_EDGE_MODE if needed
            auto edgeMode = rMetadata.entryFor(MTK_EDGE_MODE);
            if ( edgeMode.itemAt(0, Type2Type<MUINT8>())!=MTK_EDGE_MODE_ZERO_SHUTTER_LAG ) {
                edgeMode.replaceItemAt(0, MTK_EDGE_MODE_ZERO_SHUTTER_LAG, Type2Type<MUINT8>());
                rMetadata.update(edgeMode.tag(), edgeMode);
                META_LOGD("replace MTK_EDGE_MODE as MTK_EDGE_MODE_ZERO_SHUTTER_LAG (opaqueReproc:%d yuvReproc:%d)", bOpaqueReproc, bYuvReproc);
            }

            // update MTK_NOISE_REDUCTION_MODE if needed
            auto NRMode = rMetadata.entryFor(MTK_NOISE_REDUCTION_MODE);
            if ( NRMode.itemAt(0, Type2Type<MUINT8>())!=MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG ) {
                NRMode.replaceItemAt(0, MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG, Type2Type<MUINT8>());
                rMetadata.update(NRMode.tag(), NRMode);
                META_LOGD("replace MTK_NOISE_REDUCTION_MODE as MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG (opaqueReproc:%d yuvReproc:%d)", bOpaqueReproc, bYuvReproc);
            }
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
TemplateRequest::
onCreate(int iOpenId)
{
    META_LOGD("+ (%d)", iOpenId);
    //
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if(!pHalSensorList) {
        META_LOGW("Hal sensorlist is null!");
        return BAD_VALUE;
    }
    if(!pHalDeviceList) {
        META_LOGW("logical device is null!");
        return BAD_VALUE;
    }
    // get main sensor from logical device list.
    auto sensorIdList = pHalDeviceList->getSensorId(iOpenId);
    int32_t sensorType = pHalSensorList->queryType(sensorIdList[0]);
    int32_t sensorDev = pHalDeviceList->querySensorDevIdx(iOpenId);
    NSCam::SensorStaticInfo sensorStaticInfo;
    pHalDeviceList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
    int32_t sensorRawFmtType = sensorStaticInfo.rawFmtType;
    const char* sensorDrvName = pHalDeviceList->queryDriverName(iOpenId);
    mInfo = Info(iOpenId, sensorType, sensorRawFmtType, sensorDrvName);

    //  Standard template types
    for (int type = CAMERA3_TEMPLATE_PREVIEW; type < CAMERA3_TEMPLATE_COUNT; type++)
    {
        camera_metadata* metadata = NULL;
        IMetadata mtkMetadata;
        status_t status = constructRequestMetadata(type, metadata, mtkMetadata);
        if  ( OK != status || NULL == metadata || mtkMetadata.isEmpty()) {
            META_LOGE("constructRequestMetadata - type:%#x metadata:%p status[%s(%d)]", type, metadata, ::strerror(-status), -status);
            return status;
        }
        //
        mMapRequestTemplate.add(type, metadata);

        mMapRequestTemplateMetadata.add(type, mtkMetadata);
    }

#if 0
    //  vendor-defined request templates
    for (int type = CAMERA3_VENDOR_TEMPLATE_START; type < CAMERA3_VENDOR_TEMPLATE_COUNT; type++)
    {
        camera_metadata* metadata = NULL;
        status = constructRequestMetadata(type, metadata);
        if  ( OK != status || NULL == metadata ) {
            META_LOGE("constructRequestMetadata - type:%#x metadata:%p status[%s(%d)]", type, metadata, ::strerror(-status), -status);
            return  status;
        }
        //
        MapRequestTemplate.add(type, metadata);
    }
#endif
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
camera_metadata const*
TemplateRequest::
getData(int requestType)
{
    return mMapRequestTemplate.valueFor(requestType);
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadata const&
TemplateRequest::
getMtkData(int requestType)
{
    return mMapRequestTemplateMetadata.valueFor(requestType);
}


/******************************************************************************
 *
 ******************************************************************************/
ITemplateRequest*
ITemplateRequest::
getInstance(int iOpenId)
{
    TemplateRequest* p = new TemplateRequest();
    if(p != NULL) {
        p->onCreate(iOpenId);
    }
    return p;
}

