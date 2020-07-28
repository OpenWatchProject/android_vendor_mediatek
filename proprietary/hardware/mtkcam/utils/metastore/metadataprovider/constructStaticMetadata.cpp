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

#define LOG_TAG "MtkCam/MetadataProvider.constructStatic"
//
#include "MyUtils.h"
//
#include <dlfcn.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IDngInfo.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <cutils/properties.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
//
#include <custom_metadata/custom_metadata_tag.h>
//
#include <vector>
#include <mtkcam/utils/calibration/ICalibrationProvider.h>
//
#if (3==MTKCAM_HAL_VERSION)
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#endif

/******************************************************************************
 *
 ******************************************************************************/

#if 1
#define FUNC_START          META_LOGD("[%s] - E.", __FUNCTION__)
#define FUNC_END            META_LOGD("[%s] - X.", __FUNCTION__)
#else
#define FUNC_START
#define FUNC_END
#endif

#if 1
#define FUNC_PROFILE_START(_name_)  NSCam::Utils::CamProfile profile(__FUNCTION__, LOG_TAG)
#define FUNC_PROFILE_END()          profile.print("")
#else
#define FUNC_START_PROFILE(_name_)
#define FUNC_START_END()
#endif

/******************************************************************************
 *
 ******************************************************************************/

