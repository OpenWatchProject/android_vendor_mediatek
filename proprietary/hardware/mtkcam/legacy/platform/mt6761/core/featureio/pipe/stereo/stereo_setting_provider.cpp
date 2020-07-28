/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "stereo_setting_provider.h"

#include <mtkcam/featureio/pip_hal.h>   // For PIP Hal GetMaxFrameRate().
#include <mtkcam/drv/IHalSensor.h>      //in common include folder
#include <ImageFormat.h>    //hardware/include/mtkcam
#include "camera_custom_stereo.h"       // For CUST_STEREO_* definitions.
#include <utils/Log.h>
#include <math.h>
#include <mtkcam/drv/IHalSensor.h>

#include <cutils/properties.h>

#define LOG_TAG "StereoSettingProvider"

bool StereoSettingProvider::bEnableLog = true;

bool
StereoSettingProvider::getStereoSensorIndex(int32_t &main1Idx, int32_t &main2Idx)
{
    main1Idx = 0;
    main2Idx = 0;
 #if 2 == STEREO_CAM_FACING  //Stereo module at front
    String8 const s8MainIdKey("MTK_SENSOR_DEV_SUB");
    String8 const s8Main2IdKey("MTK_SENSOR_DEV_MAIN_2");
 #else
    String8 const s8MainIdKey("MTK_SENSOR_DEV_MAIN");
    String8 const s8Main2IdKey("MTK_SENSOR_DEV_MAIN_2");
 #endif
    //
    bool result1 = Utils::Property::tryGet(s8MainIdKey,  main1Idx);
    bool result2 = Utils::Property::tryGet(s8Main2IdKey, main2Idx);
    CAM_LOGD("Main sensor idx %d, Main2 sensor idx %d", main1Idx, main2Idx);
    //
    return (result1 && result2);
}

bool
StereoSettingProvider::getStereoSensorDevIndex(int32_t &main1DevIdx, int32_t &main2DevIdx)
{
    int32_t main1Idx = 0;
    int32_t main2Idx = 0;
    if (!getStereoSensorIndex(main1Idx, main2Idx)) {
        return false;
    }
    IHalSensorList *sensorList = IHalSensorList::get();
    if (NULL == sensorList) {
        return false;
    }
    const char *strCallerName = "StereoSettingProvider";
    IHalSensor *pIHalSensor = sensorList->createSensor(strCallerName, main1Idx);
    if (NULL == pIHalSensor) {
        return false;
    }
    main1DevIdx = sensorList->querySensorDevIdx(main1Idx);
    main2DevIdx = sensorList->querySensorDevIdx(main2Idx);
    pIHalSensor->destroyInstance(strCallerName);
    CAM_LOGD("Main sensor DEV idx %d, Main2 sensor DEV idx %d", main1DevIdx, main2DevIdx);
    //
    return true;
}

