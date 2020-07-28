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

#define LOG_TAG "mtkcam-SensorSettingPolicy"

#include <mtkcam3/pipeline/policy/IConfigSensorSettingPolicy.h>
//
#include "MyUtils.h"

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCamHW;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define CHECK_ERROR(_err_)                                \
    do {                                                  \
        MERROR const err = (_err_);                       \
        if( CC_UNLIKELY( err != OK ) ) {                                 \
            MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
            return err;                                   \
        }                                                 \
    } while(0)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {

enum eMode
{
    eNORMAL_PREVIEW = 0,
    eNORMAL_VIDEO,
    eNORMAL_CAPTURE,
    eSLIM_VIDEO1,
    eSLIM_VIDEO2,
    eHFPS_VIDEO,
    eCUSTOM1,
    eCUSTOM2,
    eCUSTOM3,
    eCUSTOM4,
    eCUSTOM5,
    eNUM_SENSOR_MODE,
};

const char* kModeNames[eMode::eNUM_SENSOR_MODE+1] =
{
    "PREVIEW",
    "VIDEO",
    "CAPTURE",
    "SLIM_VIDEO1",
    "SLIM_VIDEO2",
    "HFPS_VIDEO",
    "CUSTOM1",
    "CUSTOM2",
    "CUSTOM3",
    "CUSTOM4",
    "CUSTOM5",
    "UNDEFINED"
};

struct SensorParams
{
    std::unordered_map<eMode, SensorSetting> mSetting;
    std::unordered_map<eMode, eMode>         mAltMode;
    bool                                     bSupportPrvMode = true;
};

static auto
parseSensorParamsSetting(
    std::shared_ptr<SensorParams>& pParams,
    const HwInfoHelper& rHelper
) -> int
{
#define addStaticParams(idx, _scenarioId_, _item_)                                          \
    do {                                                                                    \
        int32_t fps;                                                                        \
        MSize   size;                                                                       \
        if ( CC_UNLIKELY( ! rHelper.getSensorFps( _scenarioId_, fps) ) ) {                  \
            MY_LOGW("getSensorFps fail"); break;                                            \
        }                                                                                   \
        if ( CC_UNLIKELY( ! rHelper.getSensorSize(_scenarioId_, size) ) ) {                 \
            MY_LOGW("getSensorSize fail"); break;                                           \
        }                                                                                   \
        _item_->mSetting[idx].sensorFps  = static_cast<uint32_t>(fps);                      \
        _item_->mSetting[idx].sensorSize = size;                                            \
        _item_->mSetting[idx].sensorMode = _scenarioId_;                                    \
        _item_->mAltMode[idx] = idx;                                                        \
        MY_LOGI("candidate [%d] (%4dx%-4d)@%-3d sensorMode:%d(%s)",                         \
            idx,                                                                            \
            _item_->mSetting[idx].sensorSize.w,                                             \
            _item_->mSetting[idx].sensorSize.h,                                             \
            _item_->mSetting[idx].sensorFps,                                                \
            _item_->mSetting[idx].sensorMode,                                               \
            kModeNames[idx]);                                                               \
    } while(0)

    addStaticParams(eNORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_PREVIEW, pParams);
    addStaticParams(eNORMAL_VIDEO  , SENSOR_SCENARIO_ID_NORMAL_VIDEO  , pParams);
    addStaticParams(eNORMAL_CAPTURE, SENSOR_SCENARIO_ID_NORMAL_CAPTURE, pParams);
    addStaticParams(eSLIM_VIDEO1,    SENSOR_SCENARIO_ID_SLIM_VIDEO1,    pParams);
    addStaticParams(eSLIM_VIDEO2,    SENSOR_SCENARIO_ID_SLIM_VIDEO2,    pParams);
    addStaticParams(eCUSTOM1,        SENSOR_SCENARIO_ID_CUSTOM1,        pParams);
    addStaticParams(eHFPS_VIDEO,     SENSOR_SCENARIO_ID_CUSTOM2,        pParams);
    addStaticParams(eCUSTOM3,        SENSOR_SCENARIO_ID_CUSTOM3,        pParams);
    addStaticParams(eCUSTOM4,        SENSOR_SCENARIO_ID_CUSTOM4,        pParams);
    addStaticParams(eCUSTOM5,        SENSOR_SCENARIO_ID_CUSTOM5,        pParams);

#undef addStaticParams
    //
    return OK;
}

static auto isSensorSupport(
    SensorSetting& res,
    const uint32_t hdrHalMode
) -> bool
{
    if ( hdrHalMode & MTK_HDR_FEATURE_HDR_HAL_MODE_MSTREAM_CAPTURE_PREVIEW ){
        const uint32_t MSTREAM_SUPPORT_MIN_FPS = 60;
        if ( res.sensorFps >= MSTREAM_SUPPORT_MIN_FPS ) {
            return true;
        }
    }
    else
    {
        return true;
    }
    return false;
}

static auto
checkVhdrSensor(
    SensorSetting& res,
    std::shared_ptr<SensorParams>& pParams,
    const uint32_t hdrSensorMode,
    const uint32_t hdrHalMode,
    const HwInfoHelper& rHelper
) -> int
{
    uint32_t supportHDRMode = 0;
    char forceSensorMode[PROPERTY_VALUE_MAX];
    property_get("vendor.debug.force.vhdr.sensormode", forceSensorMode, "0");
    switch( forceSensorMode[0] )
    {
        case '0':
            break;
        case 'P':
        case 'p':
            res = pParams->mSetting[eNORMAL_PREVIEW];
            MY_LOGD("set sensor mode to NORMAL_PREVIEW(%d)",SENSOR_SCENARIO_ID_NORMAL_PREVIEW);
            return OK;
        case 'V':
        case 'v':
            res = pParams->mSetting[eNORMAL_VIDEO];
            MY_LOGD("set sensor mode to NORMAL_VIDEO(%d)",SENSOR_SCENARIO_ID_NORMAL_VIDEO);
            return OK;
        case 'C':
        case 'c':
            res = pParams->mSetting[eNORMAL_CAPTURE];
            MY_LOGD("set sensor mode to NORMAL_CAPTURE(%d)",SENSOR_SCENARIO_ID_NORMAL_CAPTURE);
            return OK;
        default:
            MY_LOGW("unknown force sensor mode(%s), not used", forceSensorMode);
            MY_LOGW("usage : setprop debug.force.vhdr.sensormode P/V/C");
            break;
        }

    // 1. Current sensor mode is VHDR support, use it.
    if ( CC_UNLIKELY( ! rHelper.querySupportVHDRMode( res.sensorMode,
                                                    supportHDRMode) ) )
    {
        MY_LOGE("[vhdrhal] HwInfoHelper querySupportVHDRMode fail");
        return -EINVAL;
    }
    if(hdrSensorMode == supportHDRMode)
    {
        if ( isSensorSupport(res, hdrHalMode) ) {
            return OK;
        }
    }

    // 2. Check sensor mode in order: preview -> video -> capture
    // Find acceptable sensor mode for this hdrSensorMode
    /*
           SENSOR_SCENARIO_ID_NORMAL_PREVIEW
           SENSOR_SCENARIO_ID_NORMAL_CAPTURE
           SENSOR_SCENARIO_ID_NORMAL_VIDEO
           SENSOR_SCENARIO_ID_SLIM_VIDEO1
           SENSOR_SCENARIO_ID_SLIM_VIDEO2
      */

#define CHECK_SENSOR_MODE_VHDR_SUPPORT(senMode, scenariomode)                \
do {                                                                         \
       if ( ! rHelper.querySupportVHDRMode(senMode, supportHDRMode) )        \
       {                                                                     \
           return -EINVAL;                                                   \
       }                                                                     \
       if (hdrSensorMode == supportHDRMode)                                  \
       {                                                                     \
           res = pParams->mSetting[scenariomode];                            \
           if ( isSensorSupport(res, hdrHalMode) ) {                         \
               return OK;                                                    \
           }                                                                 \
       }                                                                     \
} while (0)
       CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_PREVIEW,eNORMAL_PREVIEW);
       CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_VIDEO,eNORMAL_VIDEO);
       CHECK_SENSOR_MODE_VHDR_SUPPORT(SENSOR_SCENARIO_ID_NORMAL_CAPTURE, eNORMAL_CAPTURE);