status_t
MetadataProvider::
impConstructStaticMetadata_by_SymbolName(
    String8 const&      s8Symbol,
    IMetadata &metadata
)
{
typedef status_t (*PFN_T)(
        IMetadata &         metadata,
        Info const&         info
    );
    //
    CAM_TRACE_CALL();
    PFN_T pfn = (PFN_T)::dlsym(RTLD_DEFAULT, s8Symbol.string());
    if  ( ! pfn ) {
        META_LOGW_IF(mLogLevel>=1, "%s not found", s8Symbol.string());
        return  NAME_NOT_FOUND;
    }
    //
    status_t const status = pfn(metadata, mInfo);
    META_LOGI_IF(mLogLevel>=1 && status, "%s: returns status[%s(%d)]", s8Symbol.string(), ::strerror(-status), -status);
    //
    return  status;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
impConstructStaticMetadata_v1(
    IMetadata &metadata
)
{
    // step1. config common setting
    String8 strResult = String8::format("<load custom folder>\n\tSTATIC_COMMON: ");
    for (int i = 0; NULL != kCommonStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kCommonStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status = OK;
        //
        String8 const s8Symbol_Common = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if ( status==OK ) loadResult|=COMMON_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if ( status==OK ) loadResult|=COMMON_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }

    //
    strResult += String8::format("\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if ( status_d==OK ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if ( status_s==OK ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
        if ( status_s!=OK && status_d!=OK ) {
            META_LOGE("%s", strResult.string());
            return NAME_NOT_FOUND;
        }
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if ( status_d==OK ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if ( status_s==OK ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    META_LOGI("%s", strResult.string());
#if 1   // design for backward compatible
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];
        status_t status = OK;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if  ( OK == status ) {
            META_LOGW("project configuration exists!");
            return NAME_NOT_FOUND;
        }
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if  ( OK == status ) {
            META_LOGW("project configuration exists!");
            return NAME_NOT_FOUND;
        }
    }
#endif
    // TODO: sanity check (no sensor & lens setting; no streamconfiguration)

    // for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++) {
    //     META_LOGD("%s: load status(0x%x)", kStaticMetadataTypeNames[i], vMap[i]);
    //     if ( vMap[i]==0 ) {
    //         META_LOGE("Fail to load %s in all custom", kStaticMetadataTypeNames[i] );
    //         return NAME_NOT_FOUND;
    //     }
    // }
    //
    return  OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
impConstructStaticMetadata_v1_ext(
    IMetadata &metadata,
    char const* const staticMetadataNames[]
)
{
    String8 strResult = String8::format("<load custom folder - optional>\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != staticMetadataNames[i]; i++)
    {
        char const*const pTypeName = staticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if ( status_d==OK ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if ( status_s==OK ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != staticMetadataNames[i]; i++)
    {
        char const*const pTypeName = staticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if ( status_d==OK ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if ( status_s==OK ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    META_LOGI("%s", strResult.string());
    //
    return  OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
impConstructStaticMetadata_v1_overwrite(
    IMetadata &metadata
)
{
    //
    String8 strResult = String8::format("<load custom folder - overwrite>\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformOverwriteStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOverwriteStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if ( status_d==OK ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if ( status_s==OK ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformOverwriteStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOverwriteStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if ( status_d==OK ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if ( status_s==OK ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    META_LOGI("%s", strResult.string());
    //
    return  OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
constructStaticMetadata_v1(sp<IMetadataConverter> pConverter, camera_metadata*& rpDstMetadata, IMetadata& mtkMetadata)
{
    FUNC_PROFILE_START(__FUNCTION__);
    META_LOGD("construct static metadata\n");

    status_t status = OK;
#if MTKCAM_CUSTOM_METADATA_COMMON
    // step1. get static informtation from sensor hal moduls (with IMetadata format)
    IMetadata sensorMetadata = MAKE_HalLogicalDeviceList()->queryStaticInfo(mInfo.getDeviceId());
    META_LOGD("Allocating %d entries from sensor HAL", sensorMetadata.count());
    if ( mLogLevel>=2 )
        pConverter->dumpAll(sensorMetadata, -1);

    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if( CC_UNLIKELY( pHalDeviceList==nullptr ) ) {
        META_LOGE("get HalLogicalDeviceList fail!");
        return BAD_VALUE;
    }
    auto physicIdsList = pHalDeviceList->getSensorId(mInfo.getDeviceId());
    auto pDngInfo = MAKE_DngInfo(LOG_TAG, physicIdsList[0]);
    if( CC_UNLIKELY( pDngInfo == nullptr ) ) {
        META_LOGE("get DngInfo fail!");
        return BAD_VALUE;
    }
    IMetadata rDngMeta = pDngInfo->getStaticMetadata();
    META_LOGD("Allocating %d entries from Dng Info", rDngMeta.count());
    if ( mLogLevel>=2 )
        pConverter->dumpAll(rDngMeta, -1);

    //get static informtation from customization (with camera_metadata format)
    //calculate its entry count and data count
    bool v1 = true;
    if  ( OK != (status = impConstructStaticMetadata_v1(mtkMetadata)) ) {
        META_LOGW("Unable construct static metadata in common hierarchy - status[%s(%d)]\n", ::strerror(-status), -status);
        mtkMetadata.clear();
        v1 = false;
        if ( OK != (status = impConstructStaticMetadata(mtkMetadata)) ) {
            META_LOGE("Unable evaluate the size for camera static info - status[%s(%d)]\n", ::strerror(-status), -status);
            return  status;
        }
    } else {
        impConstructStaticMetadata_v1_ext(mtkMetadata, kPlatformOptionalStaticMetadataNames);
    }
    META_LOGD("Allocating %d entries from customization", mtkMetadata.count());
    if ( mLogLevel>=2 )
        pConverter->dumpAll(mtkMetadata, -1);

    //--- (2.1.1) --- //
    //merge.
    sensorMetadata += rDngMeta;
    for (size_t i = 0; i < sensorMetadata.count(); i++)
    {
        IMetadata::Tag_t mTag = sensorMetadata.entryAt(i).tag();
        mtkMetadata.update(mTag, sensorMetadata.entryAt(i));
    }
    META_LOGD("Allocating %d entries from customization + sensor HAL + Dng Info", mtkMetadata.count());
    if ( mLogLevel>=2 )
        pConverter->dumpAll(mtkMetadata, -1);

    //overwrite
    updateData(mtkMetadata);
    //
    if ( v1 ) {
        impConstructStaticMetadata_v1_ext(mtkMetadata, kPlatformOverwriteStaticMetadataNames);
    }
    if ( mLogLevel>=1 )
        pConverter->dumpAll(mtkMetadata, -1);

    //validate the constructed metatata
    validation(mtkMetadata);

    #if (PLATFORM_SDK_VERSION >= 21)
    pConverter->convert(mtkMetadata, rpDstMetadata);
    //
    ::sort_camera_metadata(rpDstMetadata);
    #endif

#else
    status = constructStaticMetadata(pConverter, rpDstMetadata, mtkMetadata);
#endif  // #if MTKCAM_CUSTOM_METADATA_COMMON

    FUNC_PROFILE_END();
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
impConstructStaticMetadata(
    IMetadata &metadata
)
{
    size_t count = (sizeof(kStaticMetadataTypeNames)/sizeof(char const*)) ;
    std::vector<bool> vMap(count, false);
    //
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];
        status_t status = OK;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_DEVICE_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if  ( OK == status ) {
            vMap[i] = true;
            continue;
        }
        //
        String8 const s8Symbol_Common = String8::format("%s_DEVICE_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if  ( OK == status ) {
            vMap[i] = true;
            continue;
        }
        //
        META_LOGW_IF(0, "Fail for both %s & %s", s8Symbol_Sensor.string(), s8Symbol_Common.string());
    }
    //
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];
        status_t status = OK;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, mInfo.getSensorDrvName());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, metadata);
        if  ( OK == status ) {
            vMap[i] = true;
            continue;
        }
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, metadata);
        if  ( OK == status ) {
            vMap[i] = true;
            continue;
        }
        //
        META_LOGE_IF(0, "Fail for both %s & %s", s8Symbol_Sensor.string(), s8Symbol_Common.string());
    }
    //
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++) {
        if ( vMap[i]==false ) {
            META_LOGE("Fail to load %s in all PLATFORM/PROJECT combinations", kStaticMetadataTypeNames[i] );
            return NAME_NOT_FOUND;
        }
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
constructStaticMetadata(sp<IMetadataConverter> pConverter, camera_metadata*& rpDstMetadata, IMetadata& mtkMetadata)
{
    FUNC_PROFILE_START(__FUNCTION__);
    META_LOGD("construct static metadata\n");

    status_t status = OK;
    //-----(1)-----//
    //get static informtation from customization (with camera_metadata format)
    //calculate its entry count and data count
    if  ( OK != (status = impConstructStaticMetadata(mtkMetadata)) ) {
        META_LOGE("Unable evaluate the size for camera static info - status[%s(%d)]\n", ::strerror(-status), -status);
        return  status;
    }
    META_LOGD("Allocating %d entries from customization", mtkMetadata.count());
    if ( mLogLevel>=2 )
        pConverter->dumpAll(mtkMetadata, -1);

    //-----(2.1)------//
    //get static informtation from sensor hal moduls (with IMetadata format)
    IMetadata sensorMetadata = MAKE_HalLogicalDeviceList()->queryStaticInfo(mInfo.getDeviceId());
    META_LOGD("Allocating %d entries from sensor HAL", sensorMetadata.count());
    if ( mLogLevel>=2 )
        pConverter->dumpAll(sensorMetadata, -1);

    //
#if 1
    auto pHalDeviceList = MAKE_HalLogicalDeviceList();
    if( CC_UNLIKELY( pHalDeviceList==nullptr ) ) {
        META_LOGE("get HalLogicalDeviceList fail!");
        return BAD_VALUE;
    }
    auto physicIdsList = pHalDeviceList->getSensorId(mInfo.getDeviceId());
    auto pDngInfo = MAKE_DngInfo(LOG_TAG, physicIdsList[0]);
    if( CC_UNLIKELY( pDngInfo == nullptr ) ) {
        META_LOGE("get DngInfo fail!");
        return BAD_VALUE;
    }
    IMetadata rDngMeta = pDngInfo->getStaticMetadata();
#else
    IMetadata rDngMeta;
#endif
    META_LOGD("Allocating %d entries from Dng Info", rDngMeta.count());
    if ( mLogLevel>=2 )
        pConverter->dumpAll(rDngMeta, -1);

    //--- (2.1.1) --- //
    //merge.
    sensorMetadata += rDngMeta;
    for (size_t i = 0; i < sensorMetadata.count(); i++)
    {
        IMetadata::Tag_t mTag = sensorMetadata.entryAt(i).tag();
        mtkMetadata.update(mTag, sensorMetadata.entryAt(i));
    }
    META_LOGD("Allocating %d entries from customization + sensor HAL + Dng Info", mtkMetadata.count());
    if ( mLogLevel>=1 )
        pConverter->dumpAll(mtkMetadata, -1);

#if 0
    //-----(2.2)------//
    //get static informtation from other hal moduls (with IMetadata format)
    IMetadata halmetadata = MAKE_HalLogicalDeviceList()->queryStaticInfo(mInfo.getDeviceId());

    //calculate its entry count and data count
    entryCount = 0;
    dataCount = 0;


    status = AndroidMetadata::getIMetadata_dataCount(halmetadata, entryCount, dataCount);
    if (status != OK)
    {
        META_LOGE("get Imetadata count error - status[%s(%d)", ::strerror(-status), -status);
        return status;
    }

    META_LOGD(
        "Allocating %d entries, %d extra bytes from HAL modules",
        entryCount, dataCount
    );

    addOrSizeInfo.mEntryCount += entryCount;
    addOrSizeInfo.mDataCount += dataCount;

#endif
    //overwrite
    updateData(mtkMetadata);
    //
    //validate the constructed metatata
    validation(mtkMetadata);
    #if (PLATFORM_SDK_VERSION >= 21)
    pConverter->convert(mtkMetadata, rpDstMetadata);
    //
    ::sort_camera_metadata(rpDstMetadata);
    #endif

    FUNC_PROFILE_END();
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
template<class T>
struct converter {
    converter( T const& tag, T const& srcFormat, T const& dstFormat, IMetadata& data) {
        IMetadata::IEntry entry = data.entryFor(tag);
        copy( srcFormat, dstFormat, entry);
        data.update(tag, entry);
    }

    void copy( T const& srcFormat, T const& dstFormat, IMetadata::IEntry& entry) {
        T input = MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_INPUT;
        for(size_t i = 0; i < entry.count(); i+=4) {
            if (entry.itemAt(i, Type2Type<T>())!= srcFormat
                || entry.itemAt(i+3, Type2Type< T >()) == input) {
                continue;
            }
            entry.push_back(dstFormat, Type2Type< T >());
            entry.push_back(entry.itemAt(i+1, Type2Type< T >()), Type2Type< T >());
            entry.push_back(entry.itemAt(i+2, Type2Type< T >()), Type2Type< T >());
            entry.push_back(entry.itemAt(i+3, Type2Type< T >()), Type2Type< T >());
        }
    };
};

void
MetadataProvider::
updateData(IMetadata &rMetadata)
{
    updateStreamConfiguration(rMetadata);
    updateRecommendedStreamConfiguration(rMetadata);
    updateAfRegions(rMetadata);

    {
        MINT32 jpegYuv420Support = 0;
        jpegYuv420Support = property_get_int32("vendor.debug.camera.Jpeg.YUV420.support", 1);
        MINT32 maxJpegsize = 0;
        IMetadata::IEntry blobEntry = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
        for(size_t i = 0; i < blobEntry.count(); i+=4) {
            if (blobEntry.itemAt(i, Type2Type<MINT32>())!= HAL_PIXEL_FORMAT_BLOB) {
                continue;
            }
            //available blob size list should order in descedning.
            MSize maxBlob = MSize(blobEntry.itemAt(i+1, Type2Type<MINT32>()),
                            blobEntry.itemAt(i+2, Type2Type<MINT32>()));
            MINT32 jpegsize = (jpegYuv420Support == 1) ? (maxBlob.size()*15)/10 : maxBlob.size()*2;
#if (3==MTKCAM_HAL_VERSION && 0==IS_BUILD_MTK_LDVT)
            auto outBufList = StereoSettingProvider::getBokehJpegBufferList();
            auto physicalSensorList = MAKE_HalLogicalDeviceList()->getSensorId(mInfo.getDeviceId());
            auto featureMode = MAKE_HalLogicalDeviceList()->getSupportedFeature(mInfo.getDeviceId());
            bool isNeedPackJpeg = ((physicalSensorList.size() > 1)&&(featureMode & DEVICE_FEATURE_VSDOF))? 1 : 0 ;
            if(outBufList.count() > 1 && isNeedPackJpeg)
            {
                // [Jpeg pack] for jpeg pack requirement.
                jpegsize = (jpegYuv420Support == 1) ? maxBlob.size()*4 : maxBlob.size()*5;
                META_LOGD("dual cam device, enlarge size. id(%d) size (%d)", mInfo.getDeviceId(), jpegsize);
            }
            if (jpegsize > maxJpegsize) {
                maxJpegsize = jpegsize;
            }
#else
            if (jpegsize > maxJpegsize) {
                maxJpegsize = jpegsize;
            }
#endif
            IMetadata::IEntry entry(MTK_JPEG_MAX_SIZE);
            entry.push_back(maxJpegsize, Type2Type< MINT32 >());
            rMetadata.update(MTK_JPEG_MAX_SIZE, entry);
         }
    }

    // update implementation defined
    {
        converter<MINT32>(
            MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
            HAL_PIXEL_FORMAT_YCbCr_420_888, HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED,
            rMetadata
        );
        //
        converter<MINT64>(
            MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS,
            HAL_PIXEL_FORMAT_YCbCr_420_888, HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED,
            rMetadata
        );
        //
        converter<MINT64>(
            MTK_SCALER_AVAILABLE_STALL_DURATIONS,
            HAL_PIXEL_FORMAT_YCbCr_420_888, HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED,
            rMetadata
        );
    }

    // update yv12
    {
        converter<MINT32>(
            MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
            HAL_PIXEL_FORMAT_YCbCr_420_888, HAL_PIXEL_FORMAT_YV12,
            rMetadata
        );
        //
        converter<MINT64>(
            MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS,
            HAL_PIXEL_FORMAT_YCbCr_420_888, HAL_PIXEL_FORMAT_YV12,
            rMetadata
        );
        //
        converter<MINT64>(
            MTK_SCALER_AVAILABLE_STALL_DURATIONS,
            HAL_PIXEL_FORMAT_YCbCr_420_888, HAL_PIXEL_FORMAT_YV12,
            rMetadata
        );
    }

    // update HDR Request Common Type
    {
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_HDR_FEATURE_HDR_MODE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);

        availReqEntry.push_back(MTK_HDR_FEATURE_SESSION_PARAM_HDR_MODE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);

        IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        availResultEntry.push_back(MTK_HDR_FEATURE_HDR_DETECTION_RESULT , Type2Type< MINT32 >());
        rMetadata.update(availResultEntry.tag(), availResultEntry);

        IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
        availCharactsEntry.push_back(MTK_HDR_FEATURE_AVAILABLE_HDR_MODES_PHOTO , Type2Type< MINT32 >());
        availCharactsEntry.push_back(MTK_HDR_FEATURE_AVAILABLE_HDR_MODES_VIDEO , Type2Type< MINT32 >());
        availCharactsEntry.push_back(MTK_HDR_FEATURE_AVAILABLE_MSTREAM_HDR_MODES , Type2Type< MINT32 >());
        rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
    }

    {
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_EIS_FEATURE_EIS_MODE, Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }

    {
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_EIS_FEATURE_PREVIEW_EIS, Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }

    // update Available vHDR Mode & HDR Modes
    {

        // -----------------
        // -- isHDRSensor --
        // -----------------
        // Available vHDR mode
        IMetadata::IEntry availVhdrEntry = rMetadata.entryFor(MTK_HDR_FEATURE_AVAILABLE_VHDR_MODES);
        if(availVhdrEntry.isEmpty()){
            IMetadata::IEntry entry(MTK_HDR_FEATURE_AVAILABLE_VHDR_MODES);
            entry.push_back(MTK_HDR_FEATURE_VHDR_MODE_OFF, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
            availVhdrEntry = entry;
        }

        IMetadata::IEntry availMStreamEntry = rMetadata.entryFor(MTK_HDR_FEATURE_AVAILABLE_MSTREAM_HDR_MODES);
        if(availMStreamEntry.isEmpty()){
            IMetadata::IEntry entry(MTK_HDR_FEATURE_AVAILABLE_MSTREAM_HDR_MODES);
            entry.push_back(MTK_HDR_FEATURE_HDR_HAL_MODE_OFF, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
            availMStreamEntry = entry;
        }
        MBOOL isHDRSensor = isHdrSensor(availVhdrEntry.count());

        // --------------------------
        // -- isSingleFrameSupport --
        // --------------------------
        IMetadata::IEntry singleFrameHdrEntry = rMetadata.entryFor(MTK_HDR_FEATURE_AVAILABLE_SINGLE_FRAME_HDR);
        MBOOL isSingleFrameSupport = (singleFrameHdrEntry.count() > 0)
                    && (singleFrameHdrEntry.itemAt(0, Type2Type<MUINT8>()) == MTK_HDR_FEATURE_SINGLE_FRAME_HDR_SUPPORTED);
        MINT32 singleFrameProp = property_get_int32("debug.camera.hal3.singleFrame", -1);
        isSingleFrameSupport = (singleFrameProp != -1) ? (singleFrameProp > 0) : isSingleFrameSupport;

        // ----------------------
        // -- hdrDetectionMode --
        // ----------------------
        MINT32 hdrDetectionMode = MTKCAM_HDR_DETECTION_MODE; /* 1 : hdr sensor,  2 : generic sensor, 3 : all sensors*/
        // HDR Detection support force switch
        MINT32 hdrDetectProp = property_get_int32("debug.camera.hal3.hdrDetection", 0);
        hdrDetectionMode = (hdrDetectProp != -1) ? hdrDetectProp : hdrDetectionMode;


        // update availHdrPhoto & availHdrVideo Metadata
        updateHdrData(isHDRSensor, isSingleFrameSupport, hdrDetectionMode, rMetadata);

    }

#if 1 //MTKCAM_HAVE_MFB_SUPPORT fill Default value = off even MFB is not support
    // update AIS Request Common Type
    META_LOGD("MTKCAM_HAVE_MFB_SUPPORT = %d", MTKCAM_HAVE_MFB_SUPPORT);
    IMetadata::IEntry availAisModeEntry = rMetadata.entryFor(MTK_MFNR_FEATURE_AVAILABLE_AIS_MODES);
    if(availAisModeEntry.isEmpty()){
        META_LOGD("availAisModeEntry is empty");
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_MFNR_FEATURE_AIS_MODE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);

        IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        availResultEntry.push_back(MTK_MFNR_FEATURE_AIS_RESULT , Type2Type< MINT32 >());
        rMetadata.update(availResultEntry.tag(), availResultEntry);

        IMetadata::IEntry entry(MTK_MFNR_FEATURE_AVAILABLE_AIS_MODES);
        entry.push_back(MTK_MFNR_FEATURE_AIS_OFF , Type2Type< MINT32 >());
        rMetadata.update(entry.tag(), entry);
    }

    // update MFB Request Common Type
    IMetadata::IEntry availMfbModeEntry = rMetadata.entryFor(MTK_MFNR_FEATURE_AVAILABLE_MFB_MODES);
    if(availMfbModeEntry.isEmpty()){
        META_LOGD("availMfbModeEntry is empty");
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_MFNR_FEATURE_MFB_MODE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);

        IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        availResultEntry.push_back(MTK_MFNR_FEATURE_MFB_RESULT , Type2Type< MINT32 >());
        rMetadata.update(availResultEntry.tag(), availResultEntry);

        IMetadata::IEntry entry(MTK_MFNR_FEATURE_AVAILABLE_MFB_MODES);
        entry.push_back(MTK_MFNR_FEATURE_MFB_OFF , Type2Type< MINT32 >());
#if (MTKCAM_HAVE_MFB_SUPPORT == 1)
        entry.push_back(MTK_MFNR_FEATURE_MFB_MFLL , Type2Type< MINT32 >());
        entry.push_back(MTK_MFNR_FEATURE_MFB_AUTO , Type2Type< MINT32 >());
#elif (MTKCAM_HAVE_MFB_SUPPORT == 2)
        entry.push_back(MTK_MFNR_FEATURE_MFB_AIS , Type2Type< MINT32 >());
        entry.push_back(MTK_MFNR_FEATURE_MFB_AUTO , Type2Type< MINT32 >());
#elif (MTKCAM_HAVE_MFB_SUPPORT == 3)
        entry.push_back(MTK_MFNR_FEATURE_MFB_MFLL , Type2Type< MINT32 >());
        entry.push_back(MTK_MFNR_FEATURE_MFB_AIS , Type2Type< MINT32 >());
        entry.push_back(MTK_MFNR_FEATURE_MFB_AUTO , Type2Type< MINT32 >());
#endif
        rMetadata.update(entry.tag(), entry);
    }
#endif // MTKCAM_HAVE_MFB_SUPPORT

    // update Streaming Request Common Type
    {
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_STREAMING_FEATURE_RECORD_STATE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);

        IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        availResultEntry.push_back(MTK_STREAMING_FEATURE_RECORD_STATE , Type2Type< MINT32 >());
        rMetadata.update(availResultEntry.tag(), availResultEntry);

        IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
        availCharactsEntry.push_back(MTK_STREAMING_FEATURE_AVAILABLE_RECORD_STATES , Type2Type< MINT32 >());
        rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
    }
    // update Streaming Available EIS ControlFlow
    {
        IMetadata::IEntry availRecordEntry = rMetadata.entryFor(MTK_STREAMING_FEATURE_AVAILABLE_RECORD_STATES);
        if(availRecordEntry.isEmpty()){
            IMetadata::IEntry entry(MTK_STREAMING_FEATURE_AVAILABLE_RECORD_STATES);
            entry.push_back(MTK_STREAMING_FEATURE_RECORD_STATE_PREVIEW, Type2Type< MINT32 >());
            entry.push_back(MTK_STREAMING_FEATURE_RECORD_STATE_RECORD, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
        }
    }

    {
        // add BGService REQUEST key
        IMetadata::IEntry availRequestEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availRequestEntry.push_back(MTK_BGSERVICE_FEATURE_IMAGEREADERID , Type2Type< MINT32 >());
        rMetadata.update(availRequestEntry.tag(), availRequestEntry);
        // add BGService result key
        IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        availResultEntry.push_back(MTK_BGSERVICE_FEATURE_IMAGEREADERID , Type2Type< MINT32 >());
        rMetadata.update(availResultEntry.tag(), availResultEntry);
        // add BGService session key
        IMetadata::IEntry sessionKeyEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
        if (!sessionKeyEntry.isEmpty())
        {
            sessionKeyEntry.push_back(MTK_BGSERVICE_FEATURE_IMAGEREADERID , Type2Type< MINT32 >());
            rMetadata.update(sessionKeyEntry.tag(), sessionKeyEntry);
        }
        else
        {
            IMetadata::IEntry entry(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
            entry.push_back(MTK_BGSERVICE_FEATURE_IMAGEREADERID, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
        }
    }

    {
        // add init_request REQUEST key
        IMetadata::IEntry availRequestEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availRequestEntry.push_back(MTK_CONFIGURE_SETTING_INIT_REQUEST , Type2Type< MINT32 >());
        availRequestEntry.push_back(MTK_CONFIGURE_SETTING_PROPRIETARY  , Type2Type< MINT32 >());
        rMetadata.update(availRequestEntry.tag(), availRequestEntry);
        // add init_request SESSION key
        IMetadata::IEntry sessionKeyEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
        if (!sessionKeyEntry.isEmpty())
        {
            sessionKeyEntry.push_back(MTK_CONFIGURE_SETTING_INIT_REQUEST , Type2Type< MINT32 >());
            //
            sessionKeyEntry.push_back(MTK_CONFIGURE_SETTING_PROPRIETARY, Type2Type< MINT32 >());
            //
            rMetadata.update(sessionKeyEntry.tag(), sessionKeyEntry);
        }
        else
        {
            IMetadata::IEntry entry(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
            entry.push_back(MTK_CONFIGURE_SETTING_INIT_REQUEST, Type2Type< MINT32 >());
            //
            entry.push_back(MTK_CONFIGURE_SETTING_PROPRIETARY, Type2Type< MINT32 >());
            //
            rMetadata.update(entry.tag(), entry);
        }
    }

    {
        // add hint for raw reprocess key
        IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        availResultEntry.push_back(MTK_CONTROL_CAPTURE_HINT_FOR_RAW_REPROCESS, Type2Type< MINT32 >());
        rMetadata.update(availResultEntry.tag(), availResultEntry);
    }

    // add per-frame request enable/disable option for different capture feature
    {
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_SINGLE_YUV_NR , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }
    {
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_HIGH_QUALITY_YUV , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }
    //
    {
        // add hint for isp tuning for different feature
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }
    {
        // frame count hint for isp hidl processing
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }
    {
        // frame index hint for isp hidl processing
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }
    {
        // frame tuning index for isp hidl processing
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }

    // packed raw
    {
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_PACKED_RAW_ENABLE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }

    // process raw
    {
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }

    // bbp of raw buffer
    {
        // request key
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        availReqEntry.push_back(MTK_CONTROL_CAPTURE_RAW_BPP , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
    }

    {
        // add info for isp meta stream

        // SESSION key (enable/disable)
        IMetadata::IEntry sessionKeyEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
        if (!sessionKeyEntry.isEmpty())
        {
            sessionKeyEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE , Type2Type< MINT32 >());
            rMetadata.update(sessionKeyEntry.tag(), sessionKeyEntry);
            META_LOGD("update MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE");
        }
        else
        {
            IMetadata::IEntry entry(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
            entry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
            META_LOGD("push MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE");
        }

        {
            // add tuning data output for ISP hidl
            IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
            availResultEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_REQUEST, Type2Type< MINT32 >());
            availResultEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_RAW, Type2Type< MINT32 >());
            availResultEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_YUV, Type2Type< MINT32 >());
            rMetadata.update(availResultEntry.tag(), availResultEntry);
        }

        {
            // add tuning data output for ISP hidl
            IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
            availReqEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_REQUEST , Type2Type< MINT32 >());
            availReqEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE , Type2Type< MINT32 >());
            rMetadata.update(availReqEntry.tag(), availReqEntry);
        }

        // update (RAW / YUV) isp meta stream size (APP need to know the size)
        // for FD and pipeline common tag
        {
            // add result key
            IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
            availResultEntry.push_back(MTK_FACE_FEATURE_FACE_ADDITIONAL_RESULT , Type2Type< MINT32 >());
            rMetadata.update(availResultEntry.tag(), availResultEntry);
        }

        // characteristics keys
        IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
        availCharactsEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_RAW, Type2Type< MINT32 >());
        availCharactsEntry.push_back(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_YUV, Type2Type< MINT32 >());
        rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
        // update the RAW isp meta stream default size (if there is no default value)
        IMetadata::IEntry availRawIspTuningDataSizeEntry = rMetadata.entryFor(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_RAW);
        if (availRawIspTuningDataSizeEntry.isEmpty())
        {
            IMetadata::IEntry entry(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_RAW);
            entry.push_back(2560, Type2Type< MINT32 >());
            entry.push_back(1920, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
        }
        // update the YUV isp meta stream default size (if there is no default value)
        IMetadata::IEntry availYuvIspTuningDataSizeEntry = rMetadata.entryFor(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_YUV);
        if (availYuvIspTuningDataSizeEntry.isEmpty())
        {
            IMetadata::IEntry entry(MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_SIZE_FOR_YUV);
            entry.push_back(1280, Type2Type< MINT32 >());
            entry.push_back(720, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
        }
    }

#ifndef EIS_SUPPORTED
    //update EIS support
    {
        IMetadata::IEntry availEISEntry = rMetadata.entryFor(MTK_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES);
        if (!availEISEntry.isEmpty())
        {
            availEISEntry.clear();
            availEISEntry.push_back(MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF, Type2Type< MUINT8 >());
            rMetadata.update(availEISEntry.tag(), availEISEntry);
        }
        IMetadata::IEntry availAdvEISEntry = rMetadata.entryFor(MTK_EIS_FEATURE_EIS_MODE);
        if (!availAdvEISEntry.isEmpty())
        {
            availAdvEISEntry.clear();
            availAdvEISEntry.push_back(MTK_EIS_FEATURE_EIS_MODE_OFF, Type2Type< MINT32 >());
            rMetadata.update(availAdvEISEntry.tag(), availAdvEISEntry);
        }
        IMetadata::IEntry availPrevEISEntry = rMetadata.entryFor(MTK_EIS_FEATURE_PREVIEW_EIS);
        if (!availPrevEISEntry.isEmpty())
        {
            availPrevEISEntry.clear();
            availPrevEISEntry.push_back(MTK_EIS_FEATURE_PREVIEW_EIS_OFF, Type2Type< MINT32 >());
            rMetadata.update(availPrevEISEntry.tag(), availPrevEISEntry);
        }
    }
#else
    {
        IMetadata::IEntry availSessionEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
        if (!availSessionEntry.isEmpty())
        {
            availSessionEntry.push_back(MTK_CONTROL_VIDEO_STABILIZATION_MODE, Type2Type< MINT32 >());
            availSessionEntry.push_back(MTK_EIS_FEATURE_EIS_MODE, Type2Type< MINT32 >());
            availSessionEntry.push_back(MTK_EIS_FEATURE_PREVIEW_EIS, Type2Type< MINT32 >());
            rMetadata.update(availSessionEntry.tag(), availSessionEntry);
        }
        else
        {
            IMetadata::IEntry entry(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
            entry.push_back(MTK_CONTROL_VIDEO_STABILIZATION_MODE, Type2Type< MINT32 >());
            entry.push_back(MTK_EIS_FEATURE_EIS_MODE, Type2Type< MINT32 >());
            entry.push_back(MTK_EIS_FEATURE_PREVIEW_EIS, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
        }
    }
#endif

    // SMVRBatch
    {

        IMetadata::IEntry availSMVRBatchEntry = rMetadata.entryFor(MTK_SMVR_FEATURE_AVAILABLE_SMVR_MODES);
//        rMetadata.dump();

        META_LOGD_IF(1, "SMVRBatch: MTK_SMVR_FEATURE_AVAILABLE_SMVR_MODES.count()=%d",
            availSMVRBatchEntry.count());

        if (!availSMVRBatchEntry.isEmpty())
        {
            if (availSMVRBatchEntry.count() >= 4)
            {
                for (int i = 0; i < availSMVRBatchEntry.count(); i+=4)
                {
                    int32_t width = availSMVRBatchEntry.itemAt(i, Type2Type<MINT32>());
                    int32_t height = availSMVRBatchEntry.itemAt(i+1, Type2Type<MINT32>());
                    int32_t maxFps = availSMVRBatchEntry.itemAt(i+2, Type2Type<MINT32>());
                    int32_t p2CustomBatchNum = availSMVRBatchEntry.itemAt(i+3, Type2Type<MINT32>());

                    META_LOGD("SMVRBatch: %dx%d, maxFps=%d, p2CustomBatchNum=%d", width, height, maxFps, p2CustomBatchNum);
                }
                // update avail-smvr-modes again, mkdbg-todo: this is not necessary?
                rMetadata.update(availSMVRBatchEntry.tag(), availSMVRBatchEntry);

                // update characteristics keys
                IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
                availCharactsEntry.push_back(MTK_SMVR_FEATURE_AVAILABLE_SMVR_MODES, Type2Type< MINT32 >());
                rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);

                // update avail session keys
                IMetadata::IEntry availSessionEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
                if (!availSessionEntry.isEmpty())
                {
                    availSessionEntry.push_back(MTK_SMVR_FEATURE_SMVR_MODE, Type2Type< MINT32 >());
                    rMetadata.update(availSessionEntry.tag(), availSessionEntry);
                }
                else
                {
                    IMetadata::IEntry entry(MTK_REQUEST_AVAILABLE_SESSION_KEYS);
                    entry.push_back(MTK_SMVR_FEATURE_SMVR_MODE, Type2Type< MINT32 >());
                    rMetadata.update(entry.tag(), entry);
                }

            }
        }
    }
    // update multi-cam feature mode to static metadata
    // vendor tag
    {
        //
        auto pHalDeviceList = MAKE_HalLogicalDeviceList();
        if( CC_UNLIKELY( pHalDeviceList == nullptr ) ) {
            META_LOGE("get HalLogicalDeviceList fail!");
            return;
        }
        auto physicIdsList = pHalDeviceList->getSensorId(
                                    mInfo.getDeviceId());
        // only add MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED in single cam device.
        if(physicIdsList.size() == 1)
        {
            // for multi-cam logica device added metadata.
            // to store manual update metadata for sensor driver.
            IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
            availCharactsEntry.push_back(MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED , Type2Type< MINT32 >());
            rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
        }
        if(physicIdsList.size() > 1)
        {
            IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
            IMetadata::IEntry availRequestEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
            IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
            IMetadata::IEntry sessionKeyEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_SESSION_KEYS);

            availRequestEntry.push_back(MTK_CONTROL_CAPTURE_YUV_NO_MARGIN , Type2Type< MINT32 >());
            availResultEntry.push_back(MTK_CONTROL_CAPTURE_YUV_NO_MARGIN , Type2Type< MINT32 >());

            MBOOL needAddCharactersticsKeys = MFALSE;
            auto supportedFeature = pHalDeviceList->getSupportedFeature(mInfo.getDeviceId());
            // add feature mode
            {
                IMetadata::IEntry entry(MTK_MULTI_CAM_FEATURE_AVAILABLE_MODE);
                if(supportedFeature & DEVICE_FEATURE_ZOOM)
                {
                    META_LOGD("deviceid(%d) support zoom feature", mInfo.getDeviceId());
                    entry.push_back(MTK_MULTI_CAM_FEATURE_MODE_ZOOM, Type2Type< MINT32 >());
                    needAddCharactersticsKeys = MTRUE;
                    // zoom range
                    {
                        IMetadata::IEntry entry(MTK_MULTI_CAM_ZOOM_RANGE);
                        #if (3==MTKCAM_HAL_VERSION)
                            // before call getMulticamZoomRange, it has to call setLogicalDeviceID.
                            auto rangeArr = StereoSettingProvider::getMulticamZoomRange(mInfo.getDeviceId());
                            META_LOGD("set zoom range min(%f) max(%f)", rangeArr[0], rangeArr.back());
                            entry.push_back(rangeArr[0], Type2Type< MFLOAT >());
                            entry.push_back(rangeArr.back(), Type2Type< MFLOAT >());
                        #else
                            META_LOGD("set default zoom range");
                            entry.push_back(1.0f, Type2Type< MFLOAT >());
                            entry.push_back(1.0f, Type2Type< MFLOAT >());
                        #endif
                        rMetadata.update(entry.tag(), entry);
                    }
                    // zoom step
                    {
                        IMetadata::IEntry entry(MTK_MULTI_CAM_ZOOM_STEPS);
                        #if (3==MTKCAM_HAL_VERSION&& 0==IS_BUILD_MTK_LDVT)
                            auto stepArr = StereoSettingProvider::getMulticamZoomSteps(mInfo.getDeviceId());
                            if(stepArr.size() > 0)
                            {
                                for(auto&& step:stepArr)
                                {
                                    entry.push_back(step, Type2Type< MFLOAT >());
                                }
                            }
                            else
                            {
                                entry.push_back(1.0f, Type2Type< MFLOAT >());
                            }
                        #else
                            entry.push_back(1.0f, Type2Type< MFLOAT >());
                        #endif
                        rMetadata.update(entry.tag(), entry);
                    }

                    availCharactsEntry.push_back(MTK_MULTI_CAM_ZOOM_RANGE , Type2Type< MINT32 >());
                    availCharactsEntry.push_back(MTK_MULTI_CAM_ZOOM_STEPS , Type2Type< MINT32 >());
                    availRequestEntry.push_back(MTK_MULTI_CAM_ZOOM_VALUE , Type2Type< MINT32 >());
                    availResultEntry.push_back(MTK_MULTI_CAM_STREAMING_ID , Type2Type< MINT32 >());
                    availResultEntry.push_back(MTK_MULTI_CAM_SENSOR_CROP_REGION, Type2Type< MINT32 >());
                }
                if(supportedFeature & DEVICE_FEATURE_VSDOF)
                {
                    META_LOGD("deviceid(%d) support vsdof feature", mInfo.getDeviceId());
                    entry.push_back(MTK_MULTI_CAM_FEATURE_MODE_VSDOF, Type2Type< MINT32 >());
                    needAddCharactersticsKeys = MTRUE;
                }
                if(supportedFeature & DEVICE_FEATURE_DENOISE)
                {
                    META_LOGD("deviceid(%d) support denoise feature", mInfo.getDeviceId());
                    entry.push_back(MTK_MULTI_CAM_FEATURE_MODE_DENOISE, Type2Type< MINT32 >());
                    needAddCharactersticsKeys = MTRUE;
                }
                rMetadata.update(entry.tag(), entry);
                if(needAddCharactersticsKeys)
                {
                    META_LOGD("Add charactersticsKeys for feature mode (v1.1)");

                    availCharactsEntry.push_back(MTK_MULTI_CAM_FEATURE_AVAILABLE_MODE , Type2Type< MINT32 >());
                    availRequestEntry.push_back(MTK_MULTI_CAM_FEATURE_MODE , Type2Type< MINT32 >());
                    sessionKeyEntry.push_back(MTK_MULTI_CAM_FEATURE_MODE , Type2Type< MINT32 >());
                }
            }
            // for vsdof specificy.
            if(supportedFeature & DEVICE_FEATURE_VSDOF)
            {
                // if vsdof mode, add preview mode(full/half)
                IMetadata::IEntry entry(MTK_VSDOF_FEATURE_AVAILABLE_PREVIEW_MODE);
                entry.push_back(MTK_VSDOF_FEATURE_PREVIEW_MODE_FULL, Type2Type< MINT32 >());
                entry.push_back(MTK_VSDOF_FEATURE_PREVIEW_MODE_HALF, Type2Type< MINT32 >());
                rMetadata.update(entry.tag(), entry);
                //MTK_STEREO_FEATURE_SUPPORTED_DOF_LEVEL
                IMetadata::IEntry entry1(MTK_STEREO_FEATURE_SUPPORTED_DOF_LEVEL);
                entry1.push_back(15, Type2Type< MINT32 >()); // workaround: temp write 15
                rMetadata.update(entry1.tag(), entry1);
                // add Characterstics key
                META_LOGD("Add characterstics keys for vsdof(1.0)");
                availCharactsEntry.push_back(MTK_VSDOF_FEATURE_AVAILABLE_PREVIEW_MODE , Type2Type< MINT32 >());
                availCharactsEntry.push_back(MTK_STEREO_FEATURE_SUPPORTED_DOF_LEVEL , Type2Type< MINT32 >());
                // add request key
                META_LOGD("Add request keys for vsdof(1.2)");
                availRequestEntry.push_back(MTK_STEREO_FEATURE_DOF_LEVEL , Type2Type< MINT32 >());
                availRequestEntry.push_back(MTK_VSDOF_FEATURE_PREVIEW_MODE , Type2Type< MINT32 >());
                availRequestEntry.push_back(MTK_VSDOF_FEATURE_PREVIEW_SIZE , Type2Type< MINT32 >());
                availRequestEntry.push_back(MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG , Type2Type< MINT32 >());
                // add result key
                META_LOGD("Add result keys for vsdof(1.2)");
                availResultEntry.push_back(MTK_STEREO_FEATURE_WARNING , Type2Type< MINT32 >());
                availResultEntry.push_back(MTK_STEREO_FEATURE_RESULT_DOF_LEVEL , Type2Type< MINT32 >());
                availResultEntry.push_back(MTK_VSDOF_FEATURE_WARNING, Type2Type< MINT32 >());
                availResultEntry.push_back(MTK_MULTI_CAM_AF_ROI, Type2Type< MINT32 >());
                availResultEntry.push_back(MTK_MULTI_CAM_MASTER_ID, Type2Type< MINT32 >());
                availResultEntry.push_back(MTK_MULTI_CAM_FOV_CROP_REGION, Type2Type< MINT32 >());
                // add session key
                META_LOGD("Add session key for vsdof(1.0)");
                sessionKeyEntry.push_back(MTK_VSDOF_FEATURE_PREVIEW_MODE , Type2Type< MINT32 >());
                sessionKeyEntry.push_back(MTK_VSDOF_FEATURE_PREVIEW_SIZE , Type2Type< MINT32 >());
            }

            rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
            rMetadata.update(availRequestEntry.tag(), availRequestEntry);
            rMetadata.update(availResultEntry.tag(), availResultEntry);
            rMetadata.update(sessionKeyEntry.tag(), sessionKeyEntry);
        }
    }
    // update logic device related metadata tag
#if 1 // disable multi-cam logical device setting
    {
        auto pHalDeviceList = MAKE_HalLogicalDeviceList();
        if(  CC_UNLIKELY( pHalDeviceList == nullptr ) ) {
            META_LOGE("get HalLogicalDeviceList fail!");
            return;
        }
        auto physicIdsList = pHalDeviceList->getRemappingSensorId(
                                    mInfo.getDeviceId());
        if(physicIdsList.size() > 1)
        {
            // support multi-cam
            if(pHalDeviceList->isLogicalMulticam(mInfo.getDeviceId()))
            {
                META_LOGD("add request available capabilities.");
                IMetadata::IEntry entry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CAPABILITIES);
                entry.push_back(MTK_REQUEST_AVAILABLE_CAPABILITIES_LOGICAL_MULTI_CAMERA , Type2Type< MUINT8 >());
                rMetadata.update(entry.tag(), entry);

                // Device 3.5: add active physical ID
                IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
                availReqEntry.push_back(MTK_LOGICAL_MULTI_CAMERA_ACTIVE_PHYSICAL_ID , Type2Type< MINT32 >());
                rMetadata.update(availReqEntry.tag(), availReqEntry);
            }
            // update logic physic ids
            {
                std::string idsListString;
                IMetadata::IEntry entry(MTK_LOGICAL_MULTI_CAMERA_PHYSICAL_IDS);
                for(auto id : physicIdsList)
                {
                    // ascii
                    std::string id_to_str = std::to_string(id);
                    for(auto&& c:id_to_str)
                    {
                        entry.push_back(c , Type2Type< MUINT8 >());
                    }
                    entry.push_back('\0' , Type2Type< MUINT8 >());
                    idsListString += id_to_str;
                    idsListString += " ";
                }
                META_LOGD("update logic id (%s:%x)", idsListString.c_str(), MTK_LOGICAL_MULTI_CAMERA_PHYSICAL_IDS);
                rMetadata.update(entry.tag(), entry);
                // add characteristics key
                IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
                availCharactsEntry.push_back(MTK_LOGICAL_MULTI_CAMERA_PHYSICAL_IDS , Type2Type< MINT32 >());
                rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
            }
            // update sync type
            {
                auto syncType = pHalDeviceList->getSyncType(
                                    mInfo.getDeviceId());
                if(SensorSyncType::NOT_SUPPORT != syncType)
                {
                    MUINT8 value = MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE_APPROXIMATE;
                    switch(syncType)
                    {
                        case SensorSyncType::APPROXIMATE:
                            value = MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE_APPROXIMATE;
                            break;
                        case SensorSyncType::CALIBRATED:
                            value = MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE_CALIBRATED;
                            break;
                        default:
                            META_LOGE("invaild sync type");
                            break;
                    }
                    IMetadata::IEntry entry(MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE);
                    entry.push_back(value, Type2Type< MUINT8 >());
                    rMetadata.update(entry.tag(), entry);
                    auto toString = [&value]()
                    {
                        switch(value)
                        {
                            case MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE_APPROXIMATE:
                                return "Approximate";
                            case MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE_CALIBRATED:
                                return "Calibrated";
                            default:
                                return "not support";
                        }
                    };
                    META_LOGD("update sync type (%s:%x)", toString(), MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE);
                    // add characteristics key
                    IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
                    availCharactsEntry.push_back(MTK_LOGICAL_MULTI_CAMERA_SENSOR_SYNC_TYPE , Type2Type< MINT32 >());
                    rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
                }
            }
            // add available result key for multicam
            {
                IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
                availResultEntry.push_back(MTK_LOGICAL_MULTI_CAMERA_ACTIVE_PHYSICAL_ID , Type2Type< MINT32 >());
                rMetadata.update(availResultEntry.tag(), availResultEntry);
            }
            // add available request key for multicam
            {
                IMetadata::IEntry availRequestEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
                availRequestEntry.push_back(MTK_LOGICAL_MULTI_CAMERA_ACTIVE_PHYSICAL_ID , Type2Type< MINT32 >());
                rMetadata.update(availRequestEntry.tag(), availRequestEntry);
            }
        }
    }
#endif
    //  update lens calibration data
    ICalibrationProvider *calProvider = ICalibrationProvider::getInstance(mInfo.getDeviceId());
    if(calProvider){

        CalibrationResultInGoogleFormat calData;
        bool hasCalibration = calProvider->getCalibration(E_CALIBRATION_GOOGLE_FORMAT, &calData);
        if(hasCalibration)
        {
            IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
            availReqEntry.push_back(MTK_LENS_POSE_REFERENCE , Type2Type< MINT32 >());
            availReqEntry.push_back(MTK_LENS_POSE_ROTATION , Type2Type< MINT32 >());
            availReqEntry.push_back(MTK_LENS_POSE_TRANSLATION , Type2Type< MINT32 >());
            availReqEntry.push_back(MTK_LENS_INTRINSIC_CALIBRATION , Type2Type< MINT32 >());
            availReqEntry.push_back(MTK_LENS_DISTORTION , Type2Type< MINT32 >());
            rMetadata.update(availReqEntry.tag(), availReqEntry);

            auto updateArrayToTag = [&](MINT32 tag, const auto &array, const char *logPrefix)
            {
                auto meta = rMetadata.entryFor(tag);
                if(meta.isEmpty())
                {
                    IMetadata::IEntry data(tag);
                    std::string logString;
                    for(auto &value : array)
                    {
                        data.push_back(value, Type2Type< MFLOAT >());
                        logString += "  ";
                        logString += std::to_string(value);
                    }
                    META_LOGD("[updateData][%d] Update %s: %s", mInfo.getDeviceId(), logPrefix, logString.c_str());
                    rMetadata.update(tag, data);
                }
            };

            auto meta = rMetadata.entryFor(MTK_LENS_POSE_REFERENCE);
            if(meta.isEmpty())
            {
                IMetadata::IEntry entryPoseRef(MTK_LENS_POSE_REFERENCE);
                entryPoseRef.push_back(calData.lensPoseReference, Type2Type< MUINT8 >());
                rMetadata.update(entryPoseRef.tag(), entryPoseRef);
                META_LOGD("[updateData][%d] Update %s: [%d]", mInfo.getDeviceId(), "Lens Pose Reference", calData.lensPoseReference);
            }

            updateArrayToTag(MTK_LENS_POSE_ROTATION,         calData.lensPoseRotation,         "Lens Pose Rotation");
            updateArrayToTag(MTK_LENS_POSE_TRANSLATION,      calData.lensPoseTranslation,      "Lens Pose Translation");
            if(calData.lensIntrinsicCalibration.size() == 0)
            {
                //Try to get FOV from static metadata
                float physicalWidth = 0.0f, physicalHeight = 0.0f, focalLength = 0.0f;
                {
                    IMetadata::IEntry entry = rMetadata.entryFor(MTK_SENSOR_INFO_PHYSICAL_SIZE);
                    if( !entry.isEmpty() ) {
                        physicalWidth = entry.itemAt(0, Type2Type<MFLOAT>());
                        physicalHeight = entry.itemAt(1, Type2Type<MFLOAT>());
                    }
                }

                {
                    IMetadata::IEntry entry = rMetadata.entryFor(MTK_LENS_INFO_AVAILABLE_FOCAL_LENGTHS);
                    if( !entry.isEmpty() ) {
                        focalLength = entry.itemAt(0, Type2Type<MFLOAT>());
                    }
                }

                if(physicalWidth > 0.0f &&
                   physicalHeight > 0.0f &&
                   focalLength > 0.0f)
                {
                    calData.lensIntrinsicCalibration = calProvider->getLensIntrinsicCalibrationBySpec(physicalWidth, physicalHeight, focalLength);
                }
                else
                {
                    calData.lensIntrinsicCalibration = {0, 0, 0, 0, 0};
                }
            }

            updateArrayToTag(MTK_LENS_INTRINSIC_CALIBRATION, calData.lensIntrinsicCalibration, "Lens Intrinsic Calibration");
            updateArrayToTag(MTK_LENS_DISTORTION,            calData.lensDistortion,           "Lens Distortion");
        }
        else
        {
            META_LOGD("No Calibation for sensor %d", mInfo.getDeviceId());
        }

        delete calProvider;
    }
    // update 3A Request Common Type
    {
        auto pHalDeviceList = MAKE_HalLogicalDeviceList();
        if(  CC_UNLIKELY( pHalDeviceList == nullptr ) ) {
            META_LOGE("get HalLogicalDeviceList fail!");
            return;
        }
        auto supportedFeature = pHalDeviceList->getSupportedFeature(mInfo.getDeviceId());
        IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        //VSDOF Do not need this
        if(!(supportedFeature & DEVICE_FEATURE_VSDOF)){
            META_LOGD("MTK_3A_FEATURE_AE_REQUEST_ISO_SPEED is unneeded in VSDOF");
            availReqEntry.push_back(MTK_3A_FEATURE_AE_REQUEST_ISO_SPEED , Type2Type< MINT32 >());
        }
        availReqEntry.push_back(MTK_3A_FEATURE_AE_REQUEST_METERING_MODE , Type2Type< MINT32 >());
        availReqEntry.push_back(MTK_3A_FEATURE_AWB_REQUEST_VALUE , Type2Type< MINT32 >());
        rMetadata.update(availReqEntry.tag(), availReqEntry);
        IMetadata::IEntry availResultEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        availResultEntry.push_back(MTK_3A_FEATURE_AE_AVAILABLE_METERING , Type2Type< MINT32 >());
        availResultEntry.push_back(MTK_3A_FEATURE_AE_AVERAGE_BRIGHTNESS , Type2Type< MINT32 >());
        availResultEntry.push_back(MTK_3A_FEATURE_AWB_AVAILABL_RANGE , Type2Type< MINT32 >());
        rMetadata.update(availResultEntry.tag(), availResultEntry);
    }
    // update 4cell sensor's data
    {
        if ( mInfo.is4cellSensor() ) {
            // update vendortag for 4cell flow
            IMetadata::IEntry availReqEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
            availReqEntry.push_back(MTK_CONTROL_CAPTURE_REMOSAIC_EN , Type2Type< MINT32 >());
            rMetadata.update(availReqEntry.tag(), availReqEntry);

            // disable cshot feature
            IMetadata::IEntry entryCShot(MTK_CSHOT_FEATURE_AVAILABLE_MODES);
            entryCShot.push_back(MTK_CSHOT_FEATURE_AVAILABLE_MODE_OFF, Type2Type< MINT32 >());
            rMetadata.update(entryCShot.tag(), entryCShot);
        }
    }
    //  update zsl data.
    {
        updateZslData(rMetadata);
    }
    // update aosp hardwarelevel
    {
        updateHardwareLevel(rMetadata);
    }
    // update aosp capabilities
    {
        updateRawCapability(rMetadata);
        updateReprocessCapabilities(rMetadata);
    }
}

void
MetadataProvider::
validation(IMetadata const& metadata)
{
    const auto featureMode =
        MAKE_HalLogicalDeviceList()->getSupportedFeature(mInfo.getDeviceId());

    if (featureMode & DEVICE_FEATURE_SECURE_CAMERA)
    {
        // skip if the capability configuration does not conform to
        // CameraProviderManager's rule:
        // A valid secure camera device must specify its capability of
        // ANDROID_REQUEST_AVAILABLE_CAPABILITIES_SECURE_IMAGE_DATA.
        auto entryCap = metadata.entryFor(
                MTK_REQUEST_AVAILABLE_CAPABILITIES);

        // NOTE: this situation is invalid so we raise assertion to avoid
        //       error propagation
        META_ASSERT(!entryCap.isEmpty(),
                "available capability can not be empty: deviceID(%d)",
                mInfo.getDeviceId());

        // NOTE: this situation is invalid so we raise assertion to avoid
        //       error propagation
        META_ASSERT(entryCap.count() == 1,
                "the number of capabilities must be one for SECURE_IMAGE_DATA: " \
                "deviceID(%d)",
                mInfo.getDeviceId());
        META_ASSERT(entryCap.itemAt(0, Type2Type<MUINT8>()) ==
                MTK_REQUEST_AVAILABLE_CAPABILITIES_SECURE_IMAGE_DATA,
                "the capability is not SECURE_IMAGE_DATA: deviceID(%d)",
                mInfo.getDeviceId());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
MetadataProvider::
updateStreamConfiguration(IMetadata &rMetadata)
{
    FUNC_PROFILE_START(__FUNCTION__);
    auto constructStreamCfgMap = [&](const auto& eCandidates, auto& dstOutput, auto& dstInput, bool mandatory=false) -> void
    {
        for ( size_t i=0; i<eCandidates.count(); i+=6 ) {
            MINT64 format        = eCandidates.itemAt(i+0, Type2Type<MINT64>());
            MINT64 width         = eCandidates.itemAt(i+1, Type2Type<MINT64>());
            MINT64 height        = eCandidates.itemAt(i+2, Type2Type<MINT64>());
            MINT64 direction     = eCandidates.itemAt(i+3, Type2Type<MINT64>());
            MINT64 frameDuration = eCandidates.itemAt(i+4, Type2Type<MINT64>());
            MINT64 stallDuration = eCandidates.itemAt(i+5, Type2Type<MINT64>());
            auto& dst = (direction==MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT)?
                        dstOutput : dstInput;
            auto it = dst.find(format);
            if ( it==dst.end() ) {
                dst[format] = std::make_shared<OrderedMap_T>();
            }
            auto pStreamCfg = std::make_shared<StreamConfig_T>();
            pStreamCfg->mFormat        = format;
            pStreamCfg->mWidth         = width;
            pStreamCfg->mHeight        = height;
            pStreamCfg->mDirection     = direction;
            pStreamCfg->mFrameDuration = frameDuration;
            pStreamCfg->mStallDuration = stallDuration;
            if ( mandatory )
                pStreamCfg->mMandatory = mandatory;
            MSize const size = MSize(pStreamCfg->mWidth, pStreamCfg->mHeight);
            (*dst[format])[size] = pStreamCfg;
            // bool isOuput = direction==MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT;
            // META_LOGD_IF(mLogLevel>=2, "O(%p)I(%p)D(%p)-{%#" PRIx64 ":%" PRId64 "x%" PRId64 ",%s, %" PRId64 ",%" PRId64 "}; ",
            //                             &dstOutput, &dstInput, &dst, format, width, height, (isOuput)? "OUTPUT" : "INPUT", frameDuration, stallDuration);
        }
    };

    // step0. check if AOSP-style configuration exists or not
    auto eStreamConfig  = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
    auto eFrameDuration = rMetadata.entryFor(MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);
    auto eStallDuration = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STALL_DURATIONS);
    if ( !eStreamConfig.isEmpty() && !eFrameDuration.isEmpty() && !eStallDuration.isEmpty() ) {
        META_LOGD( "already define streamConfig(%u) frameDuration(%u) stallDuration(%u)",
                   eStreamConfig.count(), eFrameDuration.count(), eStallDuration.count() );
        return;
    }

    auto eCandidates         = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_WITH_DURATIONS_CANDIDATES);
    auto eConfigWithDuration = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_WITH_DURATIONS);
    bool bDefaultCandidates = !eCandidates.isEmpty() && eCandidates.count()%6==0;
    bool bSensorConfigured  = !eConfigWithDuration.isEmpty() && eConfigWithDuration.count()%6==0;
    if ( CC_UNLIKELY( !bDefaultCandidates && !bSensorConfigured ) )
        META_FATAL("Invalid Stream Configuration: candidates(%d) configured(%d)", eCandidates.count(), eConfigWithDuration.count());

    // step1. scan MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_WITH_DURATIONS_CANDIDATES in default if needed
    if ( bDefaultCandidates )
        constructStreamCfgMap(eCandidates, mStreamCfgs, mInputStreamCfgs);

    // step2. scan MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_WITH_DURATIONS if set
    if ( bSensorConfigured )
        constructStreamCfgMap(eConfigWithDuration, mStreamCfgs, mInputStreamCfgs, true);
    // META_LOGD_IF(mLogLevel>=2, "output(%zu) input(%zu)", mStreamCfgs.size(), mInputStreamCfgs.size());

    // step3. Construct AOSP tags whose content are ordered depending on resolution.
    //        a. Two policies on resolution which is larger than active array size:
    //            i.  if the source is from candidates (platform default), we will neglect to append
    //                this size into final result;
    //            ii. if the source is set by user(defined in sensor folder), this setting is
    //                considered to be customization. we will append this size into final result.
    //        b. The same format/resolution combination with different duration setting, we will
    //           take MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_WITH_DURATIONS as higher priority.
    {
        auto fillStaticMetadata = [&](auto& streamCfgs, int32_t activeWidth, int32_t activeHeight, auto& streamCnt,
                                      auto& e1, auto& e2, auto& e3, bool isOuput=true) -> void {
            for ( const auto &p : streamCfgs ) {
                String8 strResult;
                for ( auto it=p.second->begin(); it!=p.second->end(); ++it) {
                    const auto& pStreamCfg = (*it).second;
                    if ( pStreamCfg.get() &&
                         ( pStreamCfg->mMandatory || ( pStreamCfg->mWidth<=activeWidth && pStreamCfg->mHeight<=activeHeight ) ) ) {
                        MINT64 format        = pStreamCfg->mFormat;
                        MINT64 width         = pStreamCfg->mWidth;
                        MINT64 height        = pStreamCfg->mHeight;
                        MINT64 direction     = pStreamCfg->mDirection;
                        MINT64 frameDuration = pStreamCfg->mFrameDuration;
                        MINT64 stallDuration = pStreamCfg->mStallDuration;
                        META_LOGD_IF(mLogLevel>=2, "StreamConfig-[%d]{%#" PRIx64 ":%" PRId64 "x%" PRId64 ",%s, %" PRId64 ",%" PRId64 "}; ",
                                                    streamCnt++, format, width, height,
                                                    (isOuput)? "OUTPUT" : "INPUT", frameDuration, stallDuration);

                        e1.push_back( static_cast<MINT32>(format),    Type2Type<MINT32>() );
                        e1.push_back( static_cast<MINT32>(width),     Type2Type<MINT32>() );
                        e1.push_back( static_cast<MINT32>(height),    Type2Type<MINT32>() );
                        e1.push_back( static_cast<MINT32>(direction), Type2Type<MINT32>() );

                        // We only add durations for OUTPUT. Because user must add the format/resolution
                        // pair with OUPUT if user want the same pair with INPUT.
                        if ( isOuput ) {
                            e2.push_back( format,        Type2Type<MINT64>() );
                            e2.push_back( width,         Type2Type<MINT64>() );
                            e2.push_back( height,        Type2Type<MINT64>() );
                            e2.push_back( frameDuration, Type2Type<MINT64>() );

                            e3.push_back( format,        Type2Type<MINT64>() );
                            e3.push_back( width,         Type2Type<MINT64>() );
                            e3.push_back( height,        Type2Type<MINT64>() );
                            e3.push_back( stallDuration, Type2Type<MINT64>() );
                        }
                    }
                }
            }
        };

        IMetadata::IEntry e1(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
        IMetadata::IEntry e2(MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);
        IMetadata::IEntry e3(MTK_SCALER_AVAILABLE_STALL_DURATIONS);

        auto eActiveArray = rMetadata.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        auto activeArrayRect = eActiveArray.itemAt(0, Type2Type<MRect>());
        MINT32 activeWidth  = activeArrayRect.s.w;
        MINT32 activeHeight = activeArrayRect.s.h;

        int32_t streamCnt = 0;
        if ( mStreamCfgs.size() )
            fillStaticMetadata(mStreamCfgs, activeWidth, activeHeight, streamCnt, e1, e2, e3);
        if ( mInputStreamCfgs.size() )
            fillStaticMetadata(mInputStreamCfgs, activeWidth, activeHeight, streamCnt, e1, e2, e3, false);

        rMetadata.update(e1.tag(), e1);
        rMetadata.update(e2.tag(), e2);
        rMetadata.update(e3.tag(), e3);
    }
    FUNC_PROFILE_END();
}
/******************************************************************************
 *
 ******************************************************************************/
void
MetadataProvider::
updateRecommendedStreamConfiguration(IMetadata &rMetadata)
{
    FUNC_PROFILE_START(__FUNCTION__);
    auto constructStreamCfgMap = [&](const auto& eCandidates, auto& dst, bool mandatory=false) -> void
    {
        for ( size_t i=0; i<eCandidates.count(); i+=5 ) {
            MINT32 width        = eCandidates.itemAt(i+0, Type2Type<MINT32>());
            MINT32 height         = eCandidates.itemAt(i+1, Type2Type<MINT32>());
            MINT32 format        = eCandidates.itemAt(i+2, Type2Type<MINT32>());
            MINT32 direction     = eCandidates.itemAt(i+3, Type2Type<MINT32>());
            MINT32 usercaseID = eCandidates.itemAt(i+4, Type2Type<MINT32>());

            auto it = dst.find(format);
            if ( it==dst.end() ) {
                dst[format] = std::make_shared<OrderedMapRecommended_T>();
            }
            auto pRecommendedCfg = std::make_shared<RecommendedConfig_T>();
            pRecommendedCfg->mFormat        = format;
            pRecommendedCfg->mWidth         = width;
            pRecommendedCfg->mHeight        = height;
            pRecommendedCfg->mDirection     = direction;
            pRecommendedCfg->mUsercaseID = usercaseID;
            if ( mandatory )
                pRecommendedCfg->mMandatory = mandatory;
            MSize const size = MSize(pRecommendedCfg->mWidth, pRecommendedCfg->mHeight);
            (*dst[format])[size] = pRecommendedCfg;
        }
    };

    // step0. check if AOSP-style configuration exists or not
    auto eRecommendedConfig  = rMetadata.entryFor(MTK_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS);
    if (!eRecommendedConfig.isEmpty()) {
        META_LOGD( "already define eRecommendedConfig(%u)",eRecommendedConfig.count());
        return;
    }

    auto eCandidates         = rMetadata.entryFor(MTK_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS_CANDIDATES);
    bool bDefaultCandidates = !eCandidates.isEmpty() && eCandidates.count()%5==0;
    if (!bDefaultCandidates)
    {
        META_LOGD("Invalid Recommended Stream Configuration");
        return;
    }

    // step1. scan MTK_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS_CANDIDATES in default if needed
    if ( bDefaultCandidates )
        constructStreamCfgMap(eCandidates, mRecommendedCfgs);

    // step2. Construct AOSP tags whose content are ordered depending on resolution.
    {
        IMetadata::IEntry e1(MTK_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS);

        auto eActiveArray = rMetadata.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        auto activeArrayRect = eActiveArray.itemAt(0, Type2Type<MRect>());
        MINT32 activeWidth  = activeArrayRect.s.w;
        MINT32 activeHeight = activeArrayRect.s.h;

        String8 strResult = String8::format("Recommendestreams: ");
        int32_t streamCnt = 0;
        for ( const auto &p : mRecommendedCfgs ) {
            for ( auto it=p.second->begin(); it!=p.second->end(); ++it) {
                const auto& pRecommendedCfg = (*it).second;
                if ( pRecommendedCfg.get() &&
                     ( pRecommendedCfg->mMandatory || ( pRecommendedCfg->mWidth<=activeWidth && pRecommendedCfg->mHeight<=activeHeight ) ) ) {
                    MINT32 format        = pRecommendedCfg->mFormat;
                    MINT32 width         = pRecommendedCfg->mWidth;
                    MINT32 height        = pRecommendedCfg->mHeight;
                    MINT32 direction     = pRecommendedCfg->mDirection;
                    MINT32 usercaseID = pRecommendedCfg->mUsercaseID;
                    strResult += String8::format("[%d]{0x%x:%dx%d, %d, %d}; ", streamCnt++,
                                                 format, width, height, direction, usercaseID);

                    e1.push_back( static_cast<MINT32>(width),     Type2Type<MINT32>() );
                    e1.push_back( static_cast<MINT32>(height),    Type2Type<MINT32>() );
                    e1.push_back( static_cast<MINT32>(format),    Type2Type<MINT32>() );
                    e1.push_back( static_cast<MINT32>(direction), Type2Type<MINT32>() );
                    e1.push_back( static_cast<MINT32>(usercaseID),Type2Type<MINT32>() );

                }
            }
        }
        META_LOGD_IF(mLogLevel>=2, "%s", strResult.string());
        rMetadata.update(e1.tag(), e1);
    }
    FUNC_PROFILE_END();
}


/******************************************************************************
 *
 ******************************************************************************/
void
MetadataProvider::
updateAfRegions(IMetadata &rMetadata)
{
    auto eMinFocusDistance = rMetadata.entryFor(MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE);
    if ( eMinFocusDistance.isEmpty() ) {
        META_FATAL("MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE must not be empty");
    } else {
        MFLOAT value = eMinFocusDistance.itemAt(0, Type2Type<MFLOAT>() );
        if ( value != .0f ) {   // not fixed focus, update
            META_LOGD("MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE: %f, add AF regions in availableKeys", value);
            // update android.request.availableRequestKeys/availableResultKeys
            auto availRequestEntry  = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
            auto availResultEntry   = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
            availRequestEntry.push_back(MTK_CONTROL_AF_REGIONS, Type2Type<MINT32>() );
            availResultEntry.push_back(MTK_CONTROL_AF_REGIONS, Type2Type<MINT32>() );
            rMetadata.update(availRequestEntry.tag(), availRequestEntry);
            rMetadata.update(availResultEntry.tag(), availResultEntry);
        }
    }
}


MBOOL
MetadataProvider::
isHdrSensor(MUINT const availVhdrEntryCount)
{
    MBOOL isHDRSensor = (availVhdrEntryCount > 1);
    char strVhdrLog[100];
    memset(strVhdrLog, '\0', sizeof(strVhdrLog));

    //query sensor static info from sensor driver to decide Hal3 vHDR support
    /*NSCam::IHalSensorList *pSensorHalList = NULL;
    pSensorHalList = MAKE_HalSensorList();*/
    IHalLogicalDeviceList* pHalDeviceList;
    pHalDeviceList = MAKE_HalLogicalDeviceList();
    if( CC_UNLIKELY( pHalDeviceList == NULL) )
    {
        META_LOGE("pHalDeviceList::get fail");
    } else {
        MUINT32 sensorDev = (MUINT32)pHalDeviceList->querySensorDevIdx(mInfo.getDeviceId());
        NSCam::SensorStaticInfo sensorStaticInfo;
        pHalDeviceList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
        isHDRSensor = (sensorStaticInfo.HDR_Support > 0) ? isHDRSensor : false;
        sprintf(strVhdrLog, " sensorDev:%d, sensorStaticInfo.HDR_Support:%d,",
            sensorDev, sensorStaticInfo.HDR_Support);
    }

    //force set ON/OFF Hal3 vHDR support
    MINT32 vhdrHal3Prop = property_get_int32("debug.camera.hal3.vhdrSupport", -1);
    isHDRSensor = (vhdrHal3Prop != -1) ? (vhdrHal3Prop > 0) : isHDRSensor;
    META_LOGD("isHDRSensor:%d, vhdrHal3Prop:%d,%s availVhdrEntry.count():%d",
        isHDRSensor, vhdrHal3Prop, strVhdrLog, availVhdrEntryCount);

    return isHDRSensor;
}

MVOID
MetadataProvider::
updateHdrData(MBOOL const isHDRSensor, MBOOL const isSingleFrameSupport, MINT32 const hdrDetectionMode,
    IMetadata &rMetadata)
{
    // Available HDR modes for Photo & Video
    IMetadata::IEntry availHdrPhotoEntry(MTK_HDR_FEATURE_AVAILABLE_HDR_MODES_PHOTO);
    IMetadata::IEntry availHdrVideoEntry(MTK_HDR_FEATURE_AVAILABLE_HDR_MODES_VIDEO);

    // --- MODE_OFF ----
    availHdrPhotoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_OFF, Type2Type< MINT32 >());
    availHdrVideoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_OFF, Type2Type< MINT32 >());

#if (MTKCAM_HAVE_VHDR_SUPPORT == 1)
    // --- MODE_VIDEO_ON ----
    IMetadata::IEntry mStreamHdrEntry = rMetadata.entryFor(MTK_HDR_FEATURE_AVAILABLE_MSTREAM_HDR_MODES);
    MBOOL isMStreamSupport = (mStreamHdrEntry.count() > 1);

    if( isMStreamSupport || isHDRSensor )
    {
        availHdrPhotoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_VIDEO_ON, Type2Type< MINT32 >());
        availHdrVideoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_VIDEO_ON, Type2Type< MINT32 >());
        //if(isSingleFrameSupport)
            //availHdrPhotoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_VIDEO_ON, Type2Type< MINT32 >());
    }
#endif

#if (MTKCAM_HAVE_HDR_SUPPORT == 1)
    // --- MODE_ON ----
    availHdrPhotoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_ON, Type2Type< MINT32 >());
    /* Video mode not support MODE_ON*/

    // --- MODE_AUTO ----
    if (hdrDetectionMode == 3
        || (hdrDetectionMode == 2 && !isHDRSensor)
        || (hdrDetectionMode == 1 && isHDRSensor))
    {
        availHdrPhotoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_AUTO, Type2Type< MINT32 >());
    }
    /* Video mode not support MODE_AUTO*/

    // --- MODE_VIDEO_AUTO ----
    if(isHDRSensor && (hdrDetectionMode == 1 || hdrDetectionMode == 3))
    {
        availHdrVideoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_VIDEO_AUTO, Type2Type< MINT32 >());
        if(isSingleFrameSupport)
            availHdrPhotoEntry.push_back(MTK_HDR_FEATURE_HDR_MODE_VIDEO_AUTO, Type2Type< MINT32 >());
    }
