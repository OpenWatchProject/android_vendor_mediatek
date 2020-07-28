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

#define LOG_TAG "mtkcam-PipelineModelSession-Factory"
//
#include <impl/IPipelineModelSession.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/pipeline/policy/IPipelineSettingPolicy.h>
#include <mtkcam3/pipeline/prerelease/IPreReleaseRequest.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/std/ULog.h>
#include "PipelineModelSessionDefault.h"
#include "PipelineModelSessionSMVR.h"
#include "PipelineModelSessionMultiCam.h"
#include "PipelineModelSession4Cell.h"
#include "PipelineModelSessionStreaming.h"
#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::v3::pipeline::policy::pipelinesetting;
using namespace NSCam::v3::pipeline::prerelease;

static
auto
AddAppRaw16StreamInfo( std::unordered_map<int, android::sp<IImageStreamInfo>> &map __unused,
                                android::sp<IImageStreamInfo>   pStreamInfo __unused,
                                const PipelineStaticInfo&       rPipelineStaticInfo __unused) -> bool
{
    MRect ActiveArray;
    MSize StreamSize = pStreamInfo->getImgSize();
    int Id = rPipelineStaticInfo.openId;
    MY_LOGD("Add raw 16 Id(%d) size : %dx%d ", Id, StreamSize.w, StreamSize.h);
    if (Id != -1)
    {
        auto ret = map.emplace(Id, pStreamInfo);
        if (!ret.second)
        {
            CAM_ULOGME("raw16 already exist, check raw16 size(%dx%d)", StreamSize.w, StreamSize.h);
            return false;
        }
    }
    else
    {
        CAM_ULOGME("Cannot find match pair with sensor and raw16 size, check raw16 size(%dx%d)", StreamSize.w, StreamSize.h);
        return false;
    }
    // DEBUG
    MY_LOGD("map size : %zu", map.size());
    return true;
}

static
auto
AddPhysicalAppRaw16StreamInfo( std::unordered_map<uint32_t, std::vector<android::sp<IImageStreamInfo>>> &map __unused,
                                android::sp<IImageStreamInfo>   pStreamInfo __unused,
                                const PipelineStaticInfo&       rPipelineStaticInfo __unused) -> bool
{
    MRect ActiveArray;
    MSize StreamSize = pStreamInfo->getImgSize();
    int Id = pStreamInfo->getPhysicalCameraId();
    MY_LOGD("Add raw 16 size(physical): %dx%d, sensor ID %d", StreamSize.w, StreamSize.h, Id);
    if (Id != -1)
    {
        auto iter = map.find(Id);
        if(iter != map.end())
        {
            iter->second.push_back(pStreamInfo);
        }
        else
        {
            std::vector<android::sp<IImageStreamInfo>> vStreamInfo;
            vStreamInfo.push_back(pStreamInfo);
            map.emplace(Id, vStreamInfo);
        }
    }
    else
    {
        CAM_ULOGME("Cannot find match pair with sensor and raw16 size, check raw16 size(%dx%d)", StreamSize.w, StreamSize.h);
        return false;
    }
    // DEBUG
    MY_LOGD("map size : %zu", map.size());
    return true;
}

static
auto
AddPhysicalAppYuvStreamInfo( std::unordered_map<uint32_t, std::vector<android::sp<IImageStreamInfo>>> &map __unused,
                                android::sp<IImageStreamInfo>   pStreamInfo __unused,
                                const PipelineStaticInfo&       rPipelineStaticInfo __unused) -> bool
{
    MRect ActiveArray;
    MSize StreamSize = pStreamInfo->getImgSize();
    int Id = pStreamInfo->getPhysicalCameraId();
    MY_LOGD("Add yuv size(physical): %dx%d, sensor ID %d", StreamSize.w, StreamSize.h, Id);
    if (Id != -1)
    {
        auto iter = map.find(Id);
        if(iter != map.end())
        {
            iter->second.push_back(pStreamInfo);
        }
        else
        {
            std::vector<android::sp<IImageStreamInfo>> vStreamInfo;
            vStreamInfo.push_back(pStreamInfo);
            map.emplace(Id, vStreamInfo);
        }
    }
    else
    {
        CAM_ULOGME("Cannot find match pair with sensor and yuv, check yuv(%dx%d)", StreamSize.w, StreamSize.h);
        return false;
    }
    // DEBUG
    MY_LOGD("map size : %zu", map.size());
    return true;
}



/******************************************************************************
 *
 ******************************************************************************/