#undef CHECK_SENSOR_MODE_VHDR_SUPPORT

    //3.  PREVIEW & VIDEO & CAPTURE mode are all not acceptable
    MY_LOGE("[vhdrhal] VHDR not support preview & video & capture mode.");
    return -EINVAL;
}

static auto
determineAlternativeMode(
    std::shared_ptr<SensorParams>& pParams,
    HwTransHelper& rHelper
) -> int
{
    auto verifyFov = [](HwTransHelper& rHelper, uint32_t const mode) -> bool
    {
        #define FOV_DIFF_TOLERANCE (0.002)
        float dX = 0.0f;
        float dY = 0.0f;
        return ( rHelper.calculateFovDifference(mode, &dX, &dY) &&
                 dX < FOV_DIFF_TOLERANCE && dY < FOV_DIFF_TOLERANCE)
                ? true : false;
    };

    bool bAcceptPrv = verifyFov(rHelper, pParams->mSetting[eNORMAL_PREVIEW].sensorMode);
    bool bAcceptVid = verifyFov(rHelper, pParams->mSetting[eNORMAL_VIDEO].sensorMode);

    if ( ! bAcceptPrv ) {
        if ( ! bAcceptVid ) {
            pParams->mAltMode[eNORMAL_VIDEO]   = eNORMAL_CAPTURE;
            pParams->mAltMode[eNORMAL_PREVIEW] = eNORMAL_CAPTURE;
        }else if(pParams->mSetting[eNORMAL_CAPTURE].sensorFps>=30){
            pParams->mAltMode[eNORMAL_PREVIEW] = eNORMAL_CAPTURE;
        } else {
            pParams->mAltMode[eNORMAL_PREVIEW] = eNORMAL_VIDEO;
        }
    } else if ( ! bAcceptVid ) {
        if ( pParams->bSupportPrvMode )
            pParams->mAltMode[eNORMAL_VIDEO]   = eNORMAL_PREVIEW;
        else if ( pParams->mSetting[eNORMAL_CAPTURE].sensorFps>=30 )
            pParams->mAltMode[eNORMAL_VIDEO]   = eNORMAL_CAPTURE;
        else
            pParams->mAltMode[eNORMAL_VIDEO]   = eNORMAL_VIDEO;
    }
    // if (!pParsedAppImageStreamInfo->hasVideoConsumer)
    // {
    //     altMode(eNORMAL_VIDEO  , eNORMAL_CAPTURE);
    // }

    for ( auto const& alt : pParams->mAltMode ) {
        MY_LOGI("alt sensor mode: %s -> %s", kModeNames[alt.first], kModeNames[alt.second]);
    }

    return OK;
}


