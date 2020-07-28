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

#define LOG_TAG "MtkCam/HalSensorList"

#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include "MyUtils.h"
#include <dlfcn.h>
#include <mtkcam/utils/std/ULog.h>
// For property_get().
#include <cutils/properties.h>

extern SensorStaticInfo sensorStaticInfo[IMGSENSOR_SENSOR_IDX_MAX_NUM];

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

/******************************************************************************
 *
 ******************************************************************************/
static
MBOOL
impConstructStaticMetadata_by_SymbolName(
    String8 const&  s8Symbol,
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
typedef MBOOL (*PFN_T)(
        IMetadata &         metadata,
        Info const&         info
    );

    PFN_T pfn;
    MBOOL ret = MTRUE;
    String8 const s8LibPath = String8::format("libcam.halsensor.so");
    void *handle = ::dlopen(s8LibPath.string(), RTLD_GLOBAL);
    if ( ! handle )
    {
        char const *err_str = ::dlerror();
        MY_LOGW("dlopen library=%s %s", s8LibPath.string(), err_str?err_str:"unknown");
        ret = MFALSE;
        goto lbExit;
    }
    pfn = (PFN_T)::dlsym(handle, s8Symbol.string());
    if  ( ! pfn ) {
        MY_LOGW("%s not found", s8Symbol.string());
        ret = MFALSE;
        goto lbExit;
    }

    ret = pfn(rMetadata, rInfo);
    MY_LOGW_IF(!ret, "%s fail", s8Symbol.string());


lbExit:
    if ( handle )
    {
        ::dlclose(handle);
        handle = NULL;
    }
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kStaticMetadataTypeNames[] = {
    "LENS",
    "SENSOR",
    "TUNING_3A",
    "FLASHLIGHT",
    "SCALER",
    "FEATURE",
    "CAMERA",
    "REQUEST",
    NULL
};



static
MBOOL
impBuildStaticInfo(
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];

        MBOOL status = MTRUE;

        String8 const s8Symbol_Sensor = String8::format("%s_DEVICE_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        String8 const s8Symbol_Common = String8::format("%s_DEVICE_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        MY_LOGE_IF(0, "Fail for both %s & %s", s8Symbol_Sensor.string(), s8Symbol_Common.string());
    }

    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];

        MBOOL status = MTRUE;

        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        String8 const s8Symbol_Common = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            continue;
        }

        MY_LOGE_IF(0, "Fail for both %s & %s", s8Symbol_Sensor.string(), s8Symbol_Common.string());
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kCommonStaticMetadataNames[] =
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

/******************************************************************************
 *
 ******************************************************************************/
static
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

/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kPlatformOverwriteStaticMetadataNames[] =
{
    // "MODULE"
    // "LENS",
    "MOD_OVERWRITE",
    // // "PROJECT",
    // "PRO_OVERWRITE",
    NULL
};

enum {
    COMMON_COMMON       = 0x1,
    COMMON_SENSOR       = 0x1 << 1,
    PLATFORM_COMMON     = 0x1 << 2,
    PLATFORM_SENSOR     = 0x1 << 3,
    PROJECT_COMMON      = 0x1 << 4,
    PROJECT_SENSOR      = 0x1 << 5,
};

static
MBOOL
impBuildStaticInfo_v1(
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
    // step1. config common setting
    String8 strResult = String8::format("<load custom folder>\n\tSTATIC_COMMON: ");
    for (int i = 0; NULL != kCommonStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kCommonStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status==MTRUE ) loadResult|=COMMON_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_COMMON_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status==MTRUE ) loadResult|=COMMON_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }

    //
    strResult += String8::format("\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
        if ( status_s!=MTRUE && status_d!=MTRUE ) {
            MY_LOGE("%s", strResult.string());
            return MFALSE;
        }
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    MY_LOGI("%s", strResult.string());
#if 1   // design for backward compatible
    for (int i = 0; NULL != kStaticMetadataTypeNames[i]; i++)
    {
        char const*const pTypeName = kStaticMetadataTypeNames[i];
        MBOOL status = MTRUE;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            MY_LOGW("project configuration exists!");
            return MFALSE;
        }
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "COMMON");
        status = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if  ( MTRUE == status ) {
            MY_LOGW("project configuration exists!");
            return MFALSE;
        }
    }
#endif
    return MTRUE;
}