#endif// MTKCAM_HAVE_HDR_SUPPORT endif

    rMetadata.update(availHdrPhotoEntry.tag(), availHdrPhotoEntry);
    rMetadata.update(availHdrVideoEntry.tag(), availHdrVideoEntry);
}


MVOID
MetadataProvider::
updateZslData(IMetadata &rMetadata)
{
    // step1. update android.request.availableRequestKeys/availableResultKeys/availableCharacteristicsKeys/availableSessionKeys
    bool isLowRamDevice = ::property_get_bool("ro.config.low_ram", false);
    if( isLowRamDevice ){
        META_LOGI("ro.config.low_ram = true, skip zsl setting.");
        return;
    }

    {
        IMetadata::IEntry availRequestEntry  = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        IMetadata::IEntry availResultEntry   = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
        IMetadata::IEntry availSessionEntry  = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_SESSION_KEYS);

        // 1-1. update AOSP tag MTK_CONTROL_ENABLE_ZSL
        availRequestEntry.push_back( MTK_CONTROL_ENABLE_ZSL, Type2Type< MINT32 >());
        availResultEntry.push_back(  MTK_CONTROL_ENABLE_ZSL, Type2Type< MINT32 >());

        // 1-2. update static in vendortag MTK_CONTROL_CAPTURE_AVAILABLE_ZSL_MODES / MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE
        availCharactsEntry.push_back( MTK_CONTROL_CAPTURE_AVAILABLE_ZSL_MODES, Type2Type< MINT32 >());
        availCharactsEntry.push_back( MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE, Type2Type< MINT32 >());

        // 1-3. update control/result/session in vendortag MTK_CONTROL_CAPTURE_ZSL_MODE
        availRequestEntry.push_back( MTK_CONTROL_CAPTURE_ZSL_MODE,  Type2Type< MINT32 >());
        availResultEntry.push_back(  MTK_CONTROL_CAPTURE_ZSL_MODE,  Type2Type< MINT32 >());
        availSessionEntry.push_back( MTK_CONTROL_CAPTURE_ZSL_MODE,  Type2Type< MINT32 >());

        rMetadata.update(availRequestEntry.tag(), availRequestEntry);
        rMetadata.update(availResultEntry.tag(), availResultEntry);
        rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
        rMetadata.update(availSessionEntry.tag(), availSessionEntry);
    }

    // step2. list available zsl modes.
    IMetadata::IEntry availZslModes = rMetadata.entryFor(MTK_CONTROL_CAPTURE_AVAILABLE_ZSL_MODES);
    if ( availZslModes.isEmpty() ) {
        IMetadata::IEntry entry(MTK_CONTROL_CAPTURE_AVAILABLE_ZSL_MODES);
        entry.push_back(MTK_CONTROL_CAPTURE_ZSL_MODE_OFF, Type2Type<MUINT8>());
        entry.push_back(MTK_CONTROL_CAPTURE_ZSL_MODE_ON, Type2Type<MUINT8>());
        rMetadata.update(entry.tag(), entry);
    } else {
        META_LOGI("user defines availableZslModes in custom files");
    }

    // step3. update default zsl setting as off.
    //        do not update if already defined in custom files.
    IMetadata::IEntry defaultZslMode = rMetadata.entryFor(MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE);
    if ( defaultZslMode.isEmpty() ) {
        IMetadata::IEntry entry(MTK_CONTROL_CAPTURE_DEFAULT_ZSL_MODE);
        entry.push_back(MTK_CONTROL_CAPTURE_ZSL_MODE_OFF, Type2Type<MUINT8>());
        rMetadata.update(entry.tag(), entry);
    } else {
        META_LOGI("user defines defaultZslMode in custom files");
    }
}


