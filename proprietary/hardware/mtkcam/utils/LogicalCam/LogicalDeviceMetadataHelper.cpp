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

#define LOG_TAG "MtkCam/Util/LogicalDeviceMetadataHelper"

#include "MyUtils.h"
#include "LogicalDeviceMetadataHelper.h"
#include <dlfcn.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace android;
using namespace NSCam::NSLogicalDeviceMetadataHelper;
/******************************************************************************
 *
 ******************************************************************************/
LogicalDeviceMetadataHelper::
~LogicalDeviceMetadataHelper()
{
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
LogicalDeviceMetadataHelper::
constructStaticMetadata(
    int32_t deviceId,
    std::string deviceName,
    bool bBackSide,
    IMetadata &mtkMetadata
)
{
    // 1. [backward compatible] load old version metadata.
    //    If load V0 success, ignore load v1 version metadata.
    status_t ret = loadV0Metadata(deviceId, deviceName, bBackSide, mtkMetadata);
    if(ret != OK)
    {
        MY_LOGI("load v0 metadata incorrect reset clear static metadata");
        mtkMetadata.clear();
        // 2. load V1 metadata, if old version is exist
        status_t ret_v1 = loadV1Metadata(deviceId, deviceName, bBackSide, mtkMetadata);
        if(ret_v1 != OK)
        {
            MY_LOGA("load v1 metadat fail.");
        }
        ret_v1 = loadV1Metadata_ext(deviceId, deviceName, bBackSide, mtkMetadata);
    }
    return  OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
LogicalDeviceMetadataHelper::
loadV0Metadata(
    int32_t deviceId,
    std::string deviceName,
    bool bBackSide,
    IMetadata &mtkMetadata
)
{
    char const*const
    logicalDeviceStaticMetadataTypeNames[] =
    {
        "LENS",
        "SENSOR",
        "TUNING_3A",
        "FLASHLIGHT",
        "SCALER",
        //"FEATURE",
        "CAMERA",
        "REQUEST",
        NULL
    };
    size_t count = (sizeof(logicalDeviceStaticMetadataTypeNames)/sizeof(char const*)) ;
    std::vector<bool> vMap(count, false);
    //
    for (int i = 0; NULL != logicalDeviceStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = logicalDeviceStaticMetadataTypeNames[i];
        status_t status = OK;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_DEVICE_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, deviceName.c_str());
        status = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Sensor, bBackSide, mtkMetadata);
        if  ( OK == status ) {
            MY_LOGD("load logical device (%s)", s8Symbol_Sensor.string());
            vMap[i] = true;
            continue;
        }
        //
        MY_LOGE_IF(0, "Fail for %s", s8Symbol_Sensor.string());
    }
    //
    for (int i = 0; NULL != logicalDeviceStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = logicalDeviceStaticMetadataTypeNames[i];
        status_t status = OK;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, deviceName.c_str());
        status = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Sensor, bBackSide, mtkMetadata);
        if  ( OK == status ) {
            MY_LOGD("load logical device (%s)", s8Symbol_Sensor.string());
            vMap[i] = true;
            continue;
        }
        //
        MY_LOGE_IF(0, "Fail for both %s", s8Symbol_Sensor.string());
    }
    //
    for (int i = 0; NULL != logicalDeviceStaticMetadataTypeNames[i]; i++) {
        if ( vMap[i]==false ) {
            MY_LOGE("Fail to load %s in all PLATFORM/PROJECT combinations", logicalDeviceStaticMetadataTypeNames[i] );
            return NAME_NOT_FOUND;
        }
    }
    return OK;
}

enum {
    COMMON_COMMON       = 0x1,
    COMMON_SENSOR       = 0x1 << 1,
    PLATFORM_COMMON     = 0x1 << 2,
    PLATFORM_SENSOR     = 0x1 << 3,
    PROJECT_COMMON      = 0x1 << 4,
    PROJECT_SENSOR      = 0x1 << 5,
};

/******************************************************************************
 *
 ******************************************************************************/