static auto
determineScenRaw16(
    SensorSetting& res,
    std::shared_ptr<SensorParams>& pParams,
    std::shared_ptr<ParsedAppImageStreamInfo> const& pParsedAppImageInfo
) -> int
{
    bool hit = false;
    for ( int i=0; i<eNUM_SENSOR_MODE; ++i ) {
        auto const& mode = static_cast<eMode>(i);
        auto const& size = pParams->mSetting[mode].sensorSize;
        if ( mode == eNORMAL_VIDEO
          || mode == eSLIM_VIDEO1
          || mode == eSLIM_VIDEO2 )
        {
            MY_LOGI("skip video related mode since it didn't have full capbility");
            continue;
        }
        if ( pParsedAppImageInfo->maxImageSize.w <= size.w &&
             pParsedAppImageInfo->maxImageSize.h <= size.h )
        {
            res = pParams->mSetting[mode];
            hit = true;
            break;
        }
    }
    if ( !hit ) {
        // pick largest one
        MY_LOGW("select capture mode");
        res = pParams->mSetting[eNORMAL_CAPTURE];
    }
    return OK;
}

static auto
determineScenDefault(
    SensorSetting& res,
    std::shared_ptr<SensorParams>& pParams,
    std::shared_ptr<ParsedAppImageStreamInfo> const& pParsedAppImageInfo,
    bool isVideo
) -> int
{
    bool hit = false;
    for ( int i=0; i<eNUM_SENSOR_MODE; ++i ) {
        auto const& mode = static_cast<eMode>(i);
        auto const& size = pParams->mSetting[ pParams->mAltMode[mode] ].sensorSize;
        if ( !isVideo
          && ( mode == eNORMAL_VIDEO
            || mode == eSLIM_VIDEO1
            || mode == eSLIM_VIDEO2 ) )
        {
            MY_LOGI("skip video related mode since it didn't have full capbility");
            continue;
        }
        if ( pParsedAppImageInfo->maxImageSize.w <= size.w &&
             pParsedAppImageInfo->maxImageSize.h <= size.h )
        {
            res = pParams->mSetting[ pParams->mAltMode[mode] ];
            hit = true;
            break;
        }
    }
    if ( !hit ) {
        // pick largest one
        MY_LOGW("select capture mode");
        res = pParams->mSetting[eNORMAL_CAPTURE];
    }

    MINT32 forceSensorMode = ::property_get_int32("vendor.debug.cameng.force_sensormode", -1);
    if( forceSensorMode != -1 )
    {
        switch( forceSensorMode )
        {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
             res = pParams->mSetting[eNORMAL_PREVIEW];
             break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
             res = pParams->mSetting[eNORMAL_CAPTURE];
             break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
             res = pParams->mSetting[eNORMAL_VIDEO];
             break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            res = pParams->mSetting[eSLIM_VIDEO1];
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            res = pParams->mSetting[eSLIM_VIDEO2];
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            res = pParams->mSetting[eCUSTOM1];
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            res = pParams->mSetting[eCUSTOM2];
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            res = pParams->mSetting[eHFPS_VIDEO];
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            res = pParams->mSetting[eCUSTOM4];
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            res = pParams->mSetting[eCUSTOM5];
            break;
        default:
            MY_LOGW("Unknown sensorMode: %d", forceSensorMode);
            break;
        }
        MY_LOGD("Force set sensorMode: %d. Selected sensorMode: %d", forceSensorMode, res.sensorMode);
    }

    return OK;
}