static
auto
parseAppStreamInfo(std::shared_ptr<PipelineUserConfiguration> config __unused,
                           const PipelineStaticInfo&       rPipelineStaticInfo __unused) -> bool
{
    auto const& out = config->pParsedAppImageStreamInfo;
    //
    // secure flow debug property
    int secureType = property_get_int32("vendor.debug.camera.securetype", -1);
    if(secureType != -1)
    {
        MY_LOGD("force set secureType(%d)",secureType);
        out->secureInfo.type = (SecType)secureType;
    }
    //
    MSize maxStreamSize = MSize(0, 0);
    MSize maxYuvSize = MSize(0, 0);
    for (auto const& s : config->vImageStreams)
    {
        bool isOut = false;
        auto const& pStreamInfo = s.second;
        if  ( CC_LIKELY(pStreamInfo != nullptr) )
        {
            // check is secure flow or not
            if(pStreamInfo->isSecure())
            {
                out->secureInfo = pStreamInfo->getSecureInfo();
            }
            //
            switch  (pStreamInfo->getImgFormat())
            {
            case eImgFmt_RAW16: //(deprecated) It should be converted to
                                // the real unpack format by app stream manager.
            case eImgFmt_BAYER8_UNPAK:
            case eImgFmt_BAYER10_UNPAK:
            case eImgFmt_BAYER12_UNPAK:
            case eImgFmt_BAYER14_UNPAK:
            case eImgFmt_BAYER15_UNPAK:
                if  (pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_OUT) {
                    isOut = true;
                    if(pStreamInfo->getPhysicalCameraId() >= 0)
                    {
                        auto ret = AddPhysicalAppRaw16StreamInfo(out->vAppImage_Output_RAW16_Physical, pStreamInfo, rPipelineStaticInfo);
                        if (!ret) {
                            return false;
                        }
                        out->hasRawOut = true;
                    }
                    else
                    {
                        auto ret = AddAppRaw16StreamInfo(out->vAppImage_Output_RAW16, pStreamInfo, rPipelineStaticInfo);
                        if (!ret) {
                            return false;
                        }
                        out->hasRawOut = true;
                    }
                }
                else
                if  (pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN) {
                    out->pAppImage_Input_RAW16 = pStreamInfo;
                }
                break;
                //
            case eImgFmt_BLOB://AS-IS: should be removed in the future
            case eImgFmt_JPEG://TO-BE: Jpeg Capture
                isOut = true;
                out->pAppImage_Jpeg = pStreamInfo;
                break;
                //
            case eImgFmt_YV12:
            case eImgFmt_NV21:
            case eImgFmt_NV12:
            case eImgFmt_YUY2:
            case eImgFmt_Y8:
            case eImgFmt_Y16:
            case eImgFmt_RGB888:
            case eImgFmt_RGBA8888:
                if  (pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_OUT) {
                        isOut = true;
                        auto physicalCamId = pStreamInfo->getPhysicalCameraId();
                        MUINT8 ispTuningDataEnable = 0;
                        IMetadata::getEntry<MUINT8>(&config->pParsedAppConfiguration->sessionParams, MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE, ispTuningDataEnable);
                        MY_LOGD("ispTuningDataEnable(%d) StreamId(%#" PRIx64 ") ConsumerUsage(%" PRIu64 ") size(%d,%d) yuv_isp_tuning_data_size(%d,%d) raw_isp_tuning_data_size(%d,%d)",
                            ispTuningDataEnable,pStreamInfo->getStreamId(),pStreamInfo->getUsageForConsumer(), pStreamInfo->getImgSize().w,pStreamInfo->getImgSize().h,
                            rPipelineStaticInfo.yuvIspTuningDataStreamSize.w,rPipelineStaticInfo.yuvIspTuningDataStreamSize.h,
                            rPipelineStaticInfo.rawIspTuningDataStreamSize.w,rPipelineStaticInfo.rawIspTuningDataStreamSize.h);

                        const bool IS_RAW_ISP_TUNING_DATA_STREAM =
                                    (ispTuningDataEnable == 2 && // 0: disable tuning data / 1: send tuning data by APP Metadata / 2: send tuning data by YUV stream buffer
                                     ((pStreamInfo->getUsageForConsumer() & (GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_VIDEO_ENCODER)) == 0) &&
                                     pStreamInfo->getImgSize().w == rPipelineStaticInfo.rawIspTuningDataStreamSize.w &&
                                     pStreamInfo->getImgSize().h == rPipelineStaticInfo.rawIspTuningDataStreamSize.h);

                        const bool IS_YUV_ISP_TUNING_DATA_STREAM =
                                    (ispTuningDataEnable == 2 && // 0: disable tuning data / 1: send tuning data by APP Metadata / 2: send tuning data by YUV stream buffer
                                     ((pStreamInfo->getUsageForConsumer() & (GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_VIDEO_ENCODER)) == 0) &&
                                     pStreamInfo->getImgSize().w == rPipelineStaticInfo.yuvIspTuningDataStreamSize.w &&
                                     pStreamInfo->getImgSize().h == rPipelineStaticInfo.yuvIspTuningDataStreamSize.h);

                        if(physicalCamId >= 0)
                        {
                            if( IS_YUV_ISP_TUNING_DATA_STREAM )
                            {
                                MY_LOGD("StreamId(%#" PRIx64 ") find isp tuning data stream for yuv",pStreamInfo->getStreamId());
                                out->vAppImage_Output_IspTuningData_Yuv_Physical.emplace(physicalCamId, pStreamInfo);
                            }
                            else if( IS_RAW_ISP_TUNING_DATA_STREAM )
                            {
                                MY_LOGD("StreamId(%#" PRIx64 ") find isp tuning data stream for raw",pStreamInfo->getStreamId());
                                out->vAppImage_Output_IspTuningData_Raw_Physical.emplace(physicalCamId, pStreamInfo);
                            }
                            else
                            {
                                auto ret = AddPhysicalAppYuvStreamInfo(out->vAppImage_Output_Proc_Physical, pStreamInfo, rPipelineStaticInfo);
                                if (!ret) {
                                    return false;
                                }
                            }
                        }
                        else
                        {
                            if( IS_YUV_ISP_TUNING_DATA_STREAM )
                            {
                                MY_LOGD("StreamId(%#" PRIx64 ") find isp tuning data stream for yuv",pStreamInfo->getStreamId());
                                out->pAppImage_Output_IspTuningData_Yuv = pStreamInfo;
                            }
                            else if( IS_RAW_ISP_TUNING_DATA_STREAM )
                            {
                                MY_LOGD("StreamId(%#" PRIx64 ") find isp tuning data stream for raw",pStreamInfo->getStreamId());
                                out->pAppImage_Output_IspTuningData_Raw = pStreamInfo;
                            }
                            else
                            {
                                out->vAppImage_Output_Proc.emplace(s.first, pStreamInfo);
                                if  (   ! out->hasVideoConsumer
                                    &&  ( pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER )
                                    )
                                {
                                    out->hasVideoConsumer = true;
                                    out->videoImageSize = pStreamInfo->getImgSize();
                                    out->hasVideo4K = ( out->videoImageSize.w*out->videoImageSize.h > 8000000 )? true : false;
                                }
                                if  ( maxYuvSize.size() <= pStreamInfo->getImgSize().size() )
                                {
                                    maxYuvSize = pStreamInfo->getImgSize();
                                }
                            }
                        }
                }
                else
                if  (pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN) {
                    out->pAppImage_Input_Yuv = pStreamInfo;
                }
                break;
                //
            case eImgFmt_CAMERA_OPAQUE:
                if  (pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_OUT) {
                    isOut = true;
                    out->pAppImage_Output_Priv = pStreamInfo;
                    out->hasRawOut = true;
                }
                else
                if  (pStreamInfo->getStreamType() == eSTREAMTYPE_IMAGE_IN) {
                    out->pAppImage_Input_Priv = pStreamInfo;
                }
                break;
                //
            default:
                CAM_ULOGME("[%s] Unsupported format:0x%x", __FUNCTION__, pStreamInfo->getImgFormat());
                break;
            }
        }

        //if (isOut) //[TODO] why only for output?
        {
            if  ( maxStreamSize.size() <= pStreamInfo->getImgSize().size() ) {
                maxStreamSize = pStreamInfo->getImgSize();
            }
        }

    }

    out->maxImageSize = maxStreamSize;
    out->maxYuvSize = maxYuvSize;
    config->pParsedAppConfiguration->useP1DirectFDYUV = !(out->hasVideoConsumer) && rPipelineStaticInfo.isP1DirectFDYUV;

    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
extractSMVRBatchInfo(
    const std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>> vImageStreams,
    const IMetadata &metaSessionParams,
    MUINT32 operationMode
) -> std::shared_ptr<ParsedSMVRBatchInfo>
{
    MINT32 customP2BatchNum = 1;
    MINT32 p2IspBatchNum = 1;
    std::shared_ptr<ParsedSMVRBatchInfo> pParsedSMVRBatchInfo = nullptr;
    int32_t isFromApMeta = 0;

    IMetadata::IEntry const& entry = metaSessionParams.entryFor(MTK_SMVR_FEATURE_SMVR_MODE);
    if  ( !entry.isEmpty() && entry.count() >= 2 )
    {
        isFromApMeta = 1;
        pParsedSMVRBatchInfo = std::make_shared<ParsedSMVRBatchInfo>();
        if  ( CC_UNLIKELY(pParsedSMVRBatchInfo == nullptr) ) {
            CAM_ULOGME("[%s] Fail on make_shared<pParsedSMVRBatchInfo>", __FUNCTION__);
            return nullptr;
        }
        // get image w/h
        int k = 0;
        for (auto const& s : vImageStreams)
        {
            auto const &spStreamInfo = s.second;
            MUINT64 usage = spStreamInfo != NULL ? spStreamInfo->getUsageForAllocator() : 0;
    //        if (usage & GRALLOC_USAGE_HW_VIDEO_ENCODER)
            if (pParsedSMVRBatchInfo->imgW == 0) // !!NOTES: assume previewSize = videoOutSize
            {
                // found
                pParsedSMVRBatchInfo->imgW = spStreamInfo->getImgSize().w;
                pParsedSMVRBatchInfo->imgH = spStreamInfo->getImgSize().h;
//                break;
            }
            MY_LOGD("SMVRBatch: vImageStreams[%d]=%dx%d, isVideo=%d", k++, spStreamInfo->getImgSize().w, spStreamInfo->getImgSize().h, (usage & GRALLOC_USAGE_HW_VIDEO_ENCODER) != 0);
        }

        if  ( !entry.isEmpty() && entry.count() >= 2)
        {
            pParsedSMVRBatchInfo->maxFps = entry.itemAt(0, Type2Type< MINT32 >()); // meta[0]: LmaxFps
            customP2BatchNum = entry.itemAt(1, Type2Type< MINT32 >());             // meta[1]: customP2BatchNum
        }
        pParsedSMVRBatchInfo->maxFps = ::property_get_int32("vendor.debug.smvrb.maxFps", pParsedSMVRBatchInfo->maxFps);
//        if (pParsedSMVRBatchInfo->maxFps <= 120)
//        {
//            CAM_ULOGME("%s: SMVRBatch: !!err: only support more than 120fps: curr-maxFps=%d", __FUNCTION__, pParsedSMVRBatchInfo->maxFps);
//            return nullptr;
//        }

        // determine final P2BatchNum
        #define min(a,b)  ((a) < (b) ? (a) : (b))
        MUINT32 vOutSize = pParsedSMVRBatchInfo->imgW * pParsedSMVRBatchInfo->imgH;
        if (vOutSize <= 640*480) // vga
        {
            p2IspBatchNum = ::property_get_int32("ro.vendor.smvr.p2batch.vga", 1);
        }
        else if (vOutSize <= 1280*736) // hd
        {
            p2IspBatchNum = ::property_get_int32("ro.vendor.smvr.p2batch.hd", 1);
            pParsedSMVRBatchInfo->p2BatchNum = min(p2IspBatchNum, customP2BatchNum);
        }
        else if (vOutSize <= 1920*1088) // fhd
        {
            p2IspBatchNum = ::property_get_int32("ro.vendor.smvr.p2batch.fhd", 1);
            pParsedSMVRBatchInfo->p2BatchNum = min(p2IspBatchNum, customP2BatchNum);
        }
        else
        {
           p2IspBatchNum = 1;
        }
        // change p2BatchNum by debug adb if necessary
        pParsedSMVRBatchInfo->p2BatchNum = ::property_get_int32("vendor.debug.smvrb.P2BatchNum", p2IspBatchNum);
        pParsedSMVRBatchInfo->p2BatchNum = min(p2IspBatchNum, customP2BatchNum);

        #undef min

        // P1BatchNum
        pParsedSMVRBatchInfo->p1BatchNum = pParsedSMVRBatchInfo->maxFps/30;
        // operatioin mode
        pParsedSMVRBatchInfo->opMode = operationMode;

        // log level
        pParsedSMVRBatchInfo->logLevel = ::property_get_int32("vendor.debug.smvrb.loglevel", 0);
    }
    else
    {
        MINT32 propSmvrBatchEnable = ::property_get_int32("vendor.debug.smvrb.enable", 0);
        if (propSmvrBatchEnable)
        {
            pParsedSMVRBatchInfo = std::make_shared<ParsedSMVRBatchInfo>();
            if  ( CC_UNLIKELY(pParsedSMVRBatchInfo == nullptr) ) {
                CAM_ULOGME("[%s] Fail on make_shared<pParsedSMVRBatchInfo>", __FUNCTION__);
                return nullptr;
            }

            // get image w/h
            int k = 0;
            for (auto const& s : vImageStreams)
            {
                auto const &spStreamInfo = s.second;
                MUINT64 usage = spStreamInfo != NULL ? spStreamInfo->getUsageForAllocator() : 0;
        //        if (usage & GRALLOC_USAGE_HW_VIDEO_ENCODER)
                if (pParsedSMVRBatchInfo->imgW == 0) // !!NOTES: assume previewSize = videoOutSize
                {
                    // found
                    pParsedSMVRBatchInfo->imgW = spStreamInfo->getImgSize().w;
                    pParsedSMVRBatchInfo->imgH = spStreamInfo->getImgSize().h;
//                    break;
                }
                MY_LOGD("SMVRBatch: vImageStreams[%d]=%dx%d, isVideo=%d", k++, spStreamInfo->getImgSize().w, spStreamInfo->getImgSize().h, (usage & GRALLOC_USAGE_HW_VIDEO_ENCODER) != 0);
            }

            pParsedSMVRBatchInfo->maxFps = ::property_get_int32("vendor.debug.smvrb.maxFps", pParsedSMVRBatchInfo->maxFps);
//            if (pParsedSMVRBatchInfo->maxFps <= 120)
//            {
//                CAM_ULOGME("%s: SMVRBatch: !!err: only support more than 120fps: curr-maxFps=%d", __FUNCTION__, pParsedSMVRBatchInfo->maxFps);
//                return nullptr;
//            }

            pParsedSMVRBatchInfo->p2BatchNum = ::property_get_int32("vendor.debug.smvrb.P2BatchNum", 1);
            pParsedSMVRBatchInfo->p1BatchNum = ::property_get_int32("vendor.debug.smvrb.P1BatchNum", 1);
            pParsedSMVRBatchInfo->opMode     = operationMode;
            pParsedSMVRBatchInfo->logLevel   = ::property_get_int32("vendor.debug.smvrb.loglevel", 0);
        }
    }

    if (pParsedSMVRBatchInfo != nullptr)
    {
         MY_LOGD("SMVRBatch: config: isFromApMeta=%d, vOutImg=%dx%d, meta-info(maxFps=%d, customP2BatchNum=%d), p2IspBatchNum=%d, final-P2BatchNum=%d, p1BatchNum=%d, opMode=%d, logLevel=%d",
             isFromApMeta,
             pParsedSMVRBatchInfo->imgW, pParsedSMVRBatchInfo->imgH,
             pParsedSMVRBatchInfo->maxFps, customP2BatchNum,
             p2IspBatchNum, pParsedSMVRBatchInfo->p2BatchNum,
             pParsedSMVRBatchInfo->p1BatchNum,
             pParsedSMVRBatchInfo->opMode,
             pParsedSMVRBatchInfo->logLevel
         );

    }

    return pParsedSMVRBatchInfo;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
convertToUserConfiguration(
    const PipelineStaticInfo&       rPipelineStaticInfo __unused,
    const UserConfigurationParams&  rUserConfigurationParams __unused
) -> std::shared_ptr<PipelineUserConfiguration>
{
    auto pParsedAppConfiguration = std::make_shared<ParsedAppConfiguration>();
    if  ( CC_UNLIKELY(pParsedAppConfiguration == nullptr) ) {
        CAM_ULOGME("[%s] Fail on make_shared<ParsedAppConfiguration>", __FUNCTION__);
        return nullptr;
    }

    auto pParsedAppImageStreamInfo = std::make_shared<ParsedAppImageStreamInfo>();
    if  ( CC_UNLIKELY(pParsedAppImageStreamInfo == nullptr) ) {
        CAM_ULOGME("[%s] Fail on make_shared<ParsedAppImageStreamInfo>", __FUNCTION__);
        return nullptr;
    }

    auto pParsedMultiCamInfo = std::make_shared<ParsedMultiCamInfo>();
    if  ( CC_UNLIKELY(pParsedMultiCamInfo == nullptr) ) {
        CAM_ULOGME("[%s] Fail on make_shared<ParsedMultiCamInfo>", __FUNCTION__);
        return nullptr;
    }

    auto out = std::make_shared<PipelineUserConfiguration>();
    if  ( CC_UNLIKELY(out == nullptr) ) {
        CAM_ULOGME("[%s] Fail on make_shared<PipelineUserConfiguration>", __FUNCTION__);
        return nullptr;
    }

    static bool isLowRam = ::property_get_bool("ro.config.low_ram", false);
    int32_t initRequest = property_get_int32("vendor.debug.camera.pass1initrequestnum", 0);

    pParsedAppConfiguration->operationMode = rUserConfigurationParams.operationMode;
    pParsedAppConfiguration->sessionParams = rUserConfigurationParams.sessionParams;
    pParsedAppConfiguration->isConstrainedHighSpeedMode = (pParsedAppConfiguration->operationMode == 1/*CONSTRAINED_HIGH_SPEED_MODE*/);
    {
        const char* key = "persist.vendor.camera3.operationMode.superNightMode";
        //this property is not defined if it's 0 (normal mode).
        int32_t const operationMode_superNightMode = ::property_get_int32(key, 0);
        if (operationMode_superNightMode != 0) {
            CAM_ULOGMI("%s=%#x", key, operationMode_superNightMode);
            pParsedAppConfiguration->isSuperNightMode = ((uint32_t)operationMode_superNightMode == pParsedAppConfiguration->operationMode);
        }
        else {
            pParsedAppConfiguration->isSuperNightMode = false;
        }
    }

    pParsedAppConfiguration->isProprietaryClient = [&](){
            int32_t value = 0;
            bool ret = IMetadata::getEntry(&rUserConfigurationParams.sessionParams,
                                            MTK_CONFIGURE_SETTING_PROPRIETARY, value);
            return value;
        }();
    pParsedAppConfiguration->isType3PDSensorWithoutPDE = [&](){
            if (isLowRam && rPipelineStaticInfo.isType3PDSensorWithoutPDE) {
                bool proprietary = pParsedAppConfiguration->isProprietaryClient;
                CAM_LOGI("low_ram + ISP3 Type3-PD - proprietary client:%d", proprietary);
                return proprietary;
            }
            return rPipelineStaticInfo.isType3PDSensorWithoutPDE;
        }();

    #if MTKCAM_PASS1_UNPACK_RAW_SUPPORT
    if(rPipelineStaticInfo.isDualDevice)
    {
        // for multicam device, it not support yet.
        pParsedAppConfiguration->useP1DirectAppRaw = false;
    }
    else
    {
        pParsedAppConfiguration->useP1DirectAppRaw = true;
    }
    #endif
    // check dual path
    if(rPipelineStaticInfo.isDualDevice)
    {
        StereoSettingProvider::setLogicalDeviceID(rPipelineStaticInfo.openId);
        MINT32 mode = -1;
        IMetadata::getEntry<MINT32>(
                    &pParsedAppConfiguration->sessionParams,
                    MTK_MULTI_CAM_FEATURE_MODE,
                    mode);
        MINT32 iFeatureMode = ::property_get_int32("vendor.camera.forceFeatureMode", mode);
        // single cam mode does not set MTK_MULTI_CAM_FEATURE_MODE and vPhysicCameras should
        // be zero.
        bool isSingleCamFlow = ((iFeatureMode == -1) &&
                               (rUserConfigurationParams.vPhysicCameras.size() == 0));
        if(isSingleCamFlow)
        {
            pParsedMultiCamInfo->mDualDevicePath = NSCam::v3::pipeline::policy::DualDevicePath::Single;
        }
        else
        {
            auto outBufList = StereoSettingProvider::getBokehJpegBufferList();
            bool supportJpegPack = (outBufList.count() > 1 &&
                                    iFeatureMode == MTK_MULTI_CAM_FEATURE_MODE_VSDOF)?true:false;
            //
            pParsedMultiCamInfo->mDualDevicePath = NSCam::v3::pipeline::policy::DualDevicePath::MultiCamControl;
            pParsedMultiCamInfo->mDualFeatureMode = iFeatureMode;
            pParsedMultiCamInfo->mSupportPackJpegImages = supportJpegPack;
            pParsedMultiCamInfo->mSupportPhysicalOutput = (rUserConfigurationParams.vPhysicCameras.size() > 0);
            // update streaming feature mode
            MINT32 streamingMode = NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF;
            MINT32 captureMode = NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE;
            MINT32 forcemode = property_get_int32( "vendor.camera.stereo.mode", getStereoModeType());
            if(MTK_MULTI_CAM_FEATURE_MODE_VSDOF == iFeatureMode)
            {
                if(forcemode == 0)
                {
                    MY_LOGI("vsdof");
                    streamingMode = NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF;
                }
                else if(forcemode == 1)
                {
                    MY_LOGI("force 3rd flow");
                    streamingMode = NSCam::v1::Stereo::E_STEREO_FEATURE_THIRD_PARTY;
                }
                else if(forcemode == 2)
                {
                    MY_LOGI("force tk depth");
                    streamingMode = NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP;
                }
                else
                {
                    return nullptr;
                }
            }
            else if(MTK_MULTI_CAM_FEATURE_MODE_ZOOM == iFeatureMode)
            {
                streamingMode = NSCam::v1::Stereo::E_DUALCAM_FEATURE_ZOOM;
                captureMode = NSCam::v1::Stereo::E_DUALCAM_FEATURE_ZOOM;
            }
            else
            {
                streamingMode = NSCam::v1::Stereo::E_STEREO_FEATURE_MULTI_CAM;
                captureMode = NSCam::v1::Stereo::E_STEREO_FEATURE_MULTI_CAM;
            }
            pParsedMultiCamInfo->mStreamingFeatureMode = streamingMode;
            pParsedMultiCamInfo->mCaptureFeatureMode = captureMode;
        }
        MY_LOGI("dual cam device(s:%d,path:%d) forceFeatureMode(%d) pack jpeg(%d) physical(%d) s(%d) c(%d)",
                                isSingleCamFlow,
                                pParsedMultiCamInfo->mDualDevicePath,
                                pParsedMultiCamInfo->mDualFeatureMode,
                                pParsedMultiCamInfo->mSupportPackJpegImages,
                                pParsedMultiCamInfo->mSupportPhysicalOutput,
                                pParsedMultiCamInfo->mStreamingFeatureMode,
                                pParsedMultiCamInfo->mCaptureFeatureMode);
    }

    if (initRequest == 0)
    {
        if(IMetadata::getEntry<MINT32>(&pParsedAppConfiguration->sessionParams, MTK_CONFIGURE_SETTING_INIT_REQUEST, initRequest))
        {
            MY_LOGD("APP set init frame : %d, if not be zero, force it to be 4", initRequest);
            if (initRequest != 0)
            {
                initRequest = 4; // force 4
            }
        }
    }
    MUINT8 ispEn = 0;
    if(IMetadata::getEntry<MUINT8>(&pParsedAppConfiguration->sessionParams, MTK_CONTROL_CAPTURE_ISP_TUNING_DATA_ENABLE, ispEn))
    {
        MY_LOGD("APP set isp tuning flow : %d", ispEn);
    }
    pParsedAppConfiguration->hasTuningEnable = property_get_bool("vendor.camera.tuning.data.enable", ispEn);
    pParsedAppConfiguration->initRequest = initRequest;
    pParsedAppConfiguration->configTimestamp = rUserConfigurationParams.configTimestamp;
    out->pParsedAppConfiguration = pParsedAppConfiguration;
    out->pParsedAppConfiguration->pParsedMultiCamInfo = pParsedMultiCamInfo;
    out->pParsedAppImageStreamInfo = pParsedAppImageStreamInfo;
    out->vImageStreams = rUserConfigurationParams.vImageStreams;
    out->vMetaStreams = rUserConfigurationParams.vMetaStreams;
    out->vMinFrameDuration = rUserConfigurationParams.vMinFrameDuration;
    out->vStallFrameDuration = rUserConfigurationParams.vStallFrameDuration;
    out->vPhysicCameras = rUserConfigurationParams.vPhysicCameras;

    // SMVRBatch: get config info
    out->pParsedAppConfiguration->pParsedSMVRBatchInfo = extractSMVRBatchInfo(out->vImageStreams, pParsedAppConfiguration->sessionParams, pParsedAppConfiguration->operationMode);

    auto ret = parseAppStreamInfo(out, rPipelineStaticInfo);
    if (!ret)
    {
        return nullptr;
    }

    return out;
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
makeAppRaw16Reprocessor(
    IPipelineModelSessionFactory::CreationParams const& creationParams,
    std::shared_ptr<PipelineUserConfiguration>const& pUserConfiguration,
    const char* name
) -> android::sp<IAppRaw16Reprocessor>
{
    return IAppRaw16Reprocessor::makeInstance(
                name,
                IAppRaw16Reprocessor::CtorParams{
                    .pPipelineStaticInfo= creationParams.pPipelineStaticInfo,
                    .pUserConfiguration = pUserConfiguration,
                }
            );
}


/******************************************************************************
 *
 ******************************************************************************/
static
auto
decidePipelineModelSession(
    IPipelineModelSessionFactory::CreationParams const& creationParams,
    std::shared_ptr<PipelineUserConfiguration>const& pUserConfiguration,
    std::shared_ptr<PipelineUserConfiguration2>const& pUserConfiguration2,
    std::shared_ptr<IPipelineSettingPolicy>const& pSettingPolicy
) -> android::sp<IPipelineModelSession>
{
    auto convertTo_CtorParams = [=]() {
        return PipelineModelSessionBase::CtorParams{
            .staticInfo = {
                .pPipelineStaticInfo    = creationParams.pPipelineStaticInfo,
                .pUserConfiguration     = pUserConfiguration,
                .pUserConfiguration2    = pUserConfiguration2,
            },
            .debugInfo = {
                .pErrorPrinter          = creationParams.pErrorPrinter,
                .pWarningPrinter        = creationParams.pWarningPrinter,
                .pDebugPrinter          = creationParams.pDebugPrinter,
            },
            .pPipelineModelCallback     = creationParams.pPipelineModelCallback,
            .pPipelineSettingPolicy     = pSettingPolicy,
        };
    };

    ////////////////////////////////////////////////////////////////////////////
    //  BG Service
    ////////////////////////////////////////////////////////////////////////////
    IPreReleaseRequestMgr::getInstance()->configBGService(pUserConfiguration->pParsedAppConfiguration->sessionParams);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  [Session Policy] decide which session
    //  Add special sessions below...
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    ////////////////////////////////////////////////////////////////////////////
    //  Secure Flow
    ////////////////////////////////////////////////////////////////////////////
    if (pUserConfiguration->pParsedAppImageStreamInfo->secureInfo.type != SecType::mem_normal) {
        CAM_LOGI("[%s] Secure flow using Security Session", __FUNCTION__);
        return PipelineModelSessionDefault::makeInstance("Default/", convertTo_CtorParams());
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Super Night Mode Capture
    ////////////////////////////////////////////////////////////////////////////
    if (pUserConfiguration->pParsedAppConfiguration->isSuperNightMode) {
        CAM_ULOGMI("[%s] Super Night Mode using Default Session", __FUNCTION__);
        return PipelineModelSessionDefault::makeInstance(
                    "Default+SuperNightMode/", convertTo_CtorParams(),
                    PipelineModelSessionDefault::ExtCtorParams{
                        .pAppRaw16Reprocessor =
                            makeAppRaw16Reprocessor(
                                creationParams, pUserConfiguration,
                                "SuperNightMode"
                            ),
                    }
                );
    }

    auto const operationMode = pUserConfiguration->pParsedAppConfiguration->operationMode;
    switch  ( operationMode )
    {
    ////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////
    case 0/* NORMAL_MODE - [FIXME] hard-code */:{
        // SMVRBatch check
        {
            auto const& pParsedAppConfiguration     = pUserConfiguration->pParsedAppConfiguration;
            auto const& pParsedSMVRBatchInfo        = pParsedAppConfiguration->pParsedSMVRBatchInfo;

            if (pParsedSMVRBatchInfo != nullptr)
            {
                MY_LOGI("SMVRBatch");
                return PipelineModelSessionSMVR::makeInstance(convertTo_CtorParams());
            }
        }

        if(creationParams.pPipelineStaticInfo->is4CellSensor)
        {
            MY_LOGI("4Cell");
            return PipelineModelSession4Cell::makeInstance("4Cell/", convertTo_CtorParams());
        }
        if(creationParams.pPipelineStaticInfo->isVhdrSensor)
        {
            MY_LOGI("Streaming");
            return PipelineModelSessionStreaming::makeInstance("Streaming/", convertTo_CtorParams());
        }
        if(creationParams.pPipelineStaticInfo->isDualDevice)
        {
            auto const& pParsedAppConfiguration     = pUserConfiguration->pParsedAppConfiguration;
            auto const& pParsedMultiCamInfo          = pParsedAppConfiguration->pParsedMultiCamInfo;
            if(pParsedMultiCamInfo->mDualDevicePath == NSCam::v3::pipeline::policy::DualDevicePath::MultiCamControl)
            {
                if(MTK_MULTI_CAM_FEATURE_MODE_VSDOF == pParsedMultiCamInfo->mDualFeatureMode)
                {
                    MY_LOGI("Vsdof");
                    return PipelineModelSessionMultiCam::makeInstance(
                                                            std::string("Vsdof/"),
                                                            convertTo_CtorParams());
                }
                else if(MTK_MULTI_CAM_FEATURE_MODE_ZOOM == pParsedMultiCamInfo->mDualFeatureMode)
                {
                    MY_LOGI("Zoom");
                    return PipelineModelSessionMultiCam::makeInstance(
                                                            std::string("Zoom/"),
                                                            convertTo_CtorParams());
                }
                else
                {
                    MY_LOGI("Multicam");
                    return PipelineModelSessionMultiCam::makeInstance(std::string("Multicam/"),
                                                                convertTo_CtorParams());
                }
            }
        }
        }break;

    ////////////////////////////////////////////////////////////////////////////
    //  Session: SMVR
    ////////////////////////////////////////////////////////////////////////////
    case 1/* CONSTRAINED_HIGH_SPEED_MODE */:{
        //  Session SMVR
        MY_LOGI("SMVRConstraint");
        return PipelineModelSessionSMVR::makeInstance(convertTo_CtorParams());
        }break;

    ////////////////////////////////////////////////////////////////////////////
    default:{
        CAM_ULOGME("[%s] Unsupported operationMode:0x%x", __FUNCTION__, operationMode);
        return nullptr;
        }break;
    }
    ////////////////////////////////////////////////////////////////////////////
    //  Session: Default
    ////////////////////////////////////////////////////////////////////////////
    MY_LOGI("Default");
    return PipelineModelSessionDefault::makeInstance("Default/", convertTo_CtorParams());
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IPipelineModelSessionFactory::
createPipelineModelSession(
    CreationParams const& params __unused
) -> android::sp<IPipelineModelSession>
{
    #undef  CHECK_PTR
    #define CHECK_PTR(_ptr_, _msg_) \
        if  ( CC_UNLIKELY(_ptr_ == nullptr) ) { \
            CAM_ULOGME("[%s] nullptr pointer - %s", __FUNCTION__, _msg_); \
            return nullptr; \
        }

    //  (1) validate input parameters.
    CHECK_PTR(params.pPipelineStaticInfo, "pPipelineStaticInfo");
    CHECK_PTR(params.pUserConfigurationParams, "pUserConfigurationParams");
    CHECK_PTR(params.pErrorPrinter, "pErrorPrinter");
    CHECK_PTR(params.pWarningPrinter, "pWarningPrinter");
    CHECK_PTR(params.pDebugPrinter, "pDebugPrinter");
    CHECK_PTR(params.pPipelineModelCallback, "pPipelineModelCallback");

    //  (2) convert to UserConfiguration
    auto pUserConfiguration = convertToUserConfiguration(
        *params.pPipelineStaticInfo,
        *params.pUserConfigurationParams
    );
    CHECK_PTR(pUserConfiguration, "convertToUserConfiguration");

    auto pUserConfiguration2 = std::make_shared<PipelineUserConfiguration2>();
    CHECK_PTR(pUserConfiguration2, "make_shared<PipelineUserConfiguration2>");
    pUserConfiguration2->pStreamInfoBuilderFactory  = params.pUserConfigurationParams->pStreamInfoBuilderFactory;
    pUserConfiguration2->pImageStreamBufferProvider = params.pUserConfigurationParams->pImageStreamBufferProvider;


    //  (3) pipeline policy
    auto pSettingPolicy = IPipelineSettingPolicyFactory::createPipelineSettingPolicy(
        IPipelineSettingPolicyFactory::CreationParams{
            .pPipelineStaticInfo        = params.pPipelineStaticInfo,
            .pPipelineUserConfiguration = pUserConfiguration,
            .pAppStreamInfoBuilderFactory = params.pUserConfigurationParams->pStreamInfoBuilderFactory,
    });
    CHECK_PTR(pSettingPolicy, "Fail on createPipelineSettingPolicy");

    //  (4) pipeline session
    auto pSession = decidePipelineModelSession(params, pUserConfiguration, pUserConfiguration2, pSettingPolicy);
    CHECK_PTR(pSession, "Fail on decidePipelineModelSession");

    return pSession;
}