MVOID
MetadataProvider::
updateHardwareLevel(IMetadata &rMetadata __unused)
{
    const char* kHardwareLevel[MTK_INFO_SUPPORTED_HARDWARE_LEVEL_HIGH_RESOLUTION+1] =
    {
        "LIMITED",
        "FULL",
        "LEGACY",
        "3",
        "EXTERNAL"
    };

    IMetadata::IEntry hardwareLevel = rMetadata.entryFor(MTK_INFO_SUPPORTED_HARDWARE_LEVEL);
    if ( ! hardwareLevel.isEmpty() ) {
        META_LOGD("define hardwarelevel as %s Mode", kHardwareLevel[hardwareLevel.itemAt(0, Type2Type<MUINT8>())]);
        return;
    }

    // full requirements
    bool bPerframeControl = false;
    bool bBurstCapture = false;
    bool bManualSensorControl = false;
    bool bManualPostProcessing = false;
    bool bExposureTimeRange = false;
    bool bMaxFrameDuration = false;

    // 3 requirement
    bool bLevelFull = false;
    bool bYUVReprocessing = false;
    bool bRaw16 = false;

    IMetadata::IEntry availableCapabilities = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CAPABILITIES);
    for (int i = 0; i < availableCapabilities.count(); i++)
    {
        auto capability = availableCapabilities.itemAt(i, Type2Type<MUINT8>());
        switch ( capability ) {
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_MANUAL_SENSOR: {
                bManualSensorControl = true;
                break;
            }
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_MANUAL_POST_PROCESSING: {
                bManualPostProcessing = true;
                break;
            }
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_RAW: {
                bRaw16 = true;
                break;
            }
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_BURST_CAPTURE: {
                bBurstCapture = true;
                break;
            }
            case MTK_REQUEST_AVAILABLE_CAPABILITIES_YUV_REPROCESSING: {
                bYUVReprocessing = true;
                break;
            }
            default: {
                break;
            }
        }
    }

    // per-frame control
    IMetadata::IEntry syncMaxLatency = rMetadata.entryFor(MTK_SYNC_MAX_LATENCY);
    bPerframeControl = syncMaxLatency.itemAt(0, Type2Type<MINT32>())==MTK_SYNC_MAX_LATENCY_PER_FRAME_CONTROL;

    // exposure time range
    IMetadata::IEntry expTimeRange = rMetadata.entryFor(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    auto exposureTimeUpper = expTimeRange.itemAt(1, Type2Type<MINT64>());
    if ( exposureTimeUpper > 100000000L )
        bExposureTimeRange = true;

    // maximum frame duration
    IMetadata::IEntry maxFrmDuration = rMetadata.entryFor(MTK_SENSOR_INFO_MAX_FRAME_DURATION);
    auto maxFrameDuration = maxFrmDuration.itemAt(0, Type2Type<MINT64>());
    if ( maxFrameDuration > 100000000L )
        bMaxFrameDuration = true;

    if ( bPerframeControl && bBurstCapture && bManualSensorControl && bManualPostProcessing &&
         bExposureTimeRange && bMaxFrameDuration ) {
        bLevelFull = true;
    }

    META_LOGD("per-frame control(%d) burstCapture(%d) manualSensorControl(%d) manualPostProcessing(%d) expTimeRange(%d) maxFrmDuration(%d) raw16(%d) yuvReproc(%d)",
            bPerframeControl, bBurstCapture, bManualSensorControl, bManualPostProcessing, bExposureTimeRange, bMaxFrameDuration, bRaw16, bYUVReprocessing);

    auto hwLevel = MTK_INFO_SUPPORTED_HARDWARE_LEVEL_LIMITED;
    if ( bLevelFull ) {
        if ( bYUVReprocessing && bRaw16 )
            hwLevel = MTK_INFO_SUPPORTED_HARDWARE_LEVEL_3;
        else
            hwLevel = MTK_INFO_SUPPORTED_HARDWARE_LEVEL_FULL;
    }

    IMetadata::IEntry entry(MTK_INFO_SUPPORTED_HARDWARE_LEVEL);
    entry.push_back((MUINT8)hwLevel, Type2Type<MUINT8>());
    rMetadata.update(entry.tag(), entry);
    META_LOGI("support hardwarelevel as %s Mode", kHardwareLevel[hwLevel]);
}


