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

#define LOG_TAG "mtkcam-P1HwSettingPolicy"

#include <mtkcam3/pipeline/policy/IConfigP1HwSettingPolicy.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/aaa/IIspMgr.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam3/feature/fsc/fsc_defs.h>
#include <mtkcam/utils/std/ULog.h>
//
#include "myutils.h"
#include "MyUtils.h"


CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_POLICY);


/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam::v3::pipeline::policy;
using namespace NSCam::FSC;
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
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {


/******************************************************************************
 *
 ******************************************************************************/
static int32_t evaluateDeviceRawBitDepth(const PipelineStaticInfo* pPipelineStaticInfo)
{
    auto openId = pPipelineStaticInfo->openId;
    int32_t bitdepth = -1;

    std::string key = "persist.vendor.camera3.sensor.bitdepth.device" + std::to_string(openId);
    bitdepth = property_get_int32(key.c_str(), -1);
    if ( -1 != bitdepth ) {
        MY_LOGI("%s=%d", key.c_str(), bitdepth);
        return bitdepth;
    }

    auto pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
    if (CC_LIKELY( pMetadataProvider != nullptr )) {
        int32_t whiteLevel = 0;
        auto& staticMetadata = pMetadataProvider->getMtkStaticCharacteristics();
        if ( IMetadata::getEntry<int32_t>(&staticMetadata, MTK_SENSOR_INFO_WHITE_LEVEL, whiteLevel) ) {
            bitdepth = __builtin_ctz( (whiteLevel+1) ); // android.sensor.info.whiteLevel = 2^bitdepth - 1
            MY_LOGI("MTK_SENSOR_INFO_WHITE_LEVEL:%d bitdepth:%d", whiteLevel, bitdepth);
            return bitdepth;
        }
    }

    //Shouldn't be here.
    MY_LOGW("By default bitdepth:10");
    return 10;
}


/******************************************************************************
 *
 ******************************************************************************/
static bool evaluateImgoAllocConfig_useUnpackRaw(const PipelineStaticInfo* pPipelineStaticInfo)
{
    static bool isLowMem = ::property_get_bool("ro.config.low_ram", false);
    if ( isLowMem ) {
        return false; // false: pack raw.
    }

    auto openId = pPipelineStaticInfo->openId;

    std::string key = "persist.vendor.camera3.imgo.alloc.unpackraw.device" + std::to_string(openId);
    auto ret = property_get_int32(key.c_str(), -1);
    if ( -1 != ret ) {
        MY_LOGI("%s=%d", key.c_str(), ret);
        return (ret > 0); // true: unpack raw; false: pack raw.
    }

    return false; // false: pack raw.
}

/******************************************************************************
 * query Resize Max ratio from Isp mgr for quality-guaranteed
 ******************************************************************************/
static bool queryRecommendResizeMaxRatio(const MINT32 sensorId, MUINT32& recommendResizeRatio)
{
    // query resize quality-guaranteed ratio from isp mgr
    MUINT32 ispMaxResizeRatio = 0;
    MUINT32 ispMaxResizeDenominator = 0;
    NS3Av3::Buffer_Info tuningInfo;
    auto mpISP = MAKE_HalISP(sensorId, LOG_TAG);
    if (mpISP == nullptr) {
        MY_LOGA("sensorId : %d, create HalIsp FAILED!", sensorId);
        return false;
    } else {
        // [Work Around] use this function call to check platform
        if (mpISP->queryISPBufferInfo(tuningInfo)) {
            MINT32 ret = mpISP->sendIspCtrl(NS3Av3::EISPCtrl_GetMaxRrzRatio,
                reinterpret_cast<MINTPTR>(&ispMaxResizeDenominator), reinterpret_cast<MINTPTR>(nullptr));
            if (ret != 0) {
                MY_LOGW("EISPCtrl_GetMaxRrzRatio is not supported !, Skip..");
            }
        } else if (auto ispMgr = MAKE_IspMgr()) {
            ispMaxResizeDenominator = ispMgr->queryRRZ_MaxRatio();
        } else {
            MY_LOGA("create IspMgr FAILED!");
            return false;
        }
        mpISP->destroyInstance(LOG_TAG);
    }
    if (ispMaxResizeDenominator) {
        // The precision is 0.01
        ispMaxResizeRatio = (10000L/ispMaxResizeDenominator + 50)/ 100;
        recommendResizeRatio = ispMaxResizeRatio;
        MY_LOGD("Resize-Ratio-Percentage %u for isp quality", recommendResizeRatio);
    }

    return true;
}

/**
 * Make a function target - default version
 */
FunctionType_Configuration_P1HwSettingPolicy makePolicy_Configuration_P1HwSetting_Default()
{
    return [](Configuration_P1HwSetting_Params const& params) -> int {
        auto pvOut = params.pvOut;
        auto pSensorSetting = params.pSensorSetting;
        auto pStreamingFeatureSetting = params.pStreamingFeatureSetting;
        auto pPipelineNodesNeed = params.pPipelineNodesNeed;
        auto pPipelineStaticInfo = params.pPipelineStaticInfo;
        auto pPipelineUserConfiguration = params.pPipelineUserConfiguration;
        auto const& pParsedAppConfiguration   = pPipelineUserConfiguration->pParsedAppConfiguration;
        auto const& pParsedAppImageStreamInfo = pPipelineUserConfiguration->pParsedAppImageStreamInfo;

        int32_t bitdepth = evaluateDeviceRawBitDepth(pPipelineStaticInfo);
        for (size_t idx = 0; idx < pPipelineStaticInfo->sensorId.size(); idx++)
        {
            P1HwSetting setting;
            std::shared_ptr<NSCamHW::HwInfoHelper> infohelper = std::make_shared<NSCamHW::HwInfoHelper>(pPipelineStaticInfo->sensorId[idx]);
            bool ret = infohelper != nullptr
                    && infohelper->updateInfos()
                    && infohelper->queryPixelMode( (*pSensorSetting)[idx].sensorMode, (*pSensorSetting)[idx].sensorFps, setting.pixelMode)
                        ;
            if ( CC_UNLIKELY(!ret) ) {
                MY_LOGE("idx : %zu, queryPixelMode error", idx);
                return UNKNOWN_ERROR;
            }
            MSize const& sensorSize = (*pSensorSetting)[idx].sensorSize;

            //#define MIN_RRZO_RATIO_100X     (25)
            MINT32     rrzoMaxWidth = 0;
            infohelper->quertMaxRrzoWidth(rrzoMaxWidth);

            #define CHECK_TARGET_SIZE(_msg_, _size_) \
            MY_LOGD("%s: target size(%dx%d)", _msg_, _size_.w, _size_.h);

            #define ALIGN16(x) x = (((x) + 15) & ~(15));

            // estimate preview yuv max size
            MSize const max_preview_size = refine::align_2(
                    MSize(rrzoMaxWidth, rrzoMaxWidth * sensorSize.h / sensorSize.w));
            //
            CHECK_TARGET_SIZE("max_rrzo_size", max_preview_size);
            MSize maxYuvStreamSize(0, 0);
            MSize largeYuvStreamSize(0, 0);
            MSize maxStreamingSize(0, 0);

            #define PRV_MAX_DURATION (34000000) //ns
            for( const auto& n : pParsedAppImageStreamInfo->vAppImage_Output_Proc )
            {
                MSize const streamSize = (n.second)->getImgSize();
                auto search = pPipelineUserConfiguration->vMinFrameDuration.find((n.second)->getStreamId());
                int64_t MinFrameDuration = 0;
                if (search != pPipelineUserConfiguration->vMinFrameDuration.end())
                {
                    MinFrameDuration = search->second;
                }

                bool candidatePreview = (
                        ((n.second)->getUsageForConsumer() & (GRALLOC_USAGE_HW_VIDEO_ENCODER | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_COMPOSER)) != 0
                        || MinFrameDuration <= PRV_MAX_DURATION
                        );

                if ((n.second)->getUsageForConsumer() & (GRALLOC_USAGE_HW_VIDEO_ENCODER | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_COMPOSER))
                {
                    refine::not_smaller_than(maxStreamingSize, streamSize);
                }

                // if stream's size is suitable to use rrzo
                if( streamSize.w <= max_preview_size.w && streamSize.h <= max_preview_size.h && candidatePreview)
                    refine::not_smaller_than(maxYuvStreamSize, streamSize);
                else
                    refine::not_smaller_than(largeYuvStreamSize, streamSize);
            }

            CHECK_TARGET_SIZE("stream size with hw usage", maxStreamingSize);

            //Commented to prevent P2streaming runs as P2capture in some circumstances for reduce time consuming.
            //If it causes APK fail to run,please costomize the APK.
            /*
            if (maxStreamingSize.size() != 0)
            {
                maxYuvStreamSize = maxStreamingSize;
            }
            */
            if (maxYuvStreamSize.w < 1280)
            {
                MY_LOGI("maxYuvStreamSize is too small(%dx%d)", maxYuvStreamSize.w, maxYuvStreamSize.h);
                maxYuvStreamSize.h = maxYuvStreamSize.h * 1280 / maxYuvStreamSize.w;
                maxYuvStreamSize.w = 1280;
                MY_LOGI("maxYuvStreamSize set rrzo to %dx%d", maxYuvStreamSize.w, maxYuvStreamSize.h);
            }
            // use resized raw if
            // 1. raw sensor
            // 2. some streams need this
            if( infohelper->isRaw() )
            {
                //
                // currently, should always enable resized raw due to some reasons...
                //
                // initial value
                MSize target_rrzo_size = maxYuvStreamSize;
                target_rrzo_size.w *= pStreamingFeatureSetting->targetRrzoRatio;
                target_rrzo_size.h *= pStreamingFeatureSetting->targetRrzoRatio;
                if ( pStreamingFeatureSetting->rrzoHeightToWidth != 0 )
                {
                    ALIGN16(target_rrzo_size.w);
                    target_rrzo_size.h = target_rrzo_size.w * pStreamingFeatureSetting->rrzoHeightToWidth;
                    ALIGN16(target_rrzo_size.h);
                }
                else
                {
                    // align sensor aspect ratio
                    if (target_rrzo_size.w * sensorSize.h > target_rrzo_size.h * sensorSize.w)
                    {
                        ALIGN16(target_rrzo_size.w);
                        target_rrzo_size.h = target_rrzo_size.w * sensorSize.h / sensorSize.w;
                        ALIGN16(target_rrzo_size.h);
                    }
                    else
                    {
                        ALIGN16(target_rrzo_size.h);
                        target_rrzo_size.w = target_rrzo_size.h * sensorSize.w / sensorSize.h;
                        ALIGN16(target_rrzo_size.w);
                    }
                }
                MINT32 user_w = ::property_get_int32("vendor.camera.force_rrzo_width", 0);
                MINT32 user_h = ::property_get_int32("vendor.camera.force_rrzo_height", 0);
                if (user_w && user_h)
                {
                    target_rrzo_size.w = user_w;
                    target_rrzo_size.h = user_h;
                }
                CHECK_TARGET_SIZE("sensor size", sensorSize);
                CHECK_TARGET_SIZE("target rrzo stream", target_rrzo_size);
                // apply limitations
                //  1. lower bounds
                {
                    // get eis ownership and apply eis hw limitation
                    if ( pStreamingFeatureSetting->bNeedLMV )
                    {
                        MUINT32 minRrzoEisW = pStreamingFeatureSetting->minRrzoEisW;
                        MSize const min_rrzo_eis_size = refine::align_2(
                                MSize(minRrzoEisW, minRrzoEisW * sensorSize.h / sensorSize.w));
                        refine::not_smaller_than(target_rrzo_size, min_rrzo_eis_size);
                        MINT32 lossless = pStreamingFeatureSetting->eisInfo.lossless;
                        target_rrzo_size = refine::align_2(
                                refine::scale_roundup(target_rrzo_size,
                                lossless ? pStreamingFeatureSetting->eisInfo.factor : 100, 100)
                                );
                       CHECK_TARGET_SIZE("eis lower bound limitation", target_rrzo_size);
                    }
                    /*MSize const min_rrzo_hw_size = refine::align_2(
                            MSize(sensorSize.w*MIN_RRZO_RATIO_100X/100, sensorSize.h*MIN_RRZO_RATIO_100X/100) );
                    refine::not_smaller_than(target_rrzo_size, min_rrzo_hw_size);
                    CHECK_TARGET_SIZE("rrz hw lower bound limitation", target_rrzo_size);*/
                }
                //  2. upper bounds
                {
                    {
                        refine::not_larger_than(target_rrzo_size, max_preview_size);
                        CHECK_TARGET_SIZE("preview upper bound limitation", target_rrzo_size);
                    }
                    refine::not_larger_than(target_rrzo_size, sensorSize);
                    CHECK_TARGET_SIZE("sensor size upper bound limitation", target_rrzo_size);
                }
                MY_LOGD("rrzo size(%dx%d)", target_rrzo_size.w, target_rrzo_size.h);
                //
                // check hw limitation with pixel mode & stride
                MSize   rrzoSize = target_rrzo_size;
                int32_t rrzoFormat = 0;
                size_t  rrzoStride = 0;
                uint32_t recommendResizeRatio = 0;
                MBOOL bIsSecure = (pParsedAppImageStreamInfo->secureInfo.type != SecType::mem_normal);
                if( ! infohelper->getRrzoFmt(bitdepth, rrzoFormat, MFALSE, bIsSecure) ||
                    ! queryRecommendResizeMaxRatio(pPipelineStaticInfo->sensorId[idx], recommendResizeRatio) ||
                    ! infohelper->alignRrzoHwLimitation(target_rrzo_size, sensorSize, rrzoSize, recommendResizeRatio) ||
                    ! infohelper->alignPass1HwLimitation(rrzoFormat, MFALSE/*isImgo*/,
                                                         rrzoSize, rrzoStride ) )
                {
                    MY_LOGE("wrong params about rrzo");
                    return BAD_VALUE;
                }

                auto& rrzoDefaultRequest = setting.rrzoDefaultRequest;
                rrzoDefaultRequest.imageSize = rrzoSize;
                rrzoDefaultRequest.format = rrzoFormat;
            }
            //
            // use full raw, if
            // 1. jpeg stream (&& not met BW limit)
            // 2. raw stream
            // 3. opaque stream
            // 4. or stream's size is beyond rrzo's limit
            // 5. PDENode needs full raw if NOT dualcam
            // 6. have capture node
            bool useImgo =
                (pParsedAppImageStreamInfo->pAppImage_Jpeg.get() /*&& ! mPipelineConfigParams.mbHasRecording*/) ||
                !pParsedAppImageStreamInfo->vAppImage_Output_RAW16.empty() ||
                !pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.empty() ||
                pParsedAppImageStreamInfo->pAppImage_Input_Yuv.get() ||
                !!largeYuvStreamSize ||
                pParsedAppConfiguration->isType3PDSensorWithoutPDE ||
                (pParsedAppImageStreamInfo->secureInfo.type != SecType::mem_normal)  ||
                pPipelineNodesNeed->needP2CaptureNode ||
                property_get_int32("vendor.debug.feature.forceEnableIMGO", 0);

            if( useImgo )
            {
                bool useUFO = (pPipelineNodesNeed->needRaw16Node ||
                    pParsedAppImageStreamInfo->pAppImage_Output_Priv.get() ||
                    pParsedAppImageStreamInfo->pAppImage_Input_Priv.get() ||
                    pParsedAppImageStreamInfo->pAppImage_Input_Yuv.get()) ? false : true;

                //Config check hw limitation with pixel mode & stride
                MSize   imgoConfigSize = sensorSize;
                int32_t imgoConfigFormat = 0;
                size_t  imgoConfigStride = 0;
                MBOOL bIsSecure = (pParsedAppImageStreamInfo->secureInfo.type != SecType::mem_normal);
                bool useUnpackRaw = evaluateImgoAllocConfig_useUnpackRaw(pPipelineStaticInfo);
                if( ! infohelper->getImgoFmt(bitdepth, imgoConfigFormat, useUFO, useUnpackRaw, bIsSecure)
                 || ! infohelper->alignPass1HwLimitation(imgoConfigFormat, MTRUE,
                                                         imgoConfigSize, imgoConfigStride ) )
                {
                    MY_LOGE("wrong params about imgo");
                    return BAD_VALUE;
                }
                // check hw limitation with pixel mode & stride
                MSize   imgoSize = sensorSize;
                int32_t imgoFormat = 0;
                size_t  imgoStride = 0;
                if( ! infohelper->getImgoFmt(bitdepth, imgoFormat, useUFO, false, bIsSecure)
                 || ! infohelper->alignPass1HwLimitation(imgoFormat, MTRUE/*isImgo*/,
                                                         imgoSize, imgoStride ) )
                {
                    MY_LOGE("wrong params about imgo");
                    return BAD_VALUE;
                }

                //
                bool isLowMem = ::property_get_bool("ro.config.low_ram", false);
                auto& imgoAlloc = setting.imgoAlloc;
                if(isLowMem) {
                    imgoAlloc.imageSize = imgoSize;
                    imgoAlloc.format = imgoFormat;
                } else {
                    imgoAlloc.imageSize = imgoConfigSize;
                    imgoAlloc.format = imgoConfigFormat;
                }
                auto& imgoRequest = setting.imgoDefaultRequest;
                imgoRequest.imageSize = imgoSize;
                imgoRequest.format = imgoFormat;
            }
            else
            {
                setting.imgoAlloc.imageSize = MSize(0,0);
                setting.imgoDefaultRequest.imageSize = MSize(0,0);
            }

            // determine rsso size
            if( pStreamingFeatureSetting->bNeedLargeRsso )
            {
                setting.rssoSize = MSize(FSC_MAX_RSSO_WIDTH,512);//add extra 10% margin for FSC crop
            }
            else
            {
                setting.rssoSize = MSize(288,512);
            }

            if ( pPipelineNodesNeed->needFDNode && idx == 0 )
            {
                setting.fdyuvSize.w = 640;
                setting.fdyuvSize.h = (((640 * sensorSize.h) / sensorSize.w) + 1) & ~(1);
            }
            else
            {
                setting.fdyuvSize = MSize(0,0);
            }

            // Check HW resource
            {
                auto hwInfoHelper = IHwInfoHelperManager::get()->getHwInfoHelper(pPipelineStaticInfo->sensorId[idx]);
                IHwInfoHelper::QueryP1DrvIspParams ispParams;
                ispParams.in.push_back(IHwInfoHelper::QueryP1DrvIspParams::CameraInputParams{
                    .sensorId = pPipelineStaticInfo->sensorId[idx],
                    .sensorMode = (&(*pSensorSetting)[idx])->sensorMode,
                    .minProcessingFps = ((&(*pSensorSetting)[idx])->sensorFps < 30
                                        ? (&(*pSensorSetting)[idx])->sensorFps
                                        : 30),
                    .rrzoImgSize = setting.rrzoDefaultRequest.imageSize,
                });
                hwInfoHelper->queryP1DrvIspCapability(ispParams);
                if(ispParams.out.size() != ispParams.in.size())
                {
                    MY_LOGE("in(%zu) != out(%zu), please check setting.", ispParams.in.size(), ispParams.out.size());
                    for( size_t in_idx = 0; in_idx < ispParams.in.size(); in_idx++ )
                    {
                        MY_LOGD("idx(%zu) ispParams.in[%d]: sensorId(%d) sensorMode(%d) minProcessingFps(%d) rrzoImgSize(%dx%d)",
                            idx, in_idx, ispParams.in.at(in_idx).sensorId, ispParams.in.at(in_idx).sensorMode,
                            ispParams.in.at(in_idx).minProcessingFps, ispParams.in.at(in_idx).rrzoImgSize.w, ispParams.in.at(in_idx).rrzoImgSize.h);
                    }
                    for( size_t out_idx = 0; out_idx < ispParams.out.size(); out_idx++ )
                    {
                        MY_LOGD("id(%zu) ispParams.out[%d]: IqLevel(%d) Raw(%d) TargetTG(%d) isOffTwin(%d) rsv(%d)",
                            idx, out_idx, ispParams.out.at(out_idx).eIqLevel,
                            ispParams.out.at(out_idx).eResConfig.Raw, ispParams.out.at(out_idx).eResConfig.Bits.targetTG,
                            ispParams.out.at(out_idx).eResConfig.Bits.isOffTwin, ispParams.out.at(out_idx).eResConfig.Bits.rsv);
                    }
                    return UNKNOWN_ERROR;
                }
                if(idx < ispParams.out.size()) {
                    setting.camResConfig = ispParams.out.at(idx).eResConfig;
                }
                else
                {
                    MY_LOGW("out[%zu] is out of bound (%zu)", idx, ispParams.out.size());
                }
            }
            CAM_ULOGMI("%s", toString(setting).c_str());
            pvOut->push_back(setting);
        }
        return OK;
    };
}


};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

