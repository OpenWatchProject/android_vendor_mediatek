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
#define LOG_TAG "StereoSizeProvider"

#include <math.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/utils/std/Log.h>
#include "pass2/pass2A_size_providers.h"
#include <stereo_crop_util.h>
#include <vsdof/hal/ProfileUtil.h>
#include <stereo_tuning_provider.h>
#include "../setting-provider/stereo_setting_provider_kernel.h"

using namespace android;
using namespace NSCam;

#define STEREO_SIZE_PROVIDER_DEBUG

#ifdef STEREO_SIZE_PROVIDER_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)
#endif  // STEREO_SIZE_PROVIDER_DEBUG

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

//===============================================================
//  StereoSize implementation
//===============================================================
// 16:9
// Horizontal: 272x144, 32x8, (16,4), content 240x136
const StereoArea STEREO_BASE_SIZE_16_9_HORIZONTAL_MODULE(272, 144, 32, 8, 16, 4);
// Vertical: 176x264, 32x8, (16,4), content 144x256
const StereoArea STEREO_BASE_SIZE_16_9_VERTICAL_MODULE(176, 264, 32, 8, 16, 4);
// 4:3
// Horizontal: 240x164, 32x8, (16,4), content 208x156
const StereoArea STEREO_BASE_SIZE_4_3_HORIZONTAL_MODULE(240, 164, 32, 8, 16, 4);
// Vertical: 192x222, 32x8, (16,4), content 160x214
const StereoArea STEREO_BASE_SIZE_4_3_VERTICAL_MODULE(192, 222, 32, 8, 16, 4);

StereoArea
StereoSize::getStereoArea1x(const StereoSizeConfig &config)
{
    StereoArea area;

    ENUM_STEREO_RATIO imageRatio = StereoSettingProvider::imageRatio();
    const bool IS_VERTICAL_MODULE = (StereoSettingProvider::getModuleRotation() & 0x2);
    if(!StereoSettingProvider::isDeNoise() &&
       eRatio_16_9 == imageRatio)
    {
        area = (IS_VERTICAL_MODULE)
               ? STEREO_BASE_SIZE_16_9_VERTICAL_MODULE
               : STEREO_BASE_SIZE_16_9_HORIZONTAL_MODULE;
    } else {
        area = (IS_VERTICAL_MODULE)
               ? STEREO_BASE_SIZE_4_3_VERTICAL_MODULE
               : STEREO_BASE_SIZE_4_3_HORIZONTAL_MODULE;
    }

    if(!config.hasPadding) {
        area.removePadding();
    }

    //Vertical size is rotated, only rotate for horizontal request
    if(IS_VERTICAL_MODULE &&
       !config.needRotate)
    {
        area.rotate(eRotate_90);
    }

    return area;
}

//===============================================================
//  StereoSizeProvider implementation
//===============================================================

//===============================================================
//  Singleton and init operations
//===============================================================
Mutex StereoSizeProvider::__lock;
Mutex StereoSizeProvider::__P1Lock;

StereoSizeProvider *
StereoSizeProvider::getInstance()
{
    Mutex::Autolock lock(__lock);
    static StereoSizeProvider _instance;
    return &_instance;
}

StereoSizeProvider::StereoSizeProvider()
    : LOG_ENABLED(StereoSettingProvider::isLogEnabled())
{
}