MVOID
MetadataProvider::
updateRawCapability(IMetadata &rMetadata    __unused)
{
    FUNC_START;
    // check whether device supports raw capability or not
    bool bRaw16 = false;
    IMetadata::IEntry availableCapabilities = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CAPABILITIES);
    for (int i = 0; i < availableCapabilities.count(); i++)
    {
        auto capability = availableCapabilities.itemAt(i, Type2Type<MUINT8>());
        if ( capability==MTK_REQUEST_AVAILABLE_CAPABILITIES_RAW ) {
            bRaw16 = true;
            break;
        }
    }
    if ( !bRaw16 ) return;

    // update MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST and MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE
    updatePostRawSensitivityBoost(rMetadata);

    // always support MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES in characteristics
    // update AOSP characteristic tag MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES
    {
        // IMetadata::IEntry availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
        // availCharactsEntry.push_back( MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES , Type2Type< MINT32 >());
        // rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
    }

    // update MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES
    {
        auto availLensShadingMapModes = rMetadata.entryFor(MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES);
        if ( availLensShadingMapModes.isEmpty() ) {
            IMetadata::IEntry entry(MTK_STATISTICS_INFO_AVAILABLE_LENS_SHADING_MAP_MODES);
            entry.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF, Type2Type< MUINT8 >());
            entry.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type< MUINT8 >());
            rMetadata.update(entry.tag(), entry);
        } else if ( availLensShadingMapModes.count()==1 &&
            MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF == availLensShadingMapModes.itemAt(0, Type2Type<MUINT8>() ) ) {
            availLensShadingMapModes.push_back(MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON, Type2Type< MUINT8 >());
            rMetadata.update(availLensShadingMapModes.tag(), availLensShadingMapModes);
        } else {
            for ( auto i=0; i<availLensShadingMapModes.count(); ++i ) {
                META_LOGD("user defines availableLensShadingMapMode [%u]:%u", i, availLensShadingMapModes.itemAt(i, Type2Type<MUINT8>()) );
            }
        }
    }

    // update MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS if not defined before
    // {
    //     auto availableStreamConfigurations = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
    //     auto availableMinFrameDurations    = rMetadata.entryFor(MTK_SCALER_AVAILABLE_MIN_FRAME_DURATIONS);
    //     auto availableStallDurations       = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STALL_DURATIONS);

    //     bool defined = false;
    //     for ( int i=0; i<availableStreamConfigurations.count(); i+=4 ) {
    //         if ( HAL_PIXEL_FORMAT_RAW16==availableStreamConfigurations.itemAt(i, Type2Type<MINT32>()) ) {
    //             defined = true;
    //         }
    //     }
    //     if ( !defined ) {
    //         auto eActiveArray = rMetadata.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
    //         auto activeArrayRect = eActiveArray.itemAt(0, Type2Type<MRect>());
    //         MINT32 activeWidth  = activeArrayRect.s.w;
    //         MINT32 activeHeight = activeArrayRect.s.h;

    //         availableStreamConfigurations.push_back( HAL_PIXEL_FORMAT_RAW16, Type2Type< MINT32 >());
    //         availableStreamConfigurations.push_back( activeWidth, Type2Type< MINT32 >());
    //         availableStreamConfigurations.push_back( activeWidth, Type2Type< MINT32 >());
    //         availableStreamConfigurations.push_back( MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT, Type2Type< MINT32 >());

    //         availableMinFrameDurations.push_back( HAL_PIXEL_FORMAT_RAW16, Type2Type< MINT64 >());
    //         availableMinFrameDurations.push_back( activeWidth, Type2Type< MINT64 >());
    //         availableMinFrameDurations.push_back( activeHeight, Type2Type< MINT64 >());
    //         availableMinFrameDurations.push_back( 33333333, Type2Type< MINT64 >());     // default frame duration

    //         availableStallDurations.push_back( HAL_PIXEL_FORMAT_RAW16, Type2Type< MINT64 >());
    //         availableStallDurations.push_back( activeWidth, Type2Type< MINT64 >());
    //         availableStallDurations.push_back( activeHeight, Type2Type< MINT64 >());
    //         availableStallDurations.push_back( 33333333, Type2Type< MINT64 >());        // default stall duration
    //     }
    //     rMetadata.update(availableStreamConfigurations.tag(), availableStreamConfigurations);
    //     rMetadata.update(availableMinFrameDurations.tag(), availableMinFrameDurations);
    //     rMetadata.update(availableStallDurations.tag(), availableStallDurations);
    // }

    //
    FUNC_END;
}