bool
StereoSettingProvider::getStereoParams(MINT32 const main_idx, MINT32 const main2_idx, STEREO_PARAMS_T &OutData)
{
    // hw limiation
    MUINT32 capHwFps_Main = 0, capHwFps_Main2 = 0;
    MUINT32 prvHwFps_Main = 0, prvHwFps_Main2 = 0;
    // sensor ability
    MUINT32 capFps_Main = 0, capFps_Main2 = 0;
    MUINT32 prvFps_Main = 0, prvFps_Main2 = 0;
    PipHal *pPipHal = PipHal::createInstance(main_idx);
    pPipHal->GetHwMaxFrameRate(NSCam::SENSOR_DEV_MAIN,   capHwFps_Main,  prvHwFps_Main, true);
    pPipHal->GetHwMaxFrameRate(NSCam::SENSOR_DEV_MAIN_2, capHwFps_Main2, prvHwFps_Main2, true);
    pPipHal->GetSensorMaxFrameRate(NSCam::SENSOR_DEV_MAIN,   capFps_Main,  prvFps_Main);
    pPipHal->GetSensorMaxFrameRate(NSCam::SENSOR_DEV_MAIN_2, capFps_Main2, prvFps_Main2);  // main2 always full size(capFps)
    pPipHal->destroyInstance("StereoSettingProvider");
    //
    MUINT32 const capHwFps  = (capHwFps_Main < capHwFps_Main2) ? capHwFps_Main : capHwFps_Main2;
    MUINT32 const prvHwFps  = (prvHwFps_Main < prvHwFps_Main2) ? prvHwFps_Main : prvHwFps_Main2;
    MUINT32 const capFps    = (capFps_Main < capFps_Main2) ? capFps_Main : capFps_Main2;
    MUINT32 const prvFps    = (prvFps_Main < capFps_Main2) ? prvFps_Main : capFps_Main2;    // NOTE: main2 always full size
    OutData.captureFps      = (capHwFps < capFps) ? capHwFps : capFps;
    OutData.previewFps      = (prvHwFps < prvFps) ? prvHwFps : prvFps;
    CAM_LOGD("main1_FPS(ZSD:%d, NonZSD:%d), main2_FPS(ZSD:%d, NonZSD:%d), dualcam_FPS(ZSD:%d, NonZSD:%d)",
            capFps_Main,prvFps_Main,capFps_Main2,prvFps_Main2,OutData.captureFps,OutData.previewFps);
    //
    static char strJPSSize[12];
    sprintf(strJPSSize, "%dx%d", getAlgoImageSize(0).w*2, getAlgoImageSize(0).h);
    OutData.jpsSize     = strJPSSize;
    OutData.jpsSizesStr = strJPSSize;
    //
    StereoResolution algoResolution = getStereoAlgoInputResolution();
    CAM_LOGD("SenMaxCrop(%d),rrz_crop1(%d,%d,%dx%d),rrz_crop2(%d,%d,%dx%d),rrz1(%dx%d),rrz2(%dx%d),SenMaxMain1(%d),M1_full(%dx%d),ratio(%d:%d)",
        algoResolution.bUseSensorMaxCropSz,
        algoResolution.p1_rrz_crop_main1.p.x,algoResolution.p1_rrz_crop_main1.p.y,algoResolution.p1_rrz_crop_main1.s.w,algoResolution.p1_rrz_crop_main1.s.h,
        algoResolution.p1_rrz_crop_main2.p.x,algoResolution.p1_rrz_crop_main2.p.y,algoResolution.p1_rrz_crop_main2.s.w,algoResolution.p1_rrz_crop_main2.s.h,
        algoResolution.szMain1.w,algoResolution.szMain1.h,
        algoResolution.szMain2.w,algoResolution.szMain2.h,
        algoResolution.bUseSensorMaxMain1FullSz,algoResolution.szMain1_Full.w,algoResolution.szMain1_Full.h,
        algoResolution.uRatioDenomerator,algoResolution.uRatioNumerator);

    static char strJPGSize[15];
    switch (imageRatio()) {
    case eRatio_4_3:        // should not go into this case
        //5M 4:3, for AP output size(gallery image) and PASS2_CAP_DST
        OutData.refocusSize     = (char *)"2592x1944";
        OutData.refocusSizesStr = (char *)"2592x1944,2408x1536,1920x1440,1280x960";
        break;
    case eRatio_16_9:
    default:
        MSize SensorMaxSize = algoResolution.szMain1_Full;
        if(algoResolution.bUseSensorMaxMain1FullSz)
        {
            IHalSensorList* sensorlsit = IHalSensorList::get();
            IHalSensor* pIHalSensor = sensorlsit->createSensor("Stereo Setting Provider", 0);
            if(pIHalSensor==NULL)
            {
                CAM_LOGE("fail to get HALSensor");
            }
            else
            {
                int sensorDevIdx = sensorlsit->querySensorDevIdx(0);
                MUINT32 sensorScenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                SensorCropWinInfo rSensorCropInfo;
                MINT32 err = pIHalSensor->sendCommand(sensorDevIdx, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                               (MUINTPTR)&sensorScenario, (MUINTPTR)&rSensorCropInfo, 0);
                if(err)
                {
                    CAM_LOGE("SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO() for main1 fail. error code: %d.", err);
                }
                else
                {
                    SensorMaxSize.w = rSensorCropInfo.w2_tg_size;
                    SensorMaxSize.h = (rSensorCropInfo.w2_tg_size * 9 / 16) & ~1; //Image size must be even
                }
                pIHalSensor->destroyInstance("Stereo Setting Provider");
                pIHalSensor=NULL;
            }
        }
        sprintf(strJPGSize,"%dx%d",SensorMaxSize.w,SensorMaxSize.h);
        OutData.refocusSize     = strJPGSize;
        OutData.refocusSizesStr = strJPGSize;
        break;
    }
    CAM_LOGD("Main1 Full size resolution: %s", strJPGSize);
    OutData.dualcamCallbackItems = getStereoCallBackBufferItems();
    return true;
}