bool
StereoSizeProvider::getPass1Size( ENUM_STEREO_SENSOR sensor,
                                  EImageFormat format,
                                  EPortIndex port,
                                  ENUM_STEREO_SCENARIO scenario __attribute__((unused)),
                                  MRect &tgCropRect,
                                  MSize &outSize,
                                  MUINT32 &strideInBytes,
                                  ENUM_STEREO_RATIO requiredRatio
                                )
{
    Mutex::Autolock lock(__P1Lock);
    // AutoProfileUtil profile(LOG_TAG, "getPass1Size");

    ENUM_STEREO_RATIO currentRatio = (EPortIndex_IMGO == port) ? eRatio_4_3 : StereoSettingProvider::imageRatio();
    if(requiredRatio != eRatio_Unknown){
        MY_LOGD_IF(LOG_ENABLED, "user required ratio:%d", requiredRatio);
        currentRatio = requiredRatio;
    }

    int sensorScenario = (eSTEREO_SENSOR_MAIN1 == sensor)
                         ? StereoSettingProvider::getSensorScenarioMain1()
                         : StereoSettingProvider::getSensorScenarioMain2();
    if(SENSOR_SCENARIO_ID_UNNAMED_START == sensorScenario) {
        MY_LOGE("Unknown sensor scenario for sensor %d", sensor-1);
        return false;
    }

    enum {
        SENSOR_BITS          = 1,
        PORT_BITS            = 1,
        SENSOR_SCENARIO_BITS = 4,
        RATIO_BITS           = 2,
        PROFILE_BITS         = 3,
        FEATURE_MODE_BITS    = 6,
    };

    MUINT32 key = (eSTEREO_SENSOR_MAIN1 == sensor) ? 0 : 1;
    int shiftBits = SENSOR_BITS;
    key |= ((port == EPortIndex_RRZO) ? 0 : 1)<<shiftBits;
    shiftBits += PORT_BITS;
    key |= sensorScenario << shiftBits;
    shiftBits += SENSOR_SCENARIO_BITS;
    key |= currentRatio << shiftBits;
    shiftBits += RATIO_BITS;
    key |= StereoSettingProvider::stereoProfile() << shiftBits;
    shiftBits += PROFILE_BITS;
    key |= StereoSettingProvider::getStereoFeatureMode() << shiftBits;
    shiftBits += FEATURE_MODE_BITS;
    key |= format << shiftBits;

    if(__pass1TgCropRectMap.find(key) != __pass1TgCropRectMap.end()) {
        tgCropRect    = __pass1TgCropRectMap.find(key)->second;
        outSize       = __pass1OutSizeMap.find(key)->second;
        strideInBytes = __pass1StrideMap.find(key)->second;

        MY_LOGD_IF(LOG_ENABLED, "Use result of (sensor %d, port %s, format %d, sensor scenario %d, ratio %d, profile %d, feature %d): tg (%d,%d)(%dx%d), outSize %dx%d, stride %d",
                                sensor-1, (EPortIndex_RRZO == port) ? "RRZO" : "IMGO", format, sensorScenario, currentRatio,
                                StereoSettingProvider::stereoProfile(),
                                StereoSettingProvider::getStereoFeatureMode(),
                                tgCropRect.p.x, tgCropRect.p.y, tgCropRect.s.w, tgCropRect.s.h, outSize.w, outSize.h, strideInBytes);
    } else {
        // Get sensor senario
        int sensorScenario = (eSTEREO_SENSOR_MAIN1 == sensor)
                             ? StereoSettingProvider::getSensorScenarioMain1()
                             : StereoSettingProvider::getSensorScenarioMain2();

        // Prepare sensor hal
        IHalSensorList* sensorList = MAKE_HalSensorList();
        if(NULL == sensorList) {
            MY_LOGE("Cannot get sensor list");
            return false;
        }

        MINT32 err = 0;
        int32_t main1Idx, main2Idx;
        StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
        int sendorDevIndex = sensorList->querySensorDevIdx((eSTEREO_SENSOR_MAIN1 == sensor) ? main1Idx : main2Idx);
        IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, (eSTEREO_SENSOR_MAIN1 == sensor) ? main1Idx : main2Idx);
        if(NULL == pIHalSensor) {
            MY_LOGE("Cannot get hal sensor");
            return false;
        }

        //Get pixel mode
        int defaultFPS = 0; //result will be 10xFPS, e.g. if 30 fps, defaultFPS = 300
        err = pIHalSensor->sendCommand(sendorDevIndex, SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                       (MINTPTR)&sensorScenario, (MINTPTR)&defaultFPS, 0);
        if(err) {
            MY_LOGE("Cannot get default frame rate");
            pIHalSensor->destroyInstance(LOG_TAG);
            return false;
        }

        //Get pixel format
        MUINT32 pixelMode;
        defaultFPS /= 10;
        err = pIHalSensor->sendCommand(sendorDevIndex, SENSOR_CMD_GET_SENSOR_PIXELMODE,
                                       (MINTPTR)&sensorScenario, (MINTPTR)&defaultFPS, (MINTPTR)&pixelMode);
        if(err) {
            MY_LOGE("Cannot get pixel mode");
            pIHalSensor->destroyInstance(LOG_TAG);
            return false;
        }

        //Get sensor crop win info
        SensorCropWinInfo rSensorCropInfo;
        err = pIHalSensor->sendCommand(sendorDevIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                       (MUINTPTR)&sensorScenario, (MUINTPTR)&rSensorCropInfo, 0);
        if(err) {
            MY_LOGE("Cannot get sensor crop win info");
            return false;
        }

        tgCropRect.p.x = 0;
        tgCropRect.p.y = 0;
        tgCropRect.s.w = rSensorCropInfo.w2_tg_size;
        tgCropRect.s.h = rSensorCropInfo.h2_tg_size;

        outSize.w = rSensorCropInfo.w2_tg_size;
        outSize.h = rSensorCropInfo.h2_tg_size;

        //Calculate TG CROP without FOV cropping
        int n = 3, m = 4;   // default is 4:3
        switch(currentRatio) {
            case eRatio_16_9:
            default:
                n = 9;
                m = 16;
                break;
            case eRatio_4_3:
                n = 3;
                m = 4;
                break;
        }
        if(StereoSettingProvider::is3rdParty() &&
           EPortIndex_RRZO == port)
        {
            CropUtil::cropRectByImageRatio(tgCropRect, currentRatio);
            outSize = (eSTEREO_SENSOR_MAIN1 == sensor) ? __rrzoYuvSize[0] : __rrzoYuvSize[1];
            //Check min RRZO size
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn input;
            NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get()->query(
                port,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO,
                format,
                input,
                queryRst);
            const int RRZ_CAPIBILITY = queryRst.bs_ratio;
            MSize MIN_RRZ_SIZE((tgCropRect.s.w * RRZ_CAPIBILITY/100 + 1)&~1,
                               (tgCropRect.s.h * RRZ_CAPIBILITY/100 + 1)&~1);

            if(outSize.w < MIN_RRZ_SIZE.w ||
               outSize.h < MIN_RRZ_SIZE.h)
            {
                MY_LOGD_IF(LOG_ENABLED, "RRZO %dx%d->%dx%d (limited by RRZ capibility: %.2f)",
                           outSize.w, outSize.h, MIN_RRZ_SIZE.w, MIN_RRZ_SIZE.h,
                           RRZ_CAPIBILITY/100.0f);
                outSize = MIN_RRZ_SIZE;
            }

            MY_LOGD_IF(LOG_ENABLED, "3rdParty RRZO size=%dx%d,sensor=%d",outSize.w,outSize.h,sensor);
        }
        else if( !StereoSettingProvider::isDeNoise() &&
                EPortIndex_RRZO == port )
        {
            if(rSensorCropInfo.scale_w > rSensorCropInfo.w0_size/2)   //not binning
            {
                outSize.w /= 2;
            }
            outSize.h = outSize.w * n / m;

            CropUtil::cropRectByImageRatio(tgCropRect, currentRatio);
            MY_LOGD_IF(LOG_ENABLED, "Crop TG by image ratio %d:%d, result: offset (%d, %d) size %dx%d",
                                    m, n, tgCropRect.p.x, tgCropRect.p.y, tgCropRect.s.w, tgCropRect.s.h);

            //Calculate outSize, tg size & cropping after FOV cropping
            const float CROP_RATIO = (eSTEREO_SENSOR_MAIN1 == sensor) ? StereoSettingProvider::getFOVCropRatio() : 1.0f;
            if(CROP_RATIO < 1.0f &&
               CROP_RATIO > 0.0f)
            {
                CropUtil::cropRectByFOV(tgCropRect, CROP_RATIO, currentRatio);
                if(EPortIndex_RRZO == port) {
                    outSize.w *= CROP_RATIO;
                    outSize.h *= CROP_RATIO;
                }

                MY_LOGD_IF(LOG_ENABLED, "Crop TG by FOV ratio %.2f, result: tg (%d, %d) %dx%d",
                                        CROP_RATIO, tgCropRect.p.x, tgCropRect.p.y, tgCropRect.s.w, tgCropRect.s.h);
            }

            //Make sure output size meet the following rules:
            // 1. image ratio fit 4:3 or 16:9
            // 2. smaller than TG size
            // 3. Is even number
            if(outSize.w > tgCropRect.s.w ||
               outSize.h > tgCropRect.s.h )
            {
                outSize = tgCropRect.s;
            }

            int baseSize = std::min(outSize.w/(m*2), outSize.h/(n*2));
            outSize.w = baseSize * m * 2;
            outSize.h = baseSize * n * 2;

            //Use smaller size for RRZO if possible
            Pass2SizeInfo p2SizeInfo;
            StereoArea p2SizeNeeded;

            //RRZO is for preview/record only, if scenario is capture, query by preview
            ENUM_STEREO_SCENARIO queryScenario = (eSTEREO_SCENARIO_CAPTURE == scenario)
                                                 ? eSTEREO_SCENARIO_PREVIEW
                                                 : scenario;
            if(eSTEREO_SENSOR_MAIN1 == sensor) {
                //Compare with PASS2A.WDMA (Preview size)
                getPass2SizeInfo(PASS2A, queryScenario, p2SizeInfo);
                p2SizeNeeded = p2SizeInfo.areaWDMA;
            } else {
                //Compare with PASS2A_P.WROT (FE input size)
                getPass2SizeInfo(PASS2A_P, queryScenario, p2SizeInfo);
                p2SizeNeeded = p2SizeInfo.areaWROT.rotatedByModule();
            }

            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn input;
            NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get()->query(
                port,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO,
                format,
                input,
                queryRst);
            const int RRZ_CAPIBILITY = queryRst.bs_ratio;
            MSize MIN_RRZ_SIZE((tgCropRect.s.w * RRZ_CAPIBILITY/100 + 1)&~1,
                               (tgCropRect.s.h * RRZ_CAPIBILITY/100 + 1)&~1);

            if(p2SizeNeeded.size.w >= MIN_RRZ_SIZE.w &&
               p2SizeNeeded.size.h >= MIN_RRZ_SIZE.h)
            {
                MY_LOGD_IF(LOG_ENABLED, "RRZO %dx%d->%dx%d(%d%%) (scenario %d)",
                           outSize.w, outSize.h, p2SizeNeeded.size.w, p2SizeNeeded.size.h,
                           (int)(p2SizeNeeded.size.w*p2SizeNeeded.size.h*100.0f/(outSize.w*outSize.h)),
                           queryScenario);
            } else {
                p2SizeNeeded.size = MIN_RRZ_SIZE;

                MY_LOGD_IF(LOG_ENABLED, "RRZO %dx%d->%dx%d(%d%%) (scenario %d, limited by RRZ capibility: %.2f)",
                           outSize.w, outSize.h, p2SizeNeeded.size.w, p2SizeNeeded.size.h,
                           (int)(p2SizeNeeded.size.w*p2SizeNeeded.size.h*100.0f/(outSize.w*outSize.h)),
                           queryScenario, RRZ_CAPIBILITY/100.0f);
            }

            outSize = p2SizeNeeded.size;
        }

        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn input;
        input.width = outSize.w;
        input.pixMode = (pixelMode == 0x0)? (NSCam::NSIoPipe::NSCamIOPipe::_1_PIX_MODE) : NSCam::NSIoPipe::NSCamIOPipe::_2_PIX_MODE;
        NSCam::NSIoPipe::NSCamIOPipe::INormalPipeModule::get()->query(
                port,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_PIX|
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                format,
                input,
                queryRst);

        strideInBytes = queryRst.stride_byte;
        outSize.w = queryRst.x_pix;

        pIHalSensor->destroyInstance(LOG_TAG);

        __pass1TgCropRectMap[key] = tgCropRect;
        __pass1OutSizeMap[key]    = outSize;
        __pass1StrideMap[key]     = strideInBytes;

        MY_LOGD_IF(LOG_ENABLED, "New result for (sensor %d, port %s, format %d, sensor scenario %d, ratio %d, profile %d, feature %d): tg (%d,%d)(%dx%d), outSize %dx%d, stride %d",
                                sensor-1, (EPortIndex_RRZO == port) ? "RRZO" : "IMGO", format, sensorScenario, currentRatio,
                                StereoSettingProvider::stereoProfile(),
                                StereoSettingProvider::getStereoFeatureMode(),
                                tgCropRect.p.x, tgCropRect.p.y, tgCropRect.s.w, tgCropRect.s.h, outSize.w, outSize.h, strideInBytes);
    }

    return true;
}