MVOID
MetadataProvider::
updatePostRawSensitivityBoost(IMetadata &rMetadata    __unused)
{
    FUNC_START;
    // update android.request.availableRequestKeys/availableResultKeys/availableCharacteristicsKeys
    {
        auto availRequestEntry  = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_REQUEST_KEYS);
        auto availResultEntry   = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_RESULT_KEYS);
        auto availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);

        // 1-1. update AOSP request&result tag MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST
        availRequestEntry.push_back( MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST, Type2Type< MINT32 >());
        availResultEntry.push_back(  MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST, Type2Type< MINT32 >());

        // 1-2. update AOSP characteristic tag MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE
        availCharactsEntry.push_back( MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE, Type2Type< MINT32 >());

        rMetadata.update(availRequestEntry.tag(), availRequestEntry);
        rMetadata.update(availResultEntry.tag(), availResultEntry);
        rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
    }

    // update MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE
    {
        auto postRawSenBoostRange = rMetadata.entryFor(MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE);
        if ( postRawSenBoostRange.isEmpty() ) {
            IMetadata::IEntry entry(MTK_CONTROL_POST_RAW_SENSITIVITY_BOOST_RANGE);
            entry.push_back(100, Type2Type< MINT32 >());
            entry.push_back(100, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
        } else {
            if ( postRawSenBoostRange.count()!=2 ) {
                META_LOGE("invalid postRawSenBoostRange data.count(%d)", postRawSenBoostRange.count());
            } else {
                META_LOGD("user defines postRawSenBoostRange [%d, %d]",
                        postRawSenBoostRange.itemAt(0, Type2Type<MINT32>()), postRawSenBoostRange.itemAt(1, Type2Type<MINT32>()));
            }
        }
    }
    //
    FUNC_END;
}


