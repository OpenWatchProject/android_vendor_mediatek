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

#define LOG_TAG "mtkcam-AppRequestParser"

#include <impl/AppRequestParser.h>
#include <mtkcam/utils/std/ULog.h>

#include "MyUtils.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_MODEL);

/******************************************************************************
 *
 ******************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::pipeline::model;
using namespace NSCam::Utils::ULog;
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


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace model {


/******************************************************************************
 *
 ******************************************************************************/
static auto getDebugLogLevel() -> int32_t
{
    return ::property_get_int32("persist.vendor.debug.camera.log", 0);
}
static int32_t gLogLevel = getDebugLogLevel();

/******************************************************************************
 *
 ******************************************************************************/
static auto getProcessRawDump() -> int32_t
{
    return ::property_get_int32("persist.vendor.debug.camera.app.proc.dump", 0);
}
static int32_t gProcessRawDump = getProcessRawDump();


/******************************************************************************
 *
 ******************************************************************************/
static void print(android::Printer& printer, ParsedAppRequest const& o)
{
    printer.printFormatLine(".requestNo=%u", o.requestNo);

    if  (auto p = o.pParsedAppImageStreamBuffers) {
        android::String8 os;
        os += "App image stream buffers=";
        os += toString(*p);
        printer.printLine(os.c_str());
    }

}


/******************************************************************************
 *
 ******************************************************************************/
static void dumpToLog(
    ParsedAppRequest const& o,
    DetailsType logPriority
)
{
    ULogPrinter logPrinter(__ULOG_MODULE_ID, LOG_TAG, logPriority, "");
    logPrinter.printLine("ParsedAppRequest=");
    print(logPrinter, o);
}
/******************************************************************************
 *
 ******************************************************************************/