bool
StereoSettingProvider::getStereoParams(STEREO_PARAMS_T &OutData)
{
    int32_t main_idx;
    int32_t main2_idx;
    if (!getStereoSensorDevIndex(main_idx, main2_idx)) {
        return false;
    }
    return getStereoParams(main_idx, main2_idx, OutData);
}

bool
StereoSettingProvider::hasHWFE()
{
    static bool _hasHWFE = false;


    return _hasHWFE;
}

bool
StereoSettingProvider::getStereoCameraFOV(float &mainFOV, float &main2FOV)
{
    int32_t main1DevIdx = 0;
    int32_t main2DevIdx = 0;
    //
    if (!getStereoSensorDevIndex(main1DevIdx, main2DevIdx)) {
        return false;
    }
    //
    IHalSensorList *sensorList = IHalSensorList::get();
    if (NULL == sensorList) {
        return false;
    }
    //
    SensorStaticInfo sensorStaticInfo;
    sensorList->querySensorStaticInfo(main1DevIdx, &sensorStaticInfo);
    mainFOV = sensorStaticInfo.horizontalViewAngle;
    //
    sensorList->querySensorStaticInfo(main2DevIdx, &sensorStaticInfo);
    main2FOV = sensorStaticInfo.horizontalViewAngle;
    //
    return true;
}

bool
StereoSettingProvider::getStereoCameraSensorBit(MUINT32 &main1_bits, MUINT32 &main2_bits)
{
    int32_t main1DevIdx = 0;
    int32_t main2DevIdx = 0;
    main1_bits = main2_bits = 10;
    //
    if (!getStereoSensorDevIndex(main1DevIdx, main2DevIdx)) {
        return false;
    }
    //
    IHalSensorList *sensorList = IHalSensorList::get();
    if (NULL == sensorList) {
        return false;
    }
    //
    SensorStaticInfo sensorStaticInfo;
    sensorList->querySensorStaticInfo(main1DevIdx, &sensorStaticInfo);
    switch(sensorStaticInfo.rawSensorBit)
    {
        case RAW_SENSOR_8BIT:
            main1_bits = 8;
            break;
        case RAW_SENSOR_10BIT:
            main1_bits = 10;
            break;
        case RAW_SENSOR_12BIT:
            main1_bits = 12;
            break;
        case RAW_SENSOR_14BIT:
            main1_bits = 14;
            break;
        default:
            main1_bits = 10;
            break;
    }
    //
    sensorList->querySensorStaticInfo(main2DevIdx, &sensorStaticInfo);
    switch(sensorStaticInfo.rawSensorBit)
    {
        case RAW_SENSOR_8BIT:
            main2_bits = 8;
            break;
        case RAW_SENSOR_10BIT:
            main2_bits = 10;
            break;
        case RAW_SENSOR_12BIT:
            main2_bits = 12;
            break;
        case RAW_SENSOR_14BIT:
            main2_bits = 14;
            break;
        default:
            main2_bits = 10;
            break;
    }
    //
    return true;
}