status_t
LogicalDeviceMetadataHelper::
loadV1Metadata(
    int32_t deviceId,
    std::string deviceName,
    bool bBackSide,
    IMetadata &mtkMetadata
)
{
    // 1. process common part
    char const*const
    kCommonStaticMetadataTypeNames[] =
    {
        // MODULE
        "CAMERA",
        "CONTROL_AE",
        "CONTROL_AF",
        "CONTROL_AWB",
        "TUNING",
        "FLASHLIGHT",
        "SENSOR",
        "LENS",
        // // PROJECT
        // "AVAILABLE_KEYS",
        // "FEATURE",
        // "MULTICAM",
        // "REQUEST",
        // "SCALER",
        // "VENDOR",
        NULL
    };
    //
    String8 strResult = String8::format("<load custom folder>\n\tSTATIC_COMMON: ");
    for (int i = 0; NULL != kCommonStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kCommonStaticMetadataTypeNames[i];
        status_t status = OK;
        int32_t loadResult = 0;
        //
        String8 const s8Symbol_Common = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Common, bBackSide, mtkMetadata);
        if ( status==OK ) loadResult|=COMMON_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, deviceName.c_str());
        status = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Sensor, bBackSide, mtkMetadata);
        if ( status==OK ) loadResult|=COMMON_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    // 2. platform static metadata
    char const*const
    kPlatformStaticMetadataNames[] =
    {
        "MODULE",
        "LENS",
        // "MOD_OVERWRITE"
        // "PROJECT",
        // // "PRO_OVERWRITE"
        NULL
    };

    //
    strResult += String8::format("\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s = OK;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Common, bBackSide, mtkMetadata);
        if ( status_d==OK ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, deviceName.c_str());
        status_s = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Sensor, bBackSide, mtkMetadata);
        if ( status_s==OK ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
        if ( status_s!=OK && status_d!=OK ) {
            MY_LOGE("%s", strResult.string());
            return UNKNOWN_ERROR;
        }
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s = OK;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Common, bBackSide, mtkMetadata);
        if ( status_d==MTRUE ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, deviceName.c_str());
        status_s = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Sensor, bBackSide, mtkMetadata);
        if ( status_s==MTRUE ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    MY_LOGI("%s", strResult.string());
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
LogicalDeviceMetadataHelper::
loadV1Metadata_ext(
    int32_t deviceId,
    std::string deviceName,
    bool bBackSide,
    IMetadata &mtkMetadata
)
{
    char const*const
    kPlatformOptionalStaticMetadataNames[] =
    {
        // "MODULE"
        // "LENS",
        "MOD_OVERWRITE",
        // // "PROJECT",
        // "PRO_OVERWRITE",
        NULL,
    };
    String8 strResult = String8::format("<load custom folder - optional>\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformOptionalStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOptionalStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Common, bBackSide, mtkMetadata);
        if ( status_d==OK ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, deviceName.c_str());
        status_s = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Sensor, bBackSide, mtkMetadata);
        if ( status_s==OK ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformOptionalStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOptionalStaticMetadataNames[i];
        int32_t loadResult = 0;
        status_t status_d = OK, status_s;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Common, bBackSide, mtkMetadata);
        if ( status_d==OK ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, deviceName.c_str());
        status_s = impConstructStaticMetadata_by_SymbolName(deviceId, s8Symbol_Sensor, bBackSide, mtkMetadata);
        if ( status_s==OK ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    META_LOGI("%s", strResult.string());
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
LogicalDeviceMetadataHelper::
impConstructStaticMetadata_by_SymbolName(
    int32_t deviceId,
    String8 const&      s8Symbol,
    bool bBackSide,
    IMetadata &metadata
)
{
typedef status_t (*PFN_T)(
        IMetadata &         metadata,
        Info const&         info
    );
    Info info(deviceId, bBackSide, s8Symbol.string());
    String8 const s8LibPath = String8::format("libmtkcam.logicalmodule.so");
    void *handle = ::dlopen(s8LibPath.string(), RTLD_GLOBAL);
    status_t status = OK;
    if(handle)
    {
        PFN_T pfn = (PFN_T)::dlsym(handle, s8Symbol.string());

        //
        //PFN_T pfn = (PFN_T)::dlsym(RTLD_DEFAULT, s8Symbol.string());
        if  ( pfn )
        {
            status = pfn(metadata, info);
            MY_LOGI_IF(0, "%s: returns status[%s(%d)]", s8Symbol.string(), ::strerror(-status), -status);
        }
        else
        {
            MY_LOGW_IF(1, "%s not found", s8Symbol.string());
            status = NAME_NOT_FOUND;
        }
        //
        dlclose(handle);
    }
    else
    {
        MY_LOGE("Fail to open %s", s8LibPath.string());
        status = NAME_NOT_FOUND;
    }
    //
    return  status;
}