static void addToOutputPhysicalSensors(
    std::vector<uint32_t> &outputPhysicalSensors,
    uint32_t id
)
{
    auto iter = std::find(
                        outputPhysicalSensors.begin(),
                        outputPhysicalSensors.end(),
                        id);
    if(iter == outputPhysicalSensors.end())
    {
        outputPhysicalSensors.push_back(id);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
static auto categorizeImgStream(
    ParsedAppRequest* out,
    std::unordered_map<StreamId_T, android::sp<IImageStreamBuffer>>const* pStreamBuffers,
    std::unordered_map<StreamId_T, android::sp<IImageStreamInfo>>const* pStreamInfos,
    bool isInput,
    PipelineUserConfiguration const* pUserConfiguration
) -> int
{
    auto const& pParsedAppImageStreamInfo = out->pParsedAppImageStreamInfo;
    MSize maxStreamSize = MSize(0, 0);

    auto categorize = [&](auto const& pStreamInfo)
    {
        if ( CC_LIKELY(pStreamInfo != nullptr) )
        {
            switch( pStreamInfo->getImgFormat() )
            {
                case eImgFmt_RAW16: //(deprecated) It should be converted to
                                    // the real unpack format by app stream manager.
                case eImgFmt_BAYER10:
                case eImgFmt_BAYER8_UNPAK:
                case eImgFmt_BAYER10_UNPAK:
                case eImgFmt_BAYER12_UNPAK:
                case eImgFmt_BAYER14_UNPAK:
                case eImgFmt_BAYER15_UNPAK:
                    if (isInput)
                    {
                        pParsedAppImageStreamInfo->pAppImage_Input_RAW16 = pStreamInfo;
                    }
                    else
                    {
                        if(pStreamInfo->getPhysicalCameraId() >= 0)
                        {
                            auto sensorId = pStreamInfo->getPhysicalCameraId();
                            {
                                // stream info
                                auto iter =
                                    pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.find(sensorId);
                                if(iter != pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.end())
                                {
                                    iter->second.push_back(pStreamInfo);
                                }
                                else
                                {
                                    std::vector<android::sp<IImageStreamInfo>> vStreamInfo;
                                    vStreamInfo.push_back(pStreamInfo);
                                    pParsedAppImageStreamInfo->vAppImage_Output_RAW16_Physical.emplace(
                                                                sensorId, vStreamInfo);
                                }
                                addToOutputPhysicalSensors(
                                                    pParsedAppImageStreamInfo->outputPhysicalSensors,
                                                    sensorId);
                            }
                            pParsedAppImageStreamInfo->hasRawOut = true;
                            MY_LOGD("find raw16 physical stream, sensor Id=%d", sensorId);
                        }
                        else
                        {
                            int findId = 0;
                            for (auto const& raw16 : pUserConfiguration->pParsedAppImageStreamInfo->vAppImage_Output_RAW16)
                            {
                                if (raw16.second->getStreamId() == pStreamInfo->getStreamId())
                                {
                                    pParsedAppImageStreamInfo->vAppImage_Output_RAW16.emplace(raw16.first, pStreamInfo);
                                    findId = 1;
                                    pParsedAppImageStreamInfo->hasRawOut = true;
                                    MY_LOGD("find raw16 stream, sensor Id=%d", raw16.first);
                                    break;
                                }
                            }
                            if (!findId)
                            {
                                MY_LOGW("cannot find raw16 stream");
                            }
                        }
                    }
                    break;
                case eImgFmt_CAMERA_OPAQUE:
                    if (isInput)
                    {
                        pParsedAppImageStreamInfo->pAppImage_Input_Priv = pStreamInfo;
                    }
                    else
                    {
                        pParsedAppImageStreamInfo->pAppImage_Output_Priv = pStreamInfo;
                        pParsedAppImageStreamInfo->hasRawOut = true;
                    }
                    break;
                    //
                case eImgFmt_BLOB://AS-IS: should be removed in the future
                case eImgFmt_JPEG://TO-BE: Jpeg Capture
                    pParsedAppImageStreamInfo->pAppImage_Jpeg = pStreamInfo;
                    if (pStreamInfo->getImgSize().size() > maxStreamSize.size())
                    {
                        maxStreamSize = pStreamInfo->getImgSize();
                    }
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
                    if (isInput)
                    {
                        pParsedAppImageStreamInfo->pAppImage_Input_Yuv = pStreamInfo;
                    }
                    else
                    {
                        auto physicalCamId = pStreamInfo->getPhysicalCameraId();
                        if(physicalCamId >= 0)
                        {
                            bool isISPTuningDataStream = false;
                            {
                                auto &vYuvIspTuningDataPhysical = pUserConfiguration->pParsedAppImageStreamInfo->vAppImage_Output_IspTuningData_Yuv_Physical;
                                auto iter = vYuvIspTuningDataPhysical.find(physicalCamId);
                                if(iter != vYuvIspTuningDataPhysical.end())
                                {
                                    auto &pYuvIspTuningDataPhysical = iter->second;
                                    if( pYuvIspTuningDataPhysical.get() &&
                                        pYuvIspTuningDataPhysical->getStreamId() == pStreamInfo->getStreamId())
                                    {
                                        MY_LOGD("StreamId(%#" PRIxPTR ") find isp meta stream for yuv(physical)", pStreamInfo->getStreamId());
                                        pParsedAppImageStreamInfo->vAppImage_Output_IspTuningData_Yuv_Physical.emplace(physicalCamId, pStreamInfo);
                                        isISPTuningDataStream = true;
                                    }
                                }
                            }

                            {
                                auto &vRawIspTuningDataPhysical = pUserConfiguration->pParsedAppImageStreamInfo->vAppImage_Output_IspTuningData_Raw_Physical;
                                auto iter = vRawIspTuningDataPhysical.find(physicalCamId);
                                if(iter != vRawIspTuningDataPhysical.end())
                                {
                                    auto &pRawIspTuningDataPhysical = iter->second;
                                    if( pRawIspTuningDataPhysical.get() &&
                                        pRawIspTuningDataPhysical->getStreamId() == pStreamInfo->getStreamId())
                                    {
                                        MY_LOGD("StreamId(%#" PRIxPTR ") find isp meta stream for raw(physical)", pStreamInfo->getStreamId());
                                        pParsedAppImageStreamInfo->vAppImage_Output_IspTuningData_Raw_Physical.emplace(physicalCamId, pStreamInfo);
                                        isISPTuningDataStream = true;
                                    }
                                }
                            }

                            if( !isISPTuningDataStream )
                            {
                                // stream info
                                auto iter =
                                    pParsedAppImageStreamInfo->vAppImage_Output_Proc_Physical.find(physicalCamId);
                                if(iter != pParsedAppImageStreamInfo->vAppImage_Output_Proc_Physical.end())
                                {
                                    iter->second.push_back(pStreamInfo);
                                }
                                else
                                {
                                    std::vector<android::sp<IImageStreamInfo>> vStreamInfo;
                                    vStreamInfo.push_back(pStreamInfo);
                                    pParsedAppImageStreamInfo->vAppImage_Output_Proc_Physical.emplace(
                                                                physicalCamId, vStreamInfo);
                                }
                                addToOutputPhysicalSensors(
                                                    pParsedAppImageStreamInfo->outputPhysicalSensors,
                                                    physicalCamId);
                            }
                        }
                        else
                        {
                            //isp tuning data stream for yuv
                            if( pUserConfiguration->pParsedAppImageStreamInfo->pAppImage_Output_IspTuningData_Yuv.get() &&
                                pUserConfiguration->pParsedAppImageStreamInfo->pAppImage_Output_IspTuningData_Yuv->getStreamId() == pStreamInfo->getStreamId())
                            {
                                  MY_LOGD("StreamId(%#" PRIxPTR ") find isp tuning data stream for yuv",pStreamInfo->getStreamId());
                                  pParsedAppImageStreamInfo->pAppImage_Output_IspTuningData_Yuv = pStreamInfo;
                            }
                            else if( pUserConfiguration->pParsedAppImageStreamInfo->pAppImage_Output_IspTuningData_Raw.get() &&
                                pUserConfiguration->pParsedAppImageStreamInfo->pAppImage_Output_IspTuningData_Raw->getStreamId() == pStreamInfo->getStreamId())
                            {
                                  MY_LOGD("StreamId(%#" PRIxPTR ") find isp tuning data stream for raw",pStreamInfo->getStreamId());
                                  pParsedAppImageStreamInfo->pAppImage_Output_IspTuningData_Raw = pStreamInfo;
                            }
                            else
                            {
                                pParsedAppImageStreamInfo->vAppImage_Output_Proc.emplace(pStreamInfo->getStreamId(), pStreamInfo);
                                if  (   ! pParsedAppImageStreamInfo->hasVideoConsumer
                                        &&  ( pStreamInfo->getUsageForConsumer() & GRALLOC_USAGE_HW_VIDEO_ENCODER )
                                    )
                                {
                                        pParsedAppImageStreamInfo->hasVideoConsumer = true;
                                        pParsedAppImageStreamInfo->videoImageSize = pStreamInfo->getImgSize();
                                        pParsedAppImageStreamInfo->hasVideo4K = ( pParsedAppImageStreamInfo->videoImageSize.w*pParsedAppImageStreamInfo->videoImageSize.h > 8000000 )? true : false;
                                }
                                if (pStreamInfo->getImgSize().size() > maxStreamSize.size())
                                {
                                    maxStreamSize = pStreamInfo->getImgSize();
                                }
                            }
                        }

                    }
                    break;
                    //
                default:
                    MY_LOGE("Unsupported format:0x%x", pStreamInfo->getImgFormat());
                    break;
            }
        }
    };

    if ( pStreamInfos && ! (*pStreamInfos).empty() ) {
        for(auto const& it : *pStreamInfos) {
            auto const pStreamInfo = it.second;
            categorize(pStreamInfo);
        }
    }
    else  {
        for(auto const& it : *pStreamBuffers) {
            auto const pStreamBuffer = it.second;
            auto const pStreamInfo = const_cast<IImageStreamInfo*>(pStreamBuffer->getStreamInfo());
            categorize(pStreamInfo);
        }
    }

    pParsedAppImageStreamInfo->maxImageSize = maxStreamSize;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
auto parseAppRequest(ParseAppRequest const& arg) -> int
{
    auto to = arg.to;
    auto from = arg.from;
    auto pUserConfiguration = arg.pUserConfiguration;
    if  (CC_UNLIKELY(!to || !from || !pUserConfiguration)) {
        MY_LOGE("bad arguments - to:%p from:%p pUserConfiguration:%p", to, from, pUserConfiguration);
        return -EINVAL;
    }

    to->requestNo                    = from->requestNo;
    to->pAppMetaControlStreamBuffer  = from->vIMetaBuffers.begin()->second;
    to->pParsedAppImageStreamBuffers = std::make_shared<ParsedAppImageStreamBuffers>();
    to->pParsedAppImageStreamInfo    = std::make_shared<ParsedAppImageStreamInfo>();
    to->pParsedAppMetaControl        = std::make_shared<policy::ParsedMetaControl>();
    if  ( CC_UNLIKELY( ! to->pParsedAppImageStreamBuffers
                    || ! to->pParsedAppImageStreamInfo
                    || ! to->pParsedAppMetaControl ) )
    {
        MY_LOGE("Fail on make_shared - pParsedAppImageStreamBuffers:%p pParsedAppImageStreamInfo:%p pParsedAppMetaControl:%p",
            to->pParsedAppImageStreamBuffers.get(), to->pParsedAppImageStreamInfo.get(), to->pParsedAppMetaControl.get());
        return -EINVAL;
    }

    ////////////////////////////////////////////////////////////////////////////
    //  pParsedAppImageStreamBuffers
    //  pParsedAppImageStreamInfo

    categorizeImgStream(to, &from->vIImageBuffers, nullptr,                true, pUserConfiguration);
    categorizeImgStream(to, &from->vOImageBuffers, &from->vOImageStreams, false, pUserConfiguration);

    std::swap(to->pParsedAppImageStreamBuffers->vIImageBuffers, from->vIImageBuffers);
    std::swap(to->pParsedAppImageStreamBuffers->vOImageBuffers, from->vOImageBuffers);

    ////////////////////////////////////////////////////////////////////////////
    //  pParsedAppMetaControl

    auto const& pAppControl = to->pAppMetaControlStreamBuffer;
    IMetadata* pMetadata;
    if (gProcessRawDump)
    {
        pMetadata = pAppControl->tryWriteLock(LOG_TAG);
        IMetadata::IEntry Entry(MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE);
        Entry.push_back(1, Type2Type<MINT32>());
        pMetadata->update(MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE, Entry);
    }
    else
    {
        pMetadata = pAppControl->tryReadLock(LOG_TAG);
    }
    if ( CC_UNLIKELY( ! pMetadata ) ) {
        MY_LOGE("bad metadata(%p) SBuffer(%p)", pMetadata, pAppControl.get() );
        pAppControl->unlock(LOG_TAG, pMetadata);
        return -EINVAL;
    }

    auto const& pParsedCtrl = to->pParsedAppMetaControl;
    pParsedCtrl->repeating = pAppControl->isRepeating();

    {
        IMetadata::IEntry const entry = pMetadata->entryFor(MTK_CONTROL_AE_TARGET_FPS_RANGE);
        if ( entry.count() == 2 ) {
            pParsedCtrl->control_aeTargetFpsRange[0] = entry.itemAt(0, Type2Type<int32_t>());
            pParsedCtrl->control_aeTargetFpsRange[1] = entry.itemAt(1, Type2Type<int32_t>());
        }
    }

#define PARSE_META_CONTROL(_tag_, _value_)                              \
    do {                                                                \
        IMetadata::IEntry const entry = pMetadata->entryFor(_tag_);     \
        if ( !entry.isEmpty() )                                         \
            _value_ = entry.itemAt(0, Type2Type<decltype(_value_)>());  \
    } while(0)

    PARSE_META_CONTROL(MTK_CONTROL_CAPTURE_INTENT,           pParsedCtrl->control_captureIntent);
    PARSE_META_CONTROL(MTK_CONTROL_ENABLE_ZSL,               pParsedCtrl->control_enableZsl);
    PARSE_META_CONTROL(MTK_CONTROL_MODE,                     pParsedCtrl->control_mode);
    PARSE_META_CONTROL(MTK_CONTROL_SCENE_MODE,               pParsedCtrl->control_sceneMode);
    PARSE_META_CONTROL(MTK_CONTROL_VIDEO_STABILIZATION_MODE, pParsedCtrl->control_videoStabilizationMode);
    PARSE_META_CONTROL(MTK_EDGE_MODE,                        pParsedCtrl->edge_mode);
    PARSE_META_CONTROL(MTK_CONTROL_CAPTURE_ISP_TUNING_REQUEST,pParsedCtrl->control_isp_tuning);
    PARSE_META_CONTROL(MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE,pParsedCtrl->control_processRaw);

    PARSE_META_CONTROL(MTK_CONTROL_CAPTURE_REMOSAIC_EN,      pParsedCtrl->control_remosaicEn);
    PARSE_META_CONTROL(MTK_VSDOF_FEATURE_CAPTURE_WARNING_MSG,pParsedCtrl->control_vsdofFeatureWarning);
#undef PARSE_META_CONTROL

    pAppControl->unlock(LOG_TAG, pMetadata);

    ////////////////////////////////////////////////////////////////////////////

    if  ( CC_UNLIKELY(gLogLevel >= 2) )
    {
        dumpToLog(*to, DetailsType::DETAILS_INFO);
    }

    return OK;
}
};  //namespace model
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam

