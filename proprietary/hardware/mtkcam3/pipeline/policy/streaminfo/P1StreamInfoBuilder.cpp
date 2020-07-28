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

#define LOG_TAG "mtkcam-P1StreamInfoBuilder"

#include "P1StreamInfoBuilder.h"
//
#include <algorithm>
#include <array>
#include <functional>
//
#include <mtkcam3/pipeline/hwnode/StreamId.h>
#include <mtkcam3/pipeline/utils/streaminfo/IStreamInfoPluginManager.h>
#include <mtkcam/aaa/IIspMgr.h>
#include <mtkcam/aaa/IHalISP.h>
#include "StreamInfoPluginDecider.h"
#include "MyUtils.h"


/******************************************************************************
 *
 ******************************************************************************/
//using namespace NSCam;
//using namespace NSCam::v3;
//using namespace NSCam::v3::Utils;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA  ("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF  ("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::policy {


/******************************************************************************
 *
 ******************************************************************************/
template <class deciderT>
auto decideStreamInfoPluginInfo(
    deciderT decideStreamInfoPlugin
)
{
    using namespace NSCam::plugin::streaminfo;

    std::optional<PluginInfo> ret = std::nullopt;
    auto plugin = decideStreamInfoPlugin();
    if (CC_UNLIKELY( ! plugin.has_value() || plugin->pluginId == PluginId::BAD )) {
        MY_LOGE("Fail to decide plugin - has_value:%d", plugin.has_value());
        return ret;
    }

    auto mgr = IStreamInfoPluginManager::get();
    MY_LOGF_IF(mgr==nullptr, "Bad IStreamInfoPluginManager::get");
    ret = mgr->queryPluginInfo(plugin->pluginId);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
static std::string getP1StreamNamePostfix(size_t i)
{
    switch (i)
    {
    case 0: return std::string("main1");
    case 1: return std::string("main2");
    case 2: return std::string("main3");
    default:
        break;
    }
    return std::string("na");
}


/******************************************************************************
 *
 ******************************************************************************/
static StreamId_T getP1StreamId(size_t i, std::array<StreamId_T, 3>const& a)
{
    if (CC_UNLIKELY(i >= a.size())) {
        MY_LOGE("not support p1 node number large than 3");
        return eSTREAMID_NO_STREAM;
    }
    return a[i];
}


#define GETP1STREAMID(_type_, ...) \
    static StreamId_T getStreamId_##_type_(size_t i) { return getP1StreamId(i, { __VA_ARGS__ }); }

GETP1STREAMID(P1AppMetaDynamic,
    eSTREAMID_META_APP_DYNAMIC_01,
    eSTREAMID_META_APP_DYNAMIC_01_MAIN2,
    eSTREAMID_META_APP_DYNAMIC_01_MAIN3
)

GETP1STREAMID(P1HalMetaDynamic,
    eSTREAMID_META_PIPE_DYNAMIC_01,
    eSTREAMID_META_PIPE_DYNAMIC_01_MAIN2,
    eSTREAMID_META_PIPE_DYNAMIC_01_MAIN3
)

GETP1STREAMID(P1HalMetaControl,
    eSTREAMID_META_PIPE_CONTROL,
    eSTREAMID_META_PIPE_CONTROL_MAIN2,
    eSTREAMID_META_PIPE_CONTROL_MAIN3
)

GETP1STREAMID(P1Imgo,
    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_00,
    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_01,
    eSTREAMID_IMAGE_PIPE_RAW_OPAQUE_02
)

GETP1STREAMID(P1Rrzo,
    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_00,
    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_01,
    eSTREAMID_IMAGE_PIPE_RAW_RESIZER_02
)

GETP1STREAMID(P1Lcso,
    eSTREAMID_IMAGE_PIPE_RAW_LCSO_00,
    eSTREAMID_IMAGE_PIPE_RAW_LCSO_01,
    eSTREAMID_IMAGE_PIPE_RAW_LCSO_02
)

GETP1STREAMID(P1Rsso,
    eSTREAMID_IMAGE_PIPE_RAW_RSSO_00,
    eSTREAMID_IMAGE_PIPE_RAW_RSSO_01,
    eSTREAMID_IMAGE_PIPE_RAW_RSSO_02
)


/******************************************************************************
 *
 ******************************************************************************/
auto P1MetaStreamInfoBuilder::setP1AppMetaDynamic_Default() -> P1MetaStreamInfoBuilder&
{
    setStreamName(std::string("App:Meta:DynamicP1_")+getP1StreamNamePostfix(mIndex));
    setStreamId(getStreamId_P1AppMetaDynamic(mIndex));
    setStreamType(eSTREAMTYPE_META_OUT);
    setMaxBufNum(10);
    setMinInitBufNum(1);

    return *this;
}


auto P1MetaStreamInfoBuilder::setP1HalMetaDynamic_Default() -> P1MetaStreamInfoBuilder&
{
    setStreamName(std::string("Hal:Meta:P1:Dynamic_")+getP1StreamNamePostfix(mIndex));
    setStreamId(getStreamId_P1HalMetaDynamic(mIndex));
    setStreamType(eSTREAMTYPE_META_INOUT);
    setMaxBufNum(10);
    setMinInitBufNum(1);

    return *this;
}


auto P1MetaStreamInfoBuilder::setP1HalMetaControl_Default() -> P1MetaStreamInfoBuilder&
{
    setStreamName(std::string("Hal:Meta:Control_")+getP1StreamNamePostfix(mIndex));
    setStreamId(getStreamId_P1HalMetaControl(mIndex));
    setStreamType(eSTREAMTYPE_META_IN);
    setMaxBufNum(10);
    setMinInitBufNum(1);

    return *this;
}


/******************************************************************************
 *
 ******************************************************************************/
const bool P1ImageStreamInfoBuilder::kIsLowRam = ::property_get_bool("ro.config.low_ram", false);


/******************************************************************************
 *
 ******************************************************************************/
auto P1ImageStreamInfoBuilder::queryMinBufNum(char const* name) -> int32_t
{
    std::string prefix{"persist.vendor.camera3.pipeline.bufnum."};
    auto min_low_ram    = ::property_get_int32((prefix + "min.low_ram." + name).c_str(), -1);
    auto min_high_ram   = ::property_get_int32((prefix + "min.high_ram."+ name).c_str(), -1);

    auto bufNum_min = kIsLowRam ? min_low_ram : min_high_ram;

    MY_LOGF_IF(bufNum_min < 0,
        "%s isLowRam:%d bufNum_min:%d<0 min_low_ram:%d min_high_ram:%d",
        name, kIsLowRam, bufNum_min, min_low_ram, min_high_ram);
    return bufNum_min;
}


auto P1ImageStreamInfoBuilder::queryBaseBufNum(char const* name) -> int32_t
{
    std::string prefix{"persist.vendor.camera3.pipeline.bufnum."};
    auto bufNum_base    = ::property_get_int32((prefix + "base."        + name).c_str(), -1);
    MY_LOGF_IF(bufNum_base < 0, "%s bufNum_base:%d<0", name, bufNum_base);
    return bufNum_base;
}


#define QUERY_P1_MAX_BUF_NUM(_name_)                                                                \
    [this](){                                                                                       \
        static auto minBufNum = queryMinBufNum(#_name_);                                            \
        static auto baseBufNum= queryBaseBufNum(#_name_);                                           \
                                                                                                    \
        CAM_LOGE_IF(mpCaptureFeatureSetting==nullptr,                                               \
                #_name_ " mpCaptureFeatureSetting==nullptr");                                       \
        auto captureBufNum = mpCaptureFeatureSetting->additionalHalP1OutputBufferNum._name_;        \
                                                                                                    \
        CAM_LOGE_IF(mpStreamingFeatureSetting==nullptr,                                             \
                #_name_ " mpStreamingFeatureSetting==nullptr");                                     \
        auto streamBufNum = mpStreamingFeatureSetting->additionalHalP1OutputBufferNum._name_;       \
                                                                                                    \
        CAM_ULOGMD( #_name_ " (base:%d + capture:%d + stream:%d) v.s. min:%d",                        \
            baseBufNum, captureBufNum, streamBufNum, minBufNum);                                    \
        return std::max(baseBufNum + captureBufNum + streamBufNum, minBufNum);                      \
    }()


/******************************************************************************
 *
 ******************************************************************************/
auto P1ImageStreamInfoBuilder::setP1Imgo_Default(
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    size_t maxBufNum = QUERY_P1_MAX_BUF_NUM(imgo);
    return setP1Imgo_Default(maxBufNum, rP1HwSetting);
}


auto P1ImageStreamInfoBuilder::setP1Imgo_Default(
    size_t maxBufNum,
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    auto const& setting = rP1HwSetting.imgoDefaultRequest;
    MINT const imgFormat = setting.format;
    MSize const& imgSize = setting.imageSize;
    IImageStreamInfo::BufPlanes_t defaultBufPlanes;
    bool ret = mHwInfoHelper->getDefaultBufPlanes_Imgo(defaultBufPlanes, imgFormat, imgSize);
    MY_LOGE_IF(!ret, "IHwInfoHelper::getDefaultBufPlanes_Imgo");

    // Get config Info
    auto const& configSetting = rP1HwSetting.imgoAlloc;
    MINT const imgConfigFormat = configSetting.format;
    MSize const& imgConfigSize = configSetting.imageSize;
    IImageStreamInfo::BufPlanes_t configBufPlanes;
    ret = mHwInfoHelper->getDefaultBufPlanes_Imgo(configBufPlanes, imgConfigFormat, imgConfigSize);
    MY_LOGE_IF(!ret, "IHwInfoHelper::getDefaultBufPlanes_Imgo - configBufPlanes");
    // Allocate BufPlanes & format
    MINT allocFormat = eImgFmt_BLOB;
    auto toSize = [](IImageStreamInfo::BufPlanes_t const& bp) {
            size_t size = 0;
            for (size_t i = 0 ; i < bp.count ; i++) {
                size += bp.planes[i].sizeInBytes;
            }
            return size;
        };
    size_t allocSize = std::max(toSize(configBufPlanes), toSize(defaultBufPlanes));
    IImageStreamInfo::BufPlanes_t allocBufPlanes;
    allocBufPlanes.count = 1;
    allocBufPlanes.planes[0] = IImageStreamInfo::BufPlane{allocSize, allocSize};
    setStreamName((std::string("Hal:Image:P1:Fullraw_")+getP1StreamNamePostfix(mIndex)));
    setStreamId(getStreamId_P1Imgo(mIndex));
    setStreamType(eSTREAMTYPE_IMAGE_INOUT);
    setMaxBufNum(maxBufNum);
    setMinInitBufNum(0);
    setUsageForAllocator(0);
    setAllocImgFormat(allocFormat);
    setAllocBufPlanes(allocBufPlanes);

    setImgFormat(imgFormat);
    setImgSize(imgSize);
    setBufCount(1);
    setStartOffset(0);
    setBufStep(0);
    setBufPlanes(defaultBufPlanes);
    setSecureInfo(mpPipelineUserConfiguration->pParsedAppImageStreamInfo->secureInfo);
    return *this;
}


auto P1ImageStreamInfoBuilder::setP1Rrzo_Default(
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    size_t maxBufNum = QUERY_P1_MAX_BUF_NUM(rrzo);
    return setP1Rrzo_Default(maxBufNum, rP1HwSetting);
}


auto P1ImageStreamInfoBuilder::setP1Rrzo_Default(
    size_t maxBufNum,
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    auto const& setting = rP1HwSetting.rrzoDefaultRequest;
    MINT const imgFormat = setting.format;
    MSize const& imgSize = setting.imageSize;
    IImageStreamInfo::BufPlanes_t defaultBufPlanes;
    bool ret = mHwInfoHelper->getDefaultBufPlanes_Rrzo(defaultBufPlanes, imgFormat, imgSize);
    MY_LOGE_IF(!ret, "IHwInfoHelper::getDefaultBufPlanes_Rrzo");

    setStreamName((std::string("Hal:Image:P1:Resizeraw_")+getP1StreamNamePostfix(mIndex)));
    setStreamId(getStreamId_P1Rrzo(mIndex));
    setStreamType(eSTREAMTYPE_IMAGE_INOUT);
    setMaxBufNum(maxBufNum);
    setMinInitBufNum(0);
    setUsageForAllocator(0);
    setAllocImgFormat(imgFormat);
    setAllocBufPlanes(defaultBufPlanes);

    setImgFormat(imgFormat);
    setImgSize(imgSize);
    setBufCount(1);
    setStartOffset(0);
    setBufStep(0);
    setBufPlanes(defaultBufPlanes);
    setSecureInfo(mpPipelineUserConfiguration->pParsedAppImageStreamInfo->secureInfo);
    return *this;
}


auto P1STTImageStreamInfoBuilder::setMaxBufNum_Default() -> void
{
    size_t maxBufNum = QUERY_P1_MAX_BUF_NUM(lcso);
    setMaxBufNum(maxBufNum);
}


auto P1STTImageStreamInfoBuilder::setP1Stt_Default(
    MINT32 sensorId
) -> void
{
    setStreamName((std::string("Hal:Image:STT_")+getP1StreamNamePostfix(mIndex)));
    setStreamId(getStreamId_P1Lcso(mIndex));
    setStreamType(eSTREAMTYPE_IMAGE_INOUT);
    setMinInitBufNum(1);
    setUsageForAllocator(0);


    auto const plugin =
        decideStreamInfoPluginInfo(
            std::bind(
                decideStreamInfoPlugin_STT,
                DecideStreamInfoPluginArgument{
                    .pPipelineStaticInfo = mpPipelineStaticInfo,
                    .pPipelineUserConfiguration = mpPipelineUserConfiguration,
                }));
    if ( plugin.has_value() )
    {
        auto mgr = IStreamInfoPluginManager::get();
        MY_LOGF_IF(mgr==nullptr, "Bad IStreamInfoPluginManager::get");
        auto data = mgr->determinePluginData(
                *plugin,
                {.openId = mpPipelineStaticInfo->openId, .sensorId = {sensorId},}
            );
        MY_LOGF_IF(!data.has_value(), "Bad PluginData");

        auto allocFormat = eImgFmt_BLOB;
        auto allocSize = data->allocInfo[0].sizeInBytes;
        IImageStreamInfo::BufPlanes_t allocBufPlanes;
        allocBufPlanes.count = 1;
        allocBufPlanes.planes[0] = IImageStreamInfo::BufPlane{allocSize, allocSize};
        setAllocImgFormat(allocFormat);
        setAllocBufPlanes(allocBufPlanes);
        setImgFormat(allocFormat);
        setBufPlanes(allocBufPlanes);
        setImgSize(MSize(allocSize, 1)); // BLOB: width=size, height=1
        setBufCount(1);
		setStartOffset(0);
        setBufStep(0);

        setPrivateData(SetPrivateData{
                .privData = data->privData,
                .privDataId = static_cast<uint32_t>(data->privDataId),
            });

        return;
    }


    // p1: lcso size
    NS3Av3::LCSO_Param lcsoParam;
    NS3Av3::Buffer_Info lcsoInfo;
    lcsoParam.bSupport = false;
    auto ispHal = MAKE_HalISP(sensorId, LOG_TAG);
    if(ispHal==NULL)
    {
        MY_LOGE("ispHal is NULL!");
    }
    else
    {
        if (ispHal->queryISPBufferInfo(lcsoInfo))
        {
            MY_LOGD("queryISPBufferInfo, support : %d, format : %d, size : %dx%d",
                    lcsoInfo.LCESO_Param.bSupport, lcsoInfo.LCESO_Param.format,
                    lcsoInfo.LCESO_Param.size.w, lcsoInfo.LCESO_Param.size.h);
            lcsoParam.format = lcsoInfo.LCESO_Param.format;
            lcsoParam.size = lcsoInfo.LCESO_Param.size;
            lcsoParam.stride = lcsoInfo.LCESO_Param.stride;
        }
        else if ( auto ispMgr = MAKE_IspMgr() )
        {
            ispMgr->queryLCSOParams(lcsoParam);
        }
        else {
            MY_LOGE("Query IIspMgr FAILED!");
        }
        ispHal->destroyInstance(LOG_TAG);
    }

    ImageBufferInfo imageBufferInfo;
    toBufPlanes(imageBufferInfo.bufPlanes, lcsoParam.stride, lcsoParam.format, lcsoParam.size);
    imageBufferInfo.count     = 1;
    imageBufferInfo.bufStep   = 0;
    imageBufferInfo.startOffset = 0;
    imageBufferInfo.imgFormat = lcsoParam.format;
    imageBufferInfo.imgWidth  = lcsoParam.size.w;
    imageBufferInfo.imgHeight = lcsoParam.size.h;
    setImageBufferInfo(imageBufferInfo);

    setAllocImgFormat(lcsoParam.format);
    setAllocBufPlanes(imageBufferInfo.bufPlanes);

}


auto P1ImageStreamInfoBuilder::setP1Rsso_Default(
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    size_t maxBufNum = QUERY_P1_MAX_BUF_NUM(rsso);
    return setP1Rsso_Default(maxBufNum, rP1HwSetting);
}


auto P1ImageStreamInfoBuilder::setP1Rsso_Default(
    size_t maxBufNum,
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    MINT imgFormat = eImgFmt_STA_BYTE;
    MSize const& imgSize = rP1HwSetting.rssoSize;
    size_t const stride  = imgSize.w;

    setStreamName((std::string("Hal:Image:RSSO_")+getP1StreamNamePostfix(mIndex)));
    setStreamId(getStreamId_P1Rsso(mIndex));
    setStreamType(eSTREAMTYPE_IMAGE_INOUT);
    setMaxBufNum(maxBufNum);
    setMinInitBufNum(1);
    setUsageForAllocator(eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);

    ImageBufferInfo imageBufferInfo;
    toBufPlanes(imageBufferInfo.bufPlanes, stride, imgFormat, imgSize);
    imageBufferInfo.count     = 1;
    imageBufferInfo.bufStep   = 0;
    imageBufferInfo.startOffset = 0;
    imageBufferInfo.imgFormat = imgFormat;
    imageBufferInfo.imgWidth  = imgSize.w;
    imageBufferInfo.imgHeight = imgSize.h;
    setImageBufferInfo(imageBufferInfo);

    setAllocImgFormat(imgFormat);
    setAllocBufPlanes(imageBufferInfo.bufPlanes);
    return *this;
}


auto P1ImageStreamInfoBuilder::setP1FDYuv_Default(
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    size_t maxBufNum = queryMinBufNum("fdyuv");
    MINT imgFormat = eImgFmt_YUY2;
    MSize const& imgSize = rP1HwSetting.fdyuvSize;
    size_t const stride  = imgSize.w << 1;

    setStreamName((std::string("Hal:Image:P1FDYuv_")+getP1StreamNamePostfix(mIndex)));
    setStreamId(eSTREAMID_IMAGE_FD);
    setStreamType(eSTREAMTYPE_IMAGE_INOUT);
    setMaxBufNum(maxBufNum);
    setMinInitBufNum(1);
    setUsageForAllocator(eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);

    ImageBufferInfo imageBufferInfo;
    toBufPlanes(imageBufferInfo.bufPlanes, stride, imgFormat, imgSize);
    imageBufferInfo.count     = 1;
    imageBufferInfo.bufStep   = 0;
    imageBufferInfo.startOffset = 0;
    imageBufferInfo.imgFormat = imgFormat;
    imageBufferInfo.imgWidth  = imgSize.w;
    imageBufferInfo.imgHeight = imgSize.h;
    setImageBufferInfo(imageBufferInfo);

    setAllocImgFormat(imgFormat);
    setAllocBufPlanes(imageBufferInfo.bufPlanes);

    setSecureInfo(mpPipelineUserConfiguration->pParsedAppImageStreamInfo->secureInfo);
    return *this;
}


auto P1ImageStreamInfoBuilder::setP1ScaledYuv_Default(
    P1HwSetting const& rP1HwSetting
) -> P1ImageStreamInfoBuilder&
{
    size_t maxBufNum = 13;
    MINT imgFormat = eImgFmt_NV16;
    MSize const& imgSize = rP1HwSetting.scaledYuvSize;
    size_t const stride  = imgSize.w;
    int64_t streamId = -1;
    switch (mIndex)
    {
        case 0:
            streamId = eSTREAMID_IMAGE_PIPE_P1_SCALED_YUV_00;
            break;
        case 1:
            streamId = eSTREAMID_IMAGE_PIPE_P1_SCALED_YUV_01;
            break;
        case 2:
            streamId = eSTREAMID_IMAGE_PIPE_P1_SCALED_YUV_02;
            break;
        default:
            MY_LOGE("not support p1 node number large than 2");
            break;
    }

    setStreamName((std::string("Hal:Image:P1ScaledYuv_")+getP1StreamNamePostfix(mIndex)));
    setStreamId(streamId);
    setStreamType(eSTREAMTYPE_IMAGE_INOUT);
    setMaxBufNum(maxBufNum);
    setMinInitBufNum(1);
    setUsageForAllocator(eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE);

    ImageBufferInfo imageBufferInfo;
    toBufPlanes(imageBufferInfo.bufPlanes, stride, imgFormat, imgSize);
    imageBufferInfo.count     = 1;
    imageBufferInfo.bufStep   = 0;
    imageBufferInfo.startOffset = 0;
    imageBufferInfo.imgFormat = imgFormat;
    imageBufferInfo.imgWidth  = imgSize.w;
    imageBufferInfo.imgHeight = imgSize.h;
    setImageBufferInfo(imageBufferInfo);

    setAllocImgFormat(imgFormat);
    setAllocBufPlanes(imageBufferInfo.bufPlanes);

    return *this;
}


auto P1ImageStreamInfoBuilder::toBufPlanes(
    IImageStreamInfo::BufPlanes_t& bufPlanes,
    size_t stride,
    MINT imgFormat,
    MSize const& imgSize
) -> bool
{
    bool ret = mHwInfoHelper->getDefaultBufPlanes_Pass1(bufPlanes, imgFormat, imgSize, stride);
    MY_LOGE_IF(!ret, "IHwInfoHelper::getDefaultBufPlanes_Pass1");
    return ret;
}


};  //namespace NSCam::v3::pipeline::policy