/**
 * Make a function target - default version
 */
FunctionType_Configuration_SensorSettingPolicy makePolicy_Configuration_SensorSetting_Default()
{
    return [](Configuration_SensorSetting_Params const& params) -> int {
        auto pvOut = params.pvOut;
        auto pStreamingFeatureSetting = params.pStreamingFeatureSetting;
        auto pPipelineStaticInfo = params.pPipelineStaticInfo;
        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;

        if ( CC_UNLIKELY( ! pvOut || ! pStreamingFeatureSetting ||
                          ! pPipelineStaticInfo || ! pPipelineUserConfiguration ) ) {
            MY_LOGE("error input params");
            return -EINVAL;
        }

        auto const& pParsedAppCfg       = pPipelineUserConfiguration->pParsedAppConfiguration;
        auto const& pParsedAppImageInfo = pPipelineUserConfiguration->pParsedAppImageStreamInfo;
        MINT32 streamHfpsMode = MTK_STREAMING_FEATURE_HFPS_MODE_NORMAL;
        IMetadata::getEntry<MINT32>(&pParsedAppCfg->sessionParams, MTK_STREAMING_FEATURE_HFPS_MODE, streamHfpsMode);

        std::map<uint32_t, std::shared_ptr<SensorParams> > mStatic;
        for ( auto const id : pPipelineStaticInfo->sensorId ) {
            SensorSetting res;  // output
            HwInfoHelper  infoHelper = HwInfoHelper(id);
            infoHelper.updateInfos();
            HwTransHelper tranHelper = HwTransHelper(id);
            auto& pStatic = mStatic[id] = std::make_shared<SensorParams>();
            if ( CC_UNLIKELY( pStatic==nullptr ) ) {
                MY_LOGE("initial sensor static fail");
                return -EINVAL;
            }

            CHECK_ERROR( parseSensorParamsSetting(pStatic, infoHelper) );
            CHECK_ERROR( determineAlternativeMode(pStatic, tranHelper) );

            // sensor mode decision policy:
            // 1. Raw stream configured: find sensor mode with raw size.
            if ( !pParsedAppImageInfo->vAppImage_Output_RAW16.empty() ||
                 !pParsedAppImageInfo->vAppImage_Output_RAW16_Physical.empty() )
            {
                CHECK_ERROR( determineScenRaw16(res, pStatic, pParsedAppImageInfo) );
                goto lbDone;
            }

            // 2. has video consumer
            if ( pParsedAppImageInfo->hasVideoConsumer ) {
                if (streamHfpsMode == MTK_STREAMING_FEATURE_HFPS_MODE_60FPS)
                {
                    SensorSetting tmpRes;
                    MUINT32 minW = 0, minH = 0;
                    for (int i = 0; i < eNUM_SENSOR_MODE; i++)
                    {
                        auto const& mode = static_cast<eMode>(i);
                        tmpRes = pStatic->mSetting[mode];
                        MBOOL matched = (tmpRes.sensorFps == 60) &&
                                        ((minW == 0 && minH == 0) || (tmpRes.sensorSize.w < minW || tmpRes.sensorSize.h < minH));
                        if (matched)
                        {
                            res = tmpRes;
                            minW = tmpRes.sensorSize.w;
                            minH = tmpRes.sensorSize.h;
                        }
                    }

                    if (res.sensorFps != 0)
                    {
                        goto lbDone;
                    }
                    else
                    {
                        MY_LOGW("Cannot find HFPS sensor mode!");
                    }
                }

                if ( pParsedAppCfg->isConstrainedHighSpeedMode ) {
                    // 2-1. constrained high speed video
                    res = pStatic->mSetting[eSLIM_VIDEO1];
                } else if ( pParsedAppImageInfo->hasVideo4K ) {
                    // 2-2. 4K record
                    res = pStatic->mSetting[eNORMAL_VIDEO];
                } else {
                    // 2-3 others
                    #ifdef MTKCAM_FORCE_VIDEO_MODE
                    #define VGA_W (640)
                    #define VGA_H (480)
                    if (pParsedAppImageInfo->maxImageSize.w > VGA_W || pParsedAppImageInfo->maxImageSize.h > VGA_H)
                    {
                        res = pStatic->mSetting[eNORMAL_VIDEO];
                    }
                    else
                    {
                        res = pStatic->mSetting[eNORMAL_PREVIEW];
                    }
                    #else
                    CHECK_ERROR( determineScenDefault(res, pStatic, pParsedAppImageInfo, true) );
                    #endif
                }
                // always happen, force skip determineScenDefault()...
                goto lbDone;
            }

            // 3. default rules policy:
            //    find the smallest size that is "larger" than max of stream size
            //    (not the smallest difference)
            CHECK_ERROR( determineScenDefault(res, pStatic, pParsedAppImageInfo, false) );

            if (pStreamingFeatureSetting->hdrHalMode != MTK_HDR_FEATURE_HDR_HAL_MODE_OFF){
                CHECK_ERROR( checkVhdrSensor(res, pStatic, pStreamingFeatureSetting->hdrSensorMode, pStreamingFeatureSetting->hdrHalMode, infoHelper) );
            }
lbDone:
            CAM_ULOGMI("select size(%4dx%-4d)@%-3d sensorMode:%d hdrSensorMode:%d hdrHalMode:%d, hfpsMode:%d",
                res.sensorSize.w, res.sensorSize.h, res.sensorFps,
                res.sensorMode,
                pStreamingFeatureSetting->hdrSensorMode, pStreamingFeatureSetting->hdrHalMode, streamHfpsMode);
            pvOut->push_back(res);
        }

        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