MVOID
MetadataProvider::
updateReprocessCapabilities(IMetadata &rMetadata    __unused)
{
    FUNC_START;
    //
    // check whether device supports opaque_reproc & yuv_reproc capabilities or not
    bool bOpaqueReproc = false;
    bool bYuvReproc = false;
    IMetadata::IEntry availableCapabilities = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CAPABILITIES);
    for (int i = 0; i < availableCapabilities.count(); i++)
    {
        auto capability = availableCapabilities.itemAt(i, Type2Type<MUINT8>());
        if ( capability==MTK_REQUEST_AVAILABLE_CAPABILITIES_PRIVATE_REPROCESSING ) {
            bOpaqueReproc = true;
        } else if ( capability==MTK_REQUEST_AVAILABLE_CAPABILITIES_YUV_REPROCESSING ) {
            bYuvReproc = true;
        }
    }
    if ( !bOpaqueReproc && !bYuvReproc ) return;

    // update android.request.availableCharacteristicsKeys
    {
        // MTK_REPROCESS_MAX_CAPTURE_STALL
        auto availCharactsEntry = rMetadata.entryFor(MTK_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
        availCharactsEntry.push_back( MTK_REPROCESS_MAX_CAPTURE_STALL, Type2Type< MINT32 >());
        rMetadata.update(availCharactsEntry.tag(), availCharactsEntry);
    }

    // update MTK_SCALER_AVAILABLE_INPUT_OUTPUT_FORMATS_MAP
    {
        auto availIOFormatsMap  = rMetadata.entryFor(MTK_SCALER_AVAILABLE_INPUT_OUTPUT_FORMATS_MAP);
        if ( availIOFormatsMap.isEmpty() ) {
            IMetadata::IEntry entry(MTK_SCALER_AVAILABLE_INPUT_OUTPUT_FORMATS_MAP);
            if ( bOpaqueReproc ) {
                // input "IMPLEMENTATION_DEFINED" can produce "2" kinds of output format "YCbCr_420_888" and "BLOB"
                entry.push_back(HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, Type2Type< MINT32 >());
                entry.push_back(2, Type2Type< MINT32 >());
                entry.push_back(HAL_PIXEL_FORMAT_YCbCr_420_888, Type2Type< MINT32 >());
                entry.push_back(HAL_PIXEL_FORMAT_BLOB, Type2Type< MINT32 >());
            }
            if ( bYuvReproc ) {
                // input "YCbCr_420_888" can produce "2" kinds of output format "YCbCr_420_888" and "BLOB"
                entry.push_back(HAL_PIXEL_FORMAT_YCbCr_420_888, Type2Type< MINT32 >());
                entry.push_back(2, Type2Type< MINT32 >());
                entry.push_back(HAL_PIXEL_FORMAT_YCbCr_420_888, Type2Type< MINT32 >());
                entry.push_back(HAL_PIXEL_FORMAT_BLOB, Type2Type< MINT32 >());
            }
            rMetadata.update(entry.tag(), entry);
        } else {
            META_LOGD("user defines availableInputOutputFormatsMap");
        }
    }

    // update MTK_REQUEST_MAX_NUM_INPUT_STREAMS
    {
        auto requestMaxNumInputStreams = rMetadata.entryFor(MTK_REQUEST_MAX_NUM_INPUT_STREAMS);
        auto value = requestMaxNumInputStreams.itemAt(0, Type2Type<MINT32>());
        if ( value == 0 ) {
            requestMaxNumInputStreams.replaceItemAt(0, 1, Type2Type<MINT32>());
            rMetadata.update(requestMaxNumInputStreams.tag(), requestMaxNumInputStreams);
            META_LOGD("replace requestMaxNumInputStreams from 0 to 1 (OpaqueReproc:%d YuvReproc:%d )",
                    bOpaqueReproc, bYuvReproc);
        }
    }

    // update MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS
    {
        // TODO: maintain data structure if any other funtion needs to parse stream configurations
        int32_t max_width = -1, max_height = -1;
        auto availableStreamConfigurations = rMetadata.entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
        for ( int i=0; i<availableStreamConfigurations.count(); i+=4 ) {
            if ( HAL_PIXEL_FORMAT_YCbCr_420_888                   ==availableStreamConfigurations.itemAt(i,   Type2Type<MINT32>()) &&
                 MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT==availableStreamConfigurations.itemAt(i+3, Type2Type<MINT32>()) ) {
                if ( max_width <availableStreamConfigurations.itemAt(i+1, Type2Type<MINT32>()) &&
                     max_height<availableStreamConfigurations.itemAt(i+2, Type2Type<MINT32>()) ) {
                    max_width = availableStreamConfigurations.itemAt(i+1, Type2Type<MINT32>());
                    max_height= availableStreamConfigurations.itemAt(i+2, Type2Type<MINT32>());
                }
            }
        }
        if ( bOpaqueReproc ) {
            availableStreamConfigurations.push_back(HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, Type2Type< MINT32 >());
            availableStreamConfigurations.push_back(max_width, Type2Type< MINT32 >());
            availableStreamConfigurations.push_back(max_height, Type2Type< MINT32 >());
            availableStreamConfigurations.push_back(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_INPUT, Type2Type< MINT32 >());
        }
        if ( bYuvReproc ) {
            availableStreamConfigurations.push_back(HAL_PIXEL_FORMAT_YCbCr_420_888, Type2Type< MINT32 >());
            availableStreamConfigurations.push_back(max_width, Type2Type< MINT32 >());
            availableStreamConfigurations.push_back(max_height, Type2Type< MINT32 >());
            availableStreamConfigurations.push_back(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_INPUT, Type2Type< MINT32 >());
        }
        rMetadata.update(availableStreamConfigurations.tag(), availableStreamConfigurations);
    }

    // update MTK_REPROCESS_MAX_CAPTURE_STALL, default value = 1
    {
        auto reprocessMaxCaptureStall = rMetadata.entryFor(MTK_REPROCESS_MAX_CAPTURE_STALL);
        if ( reprocessMaxCaptureStall.isEmpty() ) {
            IMetadata::IEntry entry(MTK_REPROCESS_MAX_CAPTURE_STALL);
            entry.push_back(1, Type2Type< MINT32 >());
            rMetadata.update(entry.tag(), entry);
        } else {
            META_LOGD("user defines reprocessMaxCaptureStall(%d)", reprocessMaxCaptureStall.itemAt(0, Type2Type<MINT32>()));
        }
    }

    // update MTK_EDGE_AVAILABLE_EDGE_MODES & MTK_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES
    {
        auto availableEdgeModes = rMetadata.entryFor(MTK_EDGE_AVAILABLE_EDGE_MODES);
        if ( availableEdgeModes.isEmpty() ) {
            IMetadata::IEntry entry(MTK_EDGE_AVAILABLE_EDGE_MODES);
            entry.push_back(MTK_EDGE_MODE_OFF,              Type2Type<MUINT8>() );
            entry.push_back(MTK_EDGE_MODE_FAST,             Type2Type<MUINT8>() );
            entry.push_back(MTK_EDGE_MODE_HIGH_QUALITY,     Type2Type<MUINT8>() );
            entry.push_back(MTK_EDGE_MODE_ZERO_SHUTTER_LAG, Type2Type<MUINT8>() );
            rMetadata.update(entry.tag(), entry);
            META_LOGW("update empty availableEdgeModes setting");
        } else {
            bool hasEMZsl = false;
            for ( size_t i=0; i<availableEdgeModes.count(); ++i ) {
                if ( MTK_EDGE_MODE_ZERO_SHUTTER_LAG==availableEdgeModes.itemAt(i, Type2Type<MUINT8>()) )
                    hasEMZsl = true;
            }
            if ( !hasEMZsl ) {
                availableEdgeModes.push_back(MTK_EDGE_MODE_ZERO_SHUTTER_LAG, Type2Type<MUINT8>());
                rMetadata.update(availableEdgeModes.tag(), availableEdgeModes);
                META_LOGD("add MTK_EDGE_MODE_ZERO_SHUTTER_LAG into availableEdgeModes setting");
            }
        }

        auto availableNRModes = rMetadata.entryFor(MTK_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES);
        if ( availableNRModes.isEmpty() ) {
            IMetadata::IEntry entry(MTK_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES);
            entry.push_back(MTK_NOISE_REDUCTION_MODE_OFF,               Type2Type<MUINT8>() );
            entry.push_back(MTK_NOISE_REDUCTION_MODE_FAST,              Type2Type<MUINT8>() );
            entry.push_back(MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY,      Type2Type<MUINT8>() );
            entry.push_back(MTK_NOISE_REDUCTION_MODE_MINIMAL,           Type2Type<MUINT8>() );
            entry.push_back(MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG,  Type2Type<MUINT8>() );
            rMetadata.update(entry.tag(), entry);
            META_LOGW("update empty availableNRModes setting");
        } else {
            bool hasNRZsl = false;
            for ( size_t i=0; i<availableNRModes.count(); ++i ) {
                if ( MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG==availableNRModes.itemAt(i, Type2Type<MUINT8>()) )
                    hasNRZsl = true;
            }
            if ( !hasNRZsl ) {
                availableNRModes.push_back(MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG, Type2Type<MUINT8>());
                rMetadata.update(availableNRModes.tag(), availableNRModes);
                META_LOGD("add MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG into availableNRModes setting");
            }
        }
    }
    //
    FUNC_END;
}