template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if (pMetadata == NULL) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    //
    return MFALSE;
}

bool
StereoSizeProvider::getPass1ActiveArrayCrop(ENUM_STEREO_SENSOR sensor, MRect &cropRect)
{
    int main1Id, main2Id;
    StereoSettingProvider::getStereoSensorIndex(main1Id, main2Id);
    int sensorId = -1;
    switch(sensor)
    {
        case eSTEREO_SENSOR_MAIN1:
            sensorId = main1Id;
            break;
        case eSTEREO_SENSOR_MAIN2:
            sensorId = main2Id;
            break;
        default:
            break;
    }

    if(sensorId < 0) {
        MY_LOGW("Wrong sensor: %d", sensorId);
        return false;
    }

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(sensorId);
    if( ! pMetadataProvider.get() ) {
        MY_LOGE("MetadataProvider is NULL");
        return false;
    }

    bool result = false;
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    if( tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, cropRect) ) {
        cropRect.s.w &= ~1;
        cropRect.s.h &= ~1;
        if(eRatio_16_9 == StereoSettingProvider::imageRatio()) {
            int newHeight = cropRect.s.h;
            newHeight = (cropRect.s.w * 9 / 16) & ~1;
            cropRect.p.y = (cropRect.s.h-newHeight)/2;
            cropRect.s.h = newHeight;
        }

        result = true;
    }

    return result;
}

