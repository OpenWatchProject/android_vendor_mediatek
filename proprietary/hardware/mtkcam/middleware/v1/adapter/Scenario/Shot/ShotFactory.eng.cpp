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

#define LOG_TAG "MtkCam/Shot"
//
#include <unistd.h>
#include <dlfcn.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
using namespace NSCam;
#include <mtkcam/middleware/v1/IParamsManager.h>
//
#include <mtkcam/middleware/v1/IShot.h>
#include <cutils/properties.h>

#include "./EngShot/EngParam.h"

#include <mtkcam/feature/hdrDetection/Defs.h>
#include <cutils/compiler.h>

//
using namespace android;
using namespace NSShot;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(" (%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(" (%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(" (%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(" (%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(" (%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(" (%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(" (%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *  Engineer Shot
 ******************************************************************************/
extern
sp<IShot>
createInstance_SmartShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

extern
sp<IShot>
createInstance_EngShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);

#if MTKCAM_HAVE_MFB_SUPPORT
// if defined IVENDOR_SUPPORT, to use SmartShotEng for MFNR EM
#if MTKCAM_HAVE_IVENDOR_SUPPORT
#define MAKE_MFNREngShot(name, mode, id) createInstance_SmartShotEng(name, mode, id)
extern
sp<IShot> createInstance_SmartShotEng(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);
#else
#define MAKE_MFNREngShot(name, mode, id) createInstance_MfllShotEng(name, mode, id)
extern
sp<IShot> createInstance_MfllShotEng (
    char const * const  pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
);
#endif // MTKCAM_HAVE_IVENDOR_SUPPORT
#else
#define MAKE_MFNREngShot(name, mode, id) createInstance_EngShot(name, mode, id)
#endif


/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createEngShotInstance(
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    sp<IParamsManager>  pParamsMgr
)
{
    EngParam engParam;
    //*** Start of engineer mode part
    //isp profile
    engParam.u4ISPProfile =  pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_ISP_PROFILE);
    MY_LOGD("engParam.u4ISPProfile  = %d", engParam.u4ISPProfile);
    //
    engParam.u4EVvalue = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_EV_VALUE);
    //EVBreacket enable
    engParam.mi4EngRawEVBEn = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_EVB_ENABLE);
    // Multi-NR
    engParam.mi4ManualMultiNREn = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_MULTI_NR_ENABLE);
    if (1 == engParam.mi4ManualMultiNREn)
    {
        engParam.mi4ManualMultiNRType = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MANUAL_MULTI_NR_TYPE);
    }
    // CCT MFLL
    engParam.mi4CamShotType = EngParam::ENG_CAMSHOT_TYPE_SINGLE; // default value
    if (pParamsMgr->getStr(MtkCameraParameters::KEY_MFB_MODE) == MtkCameraParameters::KEY_MFB_MODE_MFLL
            &&
        pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MFLL_PICTURE_COUNT) > 0
       )
    {
        engParam.mi4CamShotType = EngParam::ENG_CAMSHOT_TYPE_MFLL;
        engParam.mi4MFLLpc = pParamsMgr->getInt(MtkCameraParameters::KEY_ENG_MFLL_PICTURE_COUNT);
        MY_LOGD("engParam.mi4MFLLpc = %d", engParam.mi4MFLLpc);
    }
    MY_LOGD("engParam.mi4CamShotType = %d", engParam.mi4CamShotType);

    String8 ms8IspMode = pParamsMgr->getStr(MtkCameraParameters::KEY_ISP_MODE);
    const char *strIspMode = ms8IspMode.string();
    engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_FORCE_PROCESSED_RAW; // Default value
    switch (strIspMode[0])
    {
        case EngParam::ENG_ISP_MODE_PROCESSED_RAW:
            engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_FORCE_PROCESSED_RAW;
            break;
        case EngParam::ENG_ISP_MODE_PURE_RAW:
            engParam.mi4EngIspMode = EngParam::ENG_RAW_TYPE_PURE_RAW;
            break;
    }
    MY_LOGD("engParam.mi4EngIspMode = %d", engParam.mi4EngIspMode);

    engParam.u4VHDState = (pParamsMgr->getVHdr() == SENSOR_VHDR_MODE_ZVHDR) ? SENSOR_VHDR_MODE_ZVHDR : SENSOR_VHDR_MODE_NONE;
    engParam.bIsAutoHDR = (pParamsMgr->getHDRMode() == HDRMode::AUTO) ||
                          (pParamsMgr->getHDRMode() == HDRMode::VIDEO_AUTO);
    MY_LOGD("VHDState(%d) auto(%s)",
            engParam.u4VHDState, engParam.bIsAutoHDR ? "on" : "off");

    String8 ms8SaveMode = pParamsMgr->getStr(MtkCameraParameters::KEY_RAW_SAVE_MODE);
    const char *strSaveMode = ms8SaveMode.string();
    switch (atoi(strSaveMode))
    {
        case 1: // 1: "Preview Mode",
            engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_NORMAL_PREVIEW;
            engParam.mi4EngRawSaveEn = 1;
            break;
        case 2: // 2: "Capture Mode",
            engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_NORMAL_CAPTURE;
            engParam.mi4EngRawSaveEn = 1;
            break;
        case 4: // 4: "Video Preview Mode"
            engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_NORMAL_VIDEO;
            engParam.mi4EngRawSaveEn = 1;
            break;
        case 5: // 4: "Slim Video 1"
            engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_SLIM_VIDEO1;
            engParam.mi4EngRawSaveEn = 1;
            break;
        case 6: // 4: "Slim Video 2"
            engParam.mi4EngSensorMode = EngParam::ENG_SENSOR_MODE_SLIM_VIDEO2;
            engParam.mi4EngRawSaveEn = 1;
            break;
        case 0: // 0: do not save
        case 3: // 3: "JPEG Only"
             engParam.mi4EngRawSaveEn = 0;
            break;
        default:
            // Extension for sensor scenario, map to driver scenario
                  if (atoi(strSaveMode) > 6)
                  {
                   engParam.mi4EngSensorMode = atoi(strSaveMode) - 2;
                   engParam.mi4EngRawSaveEn = 1;
                  }
                  else
              {
                   engParam.mi4EngRawSaveEn = 0;
              }
            break;
    }
    MY_LOGD("mi4EngRawSaveEn, mi4EngSensorMode =(%d, %d)", engParam.mi4EngRawSaveEn, engParam.mi4EngSensorMode);
    //
    String8 ms8RawFilePath(pParamsMgr->getStr(MtkCameraParameters::KEY_RAW_PATH)); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0.raw
    char const *mpsz8RawSaveMode = ms8SaveMode.string();
    switch (atoi(mpsz8RawSaveMode))
    {
        MY_LOGD("mpsz8RawSaveMode==1");
        case 0: // 0: do not save
        case 3: // 3: "JPEG Only"
            break;
        case 1: // 1: "Preview Mode",
        case 2: // 2: "Capture Mode",
        case 4: // 4: "Video Preview Mode"
        case 5: // 5: "Slim Video1"
        case 6: // 6: "Slim Video2"
        default:
            if (ms8RawFilePath != "")
            {
                String8 ms8RawFileExt(ms8RawFilePath.getPathExtension()); // => .raw
                ms8RawFilePath = ms8RawFilePath.getBasePath(); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0

                String8 ms8IspMode = pParamsMgr->getStr(MtkCameraParameters::KEY_ISP_MODE);
                const char *strIspMode = ms8IspMode.string();
                String8 ms8RawType;
                switch (strIspMode[0])
                {
                    case EngParam::ENG_ISP_MODE_PROCESSED_RAW:
                        ms8RawType = "proc";
                        break;
                    case EngParam::ENG_ISP_MODE_PURE_RAW:
                        ms8RawType = "pure";
                        break;
                }
                //
                char mpszSuffix[256] = {0};
                sprintf(mpszSuffix, "%s", ms8RawType.string()); /* info from EngShot::onCmd_capture */
                ms8RawFilePath.append(mpszSuffix);
                ms8RawFilePath.append(ms8RawFileExt);
                engParam.ms8RawFilePath = ms8RawFilePath;
                MY_LOGD("Raw saved path: %s", ms8RawFilePath.string());
            }
    }
    //*** End of engineer mode part
    sp<IShot> rpShot;
    if (engParam.mi4CamShotType == EngParam::ENG_CAMSHOT_TYPE_MFLL) {
        MY_LOGD("create MfllShotEng");
        rpShot = MAKE_MFNREngShot("EngShot", u4ShotMode, i4OpenId);
    }
    else {
        rpShot = createInstance_EngShot("EngShot", u4ShotMode, i4OpenId);
    }

    rpShot->sendCommand(eCmd_setEngParam,  (MUINTPTR)&engParam, sizeof(EngParam));

    return rpShot;

}