MODULE_ORIENTATION_E
StereoSettingProvider::getModuleOrientation()
{
    return eMODULE_HORIZONTAL;   // 0: horizontal, 1: vertical
}

unsigned int
StereoSettingProvider::getSensorRelativePosition()
{
    // define:
    //    0 is main-main2 (main in L)
    //    1 is main2-main (main in R)
    customSensorPos_STEREO_t SensorPos;
    SensorPos=getSensorPosSTEREO();

    return SensorPos.uSensorPos;
}

const unsigned int
StereoSettingProvider::getJPSTransformFlag()
{
    return eTransform_None;
}

void
StereoSettingProvider::CheckLogStatus()
{
    int i4LogStatusID = -1;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.stereocam.enableProfLog", value, "-1");
    i4LogStatusID = atoi(value);
    if(i4LogStatusID != 1)
    {
        bEnableLog = MFALSE;
    }
    else
    {
        bEnableLog = MTRUE;
    }
}

float
StereoSettingProvider::getFOVCropRatio()
{
    float ratio = 1.0f;
    int32_t main1Idx, main2Idx;
    getStereoSensorIndex(main1Idx, main2Idx);
    float sensorFOV = FOV_LIST[main1Idx].fov_horizontal;
    float targetFOV = TARGET_FOV_LIST[main1Idx].fov_horizontal;
    if(sensorFOV - targetFOV > 0.001f) {
        ratio = tan(targetFOV/2.0f * M_PI / 180.0f) / tan(sensorFOV/2.0f * M_PI / 180.0f);
    }

    return ratio;
}

IMAGE_RESOLUTION_INFO_STRUCT
StereoSettingProvider::imageResolution()
{
    StereoResolution algoResolution = getStereoAlgoInputResolution();
    IMAGE_RESOLUTION_INFO_STRUCT resolution(algoResolution.szMain1,
                                            algoResolution.szMain2,
                                            algoResolution.uRatioDenomerator,
                                            algoResolution.uRatioNumerator);
    return resolution;
}

MSize
StereoSettingProvider::getDepthmapSize()
{
    return getStereoDepthmapSize();
}

void
StereoSettingProvider::getMain1FullSzCrop(MBOOL &bSensorMaxCrop, MRect &main1FullCrop)
{
    StereoResolution algoResolution = getStereoAlgoInputResolution();
    bSensorMaxCrop = algoResolution.bUseSensorMaxMain1FullSz;
    main1FullCrop = algoResolution.main1FullSz_crop;
}

MSize
StereoSettingProvider::getAlgoImageSize(MINT32 idx)
{
    StereoResolution algoResolution = getStereoAlgoInputResolution();
    if(idx == 0)
        return algoResolution.szMain1;
    else
        return algoResolution.szMain2;
}

bool
StereoSettingProvider::IsMain2MonoSensor()
{
    int32_t is_main2_mono_sensor=0;
    String8 const s8MainIdKey("IS_MAIN2_MONO");
    //
    bool result1 = Utils::Property::tryGet(s8MainIdKey,  is_main2_mono_sensor);
    CAM_LOGD("IS_MAIN2_MONO : %d", is_main2_mono_sensor);
    //
    return (is_main2_mono_sensor)? true:false;
}

bool
StereoSettingProvider::IsMain2YuvSensor()
{
    int32_t is_main2_yuv_sensor=0;
    String8 const s8MainIdKey("IS_MAIN2_YUV");
    //
    bool result1 = Utils::Property::tryGet(s8MainIdKey,  is_main2_yuv_sensor);
    CAM_LOGD("IS_MAIN2_YUV : %d", is_main2_yuv_sensor);
    //
    return (is_main2_yuv_sensor)? true:false;
}