bool
StereoSizeProvider::getPass2SizeInfo(ENUM_PASS2_ROUND round, ENUM_STEREO_SCENARIO eScenario, Pass2SizeInfo &pass2SizeInfo) const
{
    bool isSuccess = true;
    switch(round) {
        case PASS2A:
            pass2SizeInfo = Pass2A_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_2:
            pass2SizeInfo = Pass2A_2_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_3:
            pass2SizeInfo = Pass2A_3_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_P:
            pass2SizeInfo = Pass2A_P_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_P_2:
            pass2SizeInfo = Pass2A_P_2_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_P_3:
            pass2SizeInfo = Pass2A_P_3_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_B:
            pass2SizeInfo = Pass2A_B_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_CROP:
            pass2SizeInfo = Pass2A_Crop_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        case PASS2A_B_CROP:
            pass2SizeInfo = Pass2A_B_Crop_SizeProvider::instance()->sizeInfo(eScenario);
            break;
        default:
            isSuccess = false;
    }

    return isSuccess;
}

StereoArea
StereoSizeProvider::getBufferSize(ENUM_BUFFER_NAME eName,
                                  ENUM_STEREO_SCENARIO eScenario,
                                  ENUM_ROTATION capOrientation)
{
#if (1 == STEREO_DENOISE_SUPPORTED)
    const bool IS_DENOISE_CAPTURE = (StereoSettingProvider::isDeNoise()
                                     && DENOISE_MODE == E_DENOISE_MODE_NORMAL
                                     && eScenario == eSTEREO_SCENARIO_CAPTURE);

    const int PADDING = 16;
#endif

    const StereoSizeConfig CONFIG_WO_PADDING_WO_ROTATE(STEREO_AREA_WO_PADDING, STEREO_AREA_WO_ROTATE);
    const StereoSizeConfig CONFIG_WO_PADDING_W_ROTATE(STEREO_AREA_WO_PADDING, STEREO_AREA_W_ROTATE);
    const StereoSizeConfig CONFIG_W_PADDING_W_ROTATE(STEREO_AREA_W_PADDING, STEREO_AREA_W_ROTATE);

    switch(eName) {
        //N3D before MDP for capture
        case E_MV_Y_LARGE:
        case E_MASK_M_Y_LARGE:
        case E_SV_Y_LARGE:
        case E_MASK_S_Y_LARGE:
            //Denoise capture size will align DPE input size(E_DMP_H)
#if (1 == STEREO_DENOISE_SUPPORTED)
            if(!IS_DENOISE_CAPTURE)
#endif
            {
                StereoArea area = StereoSize::getStereoArea2x(CONFIG_W_PADDING_W_ROTATE);
                area.apply32Align(true);    //For GPU

                return area;
            }   //no need break here since denoise has only one kind of size

        //N3D Output
        case E_LDC:
            if(eScenario != eSTEREO_SCENARIO_CAPTURE) {
                return StereoSize::getStereoArea1x(CONFIG_W_PADDING_W_ROTATE);
            }   //no need break here
        case E_MV_Y:
        case E_MASK_M_Y:
        case E_SV_Y:
        case E_MASK_S_Y:

        //DPE Output
        case E_DMP_H:
        case E_CFM_H:
        case E_RESPO:
            {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_W_PADDING_W_ROTATE);
                if(eScenario == eSTEREO_SCENARIO_CAPTURE) {
                    area *= 2;
                    area.apply32Align();    //For GPU
                }
                else if(StereoSettingProvider::getStereoBaseline() <= DOUBLE_WIDTH_THRESHOLD)
                {
                    area.applyDoubleWidth();
                }

                return area;
            }
            break;

        //For WPE
        case E_SV_CC_Y:
        case E_PRE_MASK_S_Y:
            {
                Pass2SizeInfo pass2SizeInfo;
                getInstance()->getPass2SizeInfo(PASS2A_P_2, eScenario, pass2SizeInfo);
                return pass2SizeInfo.areaWDMA;
            }
            break;

        //OCC Output
        case E_MY_S:
            {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_W_ROTATE);
                int factor = 1;
                if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                    factor *= 2;
                }

                if(E_WMF_Y_CHANNEL != StereoTuningProvider::getWMFInputChannel()) {
                    factor *= 2;
                }

                if(factor > 1) {
                    area *= factor;
                }

                return area;
            }
            break;

        case E_DMH:
        case E_OCC:
        case E_NOC:

        //WMF Output
        case E_DMW:
        case E_BOKEH_PACKED_BUFFER:
            {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_W_ROTATE);
                if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                    area *= 2;
                }

                if(E_BOKEH_PACKED_BUFFER == eName) {
                    if(capOrientation & 0x2) {  //90 or 270 degree
                        MSize::value_type s = area.size.w;
                        area.size.w = area.size.h;
                        area.size.h = s;
                    }

                    area.size.h *= 4;
                }

                return area;
            }
            break;

        //GF Input
        case E_GF_IN_IMG_4X:
            {
                return StereoSize::getStereoArea4x(CONFIG_WO_PADDING_W_ROTATE);
            }
            break;

        case E_GF_IN_IMG_2X:
            {
                return StereoSize::getStereoArea2x(CONFIG_WO_PADDING_W_ROTATE);
            }
            break;

        //GF Output
        case E_DMG:
        case E_DMBG:
            {
                StereoArea area = StereoSize::getStereoArea1x(CONFIG_WO_PADDING_WO_ROTATE);
                if(eSTEREO_SCENARIO_CAPTURE == eScenario) {
                    MY_LOGW("Should not run this if software bokeh is used");
                    area *= 2;
                }

                return area;
            }
            break;

        case E_DEPTH_MAP:
            {
                switch(CUSTOM_DEPTHMAP_SIZE) {
                case STEREO_DEPTHMAP_1X:
                    return StereoSize::getStereoArea1x(CONFIG_WO_PADDING_WO_ROTATE);
                    break;
                case STEREO_DEPTHMAP_2X:
                default:
                    return StereoSize::getStereoArea2x(CONFIG_WO_PADDING_WO_ROTATE);
                    break;
                case STEREO_DEPTHMAP_4X:
                    return StereoSize::getStereoArea4x(CONFIG_WO_PADDING_WO_ROTATE);
                    break;
                }
            }
            break;

        case E_INK:
            return StereoSize::getStereoArea2x(CONFIG_WO_PADDING_WO_ROTATE);
            break;

        //Bokeh Output
        case E_BOKEH_WROT: //Saved image
            switch(eScenario) {
                case eSTEREO_SCENARIO_PREVIEW:
                    return STEREO_AREA_ZERO;
                    break;
                case eSTEREO_SCENARIO_RECORD:
                    {
                        Pass2SizeInfo pass2SizeInfo;
                        getInstance()->getPass2SizeInfo(PASS2A, eScenario, pass2SizeInfo);
                        return pass2SizeInfo.areaWDMA;
                    }
                    break;
                case eSTEREO_SCENARIO_CAPTURE:
                    {
                        return StereoArea(__captureSize);
                    }
                    break;
                default:
                    break;
            }
            break;
        case E_BOKEH_WDMA:
            switch(eScenario) {
                case eSTEREO_SCENARIO_PREVIEW:  //Display
                case eSTEREO_SCENARIO_RECORD:
                    return (eRatio_16_9 == StereoSettingProvider::imageRatio()) ? StereoArea(1920, 1080) : StereoArea(1440, 1080);
                    break;
                case eSTEREO_SCENARIO_CAPTURE:  //Clean image
                    {
                        return StereoArea(__captureSize);
                    }
                    break;
                default:
                    break;
            }
            break;
        case E_BOKEH_3DNR:
            switch(eScenario) {
                case eSTEREO_SCENARIO_PREVIEW:
                case eSTEREO_SCENARIO_RECORD:
                    {
                        Pass2SizeInfo pass2SizeInfo;
                        getInstance()->getPass2SizeInfo(PASS2A, eScenario, pass2SizeInfo);
                        return pass2SizeInfo.areaWDMA;
                    }
                    break;
                default:
                    break;
            }
            break;