static
MBOOL
impBuildStaticInfo_v1_overwrite(
    Info const&     rInfo,
    IMetadata&      rMetadata
)
{
    // step1. config common setting
    String8 strResult = String8::format("<load custom folder - overwrite>\n\tSTATIC_PLATFORM: ");
    for (int i = 0; NULL != kPlatformOverwriteStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOverwriteStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PLATFORM_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PLATFORM_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PLATFORM_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    strResult += String8::format("\n\tSTATIC_PROJECT: ");
    for (int i = 0; NULL != kPlatformOverwriteStaticMetadataNames[i]; i++)
    {
        char const*const pTypeName = kPlatformOverwriteStaticMetadataNames[i];
        int32_t loadResult = 0;
        MBOOL status_d = MTRUE, status_s = MTRUE;
        //
        String8 const s8Symbol_Common = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, "DEFAULT");
        status_d = impConstructStaticMetadata_by_SymbolName(s8Symbol_Common, rInfo, rMetadata);
        if ( status_d==MTRUE ) loadResult|=PROJECT_COMMON;
        //
        String8 const s8Symbol_Sensor = String8::format("%s_PROJECT_V1_%s_%s", PREFIX_FUNCTION_STATIC_METADATA, pTypeName, rInfo.getSensorDrvName().string());
        status_s = impConstructStaticMetadata_by_SymbolName(s8Symbol_Sensor, rInfo, rMetadata);
        if ( status_s==MTRUE ) loadResult|=PROJECT_SENSOR;
        //
        strResult += String8::format("[%s]:%d; ", pTypeName, loadResult);
    }
    //
    MY_LOGI("%s", strResult.string());
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
static
void
updateAFData(IMetadata& rMetadata)
{
    auto eMinFocusDistance = rMetadata.entryFor(MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE);
    if ( eMinFocusDistance.isEmpty() ) {
        MY_LOGF("MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE must not be empty");
    } else {
        MFLOAT value = eMinFocusDistance.itemAt(0, Type2Type<MFLOAT>() );
        if ( value != .0f ) {   // not fixed focus, update
            MY_LOGD("MTK_LENS_INFO_MINIMUM_FOCUS_DISTANCE: %f, add AF modes & regions", value);
            // MTK_CONTROL_AF_AVAILABLE_MODES
            IMetadata::IEntry afAvailableModes(MTK_CONTROL_AF_AVAILABLE_MODES);
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_OFF, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_AUTO, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_MACRO, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO, Type2Type<MUINT8>() );
            afAvailableModes.push_back(MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE, Type2Type<MUINT8>() );
            // afAvailableModes.push_back(MTK_CONTROL_AF_MODE_EDOF, Type2Type<MUINT8>() );
            rMetadata.update(afAvailableModes.tag(), afAvailableModes);

            // MTK_CONTROL_MAX_REGIONS
            IMetadata::IEntry maxRegions(MTK_CONTROL_MAX_REGIONS);
            maxRegions.push_back(1, Type2Type<MINT32>() );
            maxRegions.push_back(1, Type2Type<MINT32>() );
            maxRegions.push_back(1, Type2Type<MINT32>() );
            rMetadata.update(maxRegions.tag(), maxRegions);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensorList::
buildStaticInfo(Info const& rInfo, IMetadata& rMetadata) const
{
    SensorStaticInfo     *pSensorStaticInfo = &sensorStaticInfo[rInfo.getDeviceId()];

    MUINT8 u8Para = 0;

#if MTKCAM_CUSTOM_METADATA_COMMON
    bool v1 = true;
    if ( property_get_int32("vendor.debug.camera.static_meta.version", 1)!=0 ) {
        if ( !impBuildStaticInfo_v1(rInfo, rMetadata) ) {
            v1 = false;
            MY_LOGW( "V1: Fail to build static info for %s index:%d type:%d",
                     rInfo.getSensorDrvName().string(), rInfo.getDeviceId(), rInfo.getSensorType());
        } else {
            updateAFData(rMetadata);
            goto lbLoadDone;
        }
    }
#endif

    if  (!impBuildStaticInfo(rInfo, rMetadata))
    {
        MY_LOGE(
            "Fail to build static info for %s index:%d type:%d",
            rInfo.getSensorDrvName().string(), rInfo.getDeviceId(), rInfo.getSensorType()
        );
        //return  MFALSE;
    }

lbLoadDone:
    /*METEDATA Ref  //system/media/camera/docs/docs.html*/
    //using full size
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        MRect region1(MPoint(pSensorStaticInfo->SensorGrabStartX_CAP,pSensorStaticInfo->SensorGrabStartY_CAP), MSize(pSensorStaticInfo->captureWidth,pSensorStaticInfo->captureHeight));
        entryA.push_back(region1, Type2Type<MRect>());
        rMetadata.update(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, entryA);

        MY_LOGD("MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION(%d, %d, %d, %d)", pSensorStaticInfo->SensorGrabStartX_CAP, pSensorStaticInfo->SensorGrabStartY_CAP,
            pSensorStaticInfo->captureWidth, pSensorStaticInfo->captureHeight);
    }
    //using full size(No correction)
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE);
        entryA.push_back(pSensorStaticInfo->SensorGrabStartX_CAP, Type2Type<MINT32>());
        entryA.push_back(pSensorStaticInfo->SensorGrabStartY_CAP, Type2Type<MINT32>());
        entryA.push_back(pSensorStaticInfo->captureWidth, Type2Type<MINT32>());
        entryA.push_back(pSensorStaticInfo->captureHeight, Type2Type<MINT32>());
        rMetadata.update(MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE, entryA);

        MY_LOGD("MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE(%d, %d, %d, %d)", pSensorStaticInfo->SensorGrabStartX_CAP, pSensorStaticInfo->SensorGrabStartY_CAP,
            pSensorStaticInfo->captureWidth, pSensorStaticInfo->captureHeight);
    }
    //Pixel arry
    {
        SensorDrv *const pSensorDrv = SensorDrv::get();
        MUINT32 scenario = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG; /*capture mode*/
        SensorCropWinInfo rSensorCropInfo;

        ::memset(&rSensorCropInfo, 0, sizeof(SensorCropWinInfo));
        pSensorDrv->sendCommand((IMGSENSOR_SENSOR_IDX) rInfo.getDeviceId(),
                CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO,
                (MUINTPTR)&scenario,
                (MUINTPTR)&rSensorCropInfo,
                0);

        IMetadata::IEntry entryA(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE);
        MSize Size1(rSensorCropInfo.full_w, rSensorCropInfo.full_h);
        entryA.push_back(Size1, Type2Type<MSize>());
        rMetadata.update(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, entryA);
    }
    //Color filter
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT);
        switch(pSensorStaticInfo->sensorFormatOrder) {
            case SENSOR_FORMAT_ORDER_RAW_B:
                u8Para = 0x3;//BGGR
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gb:
                u8Para = 0x2;//GBRG
                break;
            case SENSOR_FORMAT_ORDER_RAW_Gr:
                u8Para = 0x1;//GRBG
                break;
            case SENSOR_FORMAT_ORDER_RAW_R:
                u8Para = 0x0;//RGGB
                break;
            default:
                u8Para = 0x4;//BGR not bayer
                break;
        }
        entryA.push_back(u8Para, Type2Type<MUINT8>());
        rMetadata.update(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, entryA);
    }
    //Orientation
    {
        IMetadata::IEntry entryA(MTK_SENSOR_INFO_ORIENTATION);
        entryA.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        rMetadata.update(MTK_SENSOR_INFO_ORIENTATION, entryA);

        // IMetadata::IEntry entryB(MTK_SENSOR_INFO_WANTED_ORIENTATION);
        // entryB.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        // rMetadata.update(MTK_SENSOR_INFO_WANTED_ORIENTATION, entryB);
    }
    //AOSP Orientation & Facing
    {
        IMetadata::IEntry entryA(MTK_SENSOR_ORIENTATION);
        entryA.push_back((MINT32)pSensorStaticInfo->orientationAngle, Type2Type<MINT32>());
        rMetadata.update(MTK_SENSOR_ORIENTATION, entryA);

        IMetadata::IEntry entryB(MTK_LENS_FACING);
        if ( pSensorStaticInfo->facingDirection==0 )
            entryB.push_back(MTK_LENS_FACING_BACK, Type2Type<MUINT8>());
        else
            entryB.push_back(MTK_LENS_FACING_FRONT, Type2Type<MUINT8>());
        rMetadata.update(MTK_LENS_FACING, entryB);
    }

    //Sensor manual add tag list
    {
        IMetadata::IEntry entryA(MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED);
        entryA.push_back((MINT64)MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_ORIENTATION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_INFO_WANTED_ORIENTATION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_SENSOR_ORIENTATION, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_LENS_FACING, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_CONTROL_AF_AVAILABLE_MODES, Type2Type<MINT64>());
        entryA.push_back((MINT64)MTK_CONTROL_MAX_REGIONS, Type2Type<MINT64>());
        rMetadata.update(MTK_MULTI_CAM_FEATURE_SENSOR_MANUAL_UPDATED, entryA);
    }

#if MTKCAM_CUSTOM_METADATA_COMMON
    if ( v1 )
        impBuildStaticInfo_v1_overwrite(rInfo, rMetadata);
#endif

    rMetadata.sort();

    return  MTRUE;
}