#if (1 == STEREO_DENOISE_SUPPORTED)
        case E_BM_PREPROCESS_FULLRAW_CROP_1:
            return StereoArea(__BMDeNoiseFullRawCropSize_main1.s.w, __BMDeNoiseFullRawCropSize_main1.s.h,0,0,__BMDeNoiseFullRawCropSize_main1.p.x,__BMDeNoiseFullRawCropSize_main1.p.y);
        case E_BM_PREPROCESS_FULLRAW_CROP_2:
            return StereoArea(__BMDeNoiseFullRawCropSize_main2.s.w, __BMDeNoiseFullRawCropSize_main2.s.h,0,0,__BMDeNoiseFullRawCropSize_main2.p.x,__BMDeNoiseFullRawCropSize_main2.p.y);
        case E_BM_PREPROCESS_MFBO_1:
            return StereoArea(__BMDeNoiseFullRawSize_main1);
        case E_BM_PREPROCESS_MFBO_2:
            return StereoArea(__BMDeNoiseFullRawSize_main2);
        case E_BM_PREPROCESS_MFBO_FINAL_1:  //bayer raw
            return StereoArea(__BMDeNoiseAlgoSize_main1.w+PADDING*2, __BMDeNoiseAlgoSize_main1.h+PADDING*2, PADDING*2, PADDING*2, 0, 0)
                   .rotatedByModule(false);
        case E_BM_PREPROCESS_MFBO_FINAL_2:  //mono raw
            {
                // first we get a 32-align buffer size
                StereoArea temp(__BMDeNoiseAlgoSize_main2.w, __BMDeNoiseAlgoSize_main2.h);
                temp.rotatedByModule(false);

                MUINT32 contentWidth;
                MUINT32 contentHeight;
                contentWidth = temp.size.w;
                contentHeight = temp.size.h;

                temp.apply32AlignToWidth();

                // then return an area with dummy pixels in padding
                return StereoArea(temp.size.w, temp.size.h,
                                  temp.size.w-contentWidth, temp.size.h-contentHeight, 0, 0);
            }
        case E_BM_PREPROCESS_W_1:   //bayerW
            return getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_1);
        case E_BM_PREPROCESS_W_2:
            // W2 is not used, just allocate a small dummy buffer for pass2 to work normally
            return (eRatio_16_9 == StereoSettingProvider::imageRatio()) ? StereoArea(160, 90) : StereoArea(160, 120)
                   .rotatedByModule(false);
        case E_BM_DENOISE_HAL_OUT:  //output
            return getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_1);
        case E_BM_DENOISE_HAL_OUT_ROT_BACK:
            return StereoArea(__BMDeNoiseAlgoSize_main1);
        case E_BM_PREPROCESS_SMALL_YUV:
            return StereoArea(__BMDeNoiseFullRawSize_main2.w/2, __BMDeNoiseFullRawSize_main2.h/2).apply16Align();
#endif
        default:
            break;
    }

    return STEREO_AREA_ZERO;
}

MSize
StereoSizeProvider::getSBSImageSize()
{
    Pass2SizeInfo pass2SizeInfo;
    getPass2SizeInfo(PASS2A_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    MSize result = pass2SizeInfo.areaWDMA.size;
    result.w *= 2;

    return result;
}

void
StereoSizeProvider::setCaptureImageSize(int w, int h)
{
    __captureSize.w = w;
    __captureSize.h = h;
    MY_LOGD("Set capture size %dx%d", w, h);
}

MSize
StereoSizeProvider::postViewSize(ENUM_STEREO_RATIO ratio)
{
    StereoArea area(512, 288);  //min: 120x120
    return area.applyRatio(ratio, E_KEEP_HEIGHT).size;
}
#if (16 == STEREO_HAL_VER)

bool
StereoSizeProvider::getcustomYUVSize( ENUM_STEREO_SENSOR sensor,
                                      EPortIndex port,
                                      MSize &outSize
                                    )
{
    if( EPortIndex_IMGO == port)
    {
        MSize customIMGOYUVSize = (eSTEREO_SENSOR_MAIN1 == sensor) ? __imgoYuvSize[0] : __imgoYuvSize[1];
        outSize = customIMGOYUVSize;
    }
    else
    {
        MSize customRRZOYUVSize = (eSTEREO_SENSOR_MAIN1 == sensor) ? __rrzoYuvSize[0] : __rrzoYuvSize[1];
        outSize = customRRZOYUVSize;
    }
    return true;
}
    /**
     * \brief Get depthmap size for 3rd party
     *
     * \param ratio Image ratio of the image
     * \return Size of depthmap for 3rd party
     */
MSize
StereoSizeProvider::thirdPartyDepthmapSize(ENUM_STEREO_SENSOR_PROFILE profile, ENUM_STEREO_RATIO ratio) const
{
    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx, profile);
    StereoSensorConbinationSetting_T *sensorCombination = NULL;
    StereoSensorSetting_T *sensorSettings0 = StereoSettingProviderKernel::getInstance()->getSensorSetting(main1Idx);
    StereoSensorSetting_T *sensorSettings1 = StereoSettingProviderKernel::getInstance()->getSensorSetting(main2Idx);
    if(NULL != sensorSettings0 &&
       NULL != sensorSettings1)
    {
        sensorCombination = StereoSettingProviderKernel::getInstance()->
                            getSensorCombinationSetting(sensorSettings0->uid, sensorSettings1->uid);

    } else {
        MY_LOGW("Cannot get sensor settings");
    }

    MSize depthmapSize;
    if(sensorCombination) {
        switch(ratio) {
            case eRatio_4_3:
            default:
                depthmapSize = sensorCombination->depthmapSize_4_3;
                break;
            case eRatio_16_9:
                depthmapSize = sensorCombination->depthmapSize_16_9;
                break;
        }
    }

    if(0 == depthmapSize.w ||
       0 == depthmapSize.h)
    {
        switch(ratio) {
            case eRatio_4_3:
            default:
                depthmapSize = MSize(480, 360);
                break;
            case eRatio_16_9:
                depthmapSize = MSize(480, 272);
                break;
        }
    }

    return depthmapSize;
}
#endif

void
StereoSizeProvider::setPreviewSize(NSCam::MSize size)
{
    __previewSize = size;

    {
        Mutex::Autolock lock(__P1Lock);
        __pass1TgCropRectMap.clear();
        __pass1OutSizeMap.clear();
        __pass1StrideMap.clear();
    }
}

#if (1 == STEREO_DENOISE_SUPPORTED)
bool
StereoSizeProvider::__updateBMDeNoiseSizes()
{
    int main1Id = -1, main2Id = -1;
    IHalSensorList* sensorList = nullptr;

    StereoSettingProvider::getStereoSensorIndex(main1Id, main2Id);

    // Prepare sensor hal
    sensorList = MAKE_HalSensorList();
    if(NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
        return false;
    }

    // main1 full size
    {
        int sendorDevIndex = sensorList->querySensorDevIdx(main1Id);

        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        sensorList->querySensorStaticInfo(sendorDevIndex, &sensorStaticInfo);

        MSize main1_full_size = MSize(sensorStaticInfo.captureWidth, sensorStaticInfo.captureHeight);

        MRect crop;
        __getCenterCrop(main1_full_size, crop);

        __BMDeNoiseFullRawSize_main1 = main1_full_size;
        __BMDeNoiseFullRawCropSize_main1 = crop;
        __BMDeNoiseAlgoSize_main1 = crop.s;
    }

    // main2 full size
    {
        int sendorDevIndex = sensorList->querySensorDevIdx(main2Id);

        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        sensorList->querySensorStaticInfo(sendorDevIndex, &sensorStaticInfo);

        MSize main2_full_size = MSize(sensorStaticInfo.captureWidth, sensorStaticInfo.captureHeight);

        MRect crop;
        __getCenterCrop(main2_full_size, crop);

        __BMDeNoiseFullRawSize_main2 = main2_full_size;
        __BMDeNoiseFullRawCropSize_main2 = crop;
        __BMDeNoiseAlgoSize_main2 = crop.s;
    }

    return true;
}
#endif
/*******************************************************************************
 *
 ********************************************************************************/
bool
StereoSizeProvider::__getCenterCrop(MSize &srcSize, MRect &rCrop )
{
    // calculate the required image hight according to image ratio
    int iHeight = srcSize.h;
    switch(StereoSettingProvider::imageRatio())
    {
        case eRatio_4_3:
            // do not crop
            break;
        case eRatio_16_9:
        default:
            iHeight = ((srcSize.w * 9 / 16) >> 1 ) <<1;
            break;
    }

    if(abs(iHeight-srcSize.h) == 0)
    {
        rCrop.p = MPoint(0,0);
        rCrop.s = srcSize;
    }
    else
    {
        rCrop.p.x = 0;
        rCrop.p.y = (srcSize.h - iHeight)/2;
        rCrop.s.w = srcSize.w;
        rCrop.s.h = iHeight;
    }

    MY_LOGD_IF(LOG_ENABLED, "srcSize:(%d,%d) ratio:%d, rCropStartPt:(%d, %d) rCropSize:(%d,%d)",
                            srcSize.w, srcSize.h, StereoSettingProvider::imageRatio(),
                            rCrop.p.x, rCrop.p.y, rCrop.s.w, rCrop.s.h);

    // apply 16-align to height
    if(StereoSettingProvider::imageRatio() == eRatio_16_9){
        rCrop.s.h = ((rCrop.s.h+15)>>4)<<4;

        MY_LOGD_IF(LOG_ENABLED, "srcSize after 16 align:(%d,%d) ratio:%d, rCropStartPt:(%d, %d) rCropSize:(%d,%d)",
                                srcSize.w, srcSize.h, StereoSettingProvider::imageRatio(),
                                rCrop.p.x, rCrop.p.y, rCrop.s.w, rCrop.s.h);
    }

    return MTRUE;
}
